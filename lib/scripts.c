#include "btcinc.h"
#include <dirent.h>

void write_genesis(char *filename, keyset_t *keyset) {
    chain_tail_t *genesis_tail;
    chain_t *chain;
    chain_content_t *content;
    chain_head_t *head;
    printf("Make genesis\n");
    genesis_tail = genesis();
    printf("Make content\n");
    content = malloc(sizeof(chain_content_t));
    printf("Write content\n");
    content->content = "Here at da genesis block";
    content->content_size = strlen(content->content) + 1;
    chain = chain_new(genesis_tail, content, keyset);
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
    if (verify_head(head)) {
        if (DEBUG)
            printf("Successfully verified genesis\n");
    } else {
        fprintf(stderr, "Unable to verify genesis\n");
    }

    printf("Free used variables\n");
    free_blockchain(head);
}

bool get_last_block(char *dirname, char *last_block_name,
                    char *next_block_name) {
    DIR *block_dir;
    struct dirent *current_entity;
    uint64_t current_block = 0, max_block = 0;
    size_t blocks_scanned = 0;
    int sscanf_result;

    block_dir = opendir(dirname);
    if (block_dir == NULL) {
        fprintf(stderr, "Unable to open block directory %s. Exiting.\n", dirname);
        exit(1);
    }
    /* Clear block_name */
    last_block_name[0] = '\0';
    next_block_name[0] = '\0';
    current_entity = readdir(block_dir);
    if (current_entity == NULL) {
        printf("No files in directory\n");
        return false;
    }
    while (current_entity) {
        printf("Scanning: %s\n", current_entity->d_name);
        if ((strcmp(current_entity->d_name, ".") &&
                strcmp(current_entity->d_name, ".."))) {
            sscanf_result =
                sscanf(current_entity->d_name, "block_%lu.block", &current_block);
            printf("Scan result: %d\n", sscanf_result);
            if (sscanf_result == 1) {
                printf("Current block: %lu\n", current_block);
                if (current_block > max_block)
                    max_block = current_block;
                blocks_scanned++;
            }
        }
        current_entity = readdir(block_dir);
    }
    if (blocks_scanned == 0) {
        printf("No valid files\n");
        return false;
    }
    sprintf(last_block_name, "%s/block_%lu.block", dirname, max_block);
    sprintf(next_block_name, "%s/block_%lu.block", dirname, max_block + 1);
    printf("Last block: %s. Next block: %s\n", last_block_name, next_block_name);
    closedir(block_dir);
    return true;
}

void add_to_blockchain(char *blockchain_directory, keyset_t *keyset,
                       chain_content_t *content) {
    char last_block_name[NAME_MAX], next_block_name[NAME_MAX];
    chain_head_t *previous_blockchain;
    chain_head_t *new_blockchain;
    printf("Finding last block in %s...\n", blockchain_directory);
    if (!get_last_block(blockchain_directory, last_block_name, next_block_name)) {
        printf("Unable to find last block\n");
        sprintf(last_block_name, "%s/block_0.block", blockchain_directory);
        printf("Writing genesis to %s\n", last_block_name);
        write_genesis(last_block_name, keyset);
        get_last_block(blockchain_directory, last_block_name, next_block_name);
    }

    printf("Last block was %s\n", last_block_name);
    previous_blockchain = import_blockchain(last_block_name);

    printf("Verifying previous blockchain...\n");
    if (!verify_recursive(previous_blockchain, last_block_name)) {
        printf("Unable to verify... exiting\n");
        exit(1);
    }

    printf("Verified previous blockchain. Creating new blockchain\n");

    new_blockchain = commit_chain(
                         chain_new(tail(previous_blockchain), content, keyset), keyset);

    printf("Created new blockchain<%p>\n", (void *)new_blockchain);
    print_blockchain(new_blockchain);

    printf("Writing blockchain to %s\n", next_block_name);
    export_blockchain(next_block_name, new_blockchain);
    printf("Freeing memory\n");
    free_blockchain(previous_blockchain);
    free_blockchain(new_blockchain);
}

void do_day(char *blockchain_directory, keyset_t *keyset) {
    char content_string[NAME_MAX];
    chain_content_t *content;
    time_t current_time;

    time(&current_time);

    strftime(content_string, NAME_MAX, "Hello from %c!",
             localtime(&current_time));

    content = make_content((void *)content_string, strlen(content_string) + 1);

    add_to_blockchain(blockchain_directory, keyset, content);

    free_chain_content(content);
}
