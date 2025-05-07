/* This prevents double-inclusions */
#ifndef LIST_T
#define LIST_T

/* Some useful libraries */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Implement booleans */


/* Notes:

 List elements are of type void *
 List indices are of type size_t
 Success/failure returns are of type bool
 None returns are of type void
 */
/*typedef void ** list_t;*/
struct list_item {
    void *content;
    struct list_item *next;
};
typedef struct list_item *list_t;

#define get_next_pointer(list_item) list_item->next
#define set_next_pointer(list_item, new_next) list_item->next = new_next
#define get_content(list_item) list_item->content
#define set_content(list_item, new_content) list_item->content = new_content
#define list_is_empty(list_item)                                               \
  ((list_item == NULL) || ((NULL == get_content(list_item)) &&                 \
                           (NULL == get_next_pointer(list_item))))

/*
 _: New (a la Python __init__)
 inputs: none
 outputs: a new list_t containing no values
 side effects: none
 */
list_t list_new();

/*
 _: Free (no Python equivalent)
 inputs: a list_t l
 outputs: nothing
 side effects: frees all memory associated with l
 */
void list_free(list_t l);

/*
 _: Print (a la Python __str__)
 _: Print (a la Python __str__)
 inputs: a list_t l
 outputs: nothing
 side effects: the elements of l are printed as in python
 example:
 list_print(list_new());
 - should print "[]"
 Note: Prints void *'s as uint64_t's. (use %lu)
 */
void list_print(list_t l);

/*
 0: Append
 inputs: a list_t l, a pointer to an memory object of any type x
 outputs: nothing
 side effects: x is added to the end of l
 example:
 list_t l = list_new();
 uint64_t *val = 1;
 list_append(l, val);
 list_print(l);
 - should print "[1]"
 */
void list_append(list_t l, void *x);

/*
 1: Extend
 inputs: two list_ts, l1 and l2
 outputs: nothing
 side effects: all elements of l2 are appended to l1
 example:
 list_t l1 = list_new();
 uint64_t *val = 1;
 list_append(l1, val);
 list_t l2 = list_new();
 *val = 2;
 list_append(l2, val);
 list_print(l1);
 - should print "[1, 2]"
 */
void list_extend(list_t l1, list_t l2);

/*
 2: Insert
 inputs: a list_t l, size_t list index i, and a pointer to an memory object of
 any type x outputs: nothing side effects: x is added to l with index i and all
 elements are preserved in l example: list_t l = list_new(); uint64_t *val = 1;
 list_append(l, val);
 *val = 2;
 list_insert(l, 0, val);
 list_print(l);
 - should print "[2, 1]"
 */
void list_insert(list_t l, size_t i, void *x);

/*
 3: Remove
 inputs: a list_t l, and a pointer to an memory object of any type x
 outputs: false if an instance of x is removed, false otherwise
 side effects: the first instance of x is removed from l
 example:
 list_t l = list_new();
 uint64_t *val = 2;
 list_append(l, val);
 list_append(l, val);
 *val = 1;
 list_print(l1);
 list_remove(l, 1, val);
 list_print(l);
 - should return True
 - should print "[2, 1, 2]" then "[1, 2]"
 */
bool list_remove(list_t l, void *x);

/*
 4: Pop
 inputs: a list_t l, and a size_t list index i
 outputs: The element at index i, or exit(1) if i is out of range
 side effects: remove the element at index i
 example:
 list_t l = list_new();
 uint64_t *val = 1;
 list_append(l, val);
 *val = 2;
 list_insert(l, 0, val);
 list_print(l);
 printf("%d\n", pop(l, 1));
 list_print(l);
 - should print "[1, 2]" then "2" then "[1]"
 */
void *list_pop(list_t l, size_t i);

/*
 5: Clear
 inputs: a list_t l
 outputs: nothing
 side effects: l contains no elements
 example:
 list_t l = list_new();
 uint64_t *val = 1;
 list_append(l, val);
 clear(l)
 - should print "[]"
 */
void list_clear(list_t l);

/*
 6: Index
 inputs: a list_t l, and a pointer to an memory object of any type x
 outputs: a size_t i giving the index of x in l, or exit(1) if x is not in l.
 side effects: none
 example:
 list_t l = list_new();
 uint64_t *val = 1;
 list_append(l, val);
 printf("%d\n", index(l, val)));
 - should print "0"
 */
size_t list_index(list_t l, void *x);

/*
 7: Count
 inputs: a list_t l, and a pointer to an memory object of any type x
 outputs: The number of times x occurs in l
 side effects: none
 example:
 list_t l = list_new();
 uint64_t *val = 1;
 list_append(l, val);
 list_append(l, val);
 list_count(l, val);
 - should return 2
 */
uint64_t list_count(list_t l, void *x);

/* ~: Sort
 "Extra credit" for a sorting algorithm
 More "Extra credit" for an O(n*log(n)) sort
 */
void sort(list_t l, bool (greater_than)(void *, void *));

/*
 8: Reverse
 inputs: a list_t l
 outputs: nothing
 side effects: the elements of l are reversed
 example:
 list_t l = list_new();
 uint64_t *val = 1;
 list_append(l, val);
 *val = 2;
 list_append(l, val);
 list_print(l);
 list_reverse(l)
 list_print(l);
 - should print "[1, 2]" then "[2, 1]"
 */
void list_reverse(list_t l);

/*
 9: Copy
 inputs: a list_t l
 outputs: a list_t r containing all the elements of l
 side effects: none
 example:
 list_t l = list_new();
 uint64_t *val = 1;
 list_append(l, val);
 list_t r = copy(l);
 list_append(l, val);
 list_print(l);
 list_print(r);
 - should print "[1, 1]" then "[1]"
 */
list_t list_copy(list_t l);

#endif
