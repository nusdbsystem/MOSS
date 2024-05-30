FROM builderbase as clientbuilder

COPY --from=builderbase /secure-serverless-inference/deps /secure-serverless-inference/deps
COPY ./common /secure-serverless-inference/common
COPY ./client /secure-serverless-inference/client
COPY ./message /secure-serverless-inference/message

RUN cd /secure-serverless-inference/client && make clean && make all

# the verification still depends on some SGX libraries.
# for simplicity we used the same base image.
FROM sgx_dcap:v1

COPY --from=clientbuilder /secure-serverless-inference/client/client ./client

CMD ./client
