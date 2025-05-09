#include "math_t/math_t.h"

#include <ctype.h>
#include <stdint.h>
#include <string.h>
/* I need this or my linter will go insane */
#ifdef __APPLE__
#include "/opt/homebrew/include/gmp.h"
#else
#include <gmp.h>
#endif

/* Strings */
#define FILENAME_PUBLIC "unsafe.pub"
#define FILENAME_PRIVATE "unsafe.bad"
#define DASHES "-----"
#define PUBLIC_HEADER DASHES "BEGIN UNSAFE PUBLIC KEY" DASHES "\n"
#define PRIVATE_HEADER DASHES "BEGIN UNSAFE PRIVATE KEY" DASHES "\n"
#define PUBLIC_FOOTER DASHES "END UNSAFE PUBLIC KEY" DASHES "\n"
#define PRIVATE_FOOTER DASHES "END UNSAFE PRIVATE KEY" DASHES "\n"
#define RANDOM_PATH "/dev/random"

#define STR_USAGE                                                              \
  "Usage: bigrsa.o <mode> <input> <output>\n\
\tmode:\t-d to decrypt\
\t\t-e to encrypt\n\
\tinput:\tpath to a file to encrypt or decrypt\
\toutput:\tpath to a file to store the encrypted or decrypted text"

/* Magic numbers */
#define STD_ENCRYPTOR 0x10001
#define MAX_FIND_PRIMES_ITER 100
#define RANDOM_WORDS S / 2

/* Key modes */
#define MODE_PRIVATE 0
#define MODE_PUBLIC 1

#define BIGS 128
#define BIG_BYTES 128 * sizeof(uint64_t)

/* Helper functions from 4096_t that we're reusing */
short big_comparison(uint64_t *left, uint64_t *right);
uint64_t big_log(uint64_t *in);
uint64_t get_msb(uint64_t *n);
uint64_t big_right_shift(uint64_t *outperand);
uint64_t big_left_shift(uint64_t *outperand);
short bigger_comparison(uint64_t *left, uint64_t *right);


/* rsalib.c */
uint64_t big_out(uint64_t *n);
int big_prime(uint64_t *out, uint8_t words);
int big_extended_gcd(uint64_t *left, uint64_t *right, uint64_t *bezout_s,
                     short *bezout_s_sign, uint64_t *gcd);
int big_lcm(uint64_t *left, uint64_t *right, uint64_t *out);
int big_find_d(uint64_t e, uint64_t *lambda, uint64_t *secret_d);
int fprint_big(FILE *output_file, uint64_t *number);
int write_key(FILE *output_file, short mode, uint64_t *public_n,
              uint64_t encryptor, uint64_t *secret_d);
int read_key(FILE *file_handler, short mode, uint64_t *public_n,
             uint64_t *transcryptor);
int input_read(FILE *file_handler, uint64_t *out);
int output_write(FILE *file_handler, uint64_t *out, short shift);
int big_modexp(uint64_t *base, uint64_t *exponent, uint64_t *modulus,
               uint64_t *out);
