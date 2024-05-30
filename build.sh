#!/bin/bash

## Build project
make preparation
USE_ECDSA=1 make deps
make -C models
make -C models install
make -C worker
make -C key_server
make -C client
