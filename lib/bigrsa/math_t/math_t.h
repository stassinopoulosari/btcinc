/* 8192_t.h */
#include "4096_t.h"

uint64_t t_add(size_t ELEM, uint64_t *in0, uint64_t *in1, uint64_t *sum);
uint64_t t_sub(size_t ELEM, uint64_t *min, uint64_t *sub, uint64_t *dif);
uint64_t t_mul(size_t ELEM, uint64_t *in0, uint64_t *in1, uint64_t *out);
uint64_t t_log(size_t ELEM, uint64_t *in);
uint64_t t_msb(size_t ELEM, uint64_t *n);
uint64_t t_left_shift(size_t ELEM, uint64_t *outperand);
uint64_t t_right_shift(size_t ELEM, uint64_t *outperand);
uint64_t t_div(size_t ELEM, uint64_t *num, uint64_t *den, uint64_t *quo, uint64_t *rem);
uint64_t t_quo(size_t ELEM, uint64_t *num, uint64_t *den, uint64_t *quo);
uint64_t t_rem(size_t ELEM, uint64_t *num, uint64_t *den, uint64_t *rem);
short t_comparison(size_t ELEM, uint64_t *left, uint64_t *right);
int t_sizeup(size_t ELEM_IN, size_t ELEM_OUT, uint64_t *big, uint64_t *bigger);
uint64_t t_sizedown(size_t ELEM_OUT, uint64_t *bigger, uint64_t *big);
uint64_t lil_log(uint64_t in);

/* Signed math_t */

uint64_t t_add_signed(size_t ELEM, uint64_t *in0, short in0_sign, uint64_t *in1,
                      short in1_sign, uint64_t *sum, short *outsign);
uint64_t t_sub_signed(size_t ELEM, uint64_t *in0, short in0_sign, uint64_t *in1,
                      short in1_sign, uint64_t *out, short *outsign);
uint64_t t_mul_signed(size_t ELEM, uint64_t *in0, short in0_sign, uint64_t *in1,
                      short in1_sign, uint64_t *out, short *outsign);
uint64_t t_quo_signed(size_t ELEM, uint64_t *in0, short in0_sign, uint64_t *in1,
                      short in1_sign, uint64_t *out, short *outsign);
