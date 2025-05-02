#include "btcinc.h"

hash_t hash_with_pow(chain_head_t *head, pow_t *pow) {
	size_t hash_input_size = DIGEST_SIZE + sizeof(uint64_t) + pow->pow_size;
}

bool check_pow(chain_head_t *head, pow_t *pow) {
	/* Low proof of work threshold */

}

pow_t *do_pow(chain_head_t *head) {

}
