package fnpack

import (
	"errors"
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
	"sync"
	"sync/atomic"
	"time"

	fpreq "github.com/nusdbsystem/MOSS/fnpacker/req"
	"github.com/nusdbsystem/MOSS/fnpacker/scli"
)

//
// Function Packer
//

const DEFAULT_POOL_SIZE = 5

type FunctionPacker struct {
	pool_size  int
	cli        scli.ServerlessClient
	controller FPController
	rid        uint64
}

// need external synchronization
// call when no traffic to name
func (fp *FunctionPacker) createFG(name, kind, image string, concurrency, budget int) error {
	var has_error int32
	var wg sync.WaitGroup
	var msg string

	for i := 0; i < fp.pool_size; i++ {
		wg.Add(1)
		go func(id int) {
			defer wg.Done()
			err := fp.cli.CreateAction(createFnName(name, id), kind, image, concurrency, budget)
			if err != nil {
				atomic.StoreInt32(&has_error, 1)
				msg = err.Error()
			}
		}(i)
	}
	wg.Wait()
	if has_error != 0 {
		fp.deleteFG(name)
		return errors.New(fmt.Sprintf("Functions creation failed (%v)", msg))
	}

	// register in controller
	fp.controller.Add(name, fp.pool_size)
	return nil
}

// need external synchronization
// call when no traffic to name
func (fp *FunctionPacker) deleteFG(name string) error {
	// delete from controller
	fp.controller.Delete(name)

	// delete from serverless backend
	var has_error int32
	var wg sync.WaitGroup
	var msg string
	for i := 0; i < fp.pool_size; i++ {
		wg.Add(1)
		go func(id int) {
			defer wg.Done()
			err := fp.cli.DeleteAction(createFnName(name, id))
			if err != nil {
				atomic.StoreInt32(&has_error, 1)
				msg = err.Error()
			}
		}(i)
	}
	wg.Wait()
	if has_error != 0 {
		return errors.New(fmt.Sprintf("Functions deletion failed (%v)", msg))
	}
	return nil
}

func (fp *FunctionPacker) invokeFG(name string, req *fpreq.InvokeRequest) (string, error) {
	// check creation first
	if fp.controller.CheckStatus(name) != FPEReady {
		return "", errors.New(fmt.Sprintf("Invocation on non-existent function %v", name))
	}

	// obtain rid
	reqId := atomic.LoadUint64(&fp.rid)
	for {
		if atomic.CompareAndSwapUint64(&fp.rid, reqId, reqId+1) {
			break
		}
		reqId = atomic.LoadUint64(&fp.rid)
	}
	// finalize obtained rid
	reqId++

	targetFn, backoff, err := fp.controller.PickFnInstance(name, req.ModelType)
	// delay sending the request
	if backoff > 0 {
		time.Sleep(time.Duration(backoff) * time.Microsecond)
	}
	start_time := time.Now()
	log.Printf("[CRID-%v] picked %v for %v", reqId, targetFn, req.ModelType)
	resp_id, _, err := fp.cli.InvokeAction(targetFn, req, reqId)
	if err != nil {
		return "", errors.New(fmt.Sprintf("Function invocation failed: %v return: %v", name, err.Error()))
	}
	invoke_resp, err := fp.cli.GetResult(reqId, resp_id)
	if err != nil {
		return "", errors.New(fmt.Sprintf("Function failed to get result: %v return: %v", name, err.Error()))
	}
	end_time := time.Now()

	fp.controller.ReleaseAndUpdateStats(name, req.ModelType, start_time, end_time, invoke_resp.ExecTime)

	return fmt.Sprintf("Function %v return: %v", name, invoke_resp.Msg), nil
}

func (fp *FunctionPacker) HandleCreate(w http.ResponseWriter, r *http.Request) {
	// decode request
	buf, err := ioutil.ReadAll(r.Body)
	defer r.Body.Close()

	// handle request
	req := fpreq.FPCreateRequest{}
	if err == nil {
		req.Load(buf)
		err = fp.createFG(req.Name, req.Kind, req.Image, req.Concurrency, req.Budget)
	}

	// prepare message
	var msg string
	if err != nil {
		msg = err.Error()
	} else {
		msg = fmt.Sprintf("Functions created under %v", req.Name)
	}

	// prepare response
	if err != nil {
		http.Error(w, fmt.Sprintf("{\"error\": \"%v\"}", msg), http.StatusInternalServerError)
	} else {
		w.Header().Set("Content-Type", "application/json")
		w.Write([]byte(msg + "\n"))
	}
}

func (fp *FunctionPacker) HandleDelete(w http.ResponseWriter, r *http.Request) {
	// decode request
	buf, err := ioutil.ReadAll(r.Body)
	defer r.Body.Close()

	// handle request
	req := fpreq.FPDeleteRequest{}
	if err == nil {
		req.Load(buf)
		err = fp.deleteFG(req.Name)
	}

	// prepare message
	var msg string
	if err != nil {
		msg = err.Error()
	} else {
		msg = fmt.Sprintf("Functions deleted under %v", req.Name)
	}

	// prepare response
	if err != nil {
		http.Error(w, fmt.Sprintf("{\"error\": \"%v\"}", msg), http.StatusInternalServerError)
	} else {
		w.Header().Set("Content-Type", "application/json")
		w.Write([]byte(msg + "\n"))
	}
}

func (fp *FunctionPacker) HandleList(w http.ResponseWriter, r *http.Request) {
	// to do
	msg := fp.controller.List()
	fp.controller.PrintState()
	w.Header().Set("Content-Type", "application/json")
	w.Write([]byte(msg + "\n"))
}

func (fp *FunctionPacker) HandleInvoke(w http.ResponseWriter, r *http.Request) {

	// decode request
	buf, err := ioutil.ReadAll(r.Body)
	defer r.Body.Close()

	// handle request
	var msg string
	req := fpreq.FPInvokeRequest{}
	if err == nil {
		req.Load(buf)
		msg, err = fp.invokeFG(req.Name, &req.Req)
	}

	// prepare message
	if err != nil {
		msg = err.Error()
	} else {
		msg = fmt.Sprintf("Functions invoked under %v, return: %v", req.Name, msg)
	}

	// prepare response
	if err != nil {
		http.Error(w, fmt.Sprintf("{\"error\": \"%v\"}", msg), http.StatusInternalServerError)
	} else {
		w.Header().Set("Content-Type", "application/json")
		w.Write([]byte(msg + "\n"))
	}
}

// default ow function packer setup uses local .wskprop
func NewOwFunctionPacker() (*FunctionPacker, error) {
	client := scli.OwClient{}
	err := client.Init()
	if err != nil {
		log.Printf("failed to launc ow client: %v", err)
		return nil, errors.New(fmt.Sprintf("failed to launch ow cli: %v", err))
	}

	return &FunctionPacker{DEFAULT_POOL_SIZE, &client, NewFPController(), 0}, nil
}

func NewOwFunctionPackerWithConfig(pool_size int, ow_address, ow_port, ow_auth string) (*FunctionPacker, error) {
	client := scli.OwClient{}
	err := client.InitWithConfig(ow_address, ow_port, ow_auth)
	if err != nil {
		log.Printf("failed to launc ow client: %v", err)
		return nil, errors.New(fmt.Sprintf("failed to launch ow cli: %v", err))
	}

	return &FunctionPacker{pool_size, &client, NewFPController(), 0}, nil
}
