# TeraHeap Test Files

g1_evacuations dir:
    tests for the g1 evacuations (forces minor and major evacuations)
g1_full_gc dir:
    tests for the g1 full gc cycle
parallel_gc dir:
    tests for parallel scavenge major/full gc
                    
## Description
TeraHeap test files are used to test TeraHeap functionalities during
implementation. All these test files are implemented in JAVA. 

## Build
To build and run all test files for TeraHeap:

```sh
Choose what benchmarks you want 

cd g1_evacuations
./compile.sh

    OR

cd g1_full_gc
./compile.sh


```
## How to run the benchmarks

```sh
Usage:
      ./run.sh [option ...] [-h]
Options:
      -p, --point    <mount_point>        The mount point used for the H2 file(eg. /mnt/fmap/)
      -j, --jvm      <jvm_build>          The jvm build([release|r], [fastdebug|f], Default: release)
      -m, --mode     <execution_mode>     The jvm execution mode(0: Default, 1: Interpreter, 2: C1, 3: C2, 4: gdb, 5: ShowMessageBoxOnError)
      -t, --threads  <threads>            The number of GC threads (2, 4, 8, 16, 32)
      -f, --flexheap                      Enable flexheap
      -h  Show usage

Inside the system_gc or evacuations folder do:

# Run tests in interpreter mode
./run.sh 1

# Run tests using only C1 JIT compiler
./run.sh 2

# Run tests using only C2 JIT compiler
./run.sh 3

# Run tests using gdb
./run.sh 4

```
