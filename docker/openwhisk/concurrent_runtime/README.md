# Concurrent Runtime Docker

The c++ concurrent runtime that implements language runtime spec. It will builds the sample action. It uses the sgx supported base image from `linux-sgx`.

## Build openwhisk concurrent runtime (c++)

The concurrent runtime reimplementation leverges `libuv`.

To build the docker:

```sh
docker build -t concurrent-runtime-test -f docker/openwhisk/concurrent_runtime/Dockerfile .
```

To run the built docker for testing

```sh
docker run --net host -it --name test-build concurrent-runtime-test
```

Run it with openwhisk for testing

```sh
docker tag concurrent-runtime-test dockerHubUser/concurrent-runtime-test:v1
sudo docker push dockerHubUser/concurrent-runtime-test:v1
wsk -i action create concurrent-runtime-test-v1 --docker dockerHubUser/concurrent-runtime-test:v1
wsk -i action invoke concurrent-runtime-test-v1 --result
```

## Build and link with worker service

### prepare a local volume as model store

```bash
docker volume create worker-store
docker run -it --name test -v worker-store:/check ubuntu /bin/bash
```

populate the volume from another terminal

```bash
# in another terminal
docker cp <directory-with-encrypted-models>/. test:/check
```

### build

Use the Dockerfile.worker instead to build the concurrent runtime, this will use worker-service and link with the worker library under worker install directory.

For tflm default rt impl:

```bash
docker build -t crtw-test:v1 -f docker/openwhisk/concurrent_runtime/Dockerfile.worker --build-arg INFERENCERT="TFLM_DEFAULT" .
```

```bash
docker build -t crtw-test:v1 -f docker/openwhisk/concurrent_runtime/Dockerfile.worker --build-arg INFERENCERT="TVMCRT_DEFAULT" --build-arg TVM_MODULE_PATH=/examples/tvm-mb/mobilenet1.0 .
```

These three arguments are consumed by the ratls/option/ra_tls_option.c.sh to generate the c source file.

### test the server in docker locally

Launch the key service. Attach a volume containing encrypted models to emulate a remote storage under `/mounted_store`.

```bash
docker run -it --name test -v worker-store:/mounted_store --net host --device /dev/sgx_enclave:/dev/sgx/enclave -v /var/run/aesmd:/var/run/aesmd crtw-test:v1
```

Supply an inference request (needs to conform the Apach OpenWhisk /run message, contains users request in json keyed by `value`)

```bash
curl -X POST localhost:8080/run -H 'Content-Type: application/json' -d @<openwhisk-run-request.json>
```

The request is in the following format:

```json
{
  "action_name":"/guest/test",
  "action_version":"0.0.1",
  "activation_id":"fd5c40e3beb642789c40e3beb6d278a9",
  "deadline":"1657606129470",
  "namespace":"guest",
  "transaction_id":"vEN96FGfWe0n8X9WOvYXZPaU9EFLffHm",
  "value":{
    "model_type" : "mobilenet_v1_1.0_224",
    "use_weight" : false,
    "user_id" : "admin",
    "key_service_address" : "127.0.0.1",
    "key_service_port" : 13571,
    "encrypted_sample" : "ab6t...."
  }
}
```
