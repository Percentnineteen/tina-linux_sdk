/*
 * Copyright (C) 2022 XRADIO TECHNOLOGY CO., LTD. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the
 *       distribution.
 *    3. Neither the name of XRADIO TECHNOLOGY CO., LTD. nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef __OS_NET_ATOMIC_H__
#define __OS_NET_ATOMIC_H__

#ifdef OS_NET_LINUX_OS

#ifdef __cplusplus
extern "C" {
#endif

typedef volatile unsigned int atomic_t;
typedef atomic_t atomic_val_t;
typedef volatile unsigned int *atomic_ptr_t;

static inline bool atomic_cas(atomic_t *target, atomic_val_t old_value, atomic_val_t new_value)
{
    return __sync_bool_compare_and_swap(target, old_value, new_value);
}

static inline atomic_val_t atomic_add(atomic_t *target, atomic_val_t value)
{
    return __sync_fetch_and_add(target, value);
    // return __sync_add_and_fetch(target, value);
}

static inline atomic_val_t atomic_sub(atomic_t *target, atomic_val_t value)
{
    return __sync_fetch_and_sub(target, value);
    // return __sync_sub_and_fetch(target, value);
}

static inline atomic_val_t atomic_inc(atomic_t *target)
{
    return atomic_add(target, 1);
}

static inline atomic_val_t atomic_dec(atomic_t *target)
{
    return atomic_sub(target, 1);
}

static inline atomic_val_t atomic_get(atomic_t *target)
{
    return __sync_val_compare_and_swap(target, 0, 0);
}

static inline atomic_val_t atomic_set(atomic_t *target, atomic_val_t value)
{
    return __sync_lock_test_and_set(target, value);
}

static inline atomic_val_t atomic_clear(atomic_t *target)
{
    return atomic_set(target, 0);
}

static inline atomic_val_t atomic_or(atomic_t *target, atomic_val_t value)
{
    return __sync_fetch_and_or(target, value);
}

static inline atomic_val_t atomic_xor(atomic_t *target, atomic_val_t value)
{
    return __sync_fetch_and_xor(target, value);
}

static inline atomic_val_t atomic_and(atomic_t *target, atomic_val_t value)
{
    return __sync_fetch_and_and(target, value);
}

static inline atomic_val_t atomic_nand(atomic_t *target, atomic_val_t value)
{
    return __sync_fetch_and_nand(target, value);
}

#define ATOMIC_INIT(i) (i)
#define ATOMIC_BITS (sizeof(atomic_val_t) * 8)
#define ATOMIC_MASK(bit) (1U << ((uint32_t)(bit) & (ATOMIC_BITS - 1U)))
#define ATOMIC_ELEM(addr, bit) ((addr) + ((bit) / ATOMIC_BITS))

#define ATOMIC_DEFINE(name, num_bits) atomic_t name[1 + ((num_bits)-1) / ATOMIC_BITS]

static inline bool atomic_test_bit(const atomic_t *target, int bit)
{
    atomic_val_t val = atomic_get(ATOMIC_ELEM(target, bit));

    return (1 & (val >> (bit & (ATOMIC_BITS - 1)))) != 0;
}

static inline bool atomic_test_and_clear_bit(atomic_t *target, int bit)
{
    atomic_val_t mask = ATOMIC_MASK(bit);
    atomic_val_t old;

    old = atomic_and(ATOMIC_ELEM(target, bit), ~mask);

    return (old & mask) != 0;
}
static inline bool atomic_test_and_set_bit(atomic_t *target, int bit)
{
    atomic_val_t mask = ATOMIC_MASK(bit);
    atomic_val_t old;

    old = atomic_or(ATOMIC_ELEM(target, bit), mask);

    return (old & mask) != 0;
}

static inline void atomic_clear_bit(atomic_t *target, int bit)
{
    atomic_val_t mask = ATOMIC_MASK(bit);

    (void)atomic_and(ATOMIC_ELEM(target, bit), ~mask);
}

static inline void atomic_set_bit(atomic_t *target, int bit)
{
    atomic_val_t mask = ATOMIC_MASK(bit);

    (void)atomic_or(ATOMIC_ELEM(target, bit), mask);
}
static inline void atomic_set_bit_to(atomic_t *target, int bit, bool val)
{
    atomic_val_t mask = ATOMIC_MASK(bit);

    if (val) {
        (void)atomic_or(ATOMIC_ELEM(target, bit), mask);
    } else {
        (void)atomic_and(ATOMIC_ELEM(target, bit), ~mask);
    }
}

#ifdef __cplusplus
}; /*extern "C"*/
#endif

#endif // OS_NET_LINUX_OS

#endif //__OS_NET_ATOMIC_H__
