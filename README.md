# Camera logging (camlog)

## Build

Install libraries:

```
sudo apt-get install libjpeg-dev libexif-dev
```

Use the build script to build:

```
./build.sh
```

And run camlog:

```
./build/bin/camlog
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
