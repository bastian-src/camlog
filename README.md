# Camera logging (camlog)

## Build

Use the build script to build:

```
./build.sh
```

You can pass a certain architecture to load a certain toolchain for your build:

```
# Currently, linux-x86 (default) and linux-aarch64 are supported
./build.sh linux-x86
```

`cmake` downloads the dependencies (see `Dependencies.cmake`) and, finally, results in a binary:

```
./build-linux-x86/bin/camlog
```

## Build for arm64

Use podman/docker to cross-compile for aarch64:

```
podman build -t camlog .
./podman_build.sh linux-aarch64
```

The container is configured for aarch64 and, finally, results in a binary:

```
./build-linux-aarch64/bin/camlog
```

## Memory Check

Install `valgrind`:

```
sudo apt-get install valgrind
```

And use the script to run the memory check:

```
./check.sh
```
