# Concurrent Runtime

Implementation of serverless runtime that hosts and share enclave among concurrent invocations.

Currently, we provide an implementation that is compatible to Apache OpenWhisk. That is an async http server developed with `libuv` and `http-parser/llhttp` and implementing the [action interface spec][1] of Apache OpenWhisk.

## Build the concurrent runtime and link with worker service

This requires that the worker library is already being built. Refer [here](../worker/README.md#build-the-worker) for instructions.

Under this directory run:

```bash
make mrproper
USE_WORKER_SERVICE=1 make openwhisk_runtime
USE_WORKER_SERVICE=1 make install_openwhisk_runtime
```

### sample service

There is also a sample service that only logs users input and reply with dummy messages for testing. To build that for testing the concurrent runtime alone, configure the build by setting `USE_WORKER_SERVICE=0`.

### test the service

Launch the server locally: ([Setup key service if keys are not already cached](../worker/README.md#test-worker-locally)).

```bash
./install/bin/server <signed-enclave-shared-object> <worker-file-store-directory> <worker-store-cache-directory>
```

example:

```bash
./install/bin/server ./install/lib/Worker_Enclave.signed.so ../worker/tmp/ ../worker/tmp/cache/
```

Supply a sample request via curl:

```bash
curl -X POST localhost:8080/run -H 'Content-Type: application/json' -d @<inference-request-json>
```

[1]: https://github.com/apache/openwhisk/blob/master/docs/actions-new.md
