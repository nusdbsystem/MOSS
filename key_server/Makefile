SGX_SDK ?= /opt/intel/sgxsdk
SGX_MODE ?= HW
SGX_ARCH ?= x64
SGX_DEBUG ?= 1

PROJECT_ROOT_DIR := $(shell readlink -f ..)

INSTALL ?= install
INSTALL_PREFIX ?= $(PROJECT_ROOT_DIR)/key_server/install
INSTALL_BIN_DIR = $(INSTALL_PREFIX)/bin
INSTALL_LIB_DIR = $(INSTALL_PREFIX)/lib

.PHONY: all install clean mrproper

all:
	$(MAKE) -ef sgx_u.mk all SGX_MODE=$(SGX_MODE) SGX_DEBUG=$(SGX_DEBUG)
	$(MAKE) -ef sgx_t.mk all SGX_MODE=$(SGX_MODE) SGX_DEBUG=$(SGX_DEBUG)


install:
	$(INSTALL) -d $(INSTALL_BIN_DIR)
	$(INSTALL) -d $(INSTALL_LIB_DIR)
	$(INSTALL) -C -m 755 key_server $(INSTALL_BIN_DIR)
	$(INSTALL) -C -m 664 *.signed.so $(INSTALL_LIB_DIR)

clean:
	$(MAKE) -ef sgx_u.mk clean SGX_DEBUG=$(SGX_DEBUG)
	$(MAKE) -ef sgx_t.mk clean SGX_DEBUG=$(SGX_DEBUG)

mrproper: clean
	rm -rf $(PROJECT_ROOT_DIR)/key_server/install
