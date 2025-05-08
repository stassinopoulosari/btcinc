#include "btcinc.h"

void free_signature(signature_t signature) {
    free_4096_t(signature.signature);
    free_4096_t(signature.public_key);
}
void free_chain_tail(chain_tail_t *tail) {
    if (DEBUG)
        printf("Free tail hash\n");
    free_hash(tail->previous_hash);
    free(tail);
}
void free_chain(chain_t *chain) {
    if (DEBUG)
        printf("Free chain content\n");
    free(chain->content);
    if (DEBUG)
        printf("Free chain previous/tail\n");
    if (chain->tail != NULL)
        free_chain_tail(chain->tail);
    if (chain->previous != NULL)
        free_chain(chain->previous);
    if (DEBUG)
        printf("Free hash\n");
    free_hash(chain->previous_hash);
    if (DEBUG) {
        printf("Free signature\n");
        printf("    Block<");
        if (chain->signature.signature.contents == NULL)
            printf("NULL>");
        else {
            printf("%p>", (void *)chain->signature.signature.contents);
            if (PRINT_SIGNATURES)
                print_4096_t(chain->signature.signature);
        }
        putchar('\n');
    }
    free_signature(chain->signature);
    if (DEBUG)
        printf("Free actual chain\n");
    free(chain);
}
void free_pow(pow_t *pow) {
    free(pow->pow);
    free(pow);
}
void free_blockchain(chain_head_t *head) {
    if (DEBUG)
        printf("Free PoW\n");
    free_pow(head->pow);
    if (DEBUG)
        printf("Free chain\n");
    free_chain(head->previous);
    free(head);
}

void free_hash(hash_t to_free) {
    if (DEBUG)
        print_hash(to_free);
    free(to_free.digest);
}

void free_4096_t(uint4096_t to_free) {
    free(to_free.contents);
}
