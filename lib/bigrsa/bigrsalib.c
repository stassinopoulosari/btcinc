#include "./bigrsa.h"

/* Rewrote the bigout function to make sense (and to handle null values) */
uint64_t big_out(uint64_t *number) {
    size_t iterator;

    for (iterator = 0; iterator < S; iterator++) {
        if (number == NULL) {
            /* so graceful */
            printf("NULLNULLNULLNULL ");
        } else {
            /* Print each word with a space */
            printf("%016lx ", number[S - 1 - iterator]);
        }
        /* newline every other word for readability */
        if (iterator % 2 == 1) {
            printf("\n");
        }
    }
    /* Cherry on top */
    printf("\n");
    return 0;
}

/* Read in a 4096_t from a file */
uint64_t big_in(FILE *file_handler, uint64_t *out) {
    size_t iterator;
    /* Clear output */
    memset(out, 0, BYTES);
    /* Fill in the output backwards */
    for (iterator = 0; iterator < S; iterator++) {
        /* Fail out if we hit the end of file */
        if (fscanf(file_handler, "%016lx", out + (S - 1 - iterator)) == EOF) {
            return 1;
        }
    }
    return 0;
}

short comparison_ui(uint64_t *big, uint64_t lil) {
    uint64_t lil_wrapper[S];
    memset(lil_wrapper, 0, BYTES);
    lil_wrapper[0] = lil;
    return big_comparison(big, lil_wrapper);
}

short bigger_comparison_ui(uint64_t *bigger, uint64_t lil) {
    uint64_t lil_wrapper[BIGS];
    memset(lil_wrapper, 0, BIG_BYTES);
    lil_wrapper[0] = lil;
    return t_comparison(BIGS, bigger, lil_wrapper);
}

short is_zero(uint64_t *operand) {
    size_t iterator;
    for (iterator = 0; iterator < S; iterator++) {
        if (operand[iterator] != 0) {
            return 0;
        }
    }
    return 1;
}


/* Code taken from prigmp from Calvin */
int big_prime(uint64_t *out, uint8_t words) {
    /* Initialise libgmp pointer */
    mpz_t m;
    FILE *fp;
    /* Clear buffer */
    memset(out, 0, BYTES);
    /* Open pointer to /dev/urandom */
    fp = fopen(RANDOM_PATH, "r");
    /* Read out `words` 64-bit integers to out */
    fread(out, sizeof(uint64_t), words, fp);
    fclose(fp);
    /* Initialize m and set its value to the random bits we just read */
    mpz_init(m);
    /* Import S 64-bit words in reverse order, host endianness from out to m */
    mpz_import(m, S, -1, sizeof(uint64_t), 0, 0, out);
    mpz_nextprime(m, m);
    /* Import all 64-bit words in reverse order, host endianness from m to out */
    mpz_export(out, NULL, -1, sizeof(uint64_t), 0, 0, m);
    /* Clear libgmp pointer to be a good steward */
    mpz_clear(m);
    return 0;
}

/* Bezout t sign will be 0 for positive, 1 for negative  */
int big_extended_gcd(uint64_t *left, uint64_t *right, uint64_t *bezout_s,
                     short *bezout_s_sign, uint64_t *gcd) {
    uint64_t s[S], old_s[S], t[S], old_t[S], r[S], old_r[S], swap[S], quotient[S],
             multiplicant[S];
    short sign_s = 0, sign_t = 0, sign_r = 0, sign_old_s = 0, sign_old_t = 0,
                                                              sign_old_r = 0, sign_swap = 0, sign_quotient = 0, sign_multiplicant = 0;
    memset(s, 0, BYTES);
    memset(old_s, 0, BYTES);
    memset(t, 0, BYTES);
    memset(old_t, 0, BYTES);
    memset(r, 0, BYTES);
    memset(old_r, 0, BYTES);
    memset(swap, 0, BYTES);
    memset(quotient, 0, BYTES);
    memset(multiplicant, 0, BYTES);
    memset(bezout_s, 0, BYTES);
    memset(gcd, 0, BYTES);

    old_s[0] = 1;
    t[0] = 1;

    memcpy(old_r, left, BYTES);
    memcpy(r, right, BYTES);

    while (!is_zero(r)) {
        bigquo_signed(old_r, sign_old_r, r, sign_r, quotient, &sign_quotient);
        bigmul_signed(quotient, sign_quotient, r, sign_r, multiplicant,
                      &sign_multiplicant);
        memcpy(swap, r, BYTES);
        sign_swap = sign_r;
        bigsub_signed(old_r, sign_old_r, multiplicant, sign_multiplicant, r,
                      &sign_r);
        memcpy(old_r, swap, BYTES);
        sign_old_r = sign_swap;

        bigmul_signed(quotient, sign_quotient, s, sign_s, multiplicant,
                      &sign_multiplicant);
        memcpy(swap, s, BYTES);
        sign_swap = sign_s;
        bigsub_signed(old_s, sign_old_s, multiplicant, sign_multiplicant, s,
                      &sign_s);
        memcpy(old_s, swap, BYTES);
        sign_old_s = sign_swap;
        bigmul_signed(quotient, sign_quotient, t, sign_t, multiplicant,
                      &sign_multiplicant);
        memcpy(swap, t, BYTES);
        sign_swap = sign_t;
        bigsub_signed(old_t, sign_old_t, multiplicant, sign_multiplicant, t,
                      &sign_t);
        memcpy(old_t, swap, BYTES);
        sign_old_t = sign_swap;
    }

    memcpy(bezout_s, big_comparison(left, right) > 0 ? old_s : old_t, BYTES);
    memcpy(gcd, old_r, BYTES);
    *bezout_s_sign = sign_old_s;
    return 0;
}

int simple_gcd(uint64_t *left, uint64_t *right, uint64_t *out) {
    uint64_t bezout_s[S];
    short bezout_s_sign;
    memset(out, 0, BYTES);
    return big_extended_gcd(left, right, bezout_s, &bezout_s_sign, out);
}

/* Return 0 for valid, 1 for invalid */
int validate_d(uint64_t *encryptor, uint64_t *lambda, uint64_t *d) {
    uint64_t encryptor_wrapper[BIGS], lambda_wrapper[BIGS], d_wrapper[BIGS], swap[BIGS];
    t_sizeup(S, BIGS, encryptor, encryptor_wrapper);
    t_sizeup(S, BIGS, lambda, lambda_wrapper);
    t_sizeup(S, BIGS, d, d_wrapper);
    t_mul(BIGS, encryptor_wrapper, d_wrapper, swap);
    t_rem(BIGS, swap, lambda_wrapper, d_wrapper);
    if(t_sizedown(S, d_wrapper, swap)) {
        return 1;
    }
    return comparison_ui(swap, 1) != 0;
}

int big_find_d(uint64_t encryptor, uint64_t *lambda, uint64_t *out) {
    short sign_t = 0, sign_r = 0, sign_new_t = 0, sign_new_r = 0, sign_swap = 0,
                                                                  sign_quotient = 0, sign_multiplicant = 0;
    uint64_t encryptor_wrapper[S], t[S], new_t[S], r[S], new_r[S], swap[S],
             quotient[S], multiplicant[S];
    memset(encryptor_wrapper, 0, BYTES);
    memset(t, 0, BYTES);
    memset(new_t, 0, BYTES);
    memset(r, 0, BYTES);
    memset(new_r, 0, BYTES);
    memset(quotient, 0, BYTES);
    memset(swap, 0, BYTES);
    memset(multiplicant, 0, BYTES);
    memset(out, 0, BYTES);
    new_t[0] = 1;
    encryptor_wrapper[0] = encryptor;
    memcpy(r, lambda, BYTES);
    memcpy(new_r, encryptor_wrapper, BYTES);

    while (!is_zero(new_r)) {
        bigquo_signed(r, sign_r, new_r, sign_new_r, quotient, &sign_quotient);
        bigmul_signed(quotient, sign_quotient, new_t, sign_new_t, multiplicant,
                      &sign_multiplicant);
        sign_swap = sign_new_t;
        memcpy(swap, new_t, BYTES);
        bigsub_signed(t, sign_t, multiplicant, sign_multiplicant, new_t,
                      &sign_new_t);
        memcpy(t, swap, BYTES);
        sign_t = sign_swap;
        bigmul_signed(quotient, sign_quotient, new_r, sign_new_r, multiplicant,
                      &sign_multiplicant);
        sign_swap = sign_new_r;
        memcpy(swap, new_r, BYTES);
        bigsub_signed(r, sign_r, multiplicant, sign_multiplicant, new_r,
                      &sign_new_r);
        memcpy(r, swap, BYTES);
        sign_r = sign_swap;
    }

    if (comparison_ui(r, 1) > 0) {
        return 1;
    }

    /*bigrem(t, n, swap);*/
    /*memcpy(t, swap, BYTES);*/
    memcpy(out, t, BYTES);
    if (sign_t == 1) {
        bigadd_signed(lambda, 0, t, sign_t, swap, &sign_swap);
        memcpy(out, swap, BYTES);
    }

    return validate_d(encryptor_wrapper, lambda, out);
}

/* Least common multiple */
int big_lcm(uint64_t *left, uint64_t *right, uint64_t *out) {
    uint64_t gcd[S], swap[S];
    /* Clear out buffers */
    memset(out, 0, BYTES);
    memset(gcd, 0, BYTES);
    memset(swap, 0, BYTES);
    /* Fail out if we cannot find the gcd */
    if (simple_gcd(left, right, gcd)) {
        return 1;
    }
    /* If there's a carry bit, left and right are too big and return 1 */
    if (bigmul(left, right, swap)) {
        return 1;
    } else {
        bigquo(swap, gcd, out);
        return 0;
    }
}

/* Print a 4096_t to a file handler */
int fprint_big(FILE *output_file, uint64_t *number) {
    size_t iterator;
    /* Keep track of the output so we know if it fails */
    int print_output;
    if (output_file == NULL) {
        return 1;
    }
    for (iterator = 0; iterator <= S; iterator++) {
        if (iterator == S) {
            print_output = fprintf(output_file, "\n");
        } else {
            /* We print backwards (most significant item first) */
            print_output = fprintf(output_file, "%016lx", number[S - 1 - iterator]);
        }
        if (print_output < 0) {
            return 1;
        }
    }
    return 0;
}

/* Write a key to a file handler */
/* Mode can be MODE_PUBLIC or MODE_PRIVATE */
int write_key(FILE *output_file, short mode, uint64_t *public_n,
              uint64_t encryptor, uint64_t *secret_d) {
    if (output_file == NULL) {
        return 1;
    }
    /* Use short circuiting */
    if (!(/* if this is 1, we won't proceed past it */
                fprintf(output_file, "%s",
                        mode == MODE_PUBLIC ? PUBLIC_HEADER : PRIVATE_HEADER) < 0 ||
                /* if this is 1, we won't proceed past it */
                fprint_big(output_file, public_n) < 0 ||
                /* if this is 1, we won't proceed past it */
                fprintf(output_file, "%016lx\n", encryptor) < 0 ||
                /* If the mode is public, the fprint_big is skipped, otherwise if it's
                   true and the fprint_big fails, we will short circuit */
                (mode == MODE_PRIVATE && fprint_big(output_file, secret_d) < 0) ||
                /* If everything else succeeds, print the footer*/
                fprintf(output_file, "%s",
                        mode == MODE_PUBLIC ? PUBLIC_FOOTER : PRIVATE_FOOTER) < 0)) {
        /* If nothing fails, close the file and return success */
        fclose(output_file);
        return 0;
    }
    /* Otherwise, close the file and return failure */
    fclose(output_file);
    return 1;
}

/* Read a key from a file (tricky!) */
/* Mode can be MODE_PUBLIC or MODE_PRIVATE */
int read_key(FILE *file_handler, short mode, uint64_t *public_n,
             uint64_t *transcryptor) {
    uint64_t encryptor;
    memset(transcryptor, 0, BYTES);
    /* First move past the header based on the mode */
    if (fseek(file_handler,
              strlen(mode == MODE_PUBLIC ? PUBLIC_HEADER : PRIVATE_HEADER),
              SEEK_SET)) {
        /* If it fails, you know the drill */
        fclose(file_handler);
        return 1;
    }
    if (
        /* Take in the next big number to public_n */
        big_in(file_handler, public_n) ||
        /* Use this scanf to handle the newlines */
        fscanf(file_handler, "\n%016lx\n", &encryptor) == EOF ||
        /* Take in the next big number to secret_d only if we're in decryption
           mode */
        (mode == MODE_PRIVATE && big_in(file_handler, transcryptor))) {
        /* In case of failure, close the handler and fail out */
        fclose(file_handler);
        return 1;
    }
    if (mode == MODE_PUBLIC) {
        transcryptor[0] = encryptor;
    }
    /* Close the handler and succeed if nothing goes wrong */
    fclose(file_handler);
    return 0;
}

/* In the case that we haven't read in a full 4096_t, move everything to least
 * significant */
/* words_read represents the last iterator position. If it's S, then we don't do
 * anything, if it's anything less, we right shift by (S - words_read) words */
int shift_to_low(uint64_t *outperand, size_t words_read) {
    size_t iterator, inner_iterator;
    /* If S - words_read is 0, do nothing */
    if (words_read == S) {
        return 0;
    }
    for (iterator = 0; iterator < (S - words_read - 1); iterator++) {
        /* There should absolutely not be anything in position 0 or we will lose
         * data */
        if (outperand[0] != 0) {
            return 1;
        }
        /* Right shift by 1 word */
        for (inner_iterator = 0; inner_iterator < S - 1; inner_iterator++) {
            outperand[inner_iterator] = outperand[inner_iterator + 1];
        }
        outperand[inner_iterator] = 0;
    }
    return 0;
}

int shift_to_high(uint64_t *outperand) {
    size_t iterator;
    if (is_zero(outperand)) {
        return 0;
    }
    /*while (get_msb(outperand) % 64 != 0) {
      for (iterator = 0; iterator < 8; iterator++) {
        big_left_shift(outperand);
      }
    } */
    if (outperand[S - 1] != 0) {
        return 0;
    }
    while (outperand[S - 1] == 0) {
        for (iterator = 0; iterator < S - 1; iterator++) {
            outperand[S - 1 - iterator] = outperand[S - 2 - iterator];
        }
        outperand[0] = 0;
    }
    return 0;
}

int input_read(FILE *file_handler, uint64_t *out) {
    size_t words_read;
    memset(out, 0, BYTES);
    /* Keep track of words_read in case size of input is less than 4096 bits */
    for (words_read = 0; words_read < S; words_read++) {
        if (fread(out + (S - 1 - words_read), 1, 8, file_handler) < 8) {
            break;
        }
    }
    /* Something weird has happened if shift_to_low returns 1 */
    if (shift_to_low(out, words_read)) {
        return 1;
    }
    fclose(file_handler);
    return 0;
}

int output_write(FILE *file_handler, uint64_t *out, short shift) {
    size_t words_written, iterator;
    uint8_t *word_8;
    if (shift)
        shift_to_high(out);
    /* Keep track of words_read in case size of input is less than 4096 bits */
    for (words_written = 0; words_written < S; words_written++) {
        if (shift) {
            word_8 = (uint8_t *)&out[S - 1 - words_written];
            for (iterator = 0; iterator < 8; iterator++) {
                if (*(word_8 + iterator) != 0) {
                    fwrite(word_8 + iterator, 1, 1, file_handler);
                }
            }
        } else {
            fwrite(out + (S - 1 - words_written), 8, 1, file_handler);
        }
    }
    fclose(file_handler);
    return 0;
}

/* Modular exponentiation! I had to do something really dumb here */
int big_modexp(const uint64_t *base, const uint64_t *exponent, uint64_t *modulus,
               uint64_t *out) {
    uint64_t biggerbase[BIGS], biggermodulus[BIGS], biggerout[BIGS],
             biggerswap[BIGS], swap[S], working_exponent[S];
    /* Clear buffers */
    memset(out, 0, BYTES);
    memset(biggerbase, 0, BIG_BYTES);
    memset(biggermodulus, 0, BIG_BYTES);
    memset(biggerout, 0, BIG_BYTES);
    memset(biggerswap, 0, BIG_BYTES);
    memset(swap, 0, BYTES);
    memcpy(working_exponent, exponent, BYTES);
    /* First make sure that modulus != 1 */
    if (comparison_ui(modulus, 1) == 0) {
        return 1;
    }
    /* We're gonna need some bigger guns */
    t_sizeup(S, BIGS, base, biggerbase);
    t_sizeup(S, BIGS, modulus, biggermodulus);
    biggerout[0] = 1;
    t_rem(BIGS, biggerbase, biggermodulus, biggerswap);
    memcpy(biggerbase, biggerswap, BIG_BYTES);
    while (!is_zero(working_exponent)) {
        /* Check whether exponent is odd */
        if (working_exponent[0] % 2 == 1) {
            if (t_mul(BIGS, biggerout, biggerbase, biggerswap)) {
                return 1;
            }
            t_rem(BIGS, biggerswap, biggermodulus, biggerout);
        }
        big_right_shift(working_exponent);
        if (t_mul(BIGS, biggerbase, biggerbase, biggerswap)) {
            return 1;
        }
        t_rem(BIGS, biggerswap, biggermodulus, biggerbase);
    }

    return t_sizedown(S, biggerout, out) > 0;
}
