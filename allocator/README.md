# TeraHeap Region-Based Allocator

## Prepare TeraHeap Allocator
Before compiling TeraHeap allocator edit the  following variables in
./include/sharedDefines.h file.
```sh
#define DEV "/mnt/fmap/file.txt"	       //< Device name
#define DEV_SIZE (150*1024LU*1024*1024)  //< Device size (in bytes)
```

## Build
```sh
make clean
make distclean
make
```

## Install and Uninstall Allocator Library
More easily just run (you do not need sudo priveledges):
```sh
./build.sh
```

## Export Enviroment Variables
Add the following enviroment variables in to your ~/.bashrc file.

```sh
export LIBRARY_PATH=/your/path/teraHeap/allocator/lib/:$LIBRARY_PATH                                                                                                 
export LD_LIBRARY_PATH=/your/path/teraHeap/allocator/lib/:$LD_LIBRARY_PATH                                                                                           
export PATH=/your/path/teraheap/allocator/include/:$PATH                                                                                                             
export C_INCLUDE_PATH=/your/path/teraheap/allocator/include/:$C_INCLUDE_PATH                                                                                         
export CPLUS_INCLUDE_PATH=/your/path/teraheap/allocator/include/:$CPLUS_INCLUDE_PATH
```

## Run Tests
Run all the tests using 512MB region size in sharedDefines.h

```sh
make test
```
