# Client Docker

This containerized client in a docker.

## Build

```sh
docker build -t client-test:v1 -f docker/client/Dockerfile .
```

## Launch

### default key server store client supplied information under /tmp

launch client

```sh
docker run -it --name client --net host -v <path-to-example-queries>:<in-container-path> client-test:v1 ./client <path-to-request-json> <path-to-config-json>
```
