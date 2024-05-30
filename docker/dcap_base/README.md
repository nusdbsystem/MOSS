# SGX DCAP base image

To support DCAP-ECDSA attestation, we add the dcap dependencies to the SGX base image to build all components of this project.

The dockerfile here provides the base image in the first stage and builds the sample code to test you local setup of SGX machines with DCAP. Note that the sample assumes pccs is setup on localhost and uses out-of-proc aesm.

## build and run the sample code

```sh
docker build --build-arg PCCS_ADDR=<your pccs service address> -t dcap_test:v1 -f docker/dcap_base/Dockerfile .
docker run -it --name dcap-test --net host --device /dev/sgx_enclave:/dev/sgx/enclave -v /var/run/aesmd:/var/run/aesmd  --user root dcap_test:v1
```

## build the base image

```sh
docker build --target sgx_dcap_2.14_1.11 -t sgx_dcap:v1 -f docker/dcap_base/Dockerfile .
```
