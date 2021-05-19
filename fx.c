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

const fx * fx_one, * fx_zero, * fx_half, * fx_two, *fx_minus_one;

static void fx_init(void){
  static bool inited;
  if(inited) return;
  logd("FX_INIT\n");
  inited = true;
  bf_context_init(&bf_ctx, iron_bf_realloc, NULL);
  *((fx **)&fx_one) = fx_new1(1);
  *((fx **)&fx_zero) = fx_new1(0);
  *((fx **)&fx_two) = fx_new1(2);
  *((fx **)&fx_minus_one) = fx_new1(-1);
  *((fx **)&fx_half) = fx_new1(0.5);
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

fx * fx_clone(const fx * x){
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

bool fx_gt(const fx * a, const fx * b){
  return bf_cmp_gt(&a->v, &b->v);
}
bool fx_lt(const fx * a, const fx * b){
  return bf_cmp_lt(&a->v, &b->v);
}

f64 fx_to_f64(const fx * a){
  f64 x;
  bf_get_float64(&a->v, &x, BF_RNDN);
  return x;
}

void fx_print(const fx * v){
  size_t s = 0;
  char * text = bf_ftoa(&s, &v->v, 10, 32, BF_FTOA_FORMAT_FREE);
  logd("%s", text);
  dealloc(text);
}

void * fx_to_binary(const fx * _v, size_t * s) {
  bf_t v = _v->v;
  *s = 1 + sizeof(v.expn) + sizeof(v.len) + sizeof(v.tab[0]) * v.len;
  void * buffer = alloc(*s);
  memset(buffer, v.sign == 0 ? 0 : 1, 1); // sign
  memcpy(buffer + 1, &v.expn, sizeof(v.expn));
  memcpy(buffer + 1 + sizeof(v.expn), &v.len, sizeof(v.len));
  memcpy(buffer + 1 + sizeof(v.expn) + sizeof(v.len), v.tab, sizeof(v.tab[0]) * v.len);

  return buffer;
}

char * fx_serialize_str(const fx * v,size_t * s){
  static size_t _s = 0; 
  if(s == NULL)
    s = &_s;
  return bf_ftoa(s, &v->v, 16, 32, BF_FTOA_FORMAT_FREE_MIN);
}

fx * fx_deserialize_str(const char * text, size_t s){
  UNUSED(s);
  fx * v = fx_new();
  slimb_t exp =0;
  bf_atof2(&v->v, &exp, text, NULL, 16, BF_PREC_INF, 0);
  return v;
}

fx * fx_from_binary(const void * text, size_t s){
  UNUSED(s);
  bf_t v = {0}; 
  v.ctx = &bf_ctx;
  v.sign = ((u8 *) text)[0] == 1 ? 1 : 0;
  memcpy(&v.expn, text + 1, sizeof(v.expn));
  memcpy(&v.len, text + 1 + sizeof(v.expn), sizeof(v.len));
  v.tab = alloc(v.len * sizeof(sizeof(v.tab[0])));
  memcpy(v.tab, text + 1 + sizeof(v.expn) + sizeof(v.len), v.len * sizeof(v.tab[0]));

  fx v2 = {.v = v};
  return iron_clone(&v2, sizeof(v2));
}

fx * fx_deserialize(const void * text, size_t s){
  return fx_deserialize_str(text, s);
}

void * fx_serialize(const fx * v, size_t * s){
  return fx_serialize_str(v, s);
}


bool fx_isneg(const fx * v){
  return v->v.sign == 1;
}
bool fx_iszero(const fx * v){
  return bf_is_zero(&v->v);
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
  fx_del(b);
  
  return true;
}
#include "linmath.h"
#include "linmath_bf.h"

bool test_encode_decode(void){
  vec2bf * bf = vec2bf_new(1084321743901264, 1);
  vec2bf * bf2 = vec2bf_new(0.01, 0.01);
  vec2bf_mul(bf, bf);
  vec2bf_mul(bf, bf2);
  size_t s;
  void * text = fx_serialize(bf->x, &s);
  fx * fx2 = fx_deserialize(text, s);
  fx_print(bf->x);logd("\n");
  ASSERT(fx_eq(fx2, bf->x));
  vec2bf_del(bf);
  vec2bf_del(bf2);
  fx_del(fx2);
  dealloc(text);
  //logd("%s %s\n", text, text2);
  return true;
}


bool test_vec2x(void){
  vec2bf * a = vec2bf_new(1,1);
  vec2 a2 = vec2bf_to_vec2(a);
  vec2 b = vec2_new(1,1);
  ASSERT(vec2_eq(a2, b));
  for(int i = 0; i < 10; i++){
    vec2bf_add(a, a);
    b = vec2_add(b,b);
    a2 = vec2bf_to_vec2(a);
    ASSERT(vec2_eq(a2, b));
    vec2_print(b);logd("\n");
  }
  
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
  ASSERT(!fx_iszero(fx_one));
  ASSERT(fx_iszero(fx_zero));
  TEST(test_encode_decode);
  TEST(test_vec2x);
  return true;
}
