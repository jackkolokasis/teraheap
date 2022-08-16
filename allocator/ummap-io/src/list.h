#ifndef _LIST_H
#define _LIST_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Macro that declares the pointer structure required to support a linked-list.
 */
#define LIST_PTR_STRUCT(T) \
    struct T *prev; \
    struct T *next

/**
 * Macro that allows to declare a linked-list of certain type, including the
 * corresponding header.
 */
#define DECLARE_LIST(S,T,...) \
    typedef struct \
    { \
        T      *front; \
        T      *middle; \
        T      *back; \
        size_t count; \
    } list_##S##_t; \
    \
    __VA_ARGS__ void push_front_##S(list_##S##_t *header, T *elem); \
    __VA_ARGS__ void push_middle_##S(list_##S##_t *header, T *elem); \
    __VA_ARGS__ void push_back_##S(list_##S##_t *header, T *elem); \
    __VA_ARGS__ T *pop_elem_##S(list_##S##_t *header, T *elem); \
    __VA_ARGS__ T *pop_front_##S(list_##S##_t *header); \
    __VA_ARGS__ T *pop_back_##S(list_##S##_t *header); \
    __VA_ARGS__ int8_t contains_##S(list_##S##_t *header, T *elem); \
    __VA_ARGS__ int8_t is_empty_##S(list_##S##_t *header); \
    __VA_ARGS__ void clear_##S##_list(list_##S##_t *header)

/**
 * Helper macro that initializes a linked-list if it was empty.
 */
#define LIST_PUSH_FN_COMMON(header, elem) \
    header->count++; \
    \
    if (header->front == NULL) \
    { \
        elem->prev     = NULL; \
        elem->next     = NULL; \
        header->front  = elem; \
        header->back   = elem; \
        return; \
    }

/**
 * Macro that defines the function implementations for a linked-list of a given
 * certain type.
 */
#define CREATE_LIST_FN(S,T,...) \
    __VA_ARGS__ void push_front_##S(list_##S##_t *header, T *elem) \
    { \
        LIST_PUSH_FN_COMMON(header, elem); \
        \
        elem->prev          = NULL; \
        elem->next          = header->front; \
        header->front->prev = elem; \
        header->front       = elem; \
    } \
    \
    __VA_ARGS__ void push_middle_##S(list_##S##_t *header, T *elem) \
    { \
        if (header->middle == NULL) \
        { \
            header->middle = elem; \
            \
            push_back_##S(header, elem); \
        } \
        else \
        { \
            header->count++; \
            \
            elem->prev           = header->middle->prev; \
            elem->next           = header->middle; \
            header->middle->prev = elem; \
            header->middle       = elem; \
            \
            if (elem->next == header->front) \
            { \
                header->front    = elem; \
            } \
            else \
            {  \
                elem->prev->next = elem; \
            } \
        } \
    } \
    \
    __VA_ARGS__ void push_back_##S(list_##S##_t *header, T *elem) \
    { \
        LIST_PUSH_FN_COMMON(header, elem); \
        \
        elem->prev         = header->back; \
        elem->next         = NULL; \
        header->back->next = elem; \
        header->back       = elem; \
    } \
    \
    __VA_ARGS__ T *pop_elem_##S(list_##S##_t *header, T *elem) \
    { \
        header->count--; \
        \
        if (elem->prev == NULL && elem->next == NULL) \
        { \
            header->front  = NULL; \
            header->middle = NULL; \
            header->back   = NULL; \
        } \
        else if (elem->prev == NULL) \
        { \
            elem->next->prev = NULL; \
            header->front    = elem->next; \
            header->middle   = (header->middle == elem) ? elem->next : \
                                                          header->middle; \
        } \
        else if (elem->next == NULL) \
        { \
            elem->prev->next = NULL; \
            header->back     = elem->prev; \
            header->middle   = (header->middle == elem) ? NULL : \
                                                          header->middle; \
        } \
        else \
        { \
            elem->prev->next = elem->next; \
            elem->next->prev = elem->prev; \
            header->middle   = (header->middle == elem) ? elem->next : \
                                                          header->middle; \
        } \
        \
        elem->prev = NULL; \
        elem->next = NULL; \
        \
        return elem; \
    } \
    \
    __VA_ARGS__ T *pop_front_##S(list_##S##_t *header) \
    { \
        return (header->front) ? pop_elem_##S(header, header->front) : NULL; \
    } \
    \
    __VA_ARGS__ T *pop_back_##S(list_##S##_t *header) \
    { \
        return (header->back) ? pop_elem_##S(header, header->back) : NULL; \
    } \
    \
    __VA_ARGS__ int8_t contains_##S(list_##S##_t *header, T *elem) \
    { \
        return (elem->prev != NULL || elem->next != NULL || \
                elem == header->front); \
    } \
    \
    __VA_ARGS__ int8_t is_empty_##S(list_##S##_t *header) \
    { \
        return (header->front == NULL); \
    } \
    \
    __VA_ARGS__ void clear_##S##_list(list_##S##_t *header) \
    { \
        while (pop_front_##S(header) != NULL); \
    } \

#define CREATE_LIST(S,T,SN,...) \
    DECLARE_LIST(S,T,__VA_ARGS__); \
    CREATE_LIST_FN(S,T,__VA_ARGS__); \
    static list_##S##_t SN##_list = { 0 }

#ifdef __cplusplus
}
#endif

#endif

