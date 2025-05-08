#include "list_t.h"

/* Abstraction functions that made moving from a data clump to a struct much
 * easier */

/* For each function for internal functions */
void list_foreach(list_t list, void(action(list_t item))) {
    list_t item = list, next;
    if (list_is_empty(item))
        return;
    while (item != NULL) {
        next = get_next_pointer(item);
        action(item);
        item = next;
    }
}

bool list_empty(list_t list) {
    return list_is_empty(list);
}

/* Given any list item, advance to the end of the list */
list_t list_end_of(list_t list) {
    list_t item = list;
    while (get_next_pointer(item) != NULL) {
        item = get_next_pointer(item);
    }
    return item;
}

/* Allocate memory for a new list item */
list_t allocate_item() {
    list_t new_item = (list_t)malloc(sizeof(struct list_item));
    /* Best not to think about what the existing memory value is */
    set_content(new_item, NULL);
    set_next_pointer(new_item, NULL);
    return new_item;
}

list_t list_new() {
    /* We'll create a new list */
    /* Allocate space for two items */
    list_t new_list = allocate_item();
    if (new_list == 0) {
        /* Allocation failed */
        return NULL;
    }
    /* Assign NULL to both items to show it's an empty list */
    set_content(new_list, NULL);
    /* Assign the next-item pointer to NULL as well*/
    set_next_pointer(new_list, NULL);
    return new_list;
}

void front_remove(list_t list) {
    list_t second_item;
    /* Case: 0-1 item list */
    if (get_next_pointer(list) == NULL) {
        /* If the list is empty, this will have no effect */
        set_content(list, NULL);
        return;
    }
    /* Case: >2-item list */
    second_item = get_next_pointer(list);
    /* Replace the first item with the contents and next pointer from the second
     * item */
    set_content(list, get_content(second_item));
    set_next_pointer(list, get_next_pointer(second_item));
    /* Relegate the second item to a permanent end */
    free(second_item);
}

/* Helper function so I can use foreach in `list_free` */
void list_free_helper(list_t list) {
    free(list);
}

/*
 _: Free (no Python equivalent)
 inputs: a list_t l
 outputs: nothing
 side effects: frees all memory associated with l
 */
void list_free(list_t list) {
    if (list == NULL) {
        /* Can't free what you can't see */
        return;
    }
    list_foreach(list, list_free_helper);
}

/* Helper function so I can use foreach in `list_print` */
void list_print_item(list_t item) {
    if (get_content(item) != NULL)
        printf("%lu", (size_t)get_content(item));
    if (get_next_pointer(item) != NULL) {
        printf(", ");
    }
}
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
void list_print(list_t list) {
    putchar('[');
    if (list == NULL) {
        printf("NULL");
    } else {
        list_foreach(list, list_print_item);
    }
    putchar(']');
    putchar('\n');
}

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
void list_append(list_t list, void *value) {
    list_t item = list, put;

    /* Special case for empty list */
    if (list_is_empty(item)) {
        set_content(item, value);
        return;
    }
    /* Otherwise, loop through until we find an item with no next */
    item = list_end_of(item);
    put = allocate_item();
    /* If the allocation fails, short return */
    if (put == 0) {
        return;
    }
    set_next_pointer(item, put);
    set_content(put, value);
}

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
void list_extend(list_t base, list_t extension) {
    list_t target = list_end_of(base), source = extension;
    /* Cut if l2 is empty */
    if (list_is_empty(source)) {
        return;
    }
    do {
        list_append(target, get_content(source));
        source = get_next_pointer(source);
        if (get_next_pointer(target) != NULL)
            target = get_next_pointer(target);
    } while (source != NULL);
}

void front_insert(list_t list, void *value) {
    list_t new_item;
    /* Case where l is empty */
    if (list_is_empty(list)) {
        set_content(list, value);
        return;
    }
    new_item = allocate_item();
    /* In case allocation fails */
    if (new_item == 0)
        return;
    set_next_pointer(new_item, get_next_pointer(list));
    set_content(new_item, get_content(list));
    set_content(list, value);
    set_next_pointer(list, new_item);
}

void insert_after(list_t left, void *value) {
    list_t right, new_item = allocate_item();
    right = get_next_pointer(left);
    set_next_pointer(left, new_item);
    set_content(new_item, value);
    set_next_pointer(new_item, right);
}
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
void list_insert(list_t list, size_t position, void *value) {
    size_t current_position = 0;
    list_t left = list;
    if (position == 0) {
        front_insert(list, value);
        return;
    }
    while (get_next_pointer(left) != NULL && current_position < position - 1) {
        left = get_next_pointer(left);
        current_position++;
    }
    if (current_position != position - 1) {
        /* Position is out of bounds */
        exit(1);
    }
    insert_after(left, value);
}

void list_item_remove(list_t left, list_t item) {
    if (left == NULL) {
        front_remove(item);
        return;
    }
    set_next_pointer(left, get_next_pointer(item));
    free(item);
}
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
bool list_remove(list_t list, void *value) {
    list_t target = list, prev = NULL;
    do {
        prev = target;
        target = get_next_pointer(target);
    } while (get_next_pointer(target) != NULL && get_content(target) != value);
    /* We're either at the end of the list or at the item */
    if (get_content(target) == value) {
        list_item_remove(prev, target);
        return false;
    }
    /* We've reached the end of list */
    return false;
}

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
void *list_pop(list_t list, size_t position) {
    size_t current_position = 0;
    void *return_value;
    list_t left = list, target;
    if (position == 0) {
        return_value = get_content(list);
        list_item_remove(NULL, left);
        return return_value;
    }
    while (get_next_pointer(left) != NULL && current_position < position - 1) {
        left = get_next_pointer(left);
        current_position++;
    }
    /* If there are <i items */
    if (get_next_pointer(left) == NULL || current_position < position - 1) {
        exit(1);
    }
    target = get_next_pointer(left);
    return_value = get_content(target);
    list_item_remove(left, target);
    return return_value;
}

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
void list_clear(list_t list) {
    while (!list_is_empty(list)) {
        front_remove(list);
    }
}

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
size_t list_index(list_t list, void *value) {
    list_t target = list;
    size_t position = 0;
    /* Move forward in the list until we find the item */
    while (get_content(target) != value) {
        target = get_next_pointer(target);
        if (target == NULL) {
            exit(1);
        }
        position++;
    }
    return position;
}

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
uint64_t list_count(list_t list, void *value) {
    uint64_t count = 0;
    list_t target = list;
    do {
        if (get_content(target) == value)
            count++;
        target = get_next_pointer(target);
    } while (target != NULL);
    return count;
}

list_t sort_merge(list_t left, list_t right, bool(compare)(void *, void *)) {
    list_t merged = list_new();
    while (!list_is_empty(left) && !list_is_empty(right)) {
        if (!compare(get_content(left), get_content(right))) {
            list_append(merged, get_content(left));
            front_remove(left);
        } else {
            list_append(merged, get_content(right));
            front_remove(right);
        }
    }

    if (!list_is_empty(left)) {
        list_extend(merged, left);
    } else if (!list_is_empty(right)) {
        list_extend(merged, right);
    }
    list_free(left);
    list_free(right);
    return merged;
}

/* ~: Sort
 "Extra credit" for a sorting algorithm
 More "Extra credit" for an O(n*log(n)) sort

 https://en.wikipedia.org/wiki/Merge_sort#Bottom-up_implementation_using_lists
 */
void sort(list_t list, bool(compare)(void *, void *)) {
    /* Merge sort */
    list_t working_array[32], result = list_new(), next;
    uint8_t iterator, itermax = 0;

    /* Copy l to result because we otherwise will have some Unorthodox errors */
    list_extend(result, list);

    /* Initialize working array to NULL */
    for (iterator = 0; iterator < 32; iterator++) {
        working_array[iterator] = NULL;
    }

    /* A 0-1 item list is already sorted */
    if (get_next_pointer(list) == NULL) {
        return;
    }

    while (result != NULL) {
        next = get_next_pointer(result);
        set_next_pointer(result, NULL);
        for (iterator = 0; iterator < 32 && working_array[iterator] != NULL;
                iterator++) {
            result = sort_merge(working_array[iterator], result, compare);
            working_array[iterator] = NULL;
            itermax = itermax < iterator ? iterator : itermax;
        }
        if (iterator == 32) {
            iterator -= 1;
        }
        working_array[iterator] = result;
        result = next;
    }

    result = NULL;

    /* Final merge */
    for (iterator = 0; iterator <= itermax + 1; iterator++) {
        if (result == NULL) {
            result = working_array[iterator];
        } else if (working_array[iterator] != NULL) {
            result = sort_merge(working_array[iterator], result, compare);
            working_array[iterator] = NULL;
        }
    }

    /* Since we're doing this in place, move everything to the original list */
    list_clear(list);
    list_extend(list, result);
    list_free(result);
    list_free(next);
}

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
void list_reverse(list_t list) {
    list_t target = list, temp;

    /* Empty or single-item list*/
    if (get_next_pointer(list) == NULL)
        return;

    /* Create a list to hold the reversed list */
    temp = list_new();

    do {
        front_insert(temp, get_content(target));
        target = get_next_pointer(target);
    } while (target != NULL);

    /* Move the reversed list into the original */
    list_clear(list);
    list_extend(list, temp);
    list_free(temp);
}

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
list_t list_copy(list_t list) {
    list_t copy = list_new();
    list_extend(copy, list);
    return copy;
}
