# MOSS

Secure Cloud-Native Model Inference on Sensitive Data

## Prepare SGX machines

The project has been tested with `Ubuntu 20.04` with kernel `5.13.0.41-generic`. Intel SGX: SDK/PSW 2.14 - other libraries 1.11 (matching library version is important).

The machines should be installed with `Intel SGX AESM` service and the `Intel DCAP-ECDSA` attestation infrastructure setup. refer to Intel's online [documentation](https://www.intel.com/content/www/us/en/developer/articles/technical/quote-verification-attestation-with-intel-sgx-dcap.html) to setup the SGX environment.

After the setup:

* Kernel driver `/dev/sgx_enclave` and `/sgx_provision` should be present which will be mounted to KeyService and MossRT containers.
* SGX aesm service launched, which will be mounted to KeyService and MossRT containers. Check with `systemctl status aesmd`.
* The machine should have been registered with the Intel PCCS setup in your cluster. Confirm with the DCAP sample quote generation and verfication [codes](https://github.com/intel/SGXDataCenterAttestationPrimitives/tree/DCAP_1.11/SampleCode).

## Build MOSS components

1. Clone this repository locally and run `make preparation` to initialize all external dependencies.
2. refer to the README under `docker` folder to build respective dockers (client, KeyService, MossRT).
3. the FnPacker can be launched with `main.go`.

## Sample codes

We provide a sample tvm mobilenent model and all the requests in [example](examples/). It includes the encrypted model, inference request and all requests to update the access info in KeyService.

Please change the KeyService address to access the KeyService you deployed in the [client configuration](examples/client/client_config.json) and [inference request](examples/tvm-mb/inference_req.json).

## Setup OpenWhisk

We use OpenWhisk with its Docker container based invoker to run with MOSS components. OpenWhisk can be deployed on a kubernetes cluster with [openwhisk-deploy-kube repo](https://github.com/apache/openwhisk-deploy-kube). Here we details some important configurations.

OpenWhisk invoker needs to be configured such that it provides the appropriate flags to expose SGX drivers to the MossRT containers. This can be configured in `core/invoker/src/main/resources/application.conf` of the [OpenWhisk repo](https://github.com/apache/openwhisk) for `container-factory`. Then you can build the invoker image.

```conf
      extra-args: {
        "device": ["/dev/sgx_enclave:/dev/sgx/enclave"],
        "volume": ["/var/run/aesmd:/var/run/aesmd", "<path-for-the-model-storage>:/mounted_store"],
      }  
```

Another setting we set for our experiment is the timeout for idle containers

```conf
      # The "unusedTimeout" in the ContainerProxy,
      #aka 'How long should a container sit idle until we kill it?'
      idle-container = 3 minutes
```

configure the deployment helm charts by creating a yaml file to overwrite the default values. Here we provide a sample.

```yaml
whisk:
  ingress:
    type: NodePort
    apiHostName: <the node labeled openwhisk-role=core>
    apiHostPort: 31001
  # scale up the deployment
  limits:
    actionsInvokesPerminute: 12000 #(default: 60)
    actionsInvokesConcurrent: 12000 #(defualt: 30)
    triggersFiresPerminute: 12000 #(default: 60)
    actions:
      memory: #(default values)
        min: "128m"
        max: "8192m"
        std: "256m"
      concurrency:
        min: 1
        max: 10
        std: 1
    activation:
      payload:
        max: "2097152"
  containerPool:
    userMemory: "12800m"
  loadbalancer:
    blackboxFraction: "100%" # (default 10% can limit the invoker instances being used for blackbox action. if less than 10 then 1)

# disable dynamic volume provisioning
k8s:
  persistence:
    enabled: false

controller:
  replicaCount: 1
  jvmHeapMB: "4096"

# for single worker cluster, turn off affinity as stated in Initial setup in the root README 
affinity:
  enabled: true

toleration:
  enabled: false

# use custom wskinvoker to use sgx
invoker:
  imageName: "<invoker iamge>"
  imageTag: "<version>"
  options: "-Dwhisk.kubernetes.user-pod-node-affinity.enabled=false -Dwhisk.spi.LogStoreProvider=org.apache.openwhisk.core.containerpool.logging.LogDriverLogStoreProvider" # second options to turn off log processing at invoker
  jvmHeapMB: "4096"
  containerFactory:
    impl: "docker"
    enableConcurrency: true # for concurrency

# turn on metrics collection
metrics:
  prometheusEnabled: true # by default false (system metrics)
  userMetricsEnabled: true # by default false

# user metric grafana password (explicitly set to default here)
grafana:
  adminPassword: admin
```

Label the node in kubernetes:

```sh
# for the node to host openwhisk control plane
kubectl label node <control-node> openwhisk-role=core
# for the nodes to schedule containers
kubectl label node <invoker-node> openwhisk-role=invoker
```

In the cloned `openwhisk-deploy-kube`, run `helm install owdev openwhisk/openwhisk -n openwhisk --create-namespace -f mycluster.yaml` to deploy OpenWhisk.

## Setup KeyService

Deploy the built key server image following the instruction in the [docker](docker/key_server/README.md) folder.

Then update the access information with [client](docker/client/README.md).

```sh
# register the user
./client register_req.json client_config.json
# specify the model decryption key
./client up_model_key_req.json client_config.json
# grant the user and worker access to the model 
./client grant_model_access_req.json client_config.json
# specify the input decryption key for the worker to use
./client add_request_key_req.json client_config.json
```

Please find the sample requests and the model in [examples](examples)

## Deploy a MossRT for a model

Once the container is build following instructions under the [docker](docker/openwhisk/concurrent_runtime/README.md) folder of this repo. You can deploy the action directly with openwhisk client for testing.

```sh
wsk -i action create <action-name> -m <memory-budget> --docker <the MossRT image>
```

You can now invoke this function with the [sample request](examples/tvm-mb/inference_req.json).

To create a `Fnpool` and manage its invocation through FnPacker, please refer to the FnPacker client sample codes under [fnpacker/fpcli/example.go](fnpacker/fpcli/example.go).
