#include "btcinc.h"


uint4096_t make_4096_t() {
    uint4096_t out;
    out.contents = malloc(KEY_SIZE);
    memset(out.contents, 0, KEY_SIZE);
    return out;
}

void free_4096_t(uint4096_t to_free) {
    free(to_free.contents);
}

void free_hash(hash_t to_free) {
    free(to_free.digest);
}

hash_t hashcpy(hash_t prior) {
    hash_t new;
    new.digest = malloc(DIGEST_SIZE);
    memcpy(new.digest, prior.digest, DIGEST_SIZE);
    return new;
}

chain_tail_t *genesis() {
    chain_tail_t *chain_tail = malloc(sizeof(chain_tail_t));
    if (chain_tail == NULL)
        exit(1);
    chain_tail->previous_hash.digest = NULL;
    return chain_tail;
}

hash_t hash_signature(signature_t signature) {
    /* Signature content will be KEY_SIZE long */
    hash_t signature_hash;
    uint32_t *digest = hash(signature.signature.contents, KEY_SIZE);
    signature_hash.digest = digest;
    return signature_hash;
}

signature_t sign_hash(hash_t *hash_to_sign, keyset_t *keyset) {
    signature_t output;
    uint4096_t signature_input, signature_output;
    signature_input = make_4096_t();
    signature_output = make_4096_t();
    memcpy(signature_input.contents, hash_to_sign->digest, DIGEST_SIZE);
    big_modexp(signature_input.contents, keyset->private_key.contents,
               keyset->public_key.contents, signature_output.contents);
    free_4096_t(signature_input);
    output.public_key = keyset->public_key;
    output.signature = signature_output;
    return output;
}

chain_tail_t *tail(chain_head_t *prev_head) {
    chain_tail_t *tail = malloc(sizeof(chain_tail_t));
    tail->previous_hash = hash_signature(prev_head->signature);
    return tail;
}

hash_t hash_chain_content(chain_t *chain) {
    hash_t chain_hash;
    uint32_t *digest;
    void *hash_input;
    /* digest of previous message + Hash message content + timestamp */
    size_t hash_input_size = chain->content_size + DIGEST_SIZE + sizeof(uint64_t);
    hash_input = malloc(hash_input_size);
    memcpy(hash_input, chain->previous_hash.digest, DIGEST_SIZE);
    memcpy(memseek(hash_input, DIGEST_SIZE), chain->content, chain->content_size);
    memcpy(memseek(hash_input, hash_input_size - sizeof(uint64_t)),
           &chain->timestamp, sizeof(uint64_t));
    digest = hash(hash_input, hash_input_size);
    chain_hash.digest = digest;
    free(hash_input);
    return chain_hash;
}

hash_t hash_chain_head(chain_head_t *chain_head, pow_t *pow) {
    hash_t chain_head_hash;
    uint32_t *digest;
    void *hash_input;
    size_t hash_input_size;
    if (pow == NULL) {
        pow = chain_head->pow;
    }
    /* digest of previous message + Hash message content + timestamp */
    hash_input_size = pow->pow_size + DIGEST_SIZE + sizeof(uint64_t);
    hash_input = malloc(hash_input_size);
    memcpy(hash_input, chain_head->previous_hash.digest, DIGEST_SIZE);
    memcpy(memseek(hash_input, DIGEST_SIZE), pow->pow, pow->pow_size);
    memcpy(memseek(hash_input, hash_input_size - sizeof(uint64_t)),
           &chain_head->timestamp, sizeof(uint64_t));
    digest = hash(hash_input, hash_input_size);
    chain_head_hash.digest = digest;
    free(hash_input);
    return chain_head_hash;
}

chain_t *_chain_add(void *previous, short mode, chain_content_t *content_to_add,
                    keyset_t *keyset) {
    chain_t *chain = malloc(sizeof(chain_t));
    hash_t chain_hash;
    chain->previous = NULL;
    chain->tail = NULL;
    /* A tail doesn't have a hash in itself because it's really a reference to a
     * previous head */
    if (mode == MODE_TAIL) {
        chain->previous_hash = ((chain_tail_t *)previous)->previous_hash;
        chain->tail = (chain_tail_t *)previous;
    } else if (mode == MODE_CHAIN) {
        chain->previous_hash = hash_signature(((chain_t *)previous)->signature);
        chain->previous = (chain_t *)previous;
    }
    chain->content = malloc(content_to_add->content_size);
    memcpy(chain->content, content_to_add->content, content_to_add->content_size);
    chain->content_size = content_to_add->content_size;
    chain->timestamp = (uint64_t)time(NULL);
    /* Sign chain content */
    chain_hash = hash_chain_content(chain);
    chain->signature = sign_hash(&chain_hash, keyset);
    return chain;
}

chain_t *chain_new(chain_tail_t *tail, chain_content_t *content_to_add,
                   keyset_t *keyset) {
    return _chain_add(tail, MODE_TAIL, content_to_add, keyset);
}

chain_t *chain_add(chain_t *chain, chain_content_t *content_to_add,
                   keyset_t *keyset) {
    return _chain_add(chain, MODE_CHAIN, content_to_add, keyset);
}

chain_head_t *commit_chain(chain_t *chain, keyset_t *keyset) {
    chain_head_t *head = malloc(sizeof(chain_head_t));
    hash_t head_hash;
    head->previous_hash = hash_signature(chain->signature);
    head->previous = chain;
    head->timestamp = (uint64_t)time(NULL);
    /* Do the pow */
    head->pow = do_pow(head);
    /* Sign the hash */
    head_hash = hash_chain_head(head, NULL);
    head->signature = sign_hash(&head_hash, keyset);
    free_hash(head_hash);
    return head;
}

/* Order doesn't matter for hashes->Check equality */
bool hashcmp(hash_t left, hash_t right) {
    size_t cursor;
    for (cursor = 0; cursor < DIGEST_WORDS; cursor++) {
        if (left.digest[cursor] != right.digest[cursor])
            return false;
    }
    return true;
}
