#import "btcinc.h"
#include "list_t/list_t.h"
#include <stdio.h>

chain_tail_t *read_chain_tail_from_file(FILE *file) {
    char tail_indicator = fgetc(file);
    hash_t prev_hash;
    chain_tail_t *tail;
    if (tail_indicator == BYTE_GENESIS)
        return genesis();
    prev_hash.digest = malloc(DIGEST_SIZE);
    fread(prev_hash.digest, DIGEST_SIZE, 1, file);
    tail = malloc(sizeof(chain_tail_t));
    tail->previous_hash = prev_hash;
    return tail;
}

chain_t *read_chain_from_file(FILE *file, void *previous, short mode) {
    chain_t *chain;
    if (fgetc(file) != BYTE_CHAIN) {
        fprintf(stderr, "Invalid format\n");
        exit(1);
    }
    chain = malloc(sizeof(chain_t));
    chain->signature.signature = make_4096_t();
    chain->signature.public_key = make_4096_t();
    chain->previous_hash.digest = malloc(DIGEST_SIZE);
    fread(&chain->content_size, sizeof(size_t), 1, file);
    chain->content = malloc(chain->content_size);
    fread(chain->content, chain->content_size, 1, file);
    fread(&chain->timestamp, sizeof(uint64_t), 1, file);
    fread(chain->previous_hash.digest, DIGEST_SIZE, 1, file);
    fread(chain->signature.signature.contents, KEY_SIZE, 1, file);
    fread(chain->signature.public_key.contents, KEY_SIZE, 1, file);
    if (mode == MODE_CHAIN) {
        chain->previous = (chain_t *)previous;
        chain->tail = NULL;
        return chain;
    }
    chain->previous = NULL;
    chain->tail = (chain_tail_t *)previous;
    return chain;
}

chain_head_t *read_chain_head_from_file(FILE *file, chain_t *previous) {
    /* TODO double-check mallocs */
    chain_head_t *head;
    if (fgetc(file) != BYTE_HEAD) {
        fprintf(stderr, "Invalid format\n");
        exit(1);
    }
    head = malloc(sizeof(chain_head_t));
    head->signature.signature = make_4096_t();
    head->signature.public_key = make_4096_t();
    head->previous = previous;
    head->previous_hash.digest = malloc(DIGEST_SIZE);
    fread(&head->pow->pow_size, sizeof(size_t), 1, file);
    head->pow->pow = malloc(head->pow->pow_size);
    fread(head->pow->pow, head->pow->pow_size, 1, file);
    fread(head->previous_hash.digest, DIGEST_SIZE, 1, file);
    fread(head->signature.signature.contents, KEY_SIZE, 1, file);
    fread(head->signature.public_key.contents, KEY_SIZE, 1, file);
    return head;
}

short read_delimiter(FILE *file) {
    char delimiter = fgetc(file);
    if (delimiter == BYTE_DELIMITER) {
        return MODE_CHAIN;
    } else if (delimiter == BYTE_HEAD_DELIMITER) {
        return MODE_HEAD;
    }
    fprintf(stderr, "Unexpected character in file delimiter\n");
    exit(1);
}

chain_head_t *import_blockchain(char *filename) {
    FILE *file = fopen(filename, "r");
    chain_tail_t *tail;
    chain_t *current_item = NULL;
    chain_head_t *head;
    /* Read protocol */
    if (file == NULL || fscanf(file, "apsbtcincv0\n") == EOF)
        goto error;
    /* File position is now past the protocol to the tail */
    tail = read_chain_tail_from_file(file);
    while (read_delimiter(file) == MODE_CHAIN) {
        current_item =
            read_chain_from_file(file, current_item != NULL ? current_item : tail,
                                 current_item != NULL ? MODE_CHAIN : MODE_HEAD);
    }
    head = read_chain_head_from_file(file, current_item);
    return NULL;
error:
    fclose(file);
    fprintf(stderr, "Invalid file at %s\n", filename);
    exit(1);
}

void print_chain_to_file(FILE *file, chain_t *chain) {
    hash_t working_hash;
    /* Print message length, then message content */
    fputc(BYTE_CHAIN, file);
    fwrite(&chain->content_size, sizeof(size_t), 1, file);
    fwrite(chain->content, chain->content_size, 1, file);
    fwrite(&chain->timestamp, sizeof(uint64_t), 1, file);
    working_hash = hash_chain_content(chain);
    fwrite(working_hash.digest, DIGEST_SIZE, 1, file);
    free_hash(working_hash);
    fwrite(chain->signature.signature.contents, KEY_SIZE, 1, file);
    fwrite(chain->signature.public_key.contents, KEY_SIZE, 1, file);
    return;
}

void print_chain_head_to_file(FILE *file, chain_head_t *head) {
    hash_t working_hash;
    fputc(BYTE_HEAD, file);
    fwrite(&head->pow->pow_size, sizeof(size_t), 1, file);
    fwrite(head->pow->pow, head->pow->pow_size, 1, file);
    working_hash = hash_chain_head(head, NULL);
    fwrite(working_hash.digest, DIGEST_SIZE, 1, file);
    free_hash(working_hash);
    fwrite(head->signature.signature.contents, KEY_SIZE, 1, file);
    fwrite(head->signature.public_key.contents, KEY_SIZE, 1, file);
    return;
}

void print_chain_tail_to_file(FILE *file, chain_tail_t *tail) {
    /* We've already printed protocol */
    /* Previous file digest:
     * 	- If this is the genesis block, print TAIL
     */
    if (tail->previous_hash.digest == NULL) {
        fputc(BYTE_GENESIS, file);
        return;
    }
    /* Otherwise, write the hash digest */
    fputc(BYTE_PREVIOUS, file);
    fwrite(tail->previous_hash.digest, DIGEST_SIZE, 1, file);
}

void print_item_delimiter(FILE *file) {
    fputc(BYTE_DELIMITER, file);
}
void print_head_delimiter(FILE *file) {
    fputc(BYTE_HEAD, file);
}

void export_blockchain(char *filename, chain_head_t *head) {
    FILE *file = fopen(filename, "w");
    list_t chain_items = list_new();
    chain_t *current_item = head->previous;
    chain_tail_t *tail = NULL;

    if (!verify_head(head)) {
        fprintf(stderr, "Error: Chain could not be verified.\n");
        exit(1);
    }

    while (current_item != NULL) {
        list_append(chain_items, current_item);
        current_item = current_item->previous;
        tail = current_item->tail;
    }
    list_reverse(chain_items);
    if (tail == NULL) {
        fprintf(stderr, "Error: Invalid chain.\n");
        exit(1);
    }

    if (file == NULL) {
        fprintf(stderr, "Error: Could not open %s to read blockchain.\n", filename);
        exit(1);
    }
    fprintf(file, "apsbtcincv0\n");
    print_chain_tail_to_file(file, tail);
    while (!list_is_empty(chain_items)) {
        current_item = list_pop(chain_items, 0);
        print_item_delimiter(file);
        print_chain_to_file(file, current_item);
    }
    print_head_delimiter(file);
    print_chain_head_to_file(file, head);
    fclose(file);
}
