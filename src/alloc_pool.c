/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <Python.h>
#include <inttypes.h>
#include <memory.h>
#include <qpid/dispatch/alloc.h>
#include <qpid/dispatch/atomic.h>
#include <qpid/dispatch/ctools.h>
#include <qpid/dispatch/log.h>
#include <stdio.h>

#include "config.h"
#include "entity.h"
#include "entity_cache.h"

const char *QD_ALLOCATOR_TYPE = "allocator";

typedef struct qd_alloc_type_t          qd_alloc_type_t;
typedef struct qd_alloc_item_t          qd_alloc_item_t;
typedef struct qd_alloc_chunk_t         qd_alloc_chunk_t;
typedef struct qd_alloc_linked_stack_t  qd_alloc_linked_stack_t;

#define STACK_DEPTH   10

struct qd_alloc_item_t {
    uintmax_t             sequence;  // uintmax_t ensures proper alignment of following data
#ifdef QD_MEMORY_DEBUG
    qd_alloc_type_desc_t *desc;
    DEQ_LINKS(qd_alloc_item_t);
    void                 *backtrace[STACK_DEPTH];
    int                   backtrace_size;
    struct timeval        timestamp;
    uint32_t              header;
#endif
};

struct qd_alloc_type_t {
    DEQ_LINKS(qd_alloc_type_t);
    qd_alloc_type_desc_t *desc;
};

DEQ_DECLARE(qd_alloc_type_t, qd_alloc_type_list_t);


//128 has been chosen because many CPUs arch use an
//adjacent line prefetching optimization that load
//2*cache line bytes in batch
#define CHUNK_SIZE 128/sizeof(void*)

struct qd_alloc_chunk_t {
    qd_alloc_chunk_t     *prev;                 //do not use DEQ_LINKS here: field position could affect access cost
    qd_alloc_item_t      *items[CHUNK_SIZE];
    qd_alloc_chunk_t     *next;
};

struct qd_alloc_linked_stack_t {
    //the base
    qd_alloc_chunk_t     *top_chunk;
    uint32_t              top;                  //qd_alloc_item* top_item = top_chunk->items[top+1] <-> top > 0
    uint64_t              size;
    qd_alloc_chunk_t      base_chunk;
};


qd_alloc_config_t qd_alloc_default_config_big   = {16,  32, 0};
qd_alloc_config_t qd_alloc_default_config_small = {64, 128, 0};
#define BIG_THRESHOLD 2000

typedef struct {
    sys_atomic_t allocated;
    sys_atomic_t refcount;
} qd_refcount_t;

void *qd_alloc(qd_alloc_type_desc_t *desc, qd_alloc_pool_t **tpool)
{
    const size_t total_size = sizeof(qd_refcount_t) + desc->type_size + (desc->additional_size ? *desc->additional_size : 0) + desc->trailer;
//    assert(total_size == desc->total_size); // desc->total_size is 0
    qd_refcount_t  * item = malloc(total_size);
    sys_atomic_set(&item->refcount, 1);
    sys_atomic_set(&item->allocated, 1);
    return item+1;
}

bool qd_dealloc(qd_alloc_type_desc_t *desc, qd_alloc_pool_t **tpool, char *p)
{
    qd_refcount_t *item = ((qd_refcount_t*) p) - 1;
    if (sys_atomic_get(&item->refcount) == 1 && sys_atomic_get(&item->allocated)) {
        return true;
    } else {
        sys_atomic_dec(&item->refcount);
        sys_atomic_set(&item->allocated, 0);
        return false;
    }
}

void qd_alloc_incref(void *p) {
    qd_refcount_t *item = ((qd_refcount_t*) p) - 1;
    assert(sys_atomic_get(&item->allocated));
    sys_atomic_inc(&item->refcount);
}

bool qd_alloc_decref(void *p) {
    qd_refcount_t *item = ((qd_refcount_t*) p) - 1;
    sys_atomic_dec(&item->refcount);
    if(sys_atomic_get(&item->refcount) == 0) {
        return true;
    }
    return false;
}

void* qd_alloc_safe_deref(void *p){
    if (p == NULL) return NULL;
    qd_refcount_t *item = ((qd_refcount_t*) p) - 1;
    if(sys_atomic_get(&item->allocated)) {
        return p;
    }
    return NULL;
}


uint32_t qd_alloc_sequence(void *p)
{
    // todo, need this for identity in few places
//    if (!p)
//        return 0;
//
//    qd_alloc_item_t *item = ((qd_alloc_item_t*) p) - 1;
//    return item->sequence;
    return 42;
}

void qd_alloc_free(void *p) {
    qd_refcount_t *item = ((qd_refcount_t*) p) - 1;
    free(item);
}

void qd_alloc_initialize(void)
{
}


void qd_alloc_finalize(void)
{
}


qd_error_t qd_entity_refresh_allocator(qd_entity_t* entity, void *impl) {
    return QD_ERROR_NONE;
}

void qd_alloc_debug_dump(const char *file) {
}
