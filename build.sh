#!/bin/bash

set -euo pipefail

# Default values
BUILD_ONLY=false

# Parse arguments
POSITIONAL_ARGS=()
for arg in "$@"; do
  case "$arg" in
    --build-only)
      BUILD_ONLY=true
      ;;
    *)
      POSITIONAL_ARGS+=("$arg")
      ;;
  esac
done

# Restore positional parameters
set -- "${POSITIONAL_ARGS[@]}"

# TARGET
# must contain the filename of a toolchain file:
#   - linux-x86
#   - linux-aarch64
TARGET=${1:-linux-x86}

BUILD_DIR="build-$TARGET"
CMD_CMAKE_PRE="cmake -DCMAKE_BUILD_TYPE=Debug -B $BUILD_DIR -S . -DCMAKE_TOOLCHAIN_FILE=toolchains/$TARGET.cmake"
CMD_CMAKE_BUILD="cmake --build $BUILD_DIR"

if [ "$BUILD_ONLY" = false ]; then
  echo "🔧 Configuring for $TARGET..."
  $CMD_CMAKE_PRE
else
  echo "🚫 Skipping configuration (--build-only)"
fi

echo "🔨 Building..."
$CMD_CMAKE_BUILD
