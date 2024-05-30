package main

import (
	"fmt"
	"log"
	"net/http"
	"os"
	"strconv"

	"github.com/nusdbsystem/MOSS/fnpacker/fnpack"
)

func main() {
	fmt.Println("Hello")
	var err error

	// set controller address
	ow_address, exist := os.LookupEnv("OW_SERVICE_ADDRESS")
	if !exist {
		// log.Fatal("controller addr not set")
		ow_address = "localhost"
	} else {
		log.Println("proxy address: " + ow_address)
	}

	// set controller port
	ow_port, exist := os.LookupEnv("OW_SERVICE_PORT")
	if !exist {
		// log.Fatal("controller port not set")
		ow_port = "7312"
	} else {
		log.Printf("proxy address: %v:%v\n", ow_address, ow_port)
	}

	// set controller port
	ow_auth, exist := os.LookupEnv("OW_SERVICE_AUTH")
	if !exist {
		ow_auth = "23bc46b1-71f6-4ed5-8c54-816aa4f8c502:123zO3xZCLrMN6v2BKK1dXYFpXlPkccOFqm12CdAsMgRU4VrNZ9lyGVCGuMDGIwP"
	} else {
		log.Println("OW AUTH: " + ow_auth)
	}

	pool_size := fnpack.DEFAULT_POOL_SIZE
	pool_size_str, exist := os.LookupEnv("OW_FUNCTION_MANAGER_POOL_SIZE")

	// set pool size
	if !exist {
		log.Printf("function manager pool size set to default %v", pool_size)
	} else {
		pool_size, err = strconv.Atoi(pool_size_str)
		if err != nil {
			log.Printf("invalid pool size environment variable: %e", err)
			pool_size = fnpack.DEFAULT_POOL_SIZE
		} else {
			log.Printf("function manager pool size: %v\n", pool_size)
		}
	}

	// launch function manager
	fp, err := fnpack.NewOwFunctionPacker()
	mux := http.NewServeMux()
	mux.HandleFunc("/create", fp.HandleCreate)
	mux.HandleFunc("/delete", fp.HandleDelete)
	mux.HandleFunc("/list", fp.HandleList)
	mux.HandleFunc("/invoke", fp.HandleInvoke)

	if err != nil {
		log.Fatal("Failed to create function manager: ", err.Error())
	} else {
		log.Fatal(http.ListenAndServe(":7310", mux))
	}
}
