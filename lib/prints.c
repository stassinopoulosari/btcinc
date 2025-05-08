#include "btcinc.h"
#include <stdio.h>

void print_signature(signature_t signature) {
    printf("  Signature\n");
    printf("    Block<");
    if (signature.signature.contents == NULL)
        printf("NULL>");
    else {
        printf("%p>", (void *)signature.signature.contents);
        if (PRINT_SIGNATURES)
            print_4096_t(signature.signature);
    }
    putchar('\n');
    printf("    Public Key<");
    if (signature.public_key.contents == NULL)
        printf("NULL>");
    else {
        printf("%p>", (void *)signature.public_key.contents);
        if (PRINT_SIGNATURES)
            print_4096_t(signature.public_key);
    }
}

void print_chain_tail(chain_tail_t *tail) {
    printf("Chain tail:\n");
    if (tail == NULL) {
        printf("  NULL\n");
        return;
    }
    printf("  Previous hash\n{\n");
    print_hash(tail->previous_hash);
    printf("}\n");
    putchar('\n');
}

void print_chain_head(chain_head_t *head) {
    printf("Chain head:\n");
    if (head == NULL) {
        printf("  NULL\n");
        return;
    }
    printf("  Proof of work    ");
    if (head->pow == NULL) {
        printf("NULL\n");
    } else {
        printf("pow<%p>\n", (void *)head->pow);
        printf("    Content of size %lu\n", head->pow->pow_size);
    }
    printf("  Previous         ");
    if (head->previous == NULL) {
        printf("NULL");
    } else {
        printf("Chain<%p>", (void *)head->previous);
    }
    putchar('\n');
    printf("  Timestamp        %lu\n", head->timestamp);
    print_signature(head->signature);
    printf("    Public Key<");
    if (head->signature.public_key.contents == NULL)
        printf("NULL>");
    else {
        printf("%p>", (void *)head->signature.public_key.contents);
        if (PRINT_SIGNATURES)
            print_4096_t(head->signature.public_key);
    }
    putchar('\n');
    printf("  Previous hash \n{\n");
    print_hash(head->previous_hash);
    printf("}\n");
    if(PRINT_SIGNATURES) {
        printf("  Raw proof-of-work:\n");
        print_pow(head->pow);
    }
    putchar('\n');
}
void print_chain_recursive(chain_t *chain) {
    if(chain->previous != NULL) {
        print_chain_recursive(chain->previous);
    } else {
        print_chain_tail(chain->tail);
    }
    print_chain(chain);
}
void print_blockchain(chain_head_t *head) {
    print_chain_recursive(head->previous);
    print_chain_head(head);
}
void print_pow(pow_t *pow) {
    size_t cursor;
    for (cursor = 0; cursor < pow->pow_size / sizeof(buffer_chunk); cursor++) {
        printf("%016lx ", ((buffer_chunk *)pow->pow)[cursor]);
        if (cursor % 2 == 1)
            putchar('\n');
    }
    putchar('\n');
}


void print_chain(chain_t *chain) {
    printf("Chain:\n");
    if (chain == NULL) {
        printf("  NULL\n");
        return;
    }
    printf("  Content of size  %lu\n", chain->content_size);
    printf("  Previous         ");
    if (chain->previous == NULL) {
        printf("Tail<");
        if (chain->tail == NULL) {
            printf("NULL");
        } else {
            printf("%p", (void *)chain->tail);
        }
        printf(">");
    } else {
        printf("Chain<%p>", (void *)chain->previous);
    }
    putchar('\n');
    printf("  Timestamp        %lu\n", chain->timestamp);
    print_signature(chain->signature);
    putchar('\n');
    printf("  Previous hash \n{\n");
    print_hash(chain->previous_hash);
    printf("}\n");
    putchar('\n');
}

void print_hash(hash_t hash) {
    size_t cursor;
    printf("Hash <%p>\n", (void *)hash.digest);
    if (hash.digest == NULL) {
        printf("  NULL\n");
        return;
    }
    for (cursor = 0; cursor < DIGEST_WORDS; cursor++) {
        printf("  %08x", hash.digest[cursor]);
        if (cursor % 2 == 1) {
            putchar('\n');
        }
    }
}

void print_4096_t(uint4096_t number) {
    size_t iterator;

    for (iterator = 0; iterator < KEY_WORDS; iterator++) {
        if (number.contents == NULL) {
            /* so graceful */
            printf("NULLNULLNULLNULL ");
        } else {
            /* Print each word with a space */
            printf("%016lx ", number.contents[S - 1 - iterator]);
        }
        /* newline every other word for readability */
        if (iterator % 2 == 1) {
            printf("\n");
        }
    }
    /* Cherry on top */
    printf("\n");
}
