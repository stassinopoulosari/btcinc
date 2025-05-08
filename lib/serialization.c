#include "btcinc.h"
#include "list_t/list_t.h"

keyset_t *read_keyset_from_file(char *filename) {
    uint4096_t private, public;
    keyset_t *keyset;
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Unable to open file %s to read key", filename);
        exit(1);
    }
    private = make_4096_t(), public = make_4096_t();
    keyset = malloc(sizeof(keyset_t));
    read_key(file, MODE_PRIVATE, public.contents, private.contents);
    keyset->private_key = private;
    keyset->public_key = public;
    return keyset;
}

chain_tail_t *read_chain_tail_from_file(FILE *file) {
    char tail_indicator = fgetc(file);
    hash_t prev_hash;
    chain_tail_t *tail;
    if (tail_indicator != BYTE_PREVIOUS) {
        fprintf(stderr, "Unexpected tail signifier byte %c, expected %c.\n",
                tail_indicator, BYTE_PREVIOUS);
        exit(1);
    }
    prev_hash = make_hash();
    fread(prev_hash.digest, DIGEST_SIZE, 1, file);
    tail = malloc(sizeof(chain_tail_t));
    tail->previous_hash = prev_hash;
    return tail;
}

chain_t *read_chain_from_file(FILE *file, void *previous, short mode) {
    chain_t *chain;
    char chain_signifier = fgetc(file);
    if (chain_signifier != BYTE_CHAIN) {
        fprintf(stderr, "Unexpected signifier byte %c for a chain, expected %c\n",
                chain_signifier, BYTE_CHAIN);
        exit(1);
    }
    chain = malloc(sizeof(chain_t));
    chain->signature.signature = make_4096_t();
    chain->signature.public_key = make_4096_t();
    fread(&chain->content_size, sizeof(size_t), 1, file);
    chain->content = malloc(chain->content_size);
    fread(chain->content, chain->content_size, 1, file);
    fread(&chain->timestamp, sizeof(uint64_t), 1, file);
    fread(chain->signature.signature.contents, KEY_SIZE, 1, file);
    fread(chain->signature.public_key.contents, KEY_SIZE, 1, file);
    if (mode == MODE_CHAIN) {
        chain->previous = (chain_t *)previous;
        chain->previous_hash = hash_signature(chain->previous->signature);
        chain->tail = NULL;
        return chain;
    }
    chain->previous = NULL;
    chain->tail = (chain_tail_t *)previous;
    chain->previous_hash = hashcpy(chain->tail->previous_hash);

    return chain;
}

chain_head_t *read_chain_head_from_file(FILE *file, chain_t *previous) {
    /* TODO double-check mallocs */
    chain_head_t *head;
    char head_signifier = fgetc(file);
    if (head_signifier != BYTE_HEAD) {
        fprintf(stderr, "Unexpected signifier byte %c for a head, expected %c\n",
                head_signifier, BYTE_HEAD);
        exit(1);
    }
    head = malloc(sizeof(chain_head_t));
    head->signature.signature = make_4096_t();
    head->signature.public_key = make_4096_t();
    head->previous = previous;
    head->previous_hash = hash_signature(previous->signature);
    head->pow = malloc(sizeof(pow_t));
    fread(&head->pow->pow_size, sizeof(size_t), 1, file);
    head->pow->pow = malloc(head->pow->pow_size);
    fread(head->pow->pow, head->pow->pow_size, 1, file);
    fread(&head->timestamp, sizeof(uint64_t), 1, file);
    fread(head->signature.signature.contents, KEY_SIZE, 1, file);
    fread(head->signature.public_key.contents, KEY_SIZE, 1, file);
    return head;
}

short read_delimiter_from_file(FILE *file) {
    char delimiter = fgetc(file);
    if (delimiter == BYTE_DELIMITER) {
        return MODE_CHAIN;
    } else if (delimiter == BYTE_HEAD_DELIMITER) {
        return MODE_HEAD;
    }
    fprintf(stderr, "Unexpected character %c in delimiter, expected %c or %c.\n", delimiter, BYTE_DELIMITER, BYTE_HEAD_DELIMITER);
    exit(1);
}

chain_head_t *import_blockchain(char *filename) {
    FILE *file = fopen(filename, "r");
    chain_tail_t *tail;
    void *previous_item;
    chain_t *current_item = NULL;
    chain_head_t *head;
    /* Read protocol */
    if (file == NULL || fscanf(file, "apsbtcincv0\n") == EOF)
        goto error;
    /* File position is now past the protocol to the tail */
    tail = read_chain_tail_from_file(file);
    if(DEBUG) print_chain_tail(tail);
    previous_item = tail;
    while (read_delimiter_from_file(file) == MODE_CHAIN) {
        if (current_item != NULL)
            previous_item = current_item;
        current_item = read_chain_from_file(
                           file, previous_item, previous_item == tail ? MODE_TAIL : MODE_CHAIN);
        if(DEBUG) print_chain(current_item);
    }
    head = read_chain_head_from_file(file, current_item);
    if(DEBUG) print_chain_head(head);
    fclose(file);
    return head;
error:
    fclose(file);
    fprintf(stderr, "Invalid file at %s\n", filename);
    exit(1);
}

void write_chain_to_file(FILE *file, chain_t *chain) {
    /* Print message length, then message content */
    fputc(BYTE_CHAIN, file);
    fwrite(&chain->content_size, sizeof(size_t), 1, file);
    fwrite(chain->content, chain->content_size, 1, file);
    fwrite(&chain->timestamp, sizeof(uint64_t), 1, file);
    fwrite(chain->signature.signature.contents, KEY_SIZE, 1, file);
    fwrite(chain->signature.public_key.contents, KEY_SIZE, 1, file);
    return;
}

void write_chain_head_to_file(FILE *file, chain_head_t *head) {
    fputc(BYTE_HEAD, file);
    fwrite(&head->pow->pow_size, sizeof(size_t), 1, file);
    fwrite(head->pow->pow, head->pow->pow_size, 1, file);
    fwrite(&head->timestamp, sizeof(uint64_t), 1, file);
    fwrite(head->signature.signature.contents, KEY_SIZE, 1, file);
    fwrite(head->signature.public_key.contents, KEY_SIZE, 1, file);
    return;
}

void write_chain_tail_to_file(FILE *file, chain_tail_t *tail) {
    /* We've already printed protocol */
    /* Previous file digest:
     * 	- If this is the genesis block, print TAIL
     */
    /*if (tail->previous_hash.digest == NULL) {
        fputc(BYTE_GENESIS, file);
        return;
    }*/
    /* Otherwise, write the hash digest */
    fputc(BYTE_PREVIOUS, file);
    fwrite(tail->previous_hash.digest, DIGEST_SIZE, 1, file);
}

void write_item_delimiter_to_file(FILE *file) {
    fputc(BYTE_DELIMITER, file);
}
void write_head_delimiter_to_file(FILE *file) {
    fputc(BYTE_HEAD_DELIMITER, file);
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

    if(DEBUG) print_chain_head(head);

    if(DEBUG) printf("Add chain to list\n");

    if(DEBUG) print_chain(current_item);

    while (tail == NULL) {
        /* Insert item to the beginning of the list */
        list_insert(
            chain_items,
            0,
            (void *)current_item
        );
        if(DEBUG) printf("Insert succeeded\n");
        if (current_item->previous == NULL)
            tail = current_item->tail;
        current_item = current_item->previous;
    }

    if (tail == NULL) {
        fprintf(stderr, "Error: Invalid chain.\n");
        exit(1);
    }

    if (file == NULL) {
        fprintf(stderr, "Error: Could not open %s to read blockchain.\n", filename);
        exit(1);
    }
    if(DEBUG) printf("Printing header to file\n");
    fprintf(file, "apsbtcincv0\n");
    if(DEBUG) printf("Printing chain tail to file\n");
    write_chain_tail_to_file(file, tail);
    write_item_delimiter_to_file(file);
    while (!list_is_empty(chain_items)) {
        current_item = list_pop(chain_items, 0);
        if(DEBUG) print_chain(current_item);
        write_chain_to_file(file, current_item);
        if (!list_is_empty(chain_items))
            write_item_delimiter_to_file(file);
        if(DEBUG) printf("Successfully printed chain item.\n");
    }
    write_head_delimiter_to_file(file);
    if(DEBUG) printf("Successfully printed head.\n");

    write_chain_head_to_file(file, head);
    fclose(file);
    if(DEBUG) printf("Successfully closed file.\n");
}
