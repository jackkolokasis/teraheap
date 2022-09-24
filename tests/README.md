# TeraHeap Test Files

## Description
TeraHeap test files are used to test TeraHeap functionalities during
implementation. All these test files are implemented in JAVA. 

## Build
To build and run all test files for TeraHeap:

```sh
cd ./java
./compile.sh
cd -
```
## Run Tests
There are different modes that you can run the TeraHeap tests.

```sh
# Run tests in interpreter mode
./run.sh 1

# Run tests using only C1 JIT compiler
./run.sh 2

# Run tests using only C2 JIT compiler
./run.sh 3

# Run tests using gdb
./run.sh 4

```
