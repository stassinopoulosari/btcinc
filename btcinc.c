#include "lib/btcinc.h"

/* Chain format: */

int main() {
    keyset_t *keyset;
    printf("Read key file\n");
    keyset = read_keyset_from_file("btcinc/keys/unsafe.bad");
    do_day("btcinc/bin/blocks", keyset);
    free_keyset(keyset);
    return 0;
}
