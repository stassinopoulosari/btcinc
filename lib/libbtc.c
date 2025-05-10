#include "btcinc.h"
#include <stdio.h>

/* Allocate an empty 4096-bit number */
uint4096_t make_4096_t() {
    uint4096_t out;
    out.contents = malloc(KEY_SIZE);
    memset(out.contents, 0, KEY_SIZE);
    return out;
}

/* Create a genesis tail with an empty hash */
chain_tail_t *genesis() {
    chain_tail_t *chain_tail = malloc(sizeof(chain_tail_t));
    if (chain_tail == NULL)
        exit(1);
    chain_tail->previous_hash = make_hash();
    memset(chain_tail->previous_hash.digest, 0, DIGEST_SIZE);
    return chain_tail;
}

/* Wrapper for big_modexp using uint4096_ts */
uint4096_t rsa_encrypt(uint4096_t input, keyset_t *keyset) {
    uint4096_t encrypted_value = make_4096_t();
    big_modexp(input.contents, keyset->private_key.contents,
               keyset->public_key.contents, encrypted_value.contents);
    return encrypted_value;
}

/* Wrapper for big_modexp using uint4096_ts */
uint4096_t rsa_decrypt(uint4096_t input, uint4096_t public_key) {
    uint4096_t decrypted_value = make_4096_t(), transcryptor = make_4096_t();
    transcryptor.contents[0] = STD_ENCRYPTOR;
    if (DEBUG)
        printf("Do big_modexp\n");
    big_modexp(input.contents, transcryptor.contents, public_key.contents,
               decrypted_value.contents);
    free_4096_t(transcryptor);
    return decrypted_value;
}

/* Convert a hash to a uint4096_t */
uint4096_t hash_to_4096_t(hash_t input) {
    uint4096_t output = make_4096_t();
    memcpy(output.contents, input.digest, DIGEST_SIZE);
    return output;
}

/* Copy a uint4096_t to another uint4096_t */
uint4096_t copy_uint4096_t(uint4096_t input) {
    uint4096_t output = make_4096_t();
    memcpy(output.contents, input.contents, KEY_SIZE);
    return output;
}

/* Sign a hash */
signature_t sign_hash(hash_t hash_to_sign, keyset_t *keyset) {
    signature_t output;
    uint4096_t signature_input = hash_to_4096_t(hash_to_sign);
    output.public_key = copy_uint4096_t(keyset->public_key);
    output.signature = rsa_encrypt(signature_input, keyset);
    free_4096_t(signature_input);
    return output;
}

/* Create a new tail given a previous head */
chain_tail_t *tail(chain_head_t *prev_head) {
    chain_tail_t *tail = malloc(sizeof(chain_tail_t));
    tail->previous_hash = hash_signature(prev_head->signature);
    return tail;
}



/* Add a new chain item to either a tail or to a previous item */
chain_t *_chain_add(void *previous, short mode, chain_content_t *content_to_add,
                    keyset_t *keyset) {
    chain_t *chain = malloc(sizeof(chain_t));
    hash_t chain_hash;
    if (DEBUG)
        printf("Set previous and tail of new chain\n");
    chain->previous = NULL;
    chain->tail = NULL;
    /* A tail doesn't have a hash in itself because it's really a reference to a
     * previous head */
    if (mode == MODE_TAIL) {
        if (DEBUG)
            printf("Set previous hash\n");
        chain->previous_hash = hashcpy(((chain_tail_t *)previous)->previous_hash);
        chain->tail = (chain_tail_t *)previous;
    } else if (mode == MODE_CHAIN) {
        chain->previous_hash = hash_signature(((chain_t *)previous)->signature);
        chain->previous = (chain_t *)previous;
    }
    if (DEBUG)
        printf("Allocate memory for the content\n");
    chain->content = malloc(content_to_add->content_size);
    if (DEBUG)
        printf("Copy the content\n");
    memcpy(chain->content, content_to_add->content, content_to_add->content_size);
    chain->content_size = content_to_add->content_size;
    chain->timestamp = (uint64_t)time(NULL);
    /* Sign chain content */
    if (DEBUG)
        printf("Hash the chain content\n");
    chain_hash = hash_chain(chain);
    if (DEBUG)
        printf("Sign the hash\n");
    chain->signature = sign_hash(chain_hash, keyset);
    return chain;
}

/* Add a new chain item to a tail */
chain_t *chain_new(chain_tail_t *tail, chain_content_t *content_to_add,
                   keyset_t *keyset) {
    return _chain_add(tail, MODE_TAIL, content_to_add, keyset);
}

/* Add a new chain item to a previous item */
chain_t *chain_add(chain_t *chain, chain_content_t *content_to_add,
                   keyset_t *keyset) {
    return _chain_add(chain, MODE_CHAIN, content_to_add, keyset);
}

/* Add a new chain head*/
chain_head_t *commit_chain(chain_t *chain, keyset_t *keyset) {
    chain_head_t *head = malloc(sizeof(chain_head_t));
    hash_t head_hash;
    /* Hash the signature of the previous item */
    head->previous_hash = hash_signature(chain->signature);
    head->previous = chain;
    head->timestamp = (uint64_t)time(NULL);
    /* Do the pow */
    if (DEBUG)
        printf("Do the POW\n");
    head->pow = do_pow(head);
    head_hash = hash_chain_head(head, NULL);
    head->signature = sign_hash(head_hash, keyset);
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

chain_content_t *make_content(void *content, size_t content_size) {
    chain_content_t *chain_content = malloc(sizeof(chain_content_t));
    chain_content->content_size = content_size;
    chain_content->content = malloc(content_size);
    memcpy(chain_content->content, content, content_size);
    return chain_content;
}

bool get_block_number(char *filename, uint64_t *block_number, char *prefix) {
    size_t strpos, inner_iter, maxlen;
    char last_five[6] = {0, 0, 0, 0, 0, 0};
    bool block_found = false;
    maxlen = strlen(filename);
    for(strpos = 0; strpos + 7 < maxlen; strpos++) {
        for(inner_iter = 0; inner_iter < 4; inner_iter++) {
            last_five[inner_iter] = last_five[inner_iter + 1];
        }
        last_five[4] = filename[strpos];
        if(!strcmp(last_five, "block") && filename[strpos + 1] == '_') {
            strpos += 2;
            block_found = true;
            break;
        }
    }
    if(!block_found)
        return false;
    strcpy(prefix, filename);
    prefix[strpos] = '\0';
    /* Iter is now after "block_" */
    if(sscanf(filename + strpos, "%lu", block_number) != 1) return false;
    return true;
}
