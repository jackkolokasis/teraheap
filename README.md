## TeraHeap: Reducing Memory Pressure in Managed Big Data Frameworks

### Description

TeraHeap extends the managed runtime (JVM) to use a second,
high-capacity heap over a fast storage device that coexists with the
regular heap. TeraHeap provides direct access to objects on the second
heap (no S/D). It also reduces GC cost by fencing the garbage
collector from scanning the second heap. TeraHeap leverages
frameworks’ property of choosing specific objects for off-heap
placement and offers frameworks a hint-based interface for moving
such objects to the second heap. 

### Install Prerequisites
Install the following packages:
```sh
sudo yum install python3-pip
pip3 install scan-build --user
pip3 install compdb --user
```

### Build
1. Build allocator.
```sh
cd allocator
./build.sh
cd -
```
Read the README.md file in allocator directory to export the specific
environment variables

2. Build JVM (release mode)
```sh
cd jdk8u345
./compile.sh -r
cd -
```

3. Build JVM (fastdebug mode)
```sh
cd jdk8u345
./compile.sh -d
cd -
```

## Benchmarks
To run benchmarks please clone the repository
[tera_applications](https://github.com/jackkolokasis/tera_applications)
and read the README.md files in each application directory. There are
instructions about how to compile and run the different applications.
