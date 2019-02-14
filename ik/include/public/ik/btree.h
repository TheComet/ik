/*!
 * @file btree.h
 * @brief Implements a container of ordered key-value pairs stored in a vector
 * (ordered by key). The hash is computed from a key (string) provided by the
 * user.
 */

#ifndef BTREE_H
#define BTREE_H

#include "ik/config.h"
#include "ik/hash.h"
#include "ik/vector.h"

C_BEGIN

struct btree_hash_value_t
{
    hash32_t hash;
    void*    value;
};

struct btree_t
{
    struct vector_t vector;
};

/*!
 * @brief Creates a new btree object.
 * @return Returns the newly created btree object. It must be freed with
 * btree_destroy() when no longer required.
 */
IK_PRIVATE_API ikret_t
btree_create(struct btree_t** btree);

/*!
 * @brief Initialises an existing btree object.
 * @note This does **not** FREE existing elements. If you have elements in your
 * btree and call this, those elements will be lost and a memory leak will have
 * been created.
 * @param[in] btree The btree object to initialise.
 */
IK_PRIVATE_API void
btree_construct(struct btree_t* btree);

/*!
 * @brief Destroys an existing btree object and FREEs the underlying memory.
 * @note Elements inserted into the btree are not FREEd.
 * @param[in] btree The btree object to destroy.
 */
IK_PRIVATE_API void
btree_destroy(struct btree_t* btree);

/*!
 * @brief Inserts an element into the btree using a hashed key.
 *
 * @warning There is no way to test for hash collisions since this function
 * doesn't have access to the key which generated the hash. It is highly
 * discouraged to mix btree_insert_hash() and btree_insert(). Use one or the other.
 *
 * @note Complexity is O(log2(n)) to find the insertion point.
 *
 * @param[in] btree The btree object to insert into.
 * @param[in] hash A unique key to assign to the element being inserted. The
 * key must not exist in the btree, or the element will not be inserted.
 * @param[in] value The data to insert into the btree.
 * @note The value is **not** copied into the btree, only referenced. For this
 * reason don't insert stack allocated items into the btree.
 * @return Returns 0 if insertion was successful. Returns 1 if the key already
 * existed (in which case nothing is inserted). Returns -1 on failure.
 */
IK_PRIVATE_API int
btree_insert(struct btree_t* btree, hash32_t hash, void* value);

/*!
 * @brief Sets the value to the specified hash in the btree.
 * @note If the hash is not found, this function silently fails.
 * @param[in] btree A pointer to the btree object to change the value of.
 * @param[in] hash The unique key associated with the value you want to change.
 * @param[in] value The new value to set.
 */
IK_PRIVATE_API void
btree_set(struct btree_t* btree, hash32_t hash, void* value);

/*!
 * @brief Looks for an element in the btree and returns it if found.
 * @note Complexity is O(log2(n)).
 * @param[in] btree The btree to search in.
 * @param[in] hash The hash to search for.
 * @return Returns the data associated with the specified hash. If the hash is
 * not found in the btree, then NULL is returned.
 * @note Potential pitfall: The value could be NULL even if the hash was found,
 * as NULL is a valid thing for a value to be. If you are checking to see if a
 * hash exists, use btree_key_exists() instead.
 */
IK_PRIVATE_API void*
btree_find(const struct btree_t* btree, hash32_t hash);

/*!
 * @brief Looks for an element in the btree and returns a pointer to the element
 * in the structure. This is useful if you need to store data directly in the
 * memory occupied by the pointer and wish to modify it.
 * @note The returned pointer can be invalidated if any insertions or deletions
 * are performed.
 * @param[in] btree The btree to search in.
 * @param[in] hash The has to search for.
 */
IK_PRIVATE_API void**
btree_find_ptr(const struct btree_t* btree, hash32_t hash);

/*!
 * @brief Finds the specified element in the btree and returns its key.
 * @note Complexity is O(n).
 * @param[in] btree The btree to search.
 * @param[in] value The value to search for.
 * @return Returns the key if it was successfully found, or MAP_INVALID_KEY if
 * otherwise.
 */
IK_PRIVATE_API hash32_t
btree_find_element(const struct btree_t* btree, const void* value);

/*!
 * @brief Gets any element from the btree.
 *
 * This is useful when you want to iterate and remove all items from the btree
 * at the same time.
 * @return Returns an element as a void pointer. Which element is random.
 */
IK_PRIVATE_API void*
btree_get_any_element(const struct btree_t* btree);

/*!
 * @brief Returns 1 if the specified hash exists, 0 if otherwise.
 * @param btree The btree to find the hash in.
 * @param hash The hash to search for.
 * @return 0 if the hash was found, -1 if the hash was not found.
 */
IK_PRIVATE_API int
btree_hash_exists(struct btree_t* btree, hash32_t hash);

/*!
 * @brief Returns a hash that does not yet exist in the btree.
 * @note Complexity is O(n)
 * @param[in] btree The btree to generate a hash from.
 * @return Returns a hash that does not yet exist in the btree.
 */
IK_PRIVATE_API hash32_t
btree_find_unused_hash(struct btree_t* btree);

/*!
 * @brief Erases an element from the btree using a hash.
 * @warning It is highly discouraged to mix btree_erase_using_hash() and
 * btree_erase_using_key(). Use btree_erase_using_hash() if you used
 * btree_insert_using_hash(). Use btree_erase_using_key() if you used
 * btree_insert_using_key().
 * @note Complexity is O(log2(n))
 * @param[in] btree The btree to erase from.
 * @param[in] hash The hash that btrees to the element to remove from the btree.
 * @return Returns the data assocated with the specified hash. If the hash is
 * not found in the btree, NULL is returned.
 * @note The btree only holds references to values and does **not** FREE them. It
 * is up to the programmer to correctly free the elements being erased from the
 * btree.
 */
IK_PRIVATE_API void*
btree_erase(struct btree_t* btree, hash32_t hash);

IK_PRIVATE_API void*
btree_erase_element(struct btree_t* btree, void* value);

/*!
 * @brief Erases the entire btree, including the underlying memory.
 * @note This does **not** FREE existing elements. If you have elements in your
 * btree and call this, those elements will be lost and a memory leak will have
 * been created.
 * @param[in] btree The btree to clear.
 */
IK_PRIVATE_API void
btree_clear(struct btree_t* btree);

IK_PRIVATE_API void
btree_clear_free(struct btree_t* btree);

/*!
 * @brief Returns the number of elements in the specified btree.
 * @param[in] btree The btree to count the elements of.
 * @return The number of elements in the specified btree.
 */
#define btree_count(btree) ((btree)->vector.count)

/*!
 * @brief Iterates over the specified btree's elements and opens a FOR_EACH
 * scope.
 * @param[in] btree The btree to iterate.
 * @param[in] T The type of data being held in the btree.
 * @param[in] k The name to give the variable holding the current key. Will
 * be of type hash_t.
 * @param[in] v The name to give the variable pointing to the current
 * element. Will be of type T*.
 */
#define BTREE_FOR_EACH(btree, T, k, v) {                                                           \
    hash32_t btree_##v;                                                                            \
    hash32_t k;                                                                                    \
    T* v;                                                                                          \
    for(btree_##v = 0;                                                                             \
        btree_##v != btree_count(btree) &&                                                         \
            ((k = ((struct btree_hash_value_t*) (btree)->vector.data)[btree_##v].hash) || 1) &&    \
            ((v  = (T*)((struct btree_hash_value_t*)(btree)->vector.data)[btree_##v].value) || 1); \
        ++btree_##v) {

/*!
 * @brief Closes a for each scope previously opened by BTREE_FOR_EACH.
 */
#define BTREE_END_EACH }}

/*!
 * @brief Will erase the current selected item in a for loop from the btree.
 * @note This does not free the data being referenced by the btree. You will have
 * to erase that manually (either before or after this operation, it doesn't
 * matter).
 * @param[in] btree A pointer to the btree object currently being iterated.
 */
#define BTREE_ERASE_CURRENT_ITEM_IN_FOR_LOOP(btree, v) do {                                          \
    vector_erase_element(&(btree)->vector, ((btree_hash_value_t*)(btree)->vector.data) + btree_##v); \
    --btree_##v; } while(0)

C_END

#endif /* BTREE_H */
