# used to mainly build dependencies
PROJECT_ROOT_DIR := $(shell readlink -f .)

DEPS_INSTALL_DIR = $(PROJECT_ROOT_DIR)/deps/install

HTTP_PARSER_DIR = $(PROJECT_ROOT_DIR)/deps/http-parser
LIBUV_DIR = $(PROJECT_ROOT_DIR)/deps/libuv
LLHTTP_DIR = $(PROJECT_ROOT_DIR)/deps/llhttp

RATLS_DIR = $(PROJECT_ROOT_DIR)/deps/ratls

### TFLM
# many variables defined to facilitate local install of tflm
TFLM = tflm
TFLM_DIR = $(PROJECT_ROOT_DIR)/deps/$(TFLM)
TFLM_LOCAL_INCLUDE_DIR = $(DEPS_INSTALL_DIR)/$(TFLM)/include
TFLM_LOCAL_LIB_DIR = $(DEPS_INSTALL_DIR)/$(TFLM)/lib

TFLM_DEP_RELATIVE_PATH := $(TFLM_DIR)/tensorflow/lite/micro/tools/make/downloads/
TFLM_BUILD_TARGET_PATH = $(TFLM_DIR)/tensorflow/lite/micro/tools/make/gen
TFLM_RELEASE_LIB = $(TFLM_BUILD_TARGET_PATH)/linux_x86_64_release/lib/libtensorflow-microlite.a

# for micro and subdir of micro no recursive install
TFLM_INSTALL_HEADER_PATHS := tensorflow/lite/c \
												tensorflow/lite/core \
												tensorflow/lite/kernels \
												tensorflow/lite/schema

TFLM_INSTALL_NONRECURSIVE_HEADER_PATHS := tensorflow/lite \
												tensorflow/lite/micro \
												tensorflow/lite/micro/kernels \
												tensorflow/lite/micro/memory_planner

TFLM_DEP_FLATBUFFERS_HEADER_PATH := $(TFLM_DEP_RELATIVE_PATH)/flatbuffers/include/flatbuffers
TFLM_DEP_GEMMLOWP_HEADER_PATH := $(TFLM_DEP_RELATIVE_PATH)/gemmlowp
TFLM_DEP_RUY_HEADER_PATH := $(TFLM_DEP_RELATIVE_PATH)/ruy/ruy
TFLM_DEP_LOCAL_PATHS := flatbuffers gemmlowp ruy
### TFLM

.PHONY: all concurrent_runtime_deps deps mrproper preparation

preparation:
	git submodule update --init --recursive

# make sure ra_tls_options.c is already created
ratls_deps:
	cd $(RATLS_DIR) && make clean \
	&& make option/ra_tls_options.c && mkdir -p deps && make -j4 deps \
	&& make ratls_libs \
	&& make INSTALL_PREFIX=$(PROJECT_ROOT_DIR)/deps/install/ratls install \
	&& cd $(PROJECT_ROOT_DIR)

ratls_deps_clean:
	make -C $(RATLS_DIR) mrproper

# tflm does not have install target
tflm_deps:
	cd $(TFLM_DIR) && make -f tensorflow/lite/micro/tools/make/Makefile clean \
	&& rm -rf tensorflow/lite/micro/tools/make/downloads/ \
	&& make BUILD_TYPE=release -f tensorflow/lite/micro/tools/make/Makefile microlite -j10 \
	&& cd $(PROJECT_ROOT_DIR)
	@echo Installing tflm headers locally
	install -d $(TFLM_LOCAL_INCLUDE_DIR)/$(TFLM)
	for header_dir in $(TFLM_INSTALL_HEADER_PATHS); do \
		install -d $(TFLM_LOCAL_INCLUDE_DIR)/$(TFLM)/$$header_dir; \
		for subdir in `find $(TFLM_DIR)/$$header_dir -type d -not -path '*/.*'`; do \
			dest=$${subdir#"$(TFLM_DIR)"}; \
			install -d $(TFLM_LOCAL_INCLUDE_DIR)/$(TFLM)/$$dest; \
		done; \
		for header in `find $(TFLM_DIR)/$$header_dir -type f -name *.h -not -path '*/.*'`; do \
			dest=$${header#"$(TFLM_DIR)"}; \
			install -C -m 644 $$header $(TFLM_LOCAL_INCLUDE_DIR)/$(TFLM)/$$dest; \
		done \
	done
	for header_dir in $(TFLM_INSTALL_NONRECURSIVE_HEADER_PATHS); do \
		install -d $(TFLM_LOCAL_INCLUDE_DIR)/$(TFLM)/$$header_dir; \
		for header in `find $(TFLM_DIR)/$$header_dir -maxdepth 1 -type f -name *.h -not -path '*/.*'`; do \
			dest=$${header#"$(TFLM_DIR)"}; \
			install -C -m 644 $$header $(TFLM_LOCAL_INCLUDE_DIR)/$(TFLM)/$$dest; \
		done \
	done
	for header_dir in $(TFLM_DEP_LOCAL_PATHS); do \
		install -d $(TFLM_LOCAL_INCLUDE_DIR)/$$header_dir; \
	done
	for subdir in `find $(TFLM_DEP_FLATBUFFERS_HEADER_PATH) -type d -not -path '*/.*'`; do \
		dest=$${subdir#"$(TFLM_DEP_FLATBUFFERS_HEADER_PATH)"}; \
		install -d $(TFLM_LOCAL_INCLUDE_DIR)/flatbuffers/$$dest; \
	done
	for header in `find $(TFLM_DEP_FLATBUFFERS_HEADER_PATH) -type f -name *.h -not -path '*/.*'`; do \
		dest=$${header#"$(TFLM_DEP_FLATBUFFERS_HEADER_PATH)"}; \
		install -C -m 644 $$header $(TFLM_LOCAL_INCLUDE_DIR)/flatbuffers/$$dest; \
	done
	for subdir in `find $(TFLM_DEP_GEMMLOWP_HEADER_PATH) -type d -not -path '*/.*'`; do \
		dest=$${subdir#"$(TFLM_DEP_GEMMLOWP_HEADER_PATH)"}; \
		install -d $(TFLM_LOCAL_INCLUDE_DIR)/gemmlowp/$$dest; \
	done
	for header in `find $(TFLM_DEP_GEMMLOWP_HEADER_PATH) -type f -name *.h -not -path '*/.*'`; do \
		dest=$${header#"$(TFLM_DEP_GEMMLOWP_HEADER_PATH)"}; \
		install -C -m 644 $$header $(TFLM_LOCAL_INCLUDE_DIR)/gemmlowp/$$dest; \
	done
	for subdir in `find $(TFLM_DEP_RUY_HEADER_PATH) -type d -not -path '*/.*'`; do \
		dest=$${subdir#"$(TFLM_DEP_RUY_HEADER_PATH)"}; \
		install -d $(TFLM_LOCAL_INCLUDE_DIR)/ruy/$$dest; \
	done
	for header in `find $(TFLM_DEP_RUY_HEADER_PATH) -type f -name *.h -not -path '*/.*'`; do \
		dest=$${header#"$(TFLM_DEP_RUY_HEADER_PATH)"}; \
		install -C -m 644 $$header $(TFLM_LOCAL_INCLUDE_DIR)/ruy/$$dest; \
	done	
	@echo tflm headers installed
	@echo Installing tflm library locally
	install -d $(TFLM_LOCAL_LIB_DIR)
	install -C -m 644 $(TFLM_RELEASE_LIB) $(TFLM_LOCAL_LIB_DIR)/libtflm_t.a
	@echo tflm library installed

# buggy tflm overall Makefile clean and clean_downloads commands
# here we directly remove the folders
tflm_deps_clean:
	cd $(TFLM_DIR) \
	&& rm -rf tensorflow/lite/micro/tools/make/gen \
	&& rm -rf tensorflow/lite/micro/tools/make/downloads \
	&& cd $(PROJECT_ROOT_DIR)

concurrent_runtime_deps:
	mkdir -p $(DEPS_INSTALL_DIR)
	cd $(HTTP_PARSER_DIR) && make clean && make package && cd $(PROJECT_ROOT_DIR)
	cd $(LIBUV_DIR) && mkdir build && cd build && cmake .. -DCMAKE_INSTALL_PREFIX=$(DEPS_INSTALL_DIR)/libuv && make && make install && cd $(PROJECT_ROOT_DIR)
	mkdir -p $(DEPS_INSTALL_DIR)/llhttp/include && mkdir $(DEPS_INSTALL_DIR)/llhttp/lib
	cd $(LLHTTP_DIR) && npm install && make && PREFIX=$(DEPS_INSTALL_DIR)/llhttp make install && cd $(PROJECT_ROOT_DIR)

concurrent_runtime_deps_clean:
	make -C $(HTTP_PARSER_DIR) clean
	make -C $(LLHTTP_DIR) clean
	rm -rf $(LIBUV_DIR)/build

deps: ratls_deps tflm_deps concurrent_runtime_deps

all: preparation deps

clean: 
	@echo "nothing to clean"

mrproper: clean concurrent_runtime_deps_clean tflm_deps_clean ratls_deps_clean
	rm -rf deps/install
