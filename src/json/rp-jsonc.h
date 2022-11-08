/*
 Copyright (C) 2015-2022 IoT.bzh Company

 Author: José Bollo <jose.bollo@iot.bzh>

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is furnished
 to do so, subject to the following conditions:

 The above copyright notice and this permission notice (including the next
 paragraph) shall be included in all copies or substantial portions of the
 Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
*/

#pragma once

#ifdef __cplusplus
    extern "C" {
#endif

#include <stdarg.h>
#include <json-c/json.h>

/**
 * The type rp_jsonc_index_t helps to have a single code
 * compliant with versions lower  or greater than 0.13
 *
 * For version lower than 0.13, indexes and sizes are integers
 *
 * From 0.13 it is size_t
 */
#if JSON_C_VERSION_NUM >= 0x000d00
typedef size_t rp_jsonc_index_t;
#else
typedef int rp_jsonc_index_t;
#endif

/**
 * Definition of error codes returned by wrap/unwrap/check functions
 */
enum rp_jsonc_error_codes {
	rp_jsonc_error_none,
	rp_jsonc_error_null_object,
	rp_jsonc_error_truncated,
	rp_jsonc_error_internal_error,
	rp_jsonc_error_out_of_memory,
	rp_jsonc_error_invalid_character,
	rp_jsonc_error_too_long,
	rp_jsonc_error_too_deep,
	rp_jsonc_error_null_spec,
	rp_jsonc_error_null_key,
	rp_jsonc_error_null_string,
	rp_jsonc_error_out_of_range,
	rp_jsonc_error_dictionary_incomplete,
	rp_jsonc_error_missfit_type,
	rp_jsonc_error_key_not_found,
	rp_jsonc_error_bad_base64,
	rp_jsonc_error_array_incomplete,
	rp_jsonc_error_array_extra_field,
	_rp_jsonc_error_count_
};

/**
 * Extract the position from the given error 'rc'.
 * The error is one returned by the function rp_jsonc_vpack,
 * rp_jsonc_pack, rp_jsonc_vunpack, rp_jsonc_unpack, rp_jsonc_unpack,
 * rp_jsonc_check, rp_jsonc_vmatch, rp_jsonc_match and is the
 * position in the description.
 *
 * @param rc     a returned error
 *
 * @return the position of the error
 */
extern int rp_jsonc_get_error_position(int rc);

/**
 * Extract the error code from the given error 'rc'.
 *
 * @param rc     a returned error
 *
 * @return the code of the error
 *
 * @see rp_jsonc_error_codes
 */
extern int rp_jsonc_get_error_code(int rc);

/**
 * Extract the error string from the given error 'rc'.
 * The sting is the human representation of the error code.
 *
 * @param rc     a returned error
 *
 * @return the string of the error
 */
extern const char *rp_jsonc_get_error_string(int rc);

/**
 * Creates an object from the given description and the variable parameters.
 *
 * @param result   address where to store the result
 * @param desc     description of the pack to do
 * @param args     the arguments of the description
 *
 * @return 0 in case of success and result is filled or a negative error code
 * and result is set to NULL.
 *
 * @see rp_jsonc_get_error_position
 * @see rp_jsonc_get_error_code
 * @see rp_jsonc_get_error_string
 * @see rp_jsonc_pack
 */
extern int rp_jsonc_vpack(struct json_object **result, const char *desc, va_list args);

/**
 * Creates an object from the given description and the variable parameters.
 *
 * @param result   address where to store the result
 * @param desc     description of the pack to do
 * @param ...      the arguments of the description
 *
 * @return 0 in case of success and result is filled or a negative error code
 * and result is set to NULL.
 *
 * @see rp_jsonc_get_error_position
 * @see rp_jsonc_get_error_code
 * @see rp_jsonc_get_error_string
 * @see rp_jsonc_vpack
 */
extern int rp_jsonc_pack(struct json_object **result, const char *desc, ...);

/**
 * Scan an object according to its description and unpack the data it
 * contains following the description.
 *
 * @param object   object to scan
 * @param desc     description of the pack to do
 * @param args     the arguments of the description
 *
 * @return 0 in case of success or a negative error code
 *
 * @see rp_jsonc_get_error_position
 * @see rp_jsonc_get_error_code
 * @see rp_jsonc_get_error_string
 * @see rp_jsonc_unpack
 */
extern int rp_jsonc_vunpack(struct json_object *object, const char *desc, va_list args);

/**
 * Scan an object according to its description and unpack the data it
 * contains following the description.
 *
 * @param object   object to scan
 * @param desc     description of the pack to do
 * @param ...      the arguments of the description
 *
 * @return 0 in case of success or a negative error code
 *
 * @see rp_jsonc_get_error_position
 * @see rp_jsonc_get_error_code
 * @see rp_jsonc_get_error_string
 * @see rp_jsonc_vunpack
 */
extern int rp_jsonc_unpack(struct json_object *object, const char *desc, ...);

/**
 * Scan an object according to its description and return a status code.
 *
 * @param object   object to scan
 * @param desc     description of the pack to do
 * @param args     the arguments of the description
 *
 * @return 0 in case of the object matches the description or a negative error code
 *
 * @see rp_jsonc_get_error_position
 * @see rp_jsonc_get_error_code
 * @see rp_jsonc_get_error_string
 * @see rp_jsonc_vunpack
 * @see rp_jsonc_check
 */
extern int rp_jsonc_vcheck(struct json_object *object, const char *desc, va_list args);

/**
 * Scan an object according to its description and return a status code.
 *
 * @param object   object to scan
 * @param desc     description of the pack to do
 * @param ...      the arguments of the description
 *
 * @return 0 in case of the object matches the description or a negative error code
 *
 * @see rp_jsonc_get_error_position
 * @see rp_jsonc_get_error_code
 * @see rp_jsonc_get_error_string
 * @see rp_jsonc_vunpack
 * @see rp_jsonc_vcheck
 */
extern int rp_jsonc_check(struct json_object *object, const char *desc, ...);

/**
 * Test if an object matches its description.
 *
 * @param object   object to scan
 * @param desc     description of the pack to do
 * @param args     the arguments of the description
 *
 * @return 1 if it matches or 0 if not
 *
 * @see rp_jsonc_get_error_position
 * @see rp_jsonc_get_error_code
 * @see rp_jsonc_get_error_string
 * @see rp_jsonc_vunpack
 * @see rp_jsonc_match
 */
extern int rp_jsonc_vmatch(struct json_object *object, const char *desc, va_list args);

/**
 * Test if an object matches its description.
 *
 * @param object   object to scan
 * @param desc     description of the pack to do
 * @param ...      the arguments of the description
 *
 * @return 1 if it matches or 0 if not
 *
 * @see rp_jsonc_get_error_position
 * @see rp_jsonc_get_error_code
 * @see rp_jsonc_get_error_string
 * @see rp_jsonc_unpack
 * @see rp_jsonc_match
 */
extern int rp_jsonc_match(struct json_object *object, const char *desc, ...);

/**
 * Calls the callback for each item of an array, in order. If the object is not
 * an array, the callback is called for the object itself.
 *
 * The given callback receives 2 arguments:
 *  1. the closure
 *  2. the item
 *
 * @param object   the item to iterate
 * @param callback the callback to call
 * @param closure  the closure for the callback
 */
extern void rp_jsonc_optarray_for_all(struct json_object *object, void (*callback)(void*,struct json_object*), void *closure);

/**
 * Calls the callback for each item of an array, in order. If the object is not
 * an array, nothing is done, the callback is not called.
 *
 * The given callback receives 2 arguments:
 *  1. the closure
 *  2. the item
 *
 * @param object   the item to iterate
 * @param callback the callback to call
 * @param closure  the closure for the callback
 */
extern void rp_jsonc_array_for_all(struct json_object *object, void (*callback)(void*,struct json_object*), void *closure);

/**
 * Calls the callback for each item of an object. If the object is not
 * an object (a dictionnary), nothing is done, the callback is not called.
 *
 * The given callback receives 3 arguments:
 *  1. the closure
 *  2. the item
 *  3. the name of the item
 *
 * @param object   the item to iterate
 * @param callback the callback to call
 * @param closure  the closure for the callback
 */
extern void rp_jsonc_object_for_all(struct json_object *object, void (*callback)(void*,struct json_object*,const char*), void *closure);

/**
 * Calls the callback for each item of an object. If the object is not
 * an object (a dictionnary), the call back is called without name
 * for the object itself.
 *
 * The given callback receives 3 arguments:
 *  1. the closure
 *  2. the item
 *  3. the name of the item it its parent object or NULL
 *
 * @param object   the item to iterate
 * @param callback the callback to call
 * @param closure  the closure for the callback
 */
extern void rp_jsonc_optobject_for_all(struct json_object *object, void (*callback)(void*,struct json_object*,const char*), void *closure);

/**
 * Calls the callback for each item of object. Each item depends on
 * the nature of the object:
 *  - object: each element of the object with its name
 *  - array: each item of the array, in order, without name
 *  - other: the item without name
 *
 * The given callback receives 3 arguments:
 *  1. the closure
 *  2. the item
 *  3. the name of the item it its parent object or NULL
 *
 * @param object   the item to iterate
 * @param callback the callback to call
 * @param closure  the closure for the callback
 */
extern void rp_jsonc_for_all(struct json_object *object, void (*callback)(void*,struct json_object*,const char*), void *closure);

/**
 * Calls the callback for each item of an array, in order, until the callback
 * returns a not null value.
 *
 * If the object is not an array, the callback is called for the object itself.
 *
 * The given callback receives 2 arguments:
 *  1. the closure
 *  2. the item
 *
 * @param object   the item to iterate
 * @param callback the callback to call
 * @param closure  the closure for the callback
 *
 * @return the last not null value returned or zero
 */
extern int rp_jsonc_optarray_until(struct json_object *object, int (*callback)(void*,struct json_object*), void *closure);

/**
 * Calls the callback for each item of an array, in order, until the callback
 * returns a not null value.
 *
 * If the object is not an array, nothing is done, the callback is not called.
 *
 * The given callback receives 2 arguments:
 *  1. the closure
 *  2. the item
 *
 * @param object   the item to iterate
 * @param callback the callback to call
 * @param closure  the closure for the callback
 *
 * @return the last not null value returned or zero
 */
extern int rp_jsonc_array_until(struct json_object *object, int (*callback)(void*,struct json_object*), void *closure);

/**
 * Calls the callback for each item of an object until the callback
 * returns a not null value.
 *
 * If the object is not a dictionary, nothing is done, the callback is not called.
 *
 * The given callback receives 3 arguments:
 *  1. the closure
 *  2. the item
 *  3. the name of the item
 *
 * @param object   the item to iterate
 * @param callback the callback to call
 * @param closure  the closure for the callback
 *
 * @return the last not null value returned or zero
 */
extern int rp_jsonc_object_until(struct json_object *object, int (*callback)(void*,struct json_object*,const char*), void *closure);

/**
 * Calls the callback for each item of an object until the callback
 * returns a not null value.
 *
 * If the object is not a dictionary, the callback is called for the object itself
 * with a NULL name.
 *
 * The given callback receives 3 arguments:
 *  1. the closure
 *  2. the item
 *  3. the name of the item it its parent object or NULL
 *
 * @param object   the item to iterate
 * @param callback the callback to call
 * @param closure  the closure for the callback
 *
 * @return the last not null value returned or zero
 */
extern int rp_jsonc_optobject_until(struct json_object *object, int (*callback)(void*,struct json_object*,const char*), void *closure);

/**
 * Calls the callback for each item of object. Each item depends on
 * the nature of the object:
 *  - object: each element of the object with its name
 *  - array: each item of the array, in order, without name
 *  - other: the item without name
 *
 * The given callback receives 3 arguments:
 *  1. the closure
 *  2. the item
 *  3. the name of the item it its parent object or NULL
 *
 * @param object   the item to iterate
 * @param callback the callback to call
 * @param closure  the closure for the callback
 *
 * @return the last not null value returned or zero
 */
extern int rp_jsonc_until(struct json_object *object, int (*callback)(void*,struct json_object*,const char*), void *closure);

/**
 * Clones the 'object': returns a copy of it. But doesn't clones
 * the content. Synonym of rp_jsonc_clone_depth(object, 1).
 *
 * Be aware that this implementation doesn't clones content that is deeper
 * than 1 but it does link these contents to the original object and
 * increments their use count. So, everything deeper that 1 is still available.
 *
 * @param object the object to clone
 *
 * @return a copy of the object.
 *
 * @see rp_jsonc_clone_depth
 * @see rp_jsonc_clone_deep
 */
extern struct json_object *rp_jsonc_clone(struct json_object *object);

/**
 * Clones the 'object': returns a copy of it. Also clones all
 * the content recursively. Synonym of rp_jsonc_clone_depth(object, INT_MAX).
 *
 * @param object the object to clone
 *
 * @return a copy of the object.
 *
 * @see rp_jsonc_clone_depth
 * @see rp_jsonc_clone
 */
extern struct json_object *rp_jsonc_clone_deep(struct json_object *object);

/**
 * Clones any json 'item' for the depth 'depth'. The item is duplicated
 * and if 'depth' is not zero, its contents is recursively cloned with
 * the depth 'depth' - 1.
 *
 * Be aware that this implementation doesn't copies the primitive json
 * items (numbers, nulls, booleans, strings) but instead increments their
 * use count. This can cause issues with newer versions of libjson-c that
 * now unfortunately allows to change their values.
 *
 * @param item the item to clone. Can be of any kind.
 * @param depth the depth to use when cloning composites: object or arrays.
 *
 * @return the cloned array.
 *
 * @see rp_jsonc_clone
 * @see rp_jsonc_clone_deep
 */
extern struct json_object *rp_jsonc_clone_depth(struct json_object *object, int depth);

/**
 * Adds the items of the object 'added' to the object 'dest', replacing existing one
 * (but @see rp_jsonc_object_merge).
 *
 * @param dest the object to complete this object is modified
 * @param added the object containing fields to add
 *
 * @return the destination object 'dest'
 *
 * @example rp_jsonc_object_add({"a":"a"},{"X":"X"}) -> {"a":"a","X":"X"}
 * @example rp_jsonc_object_add({"a":"a"},{"a":"X"}) -> {"a":"X"}
 */
extern struct json_object *rp_jsonc_object_add(struct json_object *dest, struct json_object *added);

/**
 * Insert content of the array 'added' at index 'idx' of 'dest' array.
 *
 * @param dest the array to complete, this array is modified
 * @param added the array containing content to add
 * @param idx the index where the 'added' array content will be inserted into
 * 'dest' array. Negative values indicates location based on the end, -1 means
 * "at the end, after the last element", -2 means "just before the last element",
 * -X means "just before the X-1 th element"
 *
 * @return the destination array 'dest'
 *
 * @example rp_jsonc_array_insert(["a","b",5],["X","Y",0.92], 1) -> ["a","X","Y",0.92,"b",5]
 */
extern struct json_object *rp_jsonc_array_insert_array(struct json_object *dest, struct json_object *added, int idx);

/**
 * Possible options for rp_jsonc_object_merge
 */
enum rp_jsonc_merge_option {
	/** keep original value */
	rp_jsonc_merge_option_keep = 0,

	/** replace original value */
	rp_jsonc_merge_option_replace = 1,

	/** join values but if not possible keep original value */
	rp_jsonc_merge_option_join_or_keep = 2,

	/** join values but if not possible replace original value */
	rp_jsonc_merge_option_join_or_replace = 3
};

/**
 * Merges the items of the object 'merged' to the object 'dest'.
 * If a field already exist, the option treats how to process.
 *
 * When option == rp_jsonc_merge_option_keep:
 *
 *    The original value of the field is kept
 *
 * When option == rp_jsonc_merge_option_replace:
 *
 *    The merged value replaces the original value
 *
 * When option == rp_jsonc_merge_option_join_or_keep:
 *
 *    If the values can be joined, then the resul is the result
 *    of the join process. If it can't, the original value is kept.
 *
 * When option == rp_jsonc_merge_option_join_or_replace:
 *
 *    If the values can be joined, then the resul is the result
 *    of the join process. If it can't, the merged value
 *    replaces the original one.
 *
 * The join process of the merge is possible if joined values
 * are both an object or both an array.
 *
 * When both values are arrays, the values of the array merged
 * are appened to the end of the original array.
 *
 * When both values are object, the object of value merged
 * is recursively merged to the original destination object.
 *
 * @param dest the object to complete this object is modified
 * @param merged the object containing fields to merge to dest
 * @param option merge option to use in case of conflict @see rp_jsonc_merge_option
 *
 * @return the destination object 'dest'
 *
 * @example rp_jsonc_object_merge({"a":"a"},{"X":"X"},any) -> {"a":"a","X":"X"}
 * @example rp_jsonc_object_merge({"a":"a"},{"a":"X"},rp_jsonc_merge_option_keep) -> {"a":"a"}
 * @example rp_jsonc_object_merge({"a":"a"},{"a":"X"},rp_jsonc_merge_option_replace) -> {"a":"X"}
 */
extern struct json_object *rp_jsonc_object_merge(struct json_object *dest, struct json_object *merged, int option);

/**
 * Sort the 'array' and returns it. Sorting is done accordingly to the
 * order given by the function 'rp_jsonc_cmp'. If the paramater isn't
 * an array, nothing is done and the parameter is returned unchanged.
 *
 * @param array the array to sort
 *
 * @returns the array sorted
 */
extern struct json_object *rp_jsonc_sort(struct json_object *array);

/**
 * Returns a json array of the sorted keys of 'object' or null if 'object' has no keys.
 *
 * @param object the object whose keys are to be returned
 *
 * @return either NULL is 'object' isn't an object or a sorted array of the key's strings.
 */
extern struct json_object *rp_jsonc_keys(struct json_object *object);

/**
 * Compares 'x' with 'y'
 *
 * @param x first object to compare
 * @param y second object to compare
 *
 * @return an integer less than, equal to, or greater than zero
 * if 'x' is found, respectively, to be less than, to match,
 * or be greater than 'y'.
 */
extern int rp_jsonc_cmp(struct json_object *x, struct json_object *y);

/**
 * Searchs whether 'x' equals 'y'
 *
 * @param x first object to compare
 * @param y second object to compare
 *
 * @return an integer equal to zero when 'x' != 'y' or 1 when 'x' == 'y'.
 */
extern int rp_jsonc_equal(struct json_object *x, struct json_object *y);

/**
 * Searchs whether 'x' contains 'y'
 *
 * @param x first object to compare
 * @param y second object to compare
 *
 * @return an integer equal to 1 when 'y' is a subset of 'x' or zero otherwise
 */
extern int rp_jsonc_contains(struct json_object *x, struct json_object *y);

/**
 * Gets or creates *'subobject' from 'object' with 'key'
 *
 * @param object the main object
 * @param key key of the sub-object within 'object'
 * @param subobject pointer where to store the found or created subobject
 *
 * @return 1 if successful or 0 if something failed
 */
extern int rp_jsonc_subobject(struct json_object *object, const char *key, struct json_object **subobject);

/**
 * Adds the 'value' to the 'object' with the 'key', but only if 'value' is not NULL
 *
 * @param object the main object
 * @param key key of the value to add
 * @param value the value to add
 *
 * @return 1 if successful or 0 if something failed or 'value' is NULL
 */
extern int rp_jsonc_add(struct json_object *object, const char *key, struct json_object *value);

/**
 * Adds the 'string' to the 'object' with the 'key', but only if 'string' is not NULL
 *
 * @param object the main object
 * @param key key of the value to add
 * @param string the string to addd
 *
 * @return 1 if successful or 0 if something failed or 'string' is NULL
 */
extern int rp_jsonc_add_string(struct json_object *object, const char *key, const char *string);

#ifdef __cplusplus
    }
#endif
