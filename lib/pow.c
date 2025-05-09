#include "btcinc.h"

bool check_pow(chain_head_t *head, pow_t *pow) {
    uint32_t last_element;
    size_t cursor, elements_to_check = POW_DIFFICULTY / 8,
                   digits_in_last_element = POW_DIFFICULTY % 8;
    bool pow_valid;
    hash_t candidate_hash;
    pow = pow == NULL ? head->pow : pow;
    /* Low proof of work threshold */
    candidate_hash = hash_chain_head(head, pow);
    /* Check that each of the first POW_DIFFICULTY digits of the hash are zero */
    /* Find number of elements to check */
    if (head->pow == pow) {
        if(DEBUG) printf("Checking proof-of-work:\n");
        if(DEBUG) print_hash(candidate_hash);
    }
    for (cursor = 0; cursor < elements_to_check; cursor++) {
        if (candidate_hash.digest[cursor] != 0)
            return false;
    }
    last_element = candidate_hash.digest[cursor];
    /* Shift element to the right by 4 * digits */
    pow_valid = (last_element >>
                 (sizeof(last_element) * 8 - (4 * digits_in_last_element))) == 0;
    if (pow_valid) {
        if(DEBUG) printf("Valid proof-of-work hash:\n");
        if(DEBUG) print_hash(candidate_hash);
    }
    free_hash(candidate_hash);

    return pow_valid;
}

/* Add one (1) to a buffer */
bool add_to_buffer(buffer_chunk *buffer, size_t buffer_size) {
    size_t cursor;
    for (cursor = 0;
            /* Cursor doesn't overflow buffer */
            cursor < buffer_size / sizeof(buffer_chunk) &&
            /* Adding 1 to the element at cursor does overflow (carry) */
            ++buffer[cursor] == 0;
            cursor++) {
    }
    /* Return carry */
    return
        /* Cursor has passed the last element (if the last carry was zeo, then
           cursor would be buffer_size - 1 */
        cursor == buffer_size;
}

pow_t *do_pow(chain_head_t *head) {
    pow_t *pow = malloc(sizeof(pow_t));
    size_t buffer_size = sizeof(buffer_chunk);
    short carry = 1;
    buffer_chunk *buffer = NULL;
    do {
    	/* If we need to expand the buffer */
        if (carry == 1) {
        	/* Free the existing buffer and start over. We do this because the size of the input matters for SHA256 */
            if (buffer != NULL)
                free(buffer);
            buffer = malloc(buffer_size);
           	/* Clear out the new buffer */
            memset(buffer, 0, buffer_size);
            pow->pow = buffer;
            pow->pow_size = buffer_size;
        }
        carry = add_to_buffer(buffer, buffer_size);
        if (carry)
            buffer_size += sizeof(buffer_chunk);
    } while (!check_pow(head, pow));

    return pow;
}
