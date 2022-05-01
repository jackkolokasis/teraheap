# TeraCache Region-Based Allocator

## Build
'''
make clean
make distclean
make
'''

## Install and Uninstall Allocator Library
Install allocator library to the /usr/lib.
'''
sudo make install
sudo make uninstall
'''
More easily just run (you need to have sudo priveledges):
'''
./build.sh
'''

## Export Enviroment Variables
Add the following enviroment variables in to your ~/.bashrc file.
'''
export LIBRARY_PATH=/your/path/sparkPersistentMemory/allocator/lib/:$LIBRARY_PATH                                                                                                 
export LD_LIBRARY_PATH=/your/path/kolokasis/sparkPersistentMemory/allocator/lib/:$LD_LIBRARY_PATH                                                                                           
export PATH=/your/path/sparkPersistentMemory/allocator/include/:$PATH                                                                                                             
export C_INCLUDE_PATH=/your/path/sparkPersistentMemory/allocator/include/:$C_INCLUDE_PATH                                                                                         
export CPLUS_INCLUDE_PATH=/your/path/sparkPersistentMemory/allocator/include/:$CPLUS_INCLUDE_PATH
'''

## Run Tests
Run all the tests using 512MB region size in sharedDefines.h

'''
make test
'''
