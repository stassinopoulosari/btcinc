#include "bigrsa/bigrsa.h"
#include "list_t/list_t.h"
#include "sha256/sha256.h"
#include <dirent.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Definition guard */
#ifndef BTCINC

#define BTCINC
#define NAME_MAX 1024

/* Constants and magic numbers */

/* Number of words in a signature key */
#define KEY_WORDS (4096 / 64)
/* Number of bytes in a signature key */
#define KEY_SIZE (KEY_WORDS * sizeof(uint64_t))
/* Number of words in an SHA256 digest */
#define DIGEST_WORDS (256 / 32)
/* Number of bytes in an SHA256 digest */
#define DIGEST_SIZE (DIGEST_WORDS * sizeof(uint32_t))
/* Number of leading digest digits (hexadecimal) that must be zero for POW to
 * succeed */
#define POW_DIFFICULTY 2

/* Modes to use for reading and writing chains/tails */
#define MODE_TAIL 1
#define MODE_CHAIN 2
#define MODE_HEAD 3

/* Constant markers for block files */
#define BYTE_TAIL 'p'
#define BYTE_CHAIN 'c'
#define BYTE_HEAD 'h'
#define BYTE_DELIMITER '|'
#define BYTE_HEAD_DELIMITER '}'

/* Debug mode for when my code doesn't work :( */
#define DEBUG false
/* Print signatures when printing blocks */
#define PRINT_SIGNATURES false

/* In this house, we use uint64_ts for our buffers */
typedef uint64_t buffer_chunk;

/* Convenience struct to hold entities for 4096_t */
typedef struct uint4096_t {
    /* Pointer to array of KEY_WORDS words */
    uint64_t *contents;
} uint4096_t;

/* Convenience struct to hold hashes */
typedef struct hash_t {
    /* Pointer to array of DIGEST_WORDS words */
    uint32_t *digest;
} hash_t;

/* Convenience struct to hold signatures */
typedef struct signature_t {
    /* RSA-encrypted hash of current block */
    uint4096_t signature;
    /* Public key to verify the above */
    uint4096_t public_key;
} signature_t;

/* Representation of blockchain tail */
typedef struct chain_tail_t {
    /* Hash of the signature of the head of the previous file
     *  ...or all zeroes for the genesis block
     */
    hash_t previous_hash;
} chain_tail_t;

/* Representation of blockchain item */
typedef struct chain_t {
    /* RSA-encrypted hash of current chain */
    signature_t signature;
    /* Pointer to content of `content_size`*/
    void *content;
    size_t content_size;
    /* The 64-bit unix time when the chain was made*/
    uint64_t timestamp;
    /* Exactly one of previous or tail must be NULL */
    struct chain_t *previous;
    chain_tail_t *tail;
    /* Hash of the signature of the previous file's head or of the previous chain
     */
    hash_t previous_hash;
} chain_t;

/* Proof-of-work*/
typedef struct pow_t {
    /* Pointer to content of `pow_size`*/
    void *pow;
    size_t pow_size;
} pow_t;

/* Representation of blockchain head */
typedef struct chain_head_t {
    /* The 64-bit unix time when the chain was committed*/
    uint64_t timestamp;
    /* Proof-of-work */
    pow_t *pow;
    /* RSA-encrypted hash of the current head */
    signature_t signature;
    /* Last content element of the chain */
    chain_t *previous;
    /* Hash of the signature of the previous chain */
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

/* Read a private key, given a filename */
keyset_t *read_keyset_from_file(char *filename);

/* 4096_t Convenience Functions */
uint4096_t make_4096_t();

/* Hashing API */
/* Make an empty hash_t */
hash_t make_hash();
/* Hash the block of a signature */
hash_t hash_signature(signature_t signature);
/* Hash the head of a chain before signing */
hash_t hash_chain_head(chain_head_t *chain_head, pow_t *pow);
/* Hash the content of a chain before signing */
hash_t hash_chain(chain_t *chain);
/* Check whether two hashes are equal */
bool hashcmp(hash_t left, hash_t right);
/* Allocate and copy a new hash */
hash_t hashcpy(hash_t prior);

/* BTCinC API */

chain_content_t *make_content(void *content, size_t content_size);
/* Given a chain head without a Proof-of-Work, find a value that satisfied
 * Proof-of-Work */
pow_t *do_pow(chain_head_t *head);
/* Verify whether a given value satisfied Proof-of-Work. Pass NULL to pow to use
 * the pow in head.*/
bool check_pow(chain_head_t *head, pow_t *pow);
/* Create a genesis block.*/
chain_tail_t *genesis();
/* Create a tail given a previous head.*/
chain_tail_t *tail(chain_head_t *prev_head);
/* Create a new chain from a tail.*/
chain_t *chain_new(chain_tail_t *tail, chain_content_t *content_to_add,
                   keyset_t *keyset);
/* Add an item to a chain.*/
chain_t *chain_add(chain_t *chain, chain_content_t *content_to_add,
                   keyset_t *keyset);
/* Add a head to a chain.*/
chain_head_t *commit_chain(chain_t *chain, keyset_t *keyset);
/* Get the number of a block given its filename */ bool
get_block_number(char *filename, uint64_t *block_number, char *prefix);

/* Serialization */

/* Import a blockchain from a file.*/
chain_head_t *import_blockchain(char *filename);
/* Export a blockchain to a file.*/
void export_blockchain(char *filename, chain_head_t *head);

/* Printing functions */
/* Print a chain item to the console */
void print_chain(chain_t *chain);
void fprint_chain(FILE *out, chain_t *chain);
/* Print a chain head to the console */
void print_chain_head(chain_head_t *head);
void fprint_chain_head(FILE *out, chain_head_t *head);
/* Recursively print a chain to the console */
void print_blockchain(chain_head_t *head);
void fprint_blockchain(FILE *out, chain_head_t *head);
/* Print a chain tail to the console */
void print_chain_tail(chain_tail_t *tail);
void fprint_chain_tail(FILE *out, chain_tail_t *tail);
/* Print a proof-of-work to the console */
void print_pow(pow_t *pow);
void fprint_pow(FILE *out, pow_t *pow);
/* Print a hash to the console */
void print_hash(hash_t to_print);
void fprint_hash(FILE *out, hash_t to_print);
/* Print a 4096-bit number to the console */
void print_4096_t(uint4096_t number);
void fprint_4096_t(FILE *out, uint4096_t number);
/* Print a signature block to the console */
void print_signature(signature_t signature);
void fprint_signature(FILE *out, signature_t signature);

/* Cryptography */

/* Sign a given hash using a private key */
signature_t sign_hash(hash_t hash_to_sign, keyset_t *keyset);
/* Verify a chain head */
bool verify_head(chain_head_t *to_verify);
/* Verify a chain item*/
bool verify_chain(chain_t *to_verify);
/* Verify the entire blockchain */
bool verify_recursive(chain_head_t *blockchain, char *filename);

uint4096_t rsa_encrypt(uint4096_t input, keyset_t *keyset);
uint4096_t rsa_decrypt(uint4096_t input, uint4096_t public_key);
uint4096_t hash_to_4096_t(hash_t input);
uint4096_t copy_uint4096_t(uint4096_t input);

/* Memory management */
void free_blockchain(chain_head_t *head);
void free_signature(signature_t signature);
void free_chain_tail(chain_tail_t *tail);
void free_chain(chain_t *chain);
void free_pow(pow_t *pow);
void free_4096_t(uint4096_t to_free);
void free_hash(hash_t to_free);
void free_chain_content(chain_content_t *content);
void free_keyset(keyset_t *keyset);
/* Scripts */
void write_genesis(char *filename, keyset_t *keyset);
bool get_blocks(char *dirname, char *last_block_name, char *next_block_name);
void add_to_blockchain(char *blockchain_directory, keyset_t *keyset,
                       chain_content_t *content);
void do_day(char *blockchain_directory, keyset_t *keyset);

#endif
