### Project Settings ###
PROJECT_ROOT_DIR ?= $(shell readlink -f ..)
SGX_RA_TLS_ROOT_DIR ?= $(shell readlink -f ../deps/ratls)

SGX_RA_TLS_DIR=$(PROJECT_ROOT_DIR)/deps/install/ratls

Common_C_Cpp_Flags := $(SGX_COMMON_CFLAGS) -fPIC -Wno-attributes -I.
Common_C_Cpp_Flags += -Wall -Wextra -Winit-self -Wpointer-arith -Wreturn-type \
                    -Waddress -Wsequence-point -Wformat-security \
                    -Wmissing-include-dirs -Wfloat-equal -Wundef -Wshadow \
                    -Wcast-align -Wcast-qual -Wconversion -Wredundant-decls
Common_C_Flags := -Wjump-misses-init -Wstrict-prototypes \
										-Wunsuffixed-float-constants
SGX_RA_TLS_Extra_Flags := -DWOLFSSL_SGX 

App_C_Cpp_Flags := $(Common_C_Cpp_Flags) $(SGX_RA_TLS_Extra_Flags) -I$(SGX_SDK)/include -I$(PROJECT_ROOT_DIR) -I$(SGX_RA_TLS_DIR)/include

APP_DCAP_LIBS := -lsgx_dcap_ql -lsgx_dcap_quoteverify -lsgx_urts

### Linking setting ###
App_Link_Flags := -L$(SGX_RA_TLS_DIR)/lib -lratls_challenger -lratls_common_u \
	-l:libwolfssl.a -lm $(APP_DCAP_LIBS)

### Build all ###
all: client

### Sources ###
## build files needed from other directory

fileutil.o: ../common/fileutil.cc
	@echo $(CXX) $(App_C_Cpp_Flags) -c $< -o $@
	@$(CXX) $(App_C_Cpp_Flags) -c $< -o $@
	@echo "CXX  <=  $<"

json.o: ../common/json.cc
	@echo $(CXX) $(App_C_Cpp_Flags) -c $< -o $@
	@$(CXX) $(App_C_Cpp_Flags) -c $< -o $@
	@echo "CXX  <=  $<"

channel_client.o: ../message/channel_client.cc
	@echo $(CXX) $(App_C_Cpp_Flags) -c $< -o $@
	@$(CXX) $(App_C_Cpp_Flags) -c $< -o $@
	@echo "CXX  <=  $<"

client_ks.o: ../message/client_ks.cc
	@echo $(CXX) $(App_C_Cpp_Flags) -c $< -o $@
	@$(CXX) $(App_C_Cpp_Flags) -c $< -o $@
	@echo "CXX  <=  $<"

client.o: client.cc
	@echo $(CXX) $(App_C_Cpp_Flags) -c $< -o $@
	@$(CXX) $(App_C_Cpp_Flags) -c $< -o $@
	@echo "CXX  <=  $<"

App_Cpp_Objects := client.o client_ks.o channel_client.o json.o fileutil.o

## Build worker app ##
client: $(App_Cpp_Objects)
	@echo $(CXX) -c $^ -o $@ $(App_Link_Flags)
	@$(CXX) $^ -o $@ $(App_Link_Flags)
	@echo "LINK =>  $@"
### Sources ###

### Clean command ###
clean:
	@rm -f client $(App_Cpp_Objects)
