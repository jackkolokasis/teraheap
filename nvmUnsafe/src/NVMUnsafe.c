#include "NVMUnsafe.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "../libpmemmalloc/pmemalloc.h"

/**
 * @desc Inline function - Convert address of a Java Object to C
 *
 * @param addr  Java Object Address
 *
 * @ret Java-style address to C-style
 *
 */
void* addr_from_java(jlong addr) 
{
  return (void*)(uintptr_t)addr;
}

/**
 * @desc Inline function - Convert address from a C type to Java
 *
 * @param addr  C type address
 *
 * @ret C-style address to Java-style
 *
 */
jlong addr_to_java(void* p) 
{
  assert(p == (void*)(uintptr_t)p);
  return (uintptr_t)p;
}

/**
 * @desc Initialize non volatile memory pool
 *
 * @param env       JNI Enviroment interface pointer
 * @param nvmUnsafe Refer to current object itself
 * @param path      NVM file path
 * @param pool_size Total memory pool size
 *
 * @ret Start address of the nvm allocated pool size
 *
 */
JNIEXPORT jlong JNICALL 
Java_NVMUnsafe_nvmInitialPool (JNIEnv *env, jobject nvmUnsafe, jstring path, 
        jlong pool_size)
{
    void *address;                       /* Allocation pool address           */ 
    const char *nativePath;              /* Java string to native string      */

    /* Convert jstring to a C-style string */
    nativePath = (*env)->GetStringUTFChars(env, path, JNI_FALSE);

    assert(nativePath != NULL);
    assert(pool_size > 0);
   
    /* Create a pool into persistent memory for future allocations */
    address = pmemalloc_init(nativePath, pool_size);

    /* Release the string 'nativePath' in order to avoid memory leak */
    (*env)->ReleaseStringUTFChars(env, path, nativePath);

    return addr_to_java(address);
}

/**
 * @desc Allocate Memory for an Object using size
 *
 * @param env       JNI Enviroment interface pointer
 * @param nvmUnsafe Refer to current object itself
 * @param pmp       Start Address persistent memory pool
 * @param size      Object size
 *
 * @ret Allocation address
 *
 */
JNIEXPORT jlong JNICALL 
Java_NVMUnsafe_nvmAllocateMemory (JNIEnv *env, jobject nvmUnsafe, jlong pmp, 
        jlong size)
{
    void* address;                      /* Memory Pool initial address        */
    void* allocationAddr;               /* Allocation address                 */

    /* Convert pmp pointer to C-style pointer */
    address = addr_from_java(pmp);
       
    assert(address);
    assert(size > 0);

    /* Allocate memory */
    allocationAddr = pmemalloc_reserve(address, size);

    return addr_to_java(allocationAddr);
}

/**
 * @desc Save an Integer on Persistent Memory
 *
 * @param env       JNI Enviroment interface pointer
 * @param nvmUnsafe Refer to current object itself
 * @param obj       Integer object
 * @param pmp       Start Address persistent memory pool
 * @param offset    Allocation address
 *
 */
JNIEXPORT void JNICALL 
Java_NVMUnsafe_putInt (JNIEnv *env, jobject nvmUnsafe, jint value, jlong pmp,
        jlong offset)
{
    void *address;                      /* Memory Pool initial address        */
    void *allocationAddr;               /* Allocation address                 */

    address = addr_from_java(pmp);
    allocationAddr = addr_from_java(offset);
    
    assert(address != NULL);
    assert(allocationAddr != NULL);

    memcpy(PMEM(address, allocationAddr), &value, sizeof(jint));
}

/**
 * @desc Get integer saved in persistent memory pool
 *
 * @param env       JNI Enviroment interface pointer
 * @param nvmUnsafe Refer to current object itself
 * @param pmp       Start address of the memory pool
 * @param offset    Allocation address
 *
 */
JNIEXPORT jint JNICALL 
Java_NVMUnsafe_getInt (JNIEnv *env, jobject nvmUnsafe, jlong pmp, jlong offset)
{

    void* address;                      /* Memory Pool initial address        */
    void* allocationAddr;               /* Allocation address                 */
    jint retrieveValue;                 /* Value retrieve from memory         */

    address = addr_from_java(pmp);
    allocationAddr = addr_from_java(offset);

    assert(address != NULL);
    assert(allocationAddr != NULL);

    memcpy(&retrieveValue, PMEM(address, allocationAddr), sizeof(jint));

    return retrieveValue;
}

/**
 * @desc Put a long in persistent memory pool
 *
 * @param env       JNI Enviroment interface pointer
 * @param nvmUnsafe Refer to current object itself
 * @param value     Value for allocation
 * @param pmp       Start address of the memory pool
 * @param offset    Allocation address
 *
 */
JNIEXPORT void JNICALL 
Java_NVMUnsafe_putLong (JNIEnv *env, jobject nvmUnsafe, jlong value, jlong pmp,
        jlong offset)
{
    void* address;                      /* Memory Pool initial address        */
    void* allocationAddr;               /* Allocation address                 */

    address = addr_from_java(pmp);
    allocationAddr = addr_from_java(offset);

    assert(address != NULL);
    assert(allocationAddr != NULL);

    memcpy(PMEM(address, allocationAddr), &value, sizeof(jlong));

}

/**
 * @desc Put a long in persistent memory pool
 *
 * @param env       JNI Enviroment interface pointer
 * @param nvmUnsafe Refer to current object itself
 * @param value     Value for allocation
 * @param pmp       Start address of the memory pool
 * @param offset    Allocation address
 *
 * @ret Retrieved value (long)
 *
 */
JNIEXPORT jlong JNICALL 
Java_NVMUnsafe_getLong (JNIEnv *env, jobject nvmUnsafe, jlong pmp, jlong offset)
{

    void* address;                      /* Memory Pool initial address        */
    void* allocationAddr;               /* Allocation address                 */
    jlong retrieveValue;                /* Value retrieve from memory         */

    address = addr_from_java(pmp);
    allocationAddr = addr_from_java(offset);

    assert(address != NULL);
    assert(allocationAddr != NULL);

    memcpy(&retrieveValue, PMEM(address, allocationAddr), sizeof(jlong));

    return retrieveValue;
}


//JNIEXPORT void JNICALL 
//Java_NVMUnsafe_setObject (JNIEnv *env, jobject nvmUnsafe, jobject obj, 
//        jlong pmp, jlong offset)
//{
//
//    void* address;                      /* Memory Pool initial address        */
//    void* allocationAddr;               /* Allocation address                 */
//
//    assert(address != NULL);
//    assert(allocationAddr != NULL);
//
//    jclass cls = (*env)->GetObjectClass(env, obj);
//    memcpy(
//}
