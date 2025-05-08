#include "btcinc.h"

void write_genesis(char *filename) {
    chain_tail_t *genesis_tail;
    chain_t *chain;
    chain_content_t *content;
    chain_head_t *head;
    keyset_t *keyset;
    printf("Make genesis\n");
    genesis_tail = genesis();
    printf("Make content\n");
    content = malloc(sizeof(chain_content_t));
    printf("Read key file\n");
    keyset = read_keyset_from_file("btcinc/keys/unsafe.bad");
    printf("Write content\n");
    content->content = "Here at the genesis block";
    content->content_size = strlen(content->content) + 1;
    chain = chain_new(genesis_tail, content, keyset);
    chain = chain_add(chain, content, keyset);
    printf("Commit chain\n");
    head = commit_chain(chain, keyset);
    printf("Genesis block:\n");

    print_blockchain(head);

    export_blockchain(filename, head);
    printf("Free used variables\n");
    free_blockchain(head);

    printf("Verify genesis chain from %s\n", filename);

    /* Import chain */
    head = import_blockchain(filename);
    if(verify_head(head)) {
        if(DEBUG) printf("Successfully verified genesis\n");
    } else {
        fprintf(stderr, "Unable to verify genesis\n");
    }

    printf("Free used variables\n");
    free_blockchain(head);
}
