#ifndef _UMMAP_POLICY_H
#define _UMMAP_POLICY_H

#include "ummap_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates an evict policy based on the provided policy type (e.g., pLRU).
 */
int umpolicy_create(ummap_ptype_t ptype, ummap_policy_t **policy);

/**
 * Releases an evict policy object.
 */
void umpolicy_release(ummap_policy_t *policy);

#ifdef __cplusplus
}
#endif

#endif

