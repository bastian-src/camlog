#!/bin/bash

podman run --rm -v "$(pwd)":/src -w /src camlog:latest bash -c "./build.sh $@"
