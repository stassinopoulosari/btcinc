#include "btcinc.h"
#include <stdio.h>

void fprint_signature(FILE *out, signature_t signature) {
    fprintf(out, "  Signature\n");
    fprintf(out, "    Block<");
    if (signature.signature.contents == NULL)
        fprintf(out, "NULL>");
    else {
        fprintf(out, "%p>", (void *)signature.signature.contents);
        if (PRINT_SIGNATURES)
            fprint_4096_t(out, signature.signature);
    }
    fputc('\n', out);
    fprintf(out, "    Public Key<");
    if (signature.public_key.contents == NULL)
        fprintf(out, "NULL>");
    else {
        fprintf(out, "%p>", (void *)signature.public_key.contents);
        if (PRINT_SIGNATURES)
            fprint_4096_t(out, signature.public_key);
    }
}

void fprint_chain_tail(FILE *out, chain_tail_t *tail) {
    fprintf(out, "Chain tail:\n");
    if (tail == NULL) {
        fprintf(out, "  NULL\n");
        return;
    }
    fprintf(out, "  Previous hash\n{\n");
    fprint_hash(out, tail->previous_hash);
    fprintf(out, "}\n");
    fputc('\n', out);
}

void fprint_chain_head(FILE *out, chain_head_t *head) {
    fprintf(out, "Chain head:\n");
    if (head == NULL) {
        fprintf(out, "  NULL\n");
        return;
    }
    fprintf(out, "  Proof of work    ");
    if (head->pow == NULL) {
        fprintf(out, "NULL\n");
    } else {
        fprintf(out, "pow<%p>\n", (void *)head->pow);
        fprintf(out, "    Content of size %lu\n", head->pow->pow_size);
    }
    fprintf(out, "  Previous         ");
    if (head->previous == NULL) {
        fprintf(out, "NULL");
    } else {
        fprintf(out, "Chain<%p>", (void *)head->previous);
    }
    fputc('\n', out);
    fprintf(out, "  Timestamp        %lu\n", head->timestamp);
    fprint_signature(out, head->signature);
    fprintf(out, "    Public Key<");
    if (head->signature.public_key.contents == NULL)
        fprintf(out, "NULL>");
    else {
        fprintf(out, "%p>", (void *)head->signature.public_key.contents);
        if (PRINT_SIGNATURES)
            fprint_4096_t(out, head->signature.public_key);
    }
    fputc('\n', out);
    fprintf(out, "  Previous hash \n{\n");
    fprint_hash(out, head->previous_hash);
    fprintf(out, "}\n");
    if(PRINT_SIGNATURES) {
        fprintf(out, "  Raw proof-of-work:\n");
        fprint_pow(out, head->pow);
    }
    fputc('\n', out);
}

void fprint_chain_recursive(FILE *out, chain_t *chain) {
    if(chain->previous != NULL) {
        fprint_chain_recursive(out, chain->previous);
    } else {
        fprint_chain_tail(out, chain->tail);
    }
    fprint_chain(out, chain);
}
void fprint_blockchain(FILE *out, chain_head_t *head) {
    fprint_chain_recursive(out, head->previous);
    fprint_chain_head(out, head);
}
void fprint_pow(FILE *out, pow_t *pow) {
    size_t cursor;
    for (cursor = 0; cursor < pow->pow_size / sizeof(buffer_chunk); cursor++) {
        fprintf(out, "%016lx ", ((buffer_chunk *)pow->pow)[cursor]);
        if (cursor % 2 == 1)
            fputc('\n', out);
    }
    fputc('\n', out);
}


void fprint_chain(FILE *out, chain_t *chain) {
    fprintf(out, "Chain:\n");
    if (chain == NULL) {
        fprintf(out, "  NULL\n");
        return;
    }
    fprintf(out, "  Content of size  %lu\n", chain->content_size);
    fprintf(out, "  Previous         ");
    if (chain->previous == NULL) {
        fprintf(out, "Tail<");
        if (chain->tail == NULL) {
            fprintf(out, "NULL");
        } else {
            fprintf(out, "%p", (void *)chain->tail);
        }
        fprintf(out, ">");
    } else {
        fprintf(out, "Chain<%p>", (void *)chain->previous);
    }
    fputc('\n', out);
    fprintf(out, "  Timestamp        %lu\n", chain->timestamp);
    fprint_signature(out, chain->signature);
    fputc('\n', out);
    fprintf(out, "  Previous hash \n{\n");
    fprint_hash(out, chain->previous_hash);
    fprintf(out, "}\n");
    fputc('\n', out);
}

void fprint_hash(FILE *out, hash_t hash) {
    size_t cursor;
    fprintf(out, "Hash <%p>\n", (void *)hash.digest);
    if (hash.digest == NULL) {
        fprintf(out, "  NULL\n");
        return;
    }
    for (cursor = 0; cursor < DIGEST_WORDS; cursor++) {
        fprintf(out, "  %08x", hash.digest[cursor]);
        if (cursor % 2 == 1) {
            fputc('\n', out);
        }
    }
}

void fprint_4096_t(FILE *out, uint4096_t number) {
    size_t iterator;

    for (iterator = 0; iterator < KEY_WORDS; iterator++) {
        if (number.contents == NULL) {
            /* so graceful */
            fprintf(out, "NULLNULLNULLNULL ");
        } else {
            /* Print each word with a space */
            fprintf(out, "%016lx ", number.contents[S - 1 - iterator]);
        }
        /* newline every other word for readability */
        if (iterator % 2 == 1) {
            fprintf(out, "\n");
        }
    }
    /* Cherry on top */
    fprintf(out, "\n");
}
