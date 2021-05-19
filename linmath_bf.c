#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

#include "types.h"
#include "linmath.h"
#include "libbf.h"
#include "fx.h"
#include "linmath_bf.h"
#include "utils.h"
#include "mem.h"
#include "log.h"

vec2bf * vec2bf_zero;
vec2bf * vec2bf_one;
vec2bf * vec2bf_half;
vec2bf * vec2bf_one_x;
vec2bf * vec2bf_one_y;
vec2bf * vec2bf_two;


void vec2bf_set(vec2bf * v, f64 x, f64 y){
  fx_set(v->x, x);
  fx_set(v->y, y);
}

vec2bf * _vec2bf_new(f64 x, f64 y){
 vec2bf * n = alloc0(sizeof(vec2bf));
 n->x = fx_new1(x);
 n->y = fx_new1(y);
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
  fx_del(v->x);
  fx_del(v->y);
}

vec2bf * vec2bf_clone(vec2bf * x){
  vec2bf * new = vec2bf_new(0, 0);
  vec2bf_add(new, x);
  return new;
}

void vec2bf_clear(vec2bf * a){
  vec2bf_sub(a, a);
}
void vec2bf_set2(vec2bf * v, vec2bf * other){
  vec2bf_clear(v);
  vec2bf_add(v, other);
}

void vec2bf_add(vec2bf * a, vec2bf * b){
  fx_add(a->x, b->x);
  fx_add(a->y, b->y);
}

void vec2bf_sub(vec2bf * a, vec2bf * b){
  fx_sub(a->x, b->x);
  fx_sub(a->y, b->y);
}


void vec2bf_mul(vec2bf * a, vec2bf * b){
  fx_mul(a->x, b->x);
  fx_mul(a->y, b->y);
}


void vec2bf_div2(vec2bf * c, vec2bf * a, vec2bf * b){
  fx_div2(c->x, a->x, b->x);
  fx_div2(c->y, b->x, b->y);
}

void vec2bf_div(vec2bf * a, vec2bf * b){
  fx_div(a->x, b->x);
  fx_div(a->y, b->y);
}

void vec2bf_inv(vec2bf * a){
  fx_inv(a->x);
  fx_inv(a->y);
}

bool vec2bf_cmp(vec2bf * a, vec2bf * b){
  return fx_eq(a->x, b->x) && fx_eq(a->y, b->y);
}

f64 vec2bf_len(vec2bf * a){
  f64 x, y;
  vec2bf_to_xy(a, &x, &y);
  return sqrt(x * x + y * y);
}

void vec2bf_to_xy(vec2bf * v, f64 * x ,f64 * y ){
  *x = fx_to_f64(v->x);  
  *y = fx_to_f64(v->y);  
}


vec2 vec2bf_to_vec2(vec2bf * v){
  return vec2_new(fx_to_f64(v->x),fx_to_f64(v->y));
}

void vec2bf_print(vec2bf * v){
  logd("(,");
  fx_print(v->x);
  logd(", ");
  fx_print(v->y);
  logd(")");
}

void ensure_vec2bf_inited(){
  static bool vec2bf_inited = false;
  if(!vec2bf_inited){
    vec2bf_inited = true;
    
    vec2bf_two = vec2bf_new(2, 2);
    vec2bf_one = vec2bf_new(1,1);
    vec2bf_zero = vec2bf_new(0,0);
    vec2bf_half = vec2bf_new(1, 1);
    vec2bf_div2(vec2bf_half, vec2bf_one, vec2bf_two);
    vec2bf_one_x = vec2bf_new(1, 0);
    vec2bf_one_y =_vec2bf_new(0, 1);
  }
}
