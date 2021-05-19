#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

#include "types.h"
#include "utils.h"
#include "mem.h"
#include "log.h"
#include "test.h"
#include "libbf.h"
#include "fx.h"

bf_context_t bf_ctx;
static void *iron_bf_realloc(void *opaque, void *ptr, size_t size)
{
  UNUSED(opaque);
  return ralloc(ptr, size);
}

fx * fx_one, * fx_zero, * fx_half, * fx_two, *fx_minus_one;

static void fx_init(void){
  static bool inited;
  if(inited) return;
  inited = true;
  bf_context_init(&bf_ctx, iron_bf_realloc, NULL);
  fx_one = fx_new1(1);
  fx_zero = fx_new1(0);
  fx_two = fx_new1(2);
  fx_minus_one = fx_new1(-1);
}

fx * fx_new(void){
  return fx_newn(1);
}

fx * fx_newn(u32 count){
  fx_init();
  fx * v = alloc0(sizeof(*v) * count);
  for(u32 i = 0; i < count; i++)
    bf_init(&bf_ctx, &v[i].v);
  return v;
}


fx * fx_new1(f64 value){
  fx * v = fx_new();
  fx_set(v, value);
  return v;
}

void fx_set(fx * v, f64 value){
  bf_set_float64(&v->v, value);
}

void fx_set2(fx * v, const fx * value){
  bf_add(&v->v, &fx_zero->v, &value->v, BF_PREC_INF, 0);
}

void fx_set_zero(fx * v){
  bf_set_zero(&v->v, 0);
}

void fx_del(fx * v){
  fx_deln(v, 1);
}

void fx_deln(fx * v, u32 count){
  for(u32 i = 0; i < count; i++)
    bf_delete(&v[i].v);
  dealloc(v);
}

fx * fx_clone(fx * x){
  fx * new = fx_new();
  fx_set2(new, x);
  return new;
}

void fx_add(fx * a, const fx * b){
  bf_add(&a->v, &a->v, &b->v, BF_PREC_INF, 0);
}

void fx_sub(fx * a, const fx * b){
  bf_sub(&a->v, &a->v, &b->v, BF_PREC_INF, 0);

}
// this precision seems to be important for moving between bookmarks.
// how iterating a bit it can be significantly lower than otherwise.
#define div_prec 60

void fx_div(fx * a, const fx * b){
  bf_div(&a->v, &a->v, &b->v, div_prec , 0);  
}

void fx_div2(fx * c,const fx * a, const fx * b){
  bf_div(&c->v, &a->v, &b->v, div_prec , 0);  
}

void fx_mul(fx * a, const fx * b){
  bf_mul(&a->v, &a->v, &b->v, BF_PREC_INF, 0);
}

void fx_inv(fx * a){
  fx_div2(a, fx_one, a);
}

bool fx_eq(const fx * a, const fx * b){
  return bf_cmp_eq(&a->v, &b->v);
}

bool fx_gt(fx * a, fx * b){
  return bf_cmp_lt(&b->v, &a->v);
}
bool fx_lt(fx * a, fx * b){
  return fx_gt(b, a);
}

f64 fx_to_f64(fx * a){
  f64 x;
  bf_get_float64(&a->v, &x, BF_RNDN);
  return x;
}

void fx_print(fx * v){
  size_t s = 0;
  char * text = bf_ftoa(&s, &v->v, 10, 32, BF_FTOA_FORMAT_FREE);
  logd("%s,", text);
  dealloc(text);
}

void * fx_serialize(fx * v, size_t * s){
  void * text = bf_ftoa(s, &v->v, 16, 32, BF_FTOA_FORMAT_FREE_MIN);
  return text;
}

fx * fx_deserialize(void * text, size_t s){
  UNUSED(s);
  fx * v = fx_new();
  slimb_t exp =0;
  bf_atof2(&v->v, &exp, text, NULL, 16, BF_PREC_INF, 0);
  return v;
}

int fx_isneg(fx * v){
  return v->v.sign;
}

#define ASSERT_EQ_INT(exp, expr) if(__builtin_expect(!(exp == expr), 0)){ERROR("Assertion '" #expr "' Failed %i != %i", exp, expr);};

bool fx_test_serialize(fx * a){
  size_t s;
  void * buf1 = fx_serialize(a, &s);
  fx * b = fx_deserialize(buf1, s);
  ASSERT(fx_eq(b,a));
  size_t s2;
  void * buf2 = fx_serialize(b, &s2);
  ASSERT(memcmp(buf2, buf1, s) == 0);
  dealloc(buf1);
  dealloc(buf2);
  
  return true;
}

bool bf_test(void){ 
  var a = fx_new1(1);
  var b = fx_new1(-1);
  ASSERT_EQ_INT(1, fx_isneg(b));
  ASSERT_EQ_INT(0, fx_isneg(a));
  ASSERT(fx_eq(a,a));
  ASSERT(!fx_eq(a,b));
  ASSERT(fx_eq(b,b));
  TEST1(fx_test_serialize, a);
  TEST1(fx_test_serialize, b);
  var c = fx_new1(2.0);
  for(int i = 0; i < 100; i++){
    fx_mul(c,c);
    TEST1(fx_test_serialize, c);
  }
  fx_del(c);
  fx_del(a);
  fx_del(b);
  return true;
}
