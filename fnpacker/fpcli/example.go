package main

import (
	"fmt"
	"net/http"
	"os"

	"github.com/nusdbsystem/MOSS/fnpacker/req"
)

const fpaddr = "localhost"
const fpport = 7310

// const fnname = "crtw-tvm-mb-4"
const fnname = "crtw-tvm-rs-switch"

// const fnimage = "crtw-tvm-mb-4:v4"
const fnimage = "crtw-tvm-rs-1:v5"

// const fnconcurrency = 4
const fnconcurrency = 1

const fnmembudget = 384

func main() {
	url := fmt.Sprintf("http://%v:%d", fpaddr, fpport)

	// --- basics ---
	fpcli := &FPClient{cli: http.DefaultClient, url: url}
	// create
	fpcli.Create(fnname, "blackbox", fnimage, fnconcurrency, fnmembudget)

	// list
	fpcli.List()

	// invoke
	ireq := req.InvokeRequest{}
	ireqbytes, _ := os.ReadFile("data/tvm_mb_req.json")
	ireq.Load(ireqbytes)
	ireq.KeyServiceAddr = "localhost"
	fpcli.Invoke(fnname, &ireq)

	// delete
	fpcli.Delete(fnname)
	// --- basics ---
}
