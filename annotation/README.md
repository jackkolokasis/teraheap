# Scala annotation Cache and Uncache

## Description
Allocate scala objects in TeraCache using JNI

## Build
To build all shared libraries and classes for TeraCache Api:

```
$ make all  
$ make NVMUnsafe.jar
$ sudo make install 
```

Set the dynamic library path
```
$ export LD_LIBRARY_PATH = /usr/local/lib
```

## Example usage
Test directory contains tests to check if the building of the library
succeed.
```
$ cd tests
$ make clean
$ make all
```

## Authors
Iacovos G. Kolokasis,
