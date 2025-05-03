#include "math_t.h"
#include "4096_t.h"

/* From the BigAdd lab */
uint64_t bigsub(uint64_t *min, uint64_t *sub, uint64_t *dif) {
    return t_sub(S, min, sub, dif);
}

/* BigAdd */
uint64_t bigadd(uint64_t *augend, uint64_t *addend, uint64_t *sum) {
    return t_add(S, augend, addend, sum);
}

/* BigMul */
uint64_t bigmul(uint64_t *base, uint64_t *multiplier, uint64_t *out) {
    return t_mul(S, base, multiplier, out);
}

/* BigDiv helper functions - from assignment */
/* Magical calvin code i won't pretend to understand */
uint64_t big_log(uint64_t *in) {
    return t_log(S, in);
}
uint64_t lil_log(uint64_t in) {
    uint64_t log = 0;
    while ((in >> log++) && log < 64) {
    }
    return log;
}
uint64_t get_msb(uint64_t *n) {
    return t_msb(S, n);
}

/* What if a right shift was big (Divide by 2) */
uint64_t big_right_shift(uint64_t *outperand) {
    return t_right_shift(S, outperand);
}

/* Same as above but backwards */
uint64_t big_left_shift(uint64_t *outperand) {
    return t_left_shift(S, outperand);
}

/* Imagine if a comparison function was big */
/** Returns -1 if left < right, 0 if left == right, 1 if left > right */
short big_comparison(uint64_t *left, uint64_t *right) {
    return t_comparison(S, left, right);
}

/* BigDiv */
uint64_t bigdiv(uint64_t *numerator, uint64_t *denominator, uint64_t *quotient,
                uint64_t *remainder) {
    return t_div(S, numerator, denominator, quotient, remainder);
}

uint64_t bigquo(uint64_t *num, uint64_t *den, uint64_t *out) {
    return t_quo(S, num, den, out);
}

uint64_t bigrem(uint64_t *num, uint64_t *den, uint64_t *out) {
    return t_rem(S, num, den, out);
}

/* Signed 4096_t */
uint64_t bigadd_signed(uint64_t *in0, short in0_sign, uint64_t *in1, short in1_sign, uint64_t *out, short *outsign) {
    return t_add_signed(S, in0, in0_sign, in1, in1_sign, out, outsign);
}

uint64_t bigsub_signed(uint64_t *in0, short in0_sign, uint64_t *in1, short in1_sign, uint64_t *out, short *outsign) {
    return t_sub_signed(S, in0, in0_sign, in1, in1_sign, out, outsign);
}

uint64_t bigmul_signed(uint64_t *in0, short in0_sign, uint64_t *in1, short in1_sign, uint64_t *out, short *outsign) {
    return t_mul_signed(S, in0, in0_sign, in1, in1_sign, out, outsign);
}

uint64_t bigquo_signed(uint64_t *in0, short in0_sign, uint64_t *in1, short in1_sign, uint64_t *out, short *outsign) {
    return t_quo_signed(S, in0, in0_sign, in1, in1_sign, out, outsign);
}

uint64_t do_ui(uint64_t (big_function)(uint64_t*, uint64_t*, uint64_t*), uint64_t* big, uint64_t lil, uint64_t* out) {
    uint64_t lil_wrapper[S];
    memset(lil_wrapper, 0, BYTES);
    memset(out, 0, BYTES);
    lil_wrapper[0] = lil;
    return big_function(big, lil_wrapper, out);
}

uint64_t add_ui(uint64_t* big, uint64_t lil, uint64_t* out) {
    return do_ui(bigadd, big, lil, out);
}

uint64_t sub_ui(uint64_t* big, uint64_t lil, uint64_t* out) {
    return do_ui(bigsub, big, lil, out);
}

uint64_t mul_ui(uint64_t* big, uint64_t lil, uint64_t* out) {
    return do_ui(bigmul, big, lil, out);
}

uint64_t quo_ui(uint64_t* big, uint64_t lil, uint64_t* out) {
    return do_ui(bigquo, big, lil, out);
}

uint64_t rem_ui(uint64_t* big, uint64_t lil, uint64_t* out) {
    return do_ui(bigrem, big, lil, out);
}
