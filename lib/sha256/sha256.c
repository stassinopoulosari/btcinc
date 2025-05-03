#include "sha256.h"

/*macros.c*/
#define CHOICE(e, f, g) (((e) & (f)) | ((~e) & (g)))
#define MEDIAN(e, f, g) (((e) & (f)) | ((e) & (g)) | ((f) & (g)))
#define ROTATE(bits, shift) \
(((bits) >> (shift)) | ((bits) << (sizeof(bits) * 8 - (shift))))

/*rounds.c*/
uint32_t sha256_rounds[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,
    0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,
    0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,
    0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,
    0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,
    0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,
    0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,
    0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,
    0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

uint32_t sha256_initial_hashes[8] = {
    0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,
    0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19
};


/* write a given size variable to the end of a chunk */
void write_size_to_buffer(uint64_t size, uint8_t chunk[64]) {
    int buffer_pos;
    int factor;
    for(buffer_pos = 56; buffer_pos < 64; buffer_pos++) {
        factor = (63 - buffer_pos) * 8;
        /* shift right by number of bits offset, do bitwise & with 0b11111111 */

        chunk[buffer_pos] = (size >> factor) & 255;
    }
}

void hash_with_chunk(uint8_t chunk[64], uint32_t *hashes) {
    int cursor;
    uint32_t message_schedule[64];
    /* Working variables for copy loop */
    uint32_t s0;
    uint32_t s1;
    /* Working variables for compression function */
    uint32_t a = hashes[0];
    uint32_t b = hashes[1];
    uint32_t c = hashes[2];
    uint32_t d = hashes[3];
    uint32_t e = hashes[4];
    uint32_t f = hashes[5];
    uint32_t g = hashes[6];
    uint32_t h = hashes[7];
    uint32_t S1;
    uint32_t ch;
    uint32_t temp1;
    uint32_t S0;
    uint32_t maj;
    uint32_t temp2;
    /* Copy the chunk into the first 16 positions of the message schedule */
    for(cursor = 0; cursor < 16; cursor++) {
        message_schedule[cursor] =
            (chunk[cursor * 4] << 24)
            + (chunk[cursor * 4 + 1] << 16)
            + (chunk[cursor * 4 + 2] << 8)
            + (chunk[cursor * 4 + 3]);
    }
    /* Do the weird shifting stuff for the rest of the message schedule */
    for(cursor = 16; cursor < 64; cursor++) {
        s0 = ROTATE(message_schedule[cursor - 15], 7)
             ^ ROTATE(message_schedule[cursor - 15], 18)
             ^ (message_schedule[cursor - 15] >> 3);
        s1 = ROTATE(message_schedule[cursor - 2], 17)
             ^ ROTATE(message_schedule[cursor - 2], 19)
             ^ (message_schedule[cursor - 2] >> 10);
        message_schedule[cursor] = message_schedule[cursor - 16] + s0 + message_schedule[cursor - 7] + s1;
    }
    /* Compressive fuckery */
    for(cursor = 0; cursor < 64; cursor++) {
        S1 = ROTATE(e, 6) ^ ROTATE(e, 11) ^ ROTATE(e, 25);
        ch = (e & f) ^ (~e & g);
        temp1 = h + S1 + ch + sha256_rounds[cursor] + message_schedule[cursor];
        S0 = ROTATE(a, 2) ^ ROTATE(a, 13) ^ ROTATE(a, 22);
        maj = (a & b) ^ (a & c) ^ (b & c);
        temp2 = S0 + maj;
        h = g;
        g = f;
        f = e;
        e = d + temp1;
        d = c;
        c = b;
        b = a;
        a = temp1 + temp2;
    }
    /* Add working variables to hashes (stored in global variable from hashes.c) */
    hashes[0] = hashes[0] + a;
    hashes[1] = hashes[1] + b;
    hashes[2] = hashes[2] + c;
    hashes[3] = hashes[3] + d;
    hashes[4] = hashes[4] + e;
    hashes[5] = hashes[5] + f;
    hashes[6] = hashes[6] + g;
    hashes[7] = hashes[7] + h;
}

uint32_t *hash(void *input, size_t input_size) {
    uint8_t chunk[64]; /* 512-byte chunk */
    uint32_t *hashes = malloc(8 * sizeof(uint32_t));
    size_t cursor = 0;
    size_t chunk_size;
    int generate_extra_chunk = 1;
    short added_terminator = 0;
    int digest_index;
    memcpy(hashes, sha256_initial_hashes, 8 * sizeof(uint32_t));
    /* Null check for input */
    do {
        if(added_terminator == 1) {
            chunk_size = 0;
        } else {
            chunk_size = (input_size - cursor) > 64 ?
                         (input_size - cursor) : 64;
            memcpy(chunk, input + cursor, chunk_size);
            cursor += chunk_size;
        }
        /* Chunk size == 64 --> Move on to the next chunk, even if the size is 0
           Chunk size on (56, 64) --> Add 128 terminator and add an empty chunk
           Chunk size on [0, 56] --> Add 128 terminator and 8 bit message size descriptor */
        if(chunk_size == 64) {}
        else if(added_terminator == 0) {
            /* Add 0b10000000 to chunk (128) */
            chunk[chunk_size] = 128;
            added_terminator = 1;
            memset(chunk + chunk_size + 1, 0, 64 - (1 + chunk_size));
        } else {
            /* If the input size is less than 64 and we have added the terminator (extra chunk), clear out the entire chunk) */
            memset(chunk, 0, 64);
        }
        /* Zero out input buffer between the 128 terminator and the end of the buffer to continue processing */
        /* Starting position -> pointer + input_size + 1 */
        /* Total bytes -> 64, so we will be setting 64 - 1 (terminator) - input size bytes to 0 */
        /* This should do nothing to a full buffer */
        if(chunk_size < 56) {
            /* Add message size descriptor */
            write_size_to_buffer(((uint64_t) input_size) * 8, chunk);
            generate_extra_chunk = 0;
            /* Stop us from generating extra chunk and terminate the loop */
        }
        /* Do the actual continuation of the computation */
        hash_with_chunk(chunk, hashes);
        /* do while feels wrong */
    } while (chunk_size == 64 || generate_extra_chunk == 1);

    return hashes;
}
