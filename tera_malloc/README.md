# Tera Allocator for Forwarding Tables

## Build
```sh
make clean
make distclean
make
```

More easily just run (you do not need sudo priveledges):
```sh
./build.sh
```

## Export Enviroment Variables
Add the following enviroment variables in to your ~/.bashrc file.

```sh
export LIBRARY_PATH=/your/path/teraHeap/tera_malloc/lib/:$LIBRARY_PATH                                                                                                 
export LD_LIBRARY_PATH=/your/path/teraHeap/tera_malloc/lib/:$LD_LIBRARY_PATH                                                                                           
export PATH=/your/path/teraheap/tera_malloc/include/:$PATH                                                                                                             
export C_INCLUDE_PATH=/your/path/teraheap/tera_malloc/include/:$C_INCLUDE_PATH                                                                                         
export CPLUS_INCLUDE_PATH=/your/path/teraheap/tera_malloc/include/:$CPLUS_INCLUDE_PATH
```
