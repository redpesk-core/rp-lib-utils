/*
 * Copyright (C) 2015-2024 IoT.bzh Company
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

#pragma once

#include <stdint.h>

struct u16id2ptr;
struct u16id2bool;

/**********************************************************************/
/**        u16id2ptr                                                 **/
/**********************************************************************/

extern int u16id2ptr_create(struct u16id2ptr **pi2p);
extern void u16id2ptr_destroy(struct u16id2ptr **pi2p);
extern void u16id2ptr_dropall(struct u16id2ptr **pi2p);
extern int u16id2ptr_has(struct u16id2ptr *i2p, uint16_t id);
extern int u16id2ptr_add(struct u16id2ptr **pi2p, uint16_t id, void *ptr);
extern int u16id2ptr_set(struct u16id2ptr **pi2p, uint16_t id, void *ptr);
extern int u16id2ptr_put(struct u16id2ptr *i2p, uint16_t id, void *ptr);
extern int u16id2ptr_get(struct u16id2ptr *i2p, uint16_t id, void **ptr);
extern int u16id2ptr_drop(struct u16id2ptr **pi2p, uint16_t id, void **ptr);
extern int u16id2ptr_count(struct u16id2ptr *i2p);
extern int u16id2ptr_at(struct u16id2ptr *i2p, int index, uint16_t *pid, void **pptr);
extern void u16id2ptr_forall(
			struct u16id2ptr *i2p,
			void (*callback)(void*closure, uint16_t id, void *ptr),
			void *closure);

/**********************************************************************/
/**        u16id2bool                                                **/
/**********************************************************************/

/**
 * Creates in @p pi2b an array associating a boolean to a uint16 value
 *
 * @param pi2b pointer to the array to be created
 *
 * @return 0 in case of success or -ENOMEM if allocation failed
 */
extern int u16id2bool_create(struct u16id2bool **pi2b);

/**
 * Destroys the u16id2bool array.
 *
 * Because it alters the array, the pointer to that array has to be given.
 *
 * @param pi2b pointer to the array to be destroyed
 */
extern void u16id2bool_destroy(struct u16id2bool **pi2b);

/**
 * Set all associated booleans to zero.
 *
 * Because it alters the array, the pointer to that array has to be given.
 *
 * @param pi2b pointer to the array to be destroyed
 */
extern void u16id2bool_clearall(struct u16id2bool **pi2b);

/**
 * Get the value associated with the @p id in @i2b.
 * The default value if @p id isn't currently set is 0.
 *
 * Because it doesn't alter the array, the array has to be given.
 *
 * @param pi2b pointer to the array to be destroyed
 * @param id id whose value is get
 *
 * @return the value associated to @p id or 0 if none was set
 */
extern int u16id2bool_get(struct u16id2bool *i2b, uint16_t id);

/**
 * Set within the assiciative array @p pi2b the boolean associtated
 * to @p id to the given @p value.
 *
 * Because it alters the array, the pointer to that array has to be given.
 *
 * @param pi2b pointer to the array to be destroyed
 * @param id id that is set
 * @param value the value to be set (treated as zero or not zero)
 *
 * @return on success, the previous value as 0 or 1
 *         on error, if the value can not be set, the negative value -ENOMEM
 */
extern int u16id2bool_set(struct u16id2bool **pi2b, uint16_t id, int value);
