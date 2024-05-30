# Worker

Worker creates an enclave to perform model inference securely. It connects to KeyServer to obtain the keys to read the encrypted requests and encrypted model to use.

The application is developed in reference to mtclient in ratls library.

Note that the default configuration can run tflm/person_detect. Use sgx_emmt to profile and set the heap to suitable larger values to run other workloads.

## Build the worker

Building the worker requires building and install the dependencies (refer the README under project root directory) and [model runtime library](../models/README.md#building-the-model-runtime-library).

Then build worker and install it to a `install` directory under the current folder.

```bash
make mrproper && make all && make install
```

### configure the build

```Makefile
PROJECT_ROOT_DIR := $(shell readlink -f ..)
INFERENCE_RT_DIR ?= $(PROJECT_ROOT_DIR)/models/install
INSTALL_PREFIX ?= $(PROJECT_ROOT_DIR)/worker/install
```

### configure enclave memory.

During inference, models, inputs and intermediate results are all hold inside enclave. Set suitable heap memory size inside `trusted/Worker_Enclave.config.xml` is important to ensure enough space and minimize memory consumption.

Refer to [tflm memory configuration guide](../models/README.md#configure-tflm-interpreter-memory) to set a heap size 10x of that. Then set the expected max concurrency (set the macro in `App.cc`) and build worker.

```cpp
#define INFERENCE_COUNT 8
```

Launch worker in `sgx-gdb` with `sgx_emmt` enabled. Supply a sample request to profile the peak heap usage. Now we can adjust the heap size in enclave config file to just slightly above the measured peak usage.

## Test worker locally

Launch the key service and stores keys and access control info to the key service via client (if the a cached directory with sealed keys already exist, then this step can be skipped). Then launch the worker

```bash
# the cache directory will be set to <file-store-directory>/cache.
./App <file-store-directory> <inference json request>
```
