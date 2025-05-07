#include "bigrsa/bigrsa.h"
#include "sha256/sha256.h"
#include "list_t/list_t.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef BTCINC


#define BTCINC

/* ants and magic numbers */
#define KEY_WORDS (4096 / 64)
#define KEY_SIZE (KEY_WORDS * sizeof(uint64_t))
#define DIGEST_WORDS 8
#define DIGEST_SIZE (DIGEST_WORDS * sizeof(uint32_t))
#define POW_DIFFICULTY 2

#define MODE_TAIL 1
#define MODE_CHAIN 2
#define MODE_HEAD 3

#define BYTE_GENESIS 'g'
#define BYTE_PREVIOUS 'p'
#define BYTE_CHAIN 'c'
#define BYTE_HEAD 'h'
#define BYTE_DELIMITER '|'
#define BYTE_HEAD_DELIMITER '}'

typedef uint64_t buffer_chunk;

typedef struct uint4096_t {
    uint64_t *contents;
} uint4096_t;

typedef struct hash_t {
    uint32_t *digest;
} hash_t;

typedef struct signature_t {
    uint4096_t signature;
    uint4096_t public_key;
} signature_t;

/* Representation of blockchain contents */
typedef struct chain_tail_t {
    hash_t previous_hash;
} chain_tail_t;

typedef struct chain_t {
    /* Hash is hash of content + signature + timestamp*/
    signature_t signature;
    void *content;
    size_t content_size;
    uint64_t timestamp;
    struct chain_t *previous;
    chain_tail_t *tail;
    hash_t previous_hash;
} chain_t;

/* Proof-of-work*/
typedef struct pow_t {
    void *pow;
    size_t pow_size;
} pow_t;

typedef struct chain_head_t {
    uint64_t timestamp;
    pow_t *pow;
    signature_t signature;
    chain_t *previous;
    hash_t previous_hash;
} chain_head_t;

/* Intermediate representation of unsigned content */
typedef struct chain_content_t {
    void *content;
    size_t content_size;
} chain_content_t;

/* Key set to sign */
typedef struct keyset_t {
    uint4096_t public_key;
    uint4096_t private_key;
} keyset_t;

/* 4096_t Convenience Functions */
uint4096_t make_4096_t();
void free_4096_t(uint4096_t to_free);

/* Hashing API */
hash_t hash_signature(signature_t signature);
hash_t hash_chain_head(chain_head_t *chain_head, pow_t *pow);
hash_t hash_chain_content(chain_t *chain);
void free_hash(hash_t to_free);
bool hashcmp(hash_t left, hash_t right);
hash_t hashcpy(hash_t prior);

/* BTCinC API */
pow_t *do_pow(chain_head_t *head);
bool check_pow(chain_head_t *head, pow_t *pow);
chain_tail_t *genesis();
chain_head_t *import_blockchain(char *filename);
chain_tail_t *tail(chain_head_t *prev_head);
chain_t *chain_new(chain_tail_t *tail, chain_content_t *content_to_add,
                   keyset_t *keyset);
chain_t *chain_add(chain_t *chain, chain_content_t *content_to_add,
                   keyset_t *keyset);
chain_head_t *commit_chain(chain_t *chain, keyset_t *keyset);
void export_blockchain(char *filename, chain_head_t *head);
signature_t sign_hash(hash_t *hash_to_sign, keyset_t *keyset);
bool verify_head(chain_head_t *to_verify);
bool verify_chain(chain_t *to_verify);

#endif
