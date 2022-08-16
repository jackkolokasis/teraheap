
#include "common.h"
#include "ummap_policy.h"

// Create the implementation of the functions associated with the policy list
CREATE_LIST_FN(seg, ummap_seg_t);

static void empty_modify(list_seg_t *l, ummap_seg_t *s, pf_type_t t) { }

static void fifo_notify(list_seg_t *list, ummap_seg_t *seg, pf_type_t pf_type)
{
    if (!contains_seg(list, seg))
    {
        push_front_seg(list, seg);
    }
}

static void plru_notify(list_seg_t *list, ummap_seg_t *seg, pf_type_t pf_type)
{
    if (seg != list->front)
    {
        if (contains_seg(list, seg))
        {
            pop_elem_seg(list, seg);
        }
        
        push_front_seg(list, seg);
    }
}

static void wiro_f_notify(list_seg_t *list, ummap_seg_t *seg, pf_type_t pf_type)
{
    if (contains_seg(list, seg))
    {
        pop_elem_seg(list, seg);
    }
    
    if (pf_type == PAGEFAULT_READ)
    {
        push_middle_seg(list, seg);
    }
    else
    {
        push_front_seg(list, seg);
    }
    
    // <<<<<<<<<<<<<< The modify method can cause problems if the I/O thread
    //                and the main process are modifying the list.
}

static void wiro_l_notify(list_seg_t *list, ummap_seg_t *seg, pf_type_t pf_type)
{
    if (contains_seg(list, seg))
    {
        pop_elem_seg(list, seg);
    }
    
    if (pf_type == PAGEFAULT_READ)
    {
        push_front_seg(list, seg);
    }
    else
    {
        push_middle_seg(list, seg);
    }
    
    // <<<<<<<<<<<<<< The modify method can cause problems if the I/O thread
    //                and the main process are modifying the list.
}

static ummap_seg_t *prnd_evict(list_seg_t *list)
{
    static uint32_t seed = 921;
    ummap_seg_t     *seg = list->front;
    
    for (off_t count = (rand_r(&seed) % list->count); count > 0; count--)
    {
        seg = seg->next;
    }
    
    return pop_elem_seg(list, seg);
}

int umpolicy_create(ummap_ptype_t ptype, ummap_policy_t **policy) __CHK_FN__
{
    // Ensure that the requested policy exists
    CHKB((ptype < UMMAP_PTYPE_FIFO || ptype > UMMAP_PTYPE_WIRO_L), EINVAL);
    
    // Create the policy and set the functions based on the type
    *policy = (ummap_policy_t *)calloc(1, sizeof(ummap_policy_t));
    
    switch (ptype)
    {
        case UMMAP_PTYPE_FIFO:
            {
                (*policy)->notify = fifo_notify;
                (*policy)->modify = empty_modify;
                (*policy)->evict  = pop_back_seg;
            } break;
        case UMMAP_PTYPE_LIFO:
            {
                (*policy)->notify = fifo_notify;
                (*policy)->modify = empty_modify;
                (*policy)->evict  = pop_front_seg;
            } break;
        case UMMAP_PTYPE_PLRU:
            {
                (*policy)->notify = plru_notify;
                (*policy)->modify = empty_modify;
                (*policy)->evict  = pop_back_seg;
            } break;
        case UMMAP_PTYPE_PRND:
            {
                (*policy)->notify = fifo_notify;
                (*policy)->modify = empty_modify;
                (*policy)->evict  = prnd_evict;
            } break;
        case UMMAP_PTYPE_WIRO_F:
            {
                (*policy)->notify = wiro_f_notify;
                (*policy)->modify = wiro_f_notify;
                (*policy)->evict  = pop_back_seg;
            } break;
        case UMMAP_PTYPE_WIRO_L:
            {
                (*policy)->notify = wiro_l_notify;
                (*policy)->modify = wiro_l_notify;
                (*policy)->evict  = pop_front_seg;
            } break;
    }
    
    return CHK_SUCCESS(CHK_EMPTY_ERROR_FN);
}

void umpolicy_release(ummap_policy_t *policy)
{
    // Remove all the dependencies from the segments and release the policy
    if (policy != NULL)
    {
        clear_seg_list(&policy->list);
        free(policy);
    }
}

