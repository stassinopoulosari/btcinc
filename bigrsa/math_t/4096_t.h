/* 4096_t.h */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define S (size_t)(4096 / 64)
#define BYTES S * sizeof(uint64_t)

uint64_t bigadd(uint64_t *in0, uint64_t *in1, uint64_t *sum);
uint64_t bigsub(uint64_t *min, uint64_t *sub, uint64_t *dif);
uint64_t bigmul(uint64_t *in0, uint64_t *in1, uint64_t *out);
uint64_t bigquo(uint64_t *num, uint64_t *den, uint64_t *quo);
uint64_t bigrem(uint64_t *num, uint64_t *den, uint64_t *rem);

/* Signed 4096_t */
uint64_t bigadd_signed(uint64_t *in0, short in1_sign, uint64_t *in1, short in2_sign, uint64_t *sum, short *outsign);
uint64_t bigsub_signed(uint64_t *min, short in1_sign, uint64_t *sub, short in2_sign, uint64_t *dif, short *outsign);
uint64_t bigmul_signed(uint64_t *in0, short in1_sign, uint64_t *in1, short in2_sign, uint64_t *out, short *outsign);
uint64_t bigquo_signed(uint64_t *num, short in1_sign, uint64_t *den, short in2_sign, uint64_t *quo, short *outsign);

/* ops_ui */
uint64_t add_ui(uint64_t *big, uint64_t lil, uint64_t *out);
uint64_t sub_ui(uint64_t *big, uint64_t lil, uint64_t *out);
uint64_t mul_ui(uint64_t *big, uint64_t lil, uint64_t *out);
uint64_t quo_ui(uint64_t *big, uint64_t lil, uint64_t *out);
uint64_t rem_ui(uint64_t *big, uint64_t lil, uint64_t *out);
