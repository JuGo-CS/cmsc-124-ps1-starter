/*
 * dt_tuple.c: Tuples for Unit 5, Section G.
 *
 * A tuple is a record with numbered parts.
 * A field selector can show meaning, but a numeric position does not.
 * Tuples suit small temporary groups, such as two function results.
 *
 * A tuple needs no declaration. You build it from its parts and read by position.
 *
 * There is no dt_tuple_set. Construction fixes the arity and contents.
 */

#include "dt.h"

#include <stdlib.h>

struct dt_tuple {
    dt_value values[DT_TUPLE_MAX_ARITY];
    size_t   arity;
};

/*
 * dt_tuple_new builds a tuple from the first count values in order.
 * A zero count creates a valid empty tuple.
 * It returns NULL for excessive arity or an allocation failure.
 */
dt_tuple *dt_tuple_new(const dt_value *values, size_t count)
{
    /* TODO: Return NULL when count exceeds DT_TUPLE_MAX_ARITY.
       Otherwise, copy the values. Accept a zero count.
       {1, "two"}  -> a tuple of arity 2 that prints as (1, "two")
       count 0     -> a valid empty tuple that prints as ()
       count 9     -> NULL, since DT_TUPLE_MAX_ARITY is 8
       cases/normal/tuple_basics.case, cases/capacity/tuple_max_arity.case,
       cases/capacity/tuple_over_arity.case */
    
    /* {y}
    note: tuples are constructed with predetermined values, unlike
    arrays. additionally, values (as declared in the struct) is an
    array, therefore unlike the array, the values arent stored as a
    pointer but rather inside the struct itself.

    the values array has a predetermined length (max arity), no matter
    what the actual arity of the object is.
    */


    // handle case where tuple arity is invalid
    if (count > DT_TUPLE_MAX_ARITY) {
        return NULL;
    }

    // allocate memory for the tuple struct itself
    dt_tuple *t = malloc(sizeof(dt_tuple));
    if (!t) {
        return NULL;
    }

    // set the arity property to the argument [count]
    t->arity = count;

    // initialize the values passed in
    for (unsigned int i = 0; i < count; i++) {
        t->values[i] = values[i];
    }

    return t;
}

/*
 * dt_tuple_free releases the tuple. It accepts NULL.
 * The environment owns the values.
 */
void dt_tuple_free(dt_tuple *t)
{
    /* TODO: Release the tuple. Preserve its values.
       The environment owns those values. dt_array_free follows the same rule.
       a tuple holding a string  -> the tuple goes, the string stays
       dt_tuple_free(NULL)       -> returns, having done nothing */

    // do nothing, if tuple is empty (represented by NULL)
    if (!t) {
        return;
    }

    free(t);
}

/*
 * dt_tuple_arity returns the stored part count in constant time.
 */
size_t dt_tuple_arity(const dt_tuple *t)
{
    /* TODO: Return the count that the constructor stored.
       The count does not change after construction.
       after `tup new pair 1 "two"`:  dt_tuple_arity(pair) -> 2
       after `tup new empty`:         dt_tuple_arity(empty) -> 0
       cases/normal/tuple_basics.case */
    
    // arity is 0 for an empty tuple (which is represented by NULL)
       if (!t) {
        return 0;
    }
    
    // otherwise, the arity is stored as metadata in the tuple struct
    return t->arity;
}

/*
 * dt_tuple_at writes the value at zero-based position index to *out.
 * It returns DT_ERR_RANGE and does not change *out for an invalid index.
 */
dt_status dt_tuple_at(const dt_tuple *t, size_t index, dt_value *out)
{
    /* TODO: DT_ERR_RANGE at or past the arity. Positions start at 0.
       for the tuple (1, "two"):
         dt_tuple_at(t, 0, &out)  -> DT_OK, *out is the integer 1
         dt_tuple_at(t, 2, &out)  -> DT_ERR_RANGE, *out untouched
       cases/normal/tuple_basics.case, cases/boundary/tuple_index_past_arity.case */
    
    // first check if the index is within the designated arity of the tuple
    // note to self: C doesnt have classes, so t.dt_tuple_arity() makes no sense
    if (index > dt_tuple_arity(t)) {
        return DT_ERR_RANGE;
    }
    
    // otherwise just return the value at index, via out
    *out = t->values[index];
    return DT_OK;
}
