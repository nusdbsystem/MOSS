FROM sgx_dcap:v1

USER root

ENV SGX_SDK=/opt/intel/sgxsdk

# avoid tzdata interactive config during apt install.
ENV TZ=Asia/Singapore
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

ARG SPID
ARG EPID_SUBSCRIPTION_KEY
ARG QUOTE_TYPE
ARG USE_ECDSA

RUN ([ ${USE_ECDSA} = 1 ] && echo "use ecdsa") || (test -n "$SPID" \
  && test -n "$EPID_SUBSCRIPTION_KEY" \
  && test -n "$QUOTE_TYPE" \
  && echo ${SPID} ${EPID_SUBSCRIPTION_KEY} ${QUOTE_TYPE})

# RUN test -n "$SPID" \
#   && test -n "$EPID_SUBSCRIPTION_KEY" \
#   && test -n "$QUOTE_TYPE" \
#   && echo ${SPID} ${EPID_SUBSCRIPTION_KEY} ${QUOTE_TYPE}
ENV USE_ECDSA ${USE_ECDSA}
ENV SPID ${SPID}
ENV EPID_SUBSCRIPTION_KEY ${EPID_SUBSCRIPTION_KEY}
ENV QUOTE_TYPE ${QUOTE_TYPE}

# to install gcc-5 required by wolfssl in ratls.
RUN printf "deb http://dk.archive.ubuntu.com/ubuntu/ xenial main \n\
deb http://dk.archive.ubuntu.com/ubuntu/ xenial universe" > /etc/apt/sources.list.d/xenial.list

RUN apt-get update && apt-get install -y \
  # install deps (libuv/http-parser)
  cmake \
  clang \
  # install deps (ratls)
  autoconf \
  libtool \
  gcc-5 \
  git \
  # install deps (tflm)
  curl \
  python3 \
  python3-pip \
  unzip \
  wget \
  # install npm to build llhttp
  npm \
  && npm i -g npx \
  && python3 -m pip install --upgrade pip \
  && python3 -m pip install --upgrade Pillow

COPY ./deps /secure-serverless-inference/deps
COPY ./Makefile /secure-serverless-inference/
RUN cd /secure-serverless-inference/ && make mrproper && make deps
