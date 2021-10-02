# TeraCache Allocator

## Build
'''
make distclean;
make
'''

## Install and Uninstall Allocator Library
'''
sudo make install    # Install library to /usr/lib
sudo make uninstall
'''
More easily just run (you need to have sudo priveledges):
'''
./build.sh
'''

## Run Tests
Run the tests using 512MB region size in sharedDefines.h

'''
for t in $(ls *.bin); do ./$t; done
'''
