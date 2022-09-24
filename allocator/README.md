# TeraCache Region-Based Allocator

## Prepare TeraHeap Allocator
Before you

## Build
'''
make clean
make distclean
make
'''

## Install and Uninstall Allocator Library
More easily just run (you need do not need sudo priveledges):
'''
./build.sh
'''

## Export Enviroment Variables
Add the following enviroment variables in to your ~/.bashrc file.
'''
export LIBRARY_PATH=/your/path/teraHeap/allocator/lib/:$LIBRARY_PATH                                                                                                 
export LD_LIBRARY_PATH=/your/path/teraHeap/allocator/lib/:$LD_LIBRARY_PATH                                                                                           
export PATH=/your/path/teraheap/allocator/include/:$PATH                                                                                                             
export C_INCLUDE_PATH=/your/path/teraheap/allocator/include/:$C_INCLUDE_PATH                                                                                         
export CPLUS_INCLUDE_PATH=/your/path/teraheap/allocator/include/:$CPLUS_INCLUDE_PATH
'''

## Run Tests
Run all the tests using 512MB region size in sharedDefines.h

'''
make test
'''
