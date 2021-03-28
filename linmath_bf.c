#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

#include "types.h"
#include "linmath.h"
#include "libbf.h"
#include "linmath_bf.h"
#include "utils.h"
#include "mem.h"
#include "log.h"
static bf_context_t bf_ctx;

vec2bf * vec2bf_zero;
vec2bf * vec2bf_one;
vec2bf * vec2bf_half;
vec2bf * vec2bf_one_x;
vec2bf * vec2bf_one_y;
vec2bf * vec2bf_two;


static void *iron_bf_realloc(void *opaque, void *ptr, size_t size)
{
  UNUSED(opaque);
  return ralloc(ptr, size);
}

void vec2bf_set(vec2bf * v, f64 x, f64 y){
  bf_set_float64(&v->x, x);
  bf_set_float64(&v->y, y);
}

vec2bf * _vec2bf_new(f64 x, f64 y){
 vec2bf * n = alloc0(sizeof(vec2bf));
 bf_init(&bf_ctx, &n->x);
 bf_init(&bf_ctx, &n->y);
 vec2bf_set(n, x, y);
 return n;
}

vec2bf * vec2bf_new(f64 x, f64 y){
  ensure_vec2bf_inited();
  return _vec2bf_new(x, y);
}

vec2bf * vec2bf_new2(vec2 x){
  ensure_vec2bf_inited();
  return _vec2bf_new(x.x, x.y);
}

vec2bf * vec2bf_new3(vec2bf * x){
  ensure_vec2bf_inited();
  vec2bf * y =  _vec2bf_new(0, 0);
  vec2bf_add(y, x);
  return y;
}

void vec2bf_del(vec2bf * v){
  bf_delete(&v->x);
  bf_delete(&v->y);
}

void vec2bf_add(vec2bf * a, vec2bf * b){
  bf_add(&a->x, &a->x, &b->x, BF_PREC_INF, 0);
  bf_add(&a->y, &a->y, &b->y, BF_PREC_INF, 0);
}


void vec2bf_sub(vec2bf * a, vec2bf * b){
  bf_sub(&a->x, &a->x, &b->x, BF_PREC_INF, 0);
  bf_sub(&a->y, &a->y, &b->y, BF_PREC_INF, 0);
}


void vec2bf_mul(vec2bf * a, vec2bf * b){
  bf_mul(&a->x, &a->x, &b->x, BF_PREC_INF, 0);
  bf_mul(&a->y, &a->y, &b->y, BF_PREC_INF, 0);
}

// this precision seems to be important for moving between bookmarks.
// how iterating a bit it can be significantly lower than otherwise.
#define div_prec 60

void vec2bf_div2(vec2bf * c, vec2bf * a, vec2bf * b){
  bf_div(&c->x, &a->x, &b->x, div_prec, 0);
  bf_div(&c->y, &a->y, &b->y, div_prec, 0);
}

void vec2bf_div(vec2bf * a, vec2bf * b){
  bf_div(&a->x, &a->x, &b->x, div_prec , 0);
  bf_div(&a->y, &a->y, &b->y, div_prec, 0);
}

void vec2bf_inv(vec2bf * a){
  bf_div(&a->x, &vec2bf_one->x, &a->x, div_prec, 0);
  bf_div(&a->y, &vec2bf_one->x, &a->y, div_prec, 0);
}

bool vec2bf_cmp(vec2bf * a, vec2bf * b){
  return bf_cmp_eq(&a->x, &b->x) && bf_cmp_eq(&a->y, &b->y);
}

f64 vec2bf_len(vec2bf * a){
  f64 x, y;
  vec2bf_to_xy(a, &x, &y);
  return sqrt(x * x + y * y);
}

void vec2bf_to_xy(vec2bf * v, f64 * x ,f64 * y ){
  bf_get_float64(&v->x, x, BF_RNDN);
  bf_get_float64(&v->y, y, BF_RNDN);
}


vec2 vec2bf_to_vec2(vec2bf * v){
  double x ,y;
  bf_get_float64(&v->x, &x, BF_RNDN);
  bf_get_float64(&v->y, &y, BF_RNDN);
  return vec2_new((f32)x, (f32)y);
}

void vec2bf_print(vec2bf * v){
  char * text = NULL;
  size_t s = 0;
  text = bf_ftoa(&s, &v->x, 10, 32, BF_FTOA_FORMAT_FREE);
  logd("(%s,", text);
  dealloc(text);
  text = bf_ftoa(&s, &v->y, 10, 32, BF_FTOA_FORMAT_FREE);
  logd(" %s)", text);
  dealloc(text);
}

void ensure_vec2bf_inited(){
  static bool vec2bf_inited = false;
  if(!vec2bf_inited){
    vec2bf_inited = true;
    bf_context_init(&bf_ctx, iron_bf_realloc, NULL);

    vec2bf_two = vec2bf_new(2, 2);
    vec2bf_one = vec2bf_new(1,1);
    vec2bf_zero = vec2bf_new(0,0);
    vec2bf_half = vec2bf_new(1, 1);
    vec2bf_div2(vec2bf_half, vec2bf_one, vec2bf_two);
    vec2bf_one_x = vec2bf_new(1, 0);
    vec2bf_one_y =_vec2bf_new(0, 1);
  }
}
