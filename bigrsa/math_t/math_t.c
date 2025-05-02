#include "math_t.h"
#include <string.h>

/* From the biggerAdd lab */
uint64_t t_sub(size_t ELEM, uint64_t *min, uint64_t *sub, uint64_t *dif) {
    size_t i;
    uint64_t carry = 0, tmp;
    for (i = 0; i < ELEM; i++) {
        tmp = min[i] - sub[i] - carry;
        carry = min[i] < sub[i];
        dif[i] = tmp;
    }
    return carry;
}

/* biggerAdd */
uint64_t t_add(size_t ELEM, uint64_t *augend, uint64_t *addend, uint64_t *sum) {
    uint64_t tmp, carry = 0;
    size_t i;
    for (i = 0; i < ELEM; i++) {
        tmp = augend[i] + addend[i] + carry;
        /* Theory: We can tell if we carry because
        it will overflow `tmp`. Even if every bit
        in addend is 1, it cannot add up to
        anything greater than augend after
        overflowing */
        carry = tmp < augend[i] || (tmp == augend[i] && addend[i] != 0);
        sum[i] = tmp;
    }
    return carry;
}

/* biggerMul */
uint64_t t_mul(size_t ELEM, uint64_t *base, uint64_t *multiplier,
               uint64_t *out) {
    size_t base_index, multiplier_index;
    size_t bytes = ELEM * sizeof(uint64_t);
    /* We want an output array that'ELEM the maximum possible size, we can trim
     * later
     */
    uint64_t biggerout[ELEM * 2 + 1];
    memset(biggerout, 0, bytes);
    /* Use a 64-bit variable to hold temporary results */
    uint64_t temp;
    uint32_t carry32;
    /* Create 32-bit aliases so we can pretend this is a 32-bit problem */
    uint32_t *base32 = (uint32_t *)base, *multiplier32 = (uint32_t *)multiplier,
                                          *out32 = (uint32_t *)biggerout, *temp32 = (uint32_t *)&temp;
    for (base_index = 0; base_index < ELEM * 2; base_index++) {
        /* Reset the carry */
        carry32 = 0;
        for (multiplier_index = 0; multiplier_index < ELEM * 2;
                multiplier_index++) {
            /* We will not need any values past 2 * (bytes + 1) */
            if (base_index + multiplier_index > 2 * (bytes + 1)) {
                continue;
            }
            /* Set the 64-bit value to a max of 2^65 - 1 */
            temp = (uint64_t)out32[base_index + multiplier_index] +
                   (uint64_t)carry32 +
                   (uint64_t)base32[base_index] *
                   (uint64_t)multiplier32[multiplier_index];

            /* Use the lower 32 to get the output */
            out32[base_index + multiplier_index] = temp32[0];

            /* Use the upper 32 to take the carry */
            carry32 = temp32[1];
        }
        out32[base_index + multiplier_index + 1] = carry32;
    }
    /* Ignore my note here before */
    memcpy(out, biggerout, bytes);
    /* Carry bit! */
    return biggerout[ELEM + 1];
}

/* biggerDiv helper functions - from assignment */
/* Magical calvin code i won't pretend to understand */
uint64_t t_log(size_t ELEM, uint64_t *in) {
    uint64_t log = ELEM;
    while (log && !in[--log]) {
    }
    return log;
}

uint64_t t_msb(size_t ELEM, uint64_t *n) {
    uint64_t b, l;
    b = t_log(ELEM, n);
    l = lil_log(n[b]);
    return b * 64 + l;
}

/* What if a right shift was bigger (Divide by 2) */
uint64_t t_right_shift(size_t ELEM, uint64_t *outperand) {
    size_t iterator;
    uint64_t carry = 0, temp;
    /* I was rudely introduced to moving backwards through an array WHEN MY
     * ITERATOR IS UNSIGNED >:( */
    for (iterator = 0; iterator < ELEM; iterator++) {
        /* Bitwise magic */
        temp = (outperand[ELEM - 1 - iterator] >> 1) + (carry << 63);
        /* Least significant bit of this item becomes most significant of the
         * previous */
        carry = outperand[ELEM - 1 - iterator] & 1;
        outperand[ELEM - 1 - iterator] = temp;
    }
    return carry;
}

/* Same as above but backwards */
uint64_t t_left_shift(size_t ELEM, uint64_t *outperand) {
    size_t iterator;
    uint64_t carry = 0, temp;
    for (iterator = 0; iterator < ELEM; iterator++) {
        temp = (outperand[iterator] << 1) + carry;
        carry = outperand[iterator] >> 63;
        outperand[iterator] = temp;
    }
    return carry;
}

/* Imagine if a comparison function was bigger */
/** Returns -1 if left < right, 0 if left == right, 1 if left > right */
short t_comparison(size_t ELEM, uint64_t *left, uint64_t *right) {
    size_t iterator;
    for (iterator = 0; iterator < ELEM; iterator++) {
        if (left[ELEM - 1 - iterator] < right[ELEM - 1 - iterator]) {
            return -1;
        } else if (left[ELEM - 1 - iterator] > right[ELEM - 1 - iterator]) {
            return 1;
        }
    }
    return 0;
}

/* biggerDiv */
uint64_t t_div(size_t ELEM, uint64_t *numerator, uint64_t *denominator,
               uint64_t *quotient, uint64_t *remainder) {
    size_t bytes = ELEM * sizeof(uint64_t);
    /* I decided it'ELEM probably not such a hot idea to do things partially in
     * place
     */
    uint64_t working_numerator[ELEM];
    uint64_t working_denominator[ELEM];
    /* Doing biggersub in place seems like a bad idea */
    uint64_t swap[ELEM];

    size_t iterator = 0,
           /* Length of quotient in bits */
           quotient_length = 0;
    memset(quotient, 0, bytes);
    memset(remainder, 0, bytes);
    memset(swap, 0, bytes);

    /* Copy the numerator, denominator into their clones */
    memcpy(working_numerator, numerator, bytes);
    memcpy(working_denominator, denominator, bytes);
    /* Handle division by zero and division with a small numerator (should also
     * handle 0/0 or 0/x) */
    if (t_msb(ELEM, working_denominator) == 0 && working_denominator[0] == 0) {
        /* Infinity */
        memset(quotient, 255, bytes);
        memset(remainder, 255, bytes);
        return 1;
    } else if (t_comparison(ELEM, working_denominator, working_numerator) == 1) {
        /* Zero */
        memset(quotient, 0, bytes);
        memcpy(remainder, working_numerator, bytes);
        return 0;
    }

    /* Move all bits in denominator up until the msbs are aligned */
    while (t_msb(ELEM, working_numerator) != t_msb(ELEM, working_denominator) &&
            quotient_length < 4097 /* Max iterations just in case, yk? */) {
        t_left_shift(ELEM, working_denominator);
        /* The quotient can be only as long as the number of orders of magnitude
         * larger the numerator is than the denominator */
        quotient_length++;
    }

    /* i fell into a trance when i wrote this i have no idea what it does */
    for (iterator = 0; iterator <= quotient_length; iterator++) {
        t_left_shift(ELEM, quotient);
        /* If the aligned numerator >= aligned denominator, add 1 to the quotient */
        if (t_comparison(ELEM, working_denominator, working_numerator) != 1) {
            quotient[0] += 1;
            t_sub(ELEM, working_numerator, working_denominator, swap);
            memcpy(working_numerator, swap, bytes);
        }
        /* Move the denominator down 1 bit */
        t_right_shift(ELEM, working_denominator);
    }

    /* Whatever we have leftover is our remainder */
    memcpy(remainder, working_numerator, bytes);
    /* GREAT SUCCESS */
    return 0;
}

uint64_t t_quo(size_t ELEM, uint64_t *num, uint64_t *den, uint64_t *quo) {
    uint64_t rem[ELEM];
    t_div(ELEM, num, den, quo, rem);
    return 0;
}

uint64_t t_rem(size_t ELEM, uint64_t *num, uint64_t *den, uint64_t *rem) {
    uint64_t quo[ELEM];
    t_div(ELEM, num, den, quo, rem);
    return 0;
}

int t_sizeup(size_t ELEM_IN, size_t ELEM_OUT, uint64_t *big, uint64_t *bigger) {
    /* Clear output buffer */
    memset(bigger, 0, ELEM_OUT * sizeof(uint64_t));
    memcpy(bigger, big, ELEM_IN * sizeof(uint64_t));
    return 0;
}

uint64_t t_sizedown(size_t ELEM_OUT, uint64_t *bigger,
                    uint64_t *big) {
    /* Clear output buffer */
    size_t bytes = ELEM_OUT * sizeof(uint64_t);
    memset(big, 0, bytes);
    memcpy(big, bigger, bytes);
    return bigger[ELEM_OUT + 1];
}

uint64_t t_add_signed(size_t ELEM, uint64_t *in0, short in0_sign, uint64_t *in1,
                      short in1_sign, uint64_t *sum, short *outsign) {
    if (in0_sign == in1_sign) {
        *outsign = in0_sign;
        return t_add(ELEM, in0, in1, sum);
    }
    short comparison = t_comparison(ELEM, in0, in1);
    if (comparison == 0) {
        *outsign = 0;
        memset(sum, 0, ELEM * sizeof(uint64_t));
        return 0;
    } else if (comparison > 0) {
        *outsign = in0_sign;
        return t_sub(ELEM, in0, in1, sum);
    }
    *outsign = in1_sign;
    return t_add(ELEM, in0, in1, sum);
}
uint64_t t_sub_signed(size_t ELEM, uint64_t *in0, short in0_sign, uint64_t *in1,
                      short in1_sign, uint64_t *out, short *outsign) {
    if (in0_sign != in1_sign) {
        /* This will be addition no matter what: -1 - 3 = -4 or 1 - -3 = 4*/
        *outsign = in0_sign;
        return t_add(ELEM, in0, in1, out);
    }
    short comparison = t_comparison(ELEM, in0, in1);
    if (comparison == 0) {
        memset(out, 0, ELEM * sizeof(uint64_t));
        *outsign = 0;
        return 0;
    } else if (comparison > 0) {
        *outsign = in0_sign;
        return t_sub(ELEM, in0, in1, out);
    }
    *outsign = 1 - in0_sign;
    return t_sub(ELEM, in1, in0, out);
}
uint64_t t_mul_signed(size_t ELEM, uint64_t *in0, short in0_sign, uint64_t *in1,
                      short in1_sign, uint64_t *out, short *outsign) {
    *outsign = in0_sign != in1_sign;
    return t_mul(ELEM, in0, in1, out);
}
uint64_t t_quo_signed(size_t ELEM, uint64_t *in0, short in0_sign, uint64_t *in1,
                      short in1_sign, uint64_t *out, short *outsign) {
    *outsign = in0_sign != in1_sign;
    return t_quo(ELEM, in0, in1, out);
}
