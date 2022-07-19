# NVM Unsafe Java Library

## Description
This library implements an Unsafe API Library about managing unsafely
off-heap memory.


## Prerequisites
To use the Persistent Memory Java Library you have first install the
`PMDK: Persistent Memory Development Kit`. 
For more information see:
https://github.com/pmem/pmdk

```bash
sudo yum install libvmem.x86_64 libvmem-devel.x86_64
```

## Build
To build all shared libraries and classes for NVM Unsafe library :

```bash
make all  
make NVMUnsafe.jar
```

Set the dynamic library path in the .bashrc
```bash
export LD_LIBRARY_PATH=/path/to/repo/github/teracache/nvmUnsafe/lib/:$LD_LIBRARY_PATH
export PATH=/path/to/repo/github/teracache/nvmUnsafe/jni/:$PATH
export C_INCLUDE_PATH=/path/to/repo/github/teracache/nvmUnsafe/jni/:$C_INCLUDE_PATH
export CPLUS_INCLUDE_PATH=/path/to/repo/github/teracache/nvmUnsafe/jni/:$CPLUS_INCLUDE_PATH
```

## Build Library and Port it to the Spark
1. Set the SPARK_DIR and JAVA_HOME in the ./config.sh

2. Execute ./build.sh script
```bash
./build.sh
```
## Example usage
Inside the nvmUnsafe directory there is a test file to check if the
building of the library succeed.
```
$ javac -cp ../../nvmUnsafe/NVMUnsafe.jar Example.java
$ java -cp .:../../nvmUnsafe/NVMUnsafe.jar Example
```
