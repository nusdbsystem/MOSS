FROM ubuntu:20.04 as sgx_dcap_2.14_1.11

USER root

RUN apt-get update && apt-get install -y \
  build-essential \
  git \
  wget

# install sdk
RUN wget https://download.01.org/intel-sgx/sgx-dcap/1.11/linux/distro/ubuntu20.04-server/sgx_linux_x64_sdk_2.14.100.2.bin \
  && chmod 755 sgx_linux_x64_sdk_2.14.100.2.bin \
  && ./sgx_linux_x64_sdk_2.14.100.2.bin --prefix=/opt/intel \
  source /opt/intel/sgxsdk/environment

# install dcap libs
RUN echo 'deb [arch=amd64] https://download.01.org/intel-sgx/sgx_repo/ubuntu focal main' | tee /etc/apt/sources.list.d/intel-sgx.list > /dev/null \
  && wget -O - https://download.01.org/intel-sgx/sgx_repo/ubuntu/intel-sgx-deb.key | apt-key add - \
  && apt update

RUN apt install -y \
  libsgx-epid=2.14.100.2-focal1 \
  libsgx-headers=2.14.100.2-focal1 \
  libsgx-urts=2.14.100.2-focal1 \
  libsgx-launch=2.14.100.2-focal1 \
  libsgx-ae-le=2.14.100.2-focal1 \
  libsgx-ae-pce=2.14.100.2-focal1 \
  libsgx-ae-qe3=1.11.100.2-focal1 \
  libsgx-ae-qve=1.11.100.2-focal1 \
  libsgx-ae-epid=2.14.100.2-focal1 \
  libsgx-qe3-logic=1.11.100.2-focal1 \
  libsgx-pce-logic=1.11.100.2-focal1 \
  libsgx-enclave-common=2.14.100.2-focal1 \
  sgx-aesm-service=2.14.100.2-focal1 \
  libsgx-quote-ex=2.14.100.2-focal1 \
  libsgx-dcap-ql=1.11.100.2-focal1 \
  libsgx-dcap-quote-verify=1.11.100.2-focal1 \
  # workload system additional packages
  libsgx-quote-ex-dev=2.14.100.2-focal1 \
  libsgx-dcap-ql-dev=1.11.100.2-focal1 \
  libsgx-dcap-quote-verify-dev=1.11.100.2-focal1 \
  # qpl needed for verification
  libsgx-dcap-default-qpl=1.11.100.2-focal1 \
  # runtime
  libsgx-uae-service=2.14.100.2-focal1


# attester uses aesm service to do out-of-proc quote gen.
# challenger does verification with QPL. QPL needs to contact PCCS as configured by /etc/sgx_default_qcnl.conf which is installed with libsgx-dcap-default-qpl.

FROM sgx_dcap_2.14_1.11

USER root

ARG PCCS_ADDR=localhost
ARG USE_SECURE_CERT=FALSE

ENV PCCS_ADDR ${PCCS_ADDR}
ENV USE_SECURE_CERT ${USE_SECURE_CERT}

# config the qve/qvl to connect to pccs for fetching attestation collatoral.
RUN printf "# PCCS server address \n\
  PCCS_URL=https://${PCCS_ADDR}:8081/sgx/certification/v3/ \n\
  # To accept insecure HTTPS cert, set this option to FALSE \n\
  USE_SECURE_CERT=${USE_SECURE_CERT}" > /etc/sgx_default_qcnl.conf

RUN git clone https://github.com/intel/SGXDataCenterAttestationPrimitives.git \
  && cd SGXDataCenterAttestationPrimitives/ \
  && git checkout DCAP_1.11 \
  && cd SampleCode/QuoteVerificationSample \
  && make SGX_DEBUG=1 \
  && cd ../../SampleCode/QuoteGenerationSample \
  && make SGX_DEBUG=1

CMD cd /SGXDataCenterAttestationPrimitives/SampleCode/QuoteGenerationSample \
  && SGX_AESM_ADDR=1 ./app \
  && cd ../QuoteVerificationSample \
  && ./app -quote ../QuoteGenerationSample/quote.dat
