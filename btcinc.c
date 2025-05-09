#include "lib/btcinc.h"

/* Chain format: */

int main() {
    keyset_t *keyset;
    printf("Read key file\n");
    keyset = read_keyset_from_file("keys/unsafe.bad");
    do_day("bin/blocks", keyset);
    free_keyset(keyset);
    return 0;
}
