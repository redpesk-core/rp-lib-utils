/*
 * Copyright (C) 2015-2021 IoT.bzh Company
 * Author: José Bollo <jose.bollo@iot.bzh>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


#include <stdint.h>
#include <malloc.h>

#include "u16id.h"
#include "../sys/x-errno.h"

/* compute P, the count of bits of pointers */
#if UINTPTR_MAX == (18446744073709551615UL)
#  define P 64
#elif UINTPTR_MAX == (4294967295U)
#  define P 32
#elif UINTPTR_MAX == (65535U)
#  define P 16
#else
#  error "Unsupported pointer size"
#endif

/* grain of allocation */
#define N 4

/*
 * The u16id maps are made of a single block of memory structured
 * as an array of uint16_t followed by an array of void*. To ensure
 * that void* pointers are correctly aligned, the array of uint16_t
 * at head is a multiple of N items, with N being a multiple of 2
 * if void* is 32 bits or 4 if void* is 64 bits.
 *
 * The first item of the array of uint16_t is used to record the
 * upper index of valid uint16_t ids.
 *
 * +-----+-----+-----+-----+ - - - - - - - - +-----+-----+-----+-----+ - - - - - - - -
 * |upper| id1 | id2 | id3 |                 |         ptr1          |
 * +-----+-----+-----+-----+ - - - - - - - - +-----+-----+-----+-----+ - - - - - - - -
 */

static inline uint16_t get_capacity(uint16_t upper)
{
	/* capacity is the smallest kN-1 such that kN-1 >= upper) */
#if N == 2 || N == 4 || N == 8 || N == 16
	return upper | (N - 1);
#else
#	error "not supported"
#endif
}

/**
 * flattened structure
 */
typedef struct {
	/** current upper index */
	uint16_t upper;

	/** current capacity */
	uint16_t capacity;

	/** pointer to ids */
	uint16_t *ids;

	/** pointer to pointers */
	void **ptrs;
} flat_t;

/**
 * set fields of @p flat accordingly to the @p base pointer
 * of upper value being @p up
 *
 * @param flat structure to initialise
 * @param base base in memory
 * @param up upper value for base
 */
static void flatofup(flat_t *flat, void *base, uint16_t up)
{
	uint16_t cap, *ids;

	flat->upper = up;
	flat->capacity = cap = get_capacity(up);
	flat->ids = ids = base;
	flat->ptrs = ((void**)(&ids[cap + 1])) - 1; /* minus one causes indexes are from 1 */
}

/**
 * get flat for the give base
 *
 * @param flat structure to initialise
 * @param base base in memory
 */
static void flatof(flat_t *flat, void *base)
{
	if (base)
		/* not empty */
		flatofup(flat, base, *(uint16_t*)base);
	else {
		/* empty */
		flat->upper = flat->capacity = 0;
		flat->ids = NULL;
		flat->ptrs = NULL;
	}
}

/**
 * compute the byte size for a given capacity
 */
static inline size_t size(uint16_t capacity)
{
	return sizeof(uint16_t) * (capacity + 1)
		+ sizeof(void*) * capacity;
}

/**
 * search the @p id and returns its not null index if
 * found or 0 if not found
 *
 * @param flat the flat structure
 * @param id id to locate
 *
 * @return 0 if not found or else the index of @p id
 */
static inline uint16_t search(flat_t *flat, uint16_t id)
{
	uint16_t *ids = flat->ids;
	uint16_t *end = &ids[flat->upper];
	while(ids != end)
		if (id == *++ids)
			return (uint16_t)(ids - flat->ids);
	return 0;
}

/**
 * add the id and the pointer in flat
 */
static void *add(flat_t *flat, uint16_t id, void *ptr)
{
	void *grown, *result;
	flat_t oflat;
	uint16_t nupper, oupper;

	oupper = flat->upper;
	nupper = (uint16_t)(oupper + 1);
	result = flat->ids;
	if (nupper > flat->capacity) {
		grown = realloc(result, size(get_capacity(nupper)));
		if (grown == NULL)
			return NULL;
		result = grown;
		flatofup(flat, grown, nupper);
		if (oupper) {
			flatofup(&oflat, grown, oupper);
			while (oupper) {
				flat->ptrs[oupper] = oflat.ptrs[oupper];
				oupper--;
			}
		}
	}
	/* flat->upper = nupper; NOT DONE BECAUSE NOT NEEDED */
	flat->ids[0] = nupper;
	flat->ids[nupper] = id;
	flat->ptrs[nupper] = ptr;
	return result;
}

/**
 * drop the item of index
 */
static void *drop(flat_t *flat, uint16_t index)
{
	void **ptrs, *result;
	uint16_t upper, idx, capa;

	/* remove the upper element */
	upper = flat->upper;
	if (index != upper) {
		flat->ids[index] = flat->ids[upper];
		flat->ptrs[index] = flat->ptrs[upper];
	}
	flat->ids[0] = --upper;

	/* shrink capacity */
	capa = get_capacity(upper);
	result = flat->ids;
	if (capa != flat->capacity) {
		/* shrink pointers */
		ptrs = flat->ptrs;
		flatofup(flat, result, upper);
		idx = 1;
		while(idx <= upper) {
			flat->ptrs[idx] = ptrs[idx];
			idx++;
		}
#if U16ID_ALWAYS_SHRINK
		/* reallocating if shrink */
		result = realloc(flat->ids, size(capa));
		if (result == NULL)
			result = flat->ids;
#endif
	}
	return result;
}

/**
 * drop all items
 */
static void dropall(void **pbase)
{
	void *base;

	base = *pbase;
	if (base)
		*(uint16_t*)base = 0;
}

/**
 * destroy the array
 */
static void destroy(void **pbase)
{
	void *base;

	base = *pbase;
	*pbase = NULL;
	free(base);
}

/**
 * create an empty array
 */
static int create(void **pbase)
{
	void *base;

	*pbase = base = malloc(size(get_capacity(0)));
	if (base == NULL)
		return X_ENOMEM;
	*(uint16_t*)base = 0;
	return 0;
}

/**********************************************************************/
/**        u16id2ptr                                                 **/
/**********************************************************************/

/* create a u16id2ptr */
int u16id2ptr_create(struct u16id2ptr **pi2p)
{
	return create((void**)pi2p);
}

/* destroy a u16id2ptr */
void u16id2ptr_destroy(struct u16id2ptr **pi2p)
{
	destroy((void**)pi2p);
}

/* clear a u16id2ptr */
void u16id2ptr_dropall(struct u16id2ptr **pi2p)
{
	dropall((void**)pi2p);
}

/* check existence of id in u16id2ptr */
int u16id2ptr_has(struct u16id2ptr *i2p, uint16_t id)
{
	flat_t flat;

	flatof(&flat, i2p);
	return search(&flat, id) != 0;
}

/* add the id with value ptr in u16id2ptr */
int u16id2ptr_add(struct u16id2ptr **pi2p, uint16_t id, void *ptr)
{
	struct u16id2ptr *i2p;
	uint16_t index;
	flat_t flat;

	i2p = *pi2p;
	flatof(&flat, i2p);
	index = search(&flat, id);
	if (index)
		return X_EEXIST;
	i2p = add(&flat, id, ptr);
	if (!i2p)
		return X_ENOMEM;
	*pi2p = i2p;
	return 0;
}

/* add or set the id with value ptr in u16id2ptr */
int u16id2ptr_set(struct u16id2ptr **pi2p, uint16_t id, void *ptr)
{
	struct u16id2ptr *i2p;
	uint16_t index;
	flat_t flat;

	i2p = *pi2p;
	flatof(&flat, i2p);
	index = search(&flat, id);
	if (index)
		flat.ptrs[index] = ptr;
	else {
		i2p = add(&flat, id, ptr);
		if (!i2p)
			return X_ENOMEM;
		*pi2p = i2p;
	}
	return 0;
}

/* change the id with value ptr in u16id2ptr */
int u16id2ptr_put(struct u16id2ptr *i2p, uint16_t id, void *ptr)
{
	uint16_t index;
	flat_t flat;

	flatof(&flat, i2p);
	index = search(&flat, id);
	if (!index)
		return X_ENOENT;
	flat.ptrs[index] = ptr;
	return 0;
}

/* get the value of id in u16id2ptr */
int u16id2ptr_get(struct u16id2ptr *i2p, uint16_t id, void **pptr)
{
	uint16_t index;
	flat_t flat;

	flatof(&flat, i2p);
	index = search(&flat, id);
	if (!index)
		return X_ENOENT;
	*pptr = flat.ptrs[index];
	return 0;
}

/* remove the id in u16id2ptr */
int u16id2ptr_drop(struct u16id2ptr **pi2p, uint16_t id, void **pptr)
{
	struct u16id2ptr *i2p;
	uint16_t index;
	flat_t flat;

	i2p = *pi2p;
	flatof(&flat, i2p);
	index = search(&flat, id);
	if (!index)
		return X_ENOENT;
	if (pptr)
		*pptr = flat.ptrs[index];

	i2p = drop(&flat, index);
	if (!i2p)
		return X_ENOMEM;
	*pi2p = i2p;
	return 0;
}

/* count of id in u16id2ptr */
int u16id2ptr_count(struct u16id2ptr *i2p)
{
	return i2p ? ((int)*(uint16_t*)i2p) : 0;
}

/* get pair at index in u16id2ptr */
int u16id2ptr_at(struct u16id2ptr *i2p, int index, uint16_t *pid, void **pptr)
{
	flat_t flat;

	flatof(&flat, i2p);
	if (index < 0 || index >= (int)flat.upper)
		return X_EINVAL;
	*pid = flat.ids[index + 1];
	*pptr = flat.ptrs[index + 1];
	return 0;
}

/* calls the function for all pair of u16id2ptr */
void u16id2ptr_forall(struct u16id2ptr *i2p, void (*callback)(void*closure, uint16_t id, void *ptr), void *closure)
{
	flat_t flat;

	flatof(&flat, i2p);
	while (flat.upper) {
		callback(closure, flat.ids[flat.upper], flat.ptrs[flat.upper]);
		flat.upper--;
	}
}

/**********************************************************************/
/**        u16id2bool                                                **/
/**********************************************************************/

/* make the u16id2bool */
int u16id2bool_create(struct u16id2bool **pi2b)
{
	return create((void**)pi2b);
}

/* destroy the u16id2bool */
void u16id2bool_destroy(struct u16id2bool **pi2b)
{
	destroy((void**)pi2b);
}

/* set all values to 0 */
void u16id2bool_clearall(struct u16id2bool **pi2b)
{
	dropall((void**)pi2b);
}

/* get the value at id */
int u16id2bool_get(struct u16id2bool *i2b, uint16_t id)
{
	uintptr_t mask, field;
	uint16_t index, idm;
	flat_t flat;

	flatof(&flat, i2b);
	idm = (uint16_t)(id & ~(P - 1));
	index = search(&flat, idm);
	if (!index)
		return 0;

	field = (uintptr_t)flat.ptrs[index];
	mask = (uintptr_t)((uintptr_t)1 << (id & (P - 1)));
	return (field & mask) != 0;
}

int u16id2bool_set(struct u16id2bool **pi2b, uint16_t id, int value)
{
	struct u16id2bool *i2b;
	uintptr_t mask, field, ofield;
	uint16_t index, idm;
	flat_t flat;

	i2b = *pi2b;
	flatof(&flat, i2b);
	idm = (uint16_t)(id & ~(P - 1));
	index = search(&flat, idm);
	ofield = index ? (uintptr_t)flat.ptrs[index] : 0;
	mask = (uintptr_t)((uintptr_t)1 << (id & (P - 1)));
	if (value)
		field = ofield | mask;
	else
		field = ofield & ~mask;
	if (field != ofield) {
		if (field) {
			if (index)
				flat.ptrs[index] = (void*)field;
			else {
				i2b = add(&flat, idm, (void*)field);
				if (!i2b)
					return X_ENOMEM;
				*pi2b = i2b;
			}
		} else {
			if (index) {
				i2b = drop(&flat, index);
				if (!i2b)
					return X_ENOMEM;
				*pi2b = i2b;
			}
		}
	}
	return (ofield & mask) != 0;
}
