#include "btcinc.h"

/* Hash the content of a chain (for signing) */
hash_t hash_chain(chain_t *chain) {
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

/* Hash the head of a chain (for signing) */
hash_t hash_chain_head(chain_head_t *chain_head, pow_t *pow) {
  hash_t chain_head_hash;
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
  chain_head_hash.digest = hash(hash_input, hash_input_size);
  free(hash_input);
  return chain_head_hash;
}

/* Hash a signature */
hash_t hash_signature(signature_t signature) {
  /* Signature content will be KEY_SIZE long */
  hash_t signature_hash;
  uint32_t *digest = hash(signature.signature.contents, KEY_SIZE);
  signature_hash.digest = digest;
  return signature_hash;
}

/* Allocate an empty hash */
hash_t make_hash() {
  hash_t out;
  out.digest = malloc(DIGEST_SIZE);
  memset(out.digest, 0, DIGEST_SIZE);
  return out;
}

/* Copy `prior` to a new hash */
hash_t hashcpy(hash_t prior) {
  hash_t new;
  new = make_hash();
  memcpy(new.digest, prior.digest, DIGEST_SIZE);
  return new;
}
