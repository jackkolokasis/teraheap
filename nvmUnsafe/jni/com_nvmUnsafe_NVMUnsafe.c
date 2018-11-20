/**************************************************
 *
 * file: com_nvmUnsafe_NVMUnsafe.c
 *
 * @Author:   Iacovos G. Kolokasis
 * @Version:  19-10-2018
 * @email:    kolokasis@ics.forth.gr
 *
 * Implementation of NVMUnsafe.java native functions
 *
 ***************************************************
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "com_nvmUnsafe_NVMUnsafe.h"

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


jlong 
Unsafe_field_offset_to_byte_offset(jlong field_offset)
{
    return field_offset;
}


jlong 
Unsafe_field_offset_from_byte_offset(jlong byte_offset)
{
    return byte_offset;
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
Java_com_nvmUnsafe_NVMUnsafe_nvmInitialPool (JNIEnv *env, jobject nvmUnsafe, 
        jstring path, jlong pool_size)
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
Java_com_nvmUnsafe_NVMUnsafe_nvmAllocateMemory (JNIEnv *env, jobject nvmUnsafe, 
        jlong pmp, jlong size)
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
 * @param value     Integer object
 * @param pmp       Start Address persistent memory pool
 * @param offset    Allocation address
 *
 */
JNIEXPORT void JNICALL 
Java_com_nvmUnsafe_NVMUnsafe_putInt (JNIEnv *env, jobject nvmUnsafe, 
        jint value, jlong pmp, jlong offset)
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
Java_com_nvmUnsafe_NVMUnsafe_getInt (JNIEnv *env, jobject nvmUnsafe, 
        jlong pmp, jlong offset)
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
 * @desc Get boolean saved in persistent memory pool
 *
 * @param env       JNI Enviroment interface pointer
 * @param nvmUnsafe Refer to current object itself
 * @param pmp       Start address of the memory pool
 * @param offset    Allocation address
 *
 */
JNIEXPORT jboolean JNICALL 
Java_com_nvmUnsafe_NVMUnsafe_getBoolean (JNIEnv *env, jobject nvmUnsafe, 
        jlong pmp, jlong offset)
{
    void* address;                      /* Memory Pool initial address        */
    void* allocationAddr;               /* Allocation address                 */
    jboolean retrieveValue;             /* Value retrieve from memory         */

    address = addr_from_java(pmp);
    allocationAddr = addr_from_java(offset);

    assert(address != NULL);
    assert(allocationAddr != NULL);

    memcpy(&retrieveValue, PMEM(address, allocationAddr), sizeof(jboolean));

    return retrieveValue;
}

/**
 * @desc Save a Boolean on Persistent Memory
 *
 * @param env       JNI Enviroment interface pointer
 * @param nvmUnsafe Refer to current object itself
 * @param value     Boolean object
 * @param pmp       Start Address persistent memory pool
 * @param offset    Allocation address
 *
 */
JNIEXPORT void JNICALL 
Java_com_nvmUnsafe_NVMUnsafe_putBoolean (JNIEnv *env, jobject nvmUnsafe, 
        jboolean value, jlong pmp, jlong offset)
{
    void *address;                      /* Memory Pool initial address        */
    void *allocationAddr;               /* Allocation address                 */

    address = addr_from_java(pmp);
    allocationAddr = addr_from_java(offset);
    
    assert(address != NULL);
    assert(allocationAddr != NULL);

    memcpy(PMEM(address, allocationAddr), &value, sizeof(jboolean));
}

/**
 * @desc Get byte saved in persistent memory pool
 *
 * @param env       JNI Enviroment interface pointer
 * @param nvmUnsafe Refer to current object itself
 * @param pmp       Start address of the memory pool
 * @param offset    Allocation address
 *
 */
JNIEXPORT jbyte JNICALL 
Java_com_nvmUnsafe_NVMUnsafe_getByte (JNIEnv *env, jobject nvmUnsafe, 
        jlong pmp, jlong offset)
{
    void* address;                      /* Memory Pool initial address        */
    void* allocationAddr;               /* Allocation address                 */
    jbyte retrieveValue;                /* Value retrieve from memory         */

    address = addr_from_java(pmp);
    allocationAddr = addr_from_java(offset);

    assert(address != NULL);
    assert(allocationAddr != NULL);

    memcpy(&retrieveValue, PMEM(address, allocationAddr), sizeof(jbyte));

    return retrieveValue;
}

/**
 * @desc Save a Byte on Persistent Memory
 *
 * @param env       JNI Enviroment interface pointer
 * @param nvmUnsafe Refer to current object itself
 * @param value       Byte object
 * @param pmp       Start Address persistent memory pool
 * @param offset    Allocation address
 *
 */
JNIEXPORT void JNICALL 
Java_com_nvmUnsafe_NVMUnsafe_putByte (JNIEnv *env, jobject nvmUnsafe, 
        jbyte value, jlong pmp, jlong offset)
{
    void *address;                      /* Memory Pool initial address        */
    void *allocationAddr;               /* Allocation address                 */

    address = addr_from_java(pmp);
    allocationAddr = addr_from_java(offset);
    
    assert(address != NULL);
    assert(allocationAddr != NULL);

    memcpy(PMEM(address, allocationAddr), &value, sizeof(jbyte));
}

/**
 * @desc Get short saved in persistent memory pool
 *
 * @param env       JNI Enviroment interface pointer
 * @param nvmUnsafe Refer to current object itself
 * @param pmp       Start address of the memory pool
 * @param offset    Allocation address
 *
 */
JNIEXPORT jshort JNICALL 
Java_com_nvmUnsafe_NVMUnsafe_getShort (JNIEnv *env, jobject nvmUnsafe, 
        jlong pmp, jlong offset)
{
    void* address;                      /* Memory Pool initial address        */
    void* allocationAddr;               /* Allocation address                 */
    jshort retrieveValue;               /* Value retrieve from memory         */

    address = addr_from_java(pmp);
    allocationAddr = addr_from_java(offset);

    assert(address != NULL);
    assert(allocationAddr != NULL);

    memcpy(&retrieveValue, PMEM(address, allocationAddr), sizeof(jshort));

    return retrieveValue;
}

/**
 * @desc Save a short on Persistent Memory
 *
 * @param env       JNI Enviroment interface pointer
 * @param nvmUnsafe Refer to current object itself
 * @param value       Short object
 * @param pmp       Start Address persistent memory pool
 * @param offset    Allocation address
 *
 */
JNIEXPORT void JNICALL 
Java_com_nvmUnsafe_NVMUnsafe_putShort (JNIEnv *env, jobject nvmUnsafe, 
        jshort value, jlong pmp, jlong offset)
{
    void *address;                      /* Memory Pool initial address        */
    void *allocationAddr;               /* Allocation address                 */

    address = addr_from_java(pmp);
    allocationAddr = addr_from_java(offset);
    
    assert(address != NULL);
    assert(allocationAddr != NULL);

    memcpy(PMEM(address, allocationAddr), &value, sizeof(jshort));
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
Java_com_nvmUnsafe_NVMUnsafe_putLong (JNIEnv *env, jobject nvmUnsafe, 
        jlong value, jlong pmp, jlong offset)
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
 * @desc Get a long in persistent memory pool
 *
 * @param env       JNI Enviroment interface pointer
 * @param nvmUnsafe Refer to current object itself
 * @param pmp       Start address of the memory pool
 * @param offset    Allocation address
 *
 */
JNIEXPORT jlong JNICALL 
Java_com_nvmUnsafe_NVMUnsafe_getLong (JNIEnv *env, jobject nvmUnsafe, 
        jlong pmp, jlong offset)
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

/**
 * @desc Get a float from persistent memory pool
 *
 * @param env       JNI Enviroment interface pointer
 * @param nvmUnsafe Refer to current object itself
 * @param pmp       Start address of the memory pool
 * @param offset    Allocation address
 *
 */
JNIEXPORT jfloat JNICALL 
Java_com_nvmUnsafe_NVMUnsafe_getFloat (JNIEnv *env, jobject nvmUnsafe, 
        jlong pmp, jlong offset)
{
    void* address;                      /* Memory Pool initial address        */
    void* allocationAddr;               /* Allocation address                 */
    jfloat retrieveValue;               /* Value retrieve from memory         */

    address = addr_from_java(pmp);
    allocationAddr = addr_from_java(offset);

    assert(address != NULL);
    assert(allocationAddr != NULL);

    memcpy(&retrieveValue, PMEM(address, allocationAddr), sizeof(jfloat));

    return retrieveValue;
}

/**
 * @desc Put a float in persistent memory pool
 *
 * @param env       JNI Enviroment interface pointer
 * @param nvmUnsafe Refer to current object itself
 * @param value     Value for allocation
 * @param pmp       Start address of the memory pool
 * @param offset    Allocation address
 *
 */
JNIEXPORT void JNICALL 
Java_com_nvmUnsafe_NVMUnsafe_putFloat (JNIEnv *env, jobject nvmUnsafe, 
        jfloat value, jlong pmp, jlong offset)
{
    void* address;                      /* Memory Pool initial address        */
    void* allocationAddr;               /* Allocation address                 */

    address = addr_from_java(pmp);
    allocationAddr = addr_from_java(offset);

    assert(address != NULL);
    assert(allocationAddr != NULL);

    memcpy(PMEM(address, allocationAddr), &value, sizeof(jfloat));
}

/**
 * @desc Free memory
 *
 * @param env       JNI Enviroment interface pointer
 * @param nvmUnsafe Refer to current object itself
 * @param pmp       Start address of the memory pool
 * @param offset    Allocation address
 *
 */
JNIEXPORT void JNICALL 
Java_com_nvmUnsafe_NVMUnsafe_nvmFreeMemory (JNIEnv *env, jobject nvmUnsafe, 
        jlong pmp, jlong offset)
{
    void* address;                      /* Memory Pool initial address        */
    void* allocationAddr;               /* Allocation address                 */

    address = addr_from_java(pmp);
    allocationAddr = addr_from_java(offset);

    pmemalloc_free(address, allocationAddr);
}

/**
 * @desc Put a double in persistent memory pool
 *
 * @param env       JNI Enviroment interface pointer
 * @param nvmUnsafe Refer to current object itself
 * @param value     Value for allocation
 * @param pmp       Start address of the memory pool
 * @param offset    Allocation address
 *
 */
JNIEXPORT void JNICALL 
Java_com_nvmUnsafe_NVMUnsafe_putDouble (JNIEnv *env, jobject nvmUnsafe, 
        jdouble value, jlong pmp, jlong offset)
{
    void* address;                      /* Memory Pool initial address        */
    void* allocationAddr;               /* Allocation address                 */

    address = addr_from_java(pmp);
    allocationAddr = addr_from_java(offset);

    assert(address != NULL);
    assert(allocationAddr != NULL);

    memcpy(PMEM(address, allocationAddr), &value, sizeof(jdouble));
}
 
/**
 * @desc Get a double from persistent memory pool
 *
 * @param env       JNI Enviroment interface pointer
 * @param nvmUnsafe Refer to current object itself
 * @param pmp       Start address of the memory pool
 * @param offset    Allocation address
 *
 */
JNIEXPORT jdouble JNICALL 
Java_com_nvmUnsafe_NVMUnsafe_getDouble (JNIEnv *env, jobject nvmUnsafe, 
        jlong pmp, jlong offset)
{
    void* address;                      /* Memory Pool initial address        */
    void* allocationAddr;               /* Allocation address                 */
    jdouble retrieveValue;              /* Value retrieve from memory         */

    address = addr_from_java(pmp);
    allocationAddr = addr_from_java(offset);

    assert(address != NULL);
    assert(allocationAddr != NULL);

    memcpy(&retrieveValue, PMEM(address, allocationAddr), sizeof(jdouble));

    return retrieveValue;
}

/**
 * @desc Given a relative pointer, add in the base associated with the
 * given Persistent Memory Pool (pmp).
 *
 * @param env       JNI Enviroment interface pointer
 * @param nvmUnsafe Refer to current object itself
 * @param pmp       Start address of the memory pool
 * @param offset    Allocation address
 *
 */
JNIEXPORT jlong JNICALL 
Java_com_nvmUnsafe_NVMUnsafe_getPmemAddress (JNIEnv *env, jobject obj, jlong pmp,
        jlong offset)
{
    void* address;                      /* Memory Pool initial address        */
    void* allocationAddr;               /* Allocation address                 */
    
    address = addr_from_java(pmp);
    allocationAddr = addr_from_java(offset);
    
    assert(address != NULL);
    assert(allocationAddr != NULL);

    return addr_to_java(PMEM(address, allocationAddr));
}
