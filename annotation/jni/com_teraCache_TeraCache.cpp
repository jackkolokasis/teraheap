#include "com_teraCache_TeraCache.h"
#include <jni.h>
#include <iostream>

//JNIEXPORT jstring 
//JNICALL Java_com_teraCache_TeraCache_test (JNIEnv *env, jobject teraCache, jstring str)
//{
//    jclass sringClass;
//    jmethodID cid;
//    jstring result;
//
//    // Find class
//    stringClass = (*env)->FindClass(env, "java/lang/String");
//    if (stringClass == NULL) {
//        return NULL; /* exception trhown */
//    }
//
//    // Get the method ID for the String(char[]) constructor */
//    // To obtain method id of a constructor pass "<init>" as method
//    // name
//    cid = (*env)->GetMethodID(env, stringClass, "<init>", "()");
//
//    if (cid == NULL) {
//        return NULL; /* exception trhown */
//    }
//
//    result = (*env)->NewObject(env, stingClass, cid, obj);
//
//    // Free local references
//    (*env)->DeleteLocalRef(env, stringClass);
//
//    return result;
//}

JNIEXPORT void 
JNICALL Java_com_teraCache_TeraCache_sayHello (JNIEnv *env, jobject jobj)
{
    std::cout << "Hello World" << std::endl;
}

JNIEXPORT jobject 
JNICALL Java_com_teraCache_TeraCache_test_1class (JNIEnv *env, jobject obj, jstring name, jstring sur)
{
    jclass my_class;
    jmethodID cid;
    jobject result = NULL;

    // Find class
    my_class = env->FindClass("Person");
    if (my_class == NULL) {
        std::cout << "ERROR" << std::endl;
        return NULL; /* exception trhown */
    }

    cid = env->GetMethodID(my_class, "<init>", "(Ljava/lang/String;Ljava/lang/String;)V");
    if (cid == NULL) {
        std::cout << "ERROR" << std::endl;
        return NULL; /* exception trhown */
    }

    result = env->NewObject(my_class, cid, name, sur);

    if (result == NULL) {
        std::cout << "ERROR" << std::endl;
        return NULL; /* exception trhown */
    }

    return result;
}

JNIEXPORT jobject 
JNICALL Java_com_teraCache_TeraCache_cache (JNIEnv *env, jobject obj, jobject value, jlong size, jobject cTag)
{
    jclass my_class;
    jmethodID cid;
    jobject result = NULL;

    // Find class
    my_class = env->FindClass("org/apache/spark/storage/memory/DeserializedMemoryEntry");
    if (my_class == NULL) {
        std::cout << "ERROR" << std::endl;
        return NULL; /* exception trhown */
    }

    cid = env->GetMethodID(my_class, "<init>", "(Ljava/lang/Object;JLscala/reflect/ClassTag;)V");
    if (cid == NULL) {
        std::cout << "ERROR" << std::endl;
        return NULL; /* exception trhown */
    }
    
    std::cout << "Cache Objects" << std::endl;
    result = env->NewObject(my_class, cid, value, size, cTag);

    if (result == NULL) {
        std::cout << "ERROR" << std::endl;
        return NULL; /* exception trhown */
    }
    
    return result;

}

