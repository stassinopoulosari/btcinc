#include "btcinc.h"

void free_signature(signature_t signature) {
    free_4096_t(signature.signature);
    free_4096_t(signature.public_key);
}

void free_chain_tail(chain_tail_t *tail) {
    free_hash(tail->previous_hash);
    free(tail);
}

void free_chain(chain_t *chain) {
    free(chain->content);
    if (chain->tail != NULL)
        free_chain_tail(chain->tail);
    if (chain->previous != NULL)
        free_chain(chain->previous);
    free_hash(chain->previous_hash);
    free_signature(chain->signature);
    free(chain);
}
void free_pow(pow_t *pow) {
    free(pow->pow);
    free(pow);
}
void free_blockchain(chain_head_t *head) {
    free_pow(head->pow);
    free_chain(head->previous);
    free(head);
}

void free_hash(hash_t to_free) {
    free(to_free.digest);
}

void free_4096_t(uint4096_t to_free) {
    free(to_free.contents);
}

void free_chain_content(chain_content_t *content) {
    free(content->content);
    free(content);
}

void free_keyset(keyset_t *keyset) {
    free_4096_t(keyset->private_key);
    free_4096_t(keyset->public_key);
    free(keyset);
}
