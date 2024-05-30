# choose between llhttp and http_parser (default llhttp)
USE_LLHTTP ?= 1
PROJECT_ROOT_DIR ?= $(shell readlink -f ..)
USE_WORKER_SERVICE ?= 0
WORKER_INSTALL_PATH ?= $(PROJECT_ROOT_DIR)/worker/install
INSTALL_PREFIX ?= $(shell readlink -f install)

DEPS_INSTALL_DIR = $(PROJECT_ROOT_DIR)/deps/install
HTTP_PARSER_DIR = $(PROJECT_ROOT_DIR)/deps/http-parser
LIBUV_DIR = $(DEPS_INSTALL_DIR)/libuv
LLHTTP_DIR = $(DEPS_INSTALL_DIR)/llhttp

CONCURRENT_RT_LINK_FLAGS := -L$(LIBUV_DIR)/lib -l:libuv_a.a -lpthread -ldl

CONCURRENT_RT_CXX_FLAGS := -I$(LIBUV_DIR)/include

ifneq ($(USE_LLHTTP), 0)
	CONCURRENT_RT_CXX_FLAGS += -DUSE_LLHTTP -I$(LLHTTP_DIR)/include
	CONCURRENT_RT_LINK_FLAGS += -L$(LLHTTP_DIR)/lib -l:libllhttp.a
else
	CONCURRENT_RT_CXX_FLAGS += -I$(HTTP_PARSER_DIR)
	CONCURRENT_RT_LINK_FLAGS += -L$(HTTP_PARSER_DIR) -l:libhttp_parser.a
endif

deps:
	@echo "run make deps under repo root to build deps"

.PHONY: deps all clean

all: deps openwhisk_runtime

openwhisk_runtime:
	make -C openwhisk CONCURRENT_RT_CXX_FLAGS="${CONCURRENT_RT_CXX_FLAGS}" CONCURRENT_RT_LINK_FLAGS="${CONCURRENT_RT_LINK_FLAGS}" USE_WORKER_SERVICE=$(USE_WORKER_SERVICE) WORKER_INSTALL_PATH=${WORKER_INSTALL_PATH}

install_openwhisk_runtime:
	make install -C openwhisk INSTALL_PREFIX=$(INSTALL_PREFIX) USE_WORKER_SERVICE=$(USE_WORKER_SERVICE)

clean:
	make -C openwhisk clean

mrproper: clean
	rm -rf install
