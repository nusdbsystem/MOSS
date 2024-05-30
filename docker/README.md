# Build Docker Applications

This folder contains a set of Dockerfile to test and run individual components of this project. Folder are organized as follows:

* `benchmark`: baselines
* `client`: client application that update key service information
* `dcap_base`: base image that contains all dcap dependencies for sgx applications
* `key_server`: key service instance
* `openwhisk`: the openwhisk compatible MossRT image
* `test`: test the worker application
  * `tflm`: test the worker with tflm referenec default runtime impl
  * `tvmcrt`: test the worker with tvmcrt referenec default runtime impl
* `tools`: miscellaneous images
* `Dockerfile`: a builder image that builds common dependencies of MOSS

To build any of the components, one should start with the `dcap` base image and the `builderbase` image.

## build `dcap_base` image

```sh
docker build --target sgx_dcap_2.14_1.11 -t sgx_dcap:v1 -f docker/dcap_base/Dockerfile .
```

## Build `builderbase` image

Register for the Intel attestation service (IAS) to obtain the SPID, EPID_SUBSCRIPTION_KEY and QUOTE_TYPE such that the enclave can uses IAS to prepare certificate for ratls.

```sh
docker build -t builderbase -f docker/Dockerfile --build-arg SPID=<spid> --build-arg EPID_SUBSCRIPTION_KEY=<epid-subscription-key> --build-arg QUOTE_TYPE=<quote-type> .
```

for ecdsa

```sh
docker build -t builderbase -f docker/Dockerfile --build-arg USE_ECDSA=1 .
```

## Build MossRT running tvm mobilenet

refer to `openwhisk/concurrent_runtime`.
