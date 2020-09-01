# TeraCache Test Files

## Description
TeraCache test files are used to test TeraCache functionalities during
implementation. All these test files are implemented in JAVA. 

## Build
To build and run all test files for TeraCache:

```
$ make all  
$ make run
```
## List of Tests
1. Array_List
2. Simple_Array
3. List_Small
4. List_Large
5. MultiList
6. Simple_Lambda

## Validation Scripts
There are two validation tests.
1. check_memmove.py: Check if all precompacted objects has been moved to their
   new destination place
2. check_teracache.py: Check if all marked Tera Cache objects has been moved to the
   TeraCache.


## Authors
Iacovos G. Kolokasis,

