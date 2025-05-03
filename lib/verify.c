#include "btcinc.h"

bool verify_chain_signature(chain_t *chain) {
    signature_t signature = chain->signature;
    uint4096_t comparator = make_4096_t(), transcryptor = make_4096_t(),
                                           digest = make_4096_t();
    bool result;
    hash_t chain_hash = hash_chain_content(chain);
    memcpy(digest.contents, chain_hash.digest, DIGEST_SIZE);
    free_hash(chain_hash);
    transcryptor.contents[0] = STD_ENCRYPTOR;
    big_modexp(signature.signature.contents, transcryptor.contents,
               signature.public_key.contents, comparator.contents);
    result = big_comparison(comparator.contents, digest.contents) == 0;
    /* Make sure memory isn't incontinent */
    free_4096_t(comparator);
    free_4096_t(transcryptor);
    free_4096_t(digest);
    return result;
}

bool verify_chain_head_signature(chain_head_t *head) {
    signature_t signature = head->signature;
    uint4096_t comparator = make_4096_t(), transcryptor = make_4096_t(),
                                           digest = make_4096_t();
    bool result;
    hash_t chain_hash = hash_chain_head(head, NULL);
    memcpy(digest.contents, chain_hash.digest, DIGEST_SIZE);
    free_hash(chain_hash);
    transcryptor.contents[0] = STD_ENCRYPTOR;
    big_modexp(signature.signature.contents, transcryptor.contents,
               signature.public_key.contents, comparator.contents);
    result = big_comparison(comparator.contents, digest.contents) == 0;
    /* Make sure memory isn't incontinent */
    free_4096_t(comparator);
    free_4096_t(transcryptor);
    free_4096_t(digest);
    return result;
}

bool verify_head(chain_head_t *head) {
    hash_t chain_hash;
    bool hashes_match;
    /* Verify POW is valid */
    if (!check_pow(head, NULL))
        return false;
    /* Verify previous hash */
    chain_hash = hash_signature(head->previous->signature);
    if (!hashcmp(head->previous_hash, chain_hash))
        hashes_match = false;
    free_hash(chain_hash);
    if (!hashes_match)
        return false;
    if(!verify_chain_head_signature(head))
        return false;
    return verify_chain(head->previous);
}

bool verify_chain(chain_t *chain) {
    hash_t chain_hash;
    short mode;
    bool hashes_match;
    /* Check components */
    /* Exactly one of previous or tail must be null */
    if (!((chain->previous == NULL) ^ (chain->tail == NULL)))
        return false;
    mode = chain->previous != NULL ? MODE_CHAIN : MODE_TAIL;
    /* Verify previous hash */
    chain_hash = mode == MODE_CHAIN ? hash_signature(chain->previous->signature)
                 : hashcpy(chain->tail->previous_hash);
    hashes_match = hashcmp(chain_hash, chain->previous_hash);
    free_hash(chain_hash);
    if (!hashes_match)
        return false;
    /* Verify signature */
    if (!verify_chain_signature(chain))
        return false;
    /* If verification of actual chain succeeds, propagate back */
    if (chain->previous != NULL)
        return true;
    return verify_chain(chain->previous);
}
