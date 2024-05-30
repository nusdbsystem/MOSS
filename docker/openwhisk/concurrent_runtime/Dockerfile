FROM ubuntu:20.04 as builder

# avoid tzdata interactive config during apt install.
ENV TZ=Asia/Singapore
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update && apt-get install -y \
  # install deps (libuv/http-parser)
  cmake \
  clang \
  # install npm to build llhttp
  npm \
  && npm i -g npx

COPY ./deps/ /secure-serverless-inference/deps
COPY ./Makefile /secure-serverless-inference/
RUN cd /secure-serverless-inference/ && make mrproper && make concurrent_runtime_deps

COPY ./common/ /secure-serverless-inference/common
COPY ./concurrent_runtime/ /secure-serverless-inference/concurrent_runtime

RUN cd /secure-serverless-inference/concurrent_runtime \
  && make mrproper && make openwhisk_runtime && make install_openwhisk_runtime

FROM ubuntu:20.04

COPY --from=builder /secure-serverless-inference/concurrent_runtime/install /install

CMD /install/bin/server
