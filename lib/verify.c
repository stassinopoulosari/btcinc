#include "btcinc.h"

bool verify_chain_signature(chain_t *chain) {
    signature_t signature = chain->signature;
    uint4096_t comparator, digest;
    bool result;
    hash_t chain_hash = hash_chain(chain);
    digest = hash_to_4096_t(chain_hash);
    free_hash(chain_hash);
    comparator = rsa_decrypt(signature.signature, signature.public_key);
    result = big_comparison(comparator.contents, digest.contents) == 0;
    /* Make sure memory isn't incontinent */
    free_4096_t(comparator);
    free_4096_t(digest);
    return result;
}

bool verify_chain_head_signature(chain_head_t *head) {
    signature_t signature = head->signature;
    uint4096_t comparator, digest;
    bool result;
    hash_t chain_hash = hash_chain_head(head, NULL);
    if (DEBUG)
        printf("Copy digest to 4096\n");
    digest = hash_to_4096_t(chain_hash);
    free_hash(chain_hash);
    if (DEBUG)
        printf("Store transcryptor\n");
    comparator = rsa_decrypt(signature.signature, signature.public_key);
    if (DEBUG) {
        printf("Store result\n");
        print_4096_t(comparator);
    }
    result = big_comparison(comparator.contents, digest.contents) == 0;
    /* Make sure memory isn't incontinent */
    free_4096_t(comparator);
    free_4096_t(digest);
    return result;
}

bool verify_head(chain_head_t *head) {
    hash_t chain_hash;
    bool hashes_match;
    /* Verify POW is valid */
    if (!check_pow(head, NULL)) {
        fprintf(stderr, "Unable to check the POW\n");
        return false;
    }
    /* Verify timestamp makes sense */
    if (head->timestamp < head->previous->timestamp ||
            head->timestamp > (uint64_t)time(NULL)) {
        fprintf(stderr, "Entity made by time traveller\n");
        return false;
    }
    /* Verify previous hash */
    chain_hash = hash_signature(head->previous->signature);
    hashes_match = hashcmp(head->previous_hash, chain_hash);
    free_hash(chain_hash);
    if (DEBUG)
        printf("Hashes match: %d\n", hashes_match);
    if (!hashes_match)
        return false;
    if (DEBUG)
        printf("Verify chain head signature\n");
    if (!verify_chain_head_signature(head))
        return false;
    if (DEBUG)
        printf("Head verification succeeds\n");
    return verify_chain(head->previous);
}

bool verify_chain(chain_t *chain) {
    hash_t chain_hash;
    short mode;
    bool hashes_match;
    /* Check components */
    /* Exactly one of previous or tail must be null */
    if (!((chain->previous == NULL) ^ (chain->tail == NULL))) {
        fprintf(stderr, "Either previous or tail must be null\n");
        return false;
    }

    mode = chain->previous != NULL ? MODE_CHAIN : MODE_TAIL;
    /* Verify timestamp makes sense */
    if ((mode == MODE_CHAIN && chain->timestamp < chain->previous->timestamp) ||
            chain->timestamp > (uint64_t)time(NULL)) {
        fprintf(stderr, "Entity made by time traveller\n");
        return false;
    }
    /* Verify previous hash */
    chain_hash = mode == MODE_CHAIN ? hash_signature(chain->previous->signature)
                 : hashcpy(chain->tail->previous_hash);
    hashes_match = hashcmp(chain_hash, chain->previous_hash);
    free_hash(chain_hash);
    if (DEBUG)
        printf("Hashes match: %d\n", hashes_match);
    if (!hashes_match)
        return false;
    /* Verify signature */
    if (DEBUG)
        printf("Verify signature\n");
    if (!verify_chain_signature(chain))
        return false;
    /* If verification of actual chain succeeds, propagate back unless we hit the
     * tail of this block */
    if (chain->previous == NULL) {
        if (DEBUG)
            printf("Chain verification succeeds\n");
        return true;
    }
    if (DEBUG)
        printf("Chain verification succeeds. Moving on to next\n");
    return verify_chain(chain->previous);
}
