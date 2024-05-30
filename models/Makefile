### Project Settings ###
PROJECT_ROOT_DIR := $(shell readlink -f ..)

## Dependency path ##
DEP_SGXDNET_DIR := $(shell readlink -f sgxdnet)
DEP_TFLM_DIR := $(shell readlink -f tflm)
DEP_TVMCRT_DIR := $(shell readlink -f tvm_crt)
### Project Settings ###

### Choose the inference runtime ###
# INFERENCERT ?= DNET_MNIST
INFERENCERT ?= TFLM_DEFAULT
# INFERENCERT ?= TVMCRT_DEFAULT
### Choose the inference runtime ###

## pick the dependencies for inference runtime ##
ifeq ($(INFERENCERT), DNET_MNIST)
	DEP_RT_DIR := $(DEP_SGXDNET_DIR)
	RT_DIR := ./sgxdnet/examples/mnist
else ifeq ($(INFERENCERT), TFLM_DEFAULT)
	DEP_RT_DIR := $(DEP_TFLM_DIR)
	RT_DIR := $(DEP_TFLM_DIR)/examples/default
	RT_BUILD_OPTIONS := DEP_TFLM_DIR=$(DEP_TFLM_DIR)
else ifeq ($(INFERENCERT), TVMCRT_DEFAULT)
	DEP_RT_DIR := $(DEP_TVMCRT_DIR)
	RT_DIR := $(DEP_TVMCRT_DIR)/default
	TVM_MODULE_PATH ?= $(DEP_TVMCRT_DIR)/module
	RT_BUILD_OPTIONS := DEP_TVMCRT_DIR=$(DEP_TVMCRT_DIR) \
		MODULE_OBJS_DIR=$(shell readlink -f $(TVM_MODULE_PATH))
endif

.PHONY: all inference_rt_libs clean install mrproper

all: inference_rt_libs

inference_rt_libs:
	$(MAKE) -C $(DEP_RT_DIR) all PROJECT_ROOT_DIR=$(PROJECT_ROOT_DIR)
	$(MAKE) -C $(RT_DIR) all PROJECT_ROOT_DIR=$(PROJECT_ROOT_DIR) $(RT_BUILD_OPTIONS)

# install
INSTALL_PREFIX ?= $(abspath ./install)
install: 
	@echo Installing inference runtime
	install -d $(INSTALL_PREFIX)/include
	install -d $(INSTALL_PREFIX)/lib
	install -C -m 644 include/model.h $(INSTALL_PREFIX)/include/model.h
	install -C -m 644 include/model.edl $(INSTALL_PREFIX)/include/model.edl
	$(MAKE) -C $(RT_DIR) install INSTALL_PREFIX=$(INSTALL_PREFIX)
	@echo inference runtime installed

clean:
	$(MAKE) -C $(DEP_RT_DIR) clean
	$(MAKE) -C $(RT_DIR) clean
	@rm -rf lib

mrproper: clean
	$(MAKE) -C $(DEP_RT_DIR) mrproper
	[ -e install ] && rm -rf install || :
