# NVM Unsafe Java Library

## Description
This library implements an Unsafe API Library about managing unsafely off-heap memory.


## Prerequisites
To use the Persistent Memory Java Library you have first install the
`PMDK: Persistent Memory Development Kit`. 
For more information see:
https://github.com/pmem/pmdk

```
$ wget https://github.com/pmem/pmdk/archive/1.4.zip
$ unzip 1.4.zip
$ cd pmdk-1.4
$ make
$ sudo make install prefix=/usr/local
```

## Build
To build all shared libraries and classes for NVM Unsafe library :

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
Inside the nvmUnsafe directory there is a test file to check if the
building of the library succeed.
```
$ javac -cp ../../nvmUnsafe/NVMUnsafe.jar Example.java
$ java -cp .:../../nvmUnsafe/NVMUnsafe.jar Example
```

## Authors
Iacovos G. Kolokasis,
Polyvios Pratikakis,
Angelos Bilas
