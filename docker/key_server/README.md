# Key Server Docker

This containerized key server in a docker.

## Build

```sh
docker build -t key-server-test:v1 -f docker/key_server/Dockerfile .
```

## Launch

### default key server store client supplied information under /tmp

launch key server

```sh
docker run -it --name key-server --net host --device /dev/sgx/enclave -v /var/run/aesmd:/var/run/aesmd key-server-test:v1
```

or (for docker aesmd and in-kernel driver only)

```sh
docker run -it --name key-server --net host --device /dev/sgx_enclave:/dev/sgx/enclave -v aesmd-socket:/var/run/aesmd --user root key-server-test:v1
```

### attach a volume to manage enclave sealed data

Prepare a local volume as key server store

```sh
docker volume create ks-store
docker run -it --name test -v ks-store:/check ubuntu /bin/bash
```

populate the volume from another terminal

```bash
docker cp <directory-with-sealed-key-server-data>/. test:/check
```

launch key server and attach the ks-store volume

```sh
docker run -it --name key-server -v ks-store:/tmp --net host --device /dev/sgx/enclave -v /var/run/aesmd:/var/run/aesmd key-server-test:v1
```
