FROM builderbase as ksbuilder

USER root

COPY --from=builderbase /secure-serverless-inference/deps /secure-serverless-inference/deps
COPY ./common/ /secure-serverless-inference/common
COPY ./message/ /secure-serverless-inference/message
COPY ./store/ /secure-serverless-inference/store
COPY ./key_server/ /secure-serverless-inference/key_server

RUN cd /secure-serverless-inference/key_server && make mrproper && make all && make install

FROM sgx_dcap:v1

USER root

COPY --from=ksbuilder /secure-serverless-inference/key_server/install /install

CMD SGX_AESM_ADDR=1 /install/bin/key_server /tmp /install/lib/KeyServer_Enclave.signed.so
