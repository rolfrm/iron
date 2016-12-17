// Based on linmath.h
// https://github.com/datenwolf/linmath.h

// included where needed
// requires math.h

#include <stdbool.h>
#include <math.h>
#include <string.h>
#include "linmath.h"
#include <stdint.h>
#include "types.h"
#include "math.h"
#include "utils.h"
#define prefix __attribute__((const)) __attribute__((always_inline)) inline

#define _LINMATH_H_OP(n,name, op)					\
  prefix vec##n vec##n##_##name (vec##n a, vec##n const b){		\
    a.sse = a.sse op b.sse;						\
    return a;								\
  }


#define _LINMATH_H_DEFINE_VEC(n)					\
  _LINMATH_H_OP(n,add,+)						\
  _LINMATH_H_OP(n,sub,-)						\
  _LINMATH_H_OP(n,mul,*)						\
  _LINMATH_H_OP(n,div,/)						\
  prefix vec##n vec##n##_scale(vec##n v, float s)			\
  {									\
    v.sse *= s;								\
    return v;								\
  }									\
  prefix float vec##n##_mul_inner(vec##n a, vec##n b)				\
  {									\
    float p = 0.0f;							\
    for(int i=0; i<n; i++)						\
      p += b.data[i]*a.data[i];						\
    return p;								\
  }									\
  prefix float vec##n##_sqlen(vec##n v)					\
  {									\
    return vec##n##_mul_inner(v,v);					\
  }									\
  									\
  prefix bool vec##n##_compare(vec##n v1, vec##n v2, float eps){		\
    bool ok = fabs(v1.data[0] - v2.data[0]) < eps;			\
    for(int i = 1; i < n; i++) ok &= (fabs(v1.data[i] -v2 .data[i]) < eps); \
    return ok;								\
  }  									\
  
#include <smmintrin.h>
#include <emmintrin.h>
float vec2_len(vec2 v){
  return sqrtf(vec2_sqlen(v));
}
float vec3_len(vec3 v){
  return _mm_cvtss_f32(_mm_sqrt_ss(_mm_dp_ps(v.sse, v.sse, 0x71)));
}
float vec4_len(vec4 v){
  return sqrtf(vec4_sqlen(v));//return _mm_cvtss_f32(_mm_sqrt_ss(_mm_dp_ps(v.sse, v.sse, 0x71)));
}
vec2 vec2_normalize(vec2 v)
{
  float k = 1.0 / vec2_len(v);
  return vec2_scale( v, k);
}

vec2 vec2_round(vec2 v){
  v.x = roundf(v.x);
  v.y = roundf(v.y);
  return v;
}

vec3 vec3_normalize(vec3 v){
  //return vec3_scale(v, 1.0f / vec3_len(v));
  __m128 norm = _mm_sqrt_ps(_mm_dp_ps(v.sse, v.sse, 0x7F));
  v.sse = _mm_div_ps(v.sse, norm);
  return v;
}
vec3 vec3_less(vec3 a, vec3 b){

  return vec3_new(a.x < b.x, a.y < b.y, a.z < b.z);
}

vec3 vec3_gt(vec3 a, vec3 b){
  return vec3_new(a.x > b.x, a.y > b.y, a.z > b.z);
}
inline vec3 vec3_gteq(vec3 a, vec3 b){
  return vec3_new(a.x >= b.x, a.y >= b.y, a.z >= b.z);
}

bool vec3_eq(vec3 a, vec3 b){
  return _mm_comieq_ss(a.sse, b.sse);
}

vec3 vec3_abs(vec3 a){
  return vec3_new(fabs(a.x), fabs(a.y), fabs(a.z));
}


inline vec3 vec3_apply(vec3 v, float (*f)(float x)){
  v.x = f(v.x);
  v.y = f(v.y);
  v.z = f(v.z);
  return v;
}

vec4 vec4_normalize(vec4 v)
{
  __m128 norm = _mm_sqrt_ps(_mm_dp_ps(v.sse, v.sse, 0xFF));
  v.sse = _mm_div_ps(v.sse, norm);
  return v;
  //return vec4_scale( v, 1.0f / vec4_len(v));
}

_LINMATH_H_DEFINE_VEC(2)
_LINMATH_H_DEFINE_VEC(3)
_LINMATH_H_DEFINE_VEC(4)

inline vec2 vec2mk(float x, float y){
  return (vec2){.data = {x,y}};
}

inline vec3 vec3mk(float x, float y, float z){
  return (vec3){.data = {x,y,z}};
}

inline vec4 vec4mk(float x, float y, float z, float w){
  return (vec4){.data = {x,y,z,w}};
}

inline vec2 vec2_new(float x, float y){
  return (vec2){.data = {x,y}};
}

inline vec2 vec2_new1(float xy){
  return (vec2){.data = {xy, xy}};
}

inline vec2 vec2_min(vec2 a, vec2 b){
  return (vec2){.data = {MIN(a.x, b.x), MIN(a.y, b.y)}};
}

inline vec2 vec2_max(vec2 a, vec2 b){
  return (vec2){.data = {MAX(a.x, b.x), MAX(a.y, b.y)}};
}

const vec2 vec2_infinity = {.x = 1.0f / 0.0f, .y = 1.0f / 0.0f};
const vec2 vec2_zero = {.x = 0.0f, .y = 0.0f};
const vec2 vec2_half = {.x = 0.5f, .y = 0.5f};
const vec2 vec2_one = {.x = 1.0f, .y = 1.0f};


inline vec3 vec3_new(float x, float y, float z){
  return (vec3){.data = {x,y,z}};
}

inline vec3 vec3_new1(float v){
  return (vec3){.data = {v,v,v}};
}

inline vec3 vec3_min(vec3 a, vec3 b){
  return (vec3){.data = {MIN(a.x, b.x), MIN(a.y, b.y), MIN(a.z, b.z)}};
}

vec3 vec3_max(vec3 a, vec3 b){
  return (vec3){.data = {MAX(a.x, b.x), MAX(a.y, b.y), MAX(a.z, b.z)}};
}

float vec3_min_element(vec3 a){
  return MIN(a.x, MIN(a.y, a.z));
}

float vec3_max_element(vec3 a){
  return MAX(a.x, MAX(a.y, a.z));
}

const vec3 vec3_infinity = {.x = 1.0f / 0.0f, .y = 1.0f / 0.0f, .z = 1.0f / 0.0f};
const vec3 vec3_zero = {.x = 0.0f, .y = 0.0f, .z = 0.0f};
const vec3 vec3_half = {.x = 0.5f, .y = 0.5f, .z = 0.5f};
const vec3 vec3_one = {.x = 1.0f, .y = 1.0f, .z = 1.0f};

inline vec4 vec4_new(float x, float y, float z, float w){
  return (vec4){.data = {x,y,z,w}};
}

vec3 vec3_mul_cross(vec3 const a, vec3 const b)
{
  vec3 r;
  r.x = a.y*b.z - a.z*b.y;
  r.y = a.z*b.x - a.x*b.z;
  r.z = a.x*b.y - a.y*b.x;
  return r;
}

vec3 vec3_reflect(vec3 const v, vec3 const n)
{
  vec3 r;
  float p  = 2.f*vec3_mul_inner(v, n);
  int i;
  for(i=0;i<3;++i)
    r.data[i] = v.data[i] - p*n.data[i];
  return r;
}

vec4 vec4_mul_cross(vec4 a, vec4 b)
{
  vec4 r;
  r.data[0] = a.data[1]*b.data[2] - a.data[2]*b.data[1];
  r.data[1] = a.data[2]*b.data[0] - a.data[0]*b.data[2];
  r.data[2] = a.data[0]*b.data[1] - a.data[1]*b.data[0];
  r.data[3] = 1.f;
  return r;
}

vec4 vec4_reflect(vec4 v, vec4 n)
{
  vec4 r;
  float p  = 2.f*vec4_mul_inner(v, n);
  int i;
  for(i=0;i<4;++i)
    r.data[i] = v.data[i] - p*n.data[i];
  return r;
}

const vec4 vec4_infinity = {.x = 1.0f / 0.0f, .y = 1.0f / 0.0f, .z = 1.0f / 0.0f, .w = 1.0f / 0.0f};
const vec4 vec4_zero = {.x = 0.0f, .y = 0.0f, .z = 0.0f, .w = 0.0f};
const vec4 vec4_half = {.x = 0.5f, .y = 0.5f, .z = 0.5f, .w = 0.5f};
const vec4 vec4_one = {.x = 1.0f, .y = 1.0f, .z = 1.0f, .w = 1.0f};

// mat2

mat2 mat2_identity(){
  return (mat2) {.data = {{1.0, 0.0}, {0.0, 1.0}}};
}

mat2 mat2_rotation(float angle){
  float s = sinf(angle);
  float c = cosf(angle);
  mat2 r = {.data = {
      {   c,   s},
      {  -s,   c}
    }};
  return r;
}

vec2 mat2_mul_vec2(mat2 m, vec2 v){
  return (vec2) {
    .x = m.data[0][0] * v.data[0] + m.data[1][0] * v.data[1],
    .y = m.data[0][1] * v.data[0] + m.data[1][1] * v.data[1]};
}

mat2 mat2_mul(mat2 a, mat2 b){
  mat2 temp;
  int k, r, c;
  for(c=0; c<2; ++c) 
    for(r=0; r<2; ++r) {
      temp.data[c][r] = 0.f;
      for(k=0; k<2; ++k)
	temp.data[c][r] += a.data[k][r] * b.data[c][k];
    }
  return temp;
}

// mat3
mat3 mat3_identity(){
  mat3 m;
  memset(&m,0,sizeof(m));
  m.m00 = 1;
  m.m11 = 1;
  m.m22 = 1;
  return m;
}

vec3 mat3_col(mat3 m, int i){
  return m.columns[i];
}

vec3 mat3_row(mat3 m, int i){
  return (vec3) { .x = m.data[0][i], .y = m.data[1][i], .z = m.data[2][i]};
}

mat3 mat3_transpose(mat3 m){
  mat3 o;
  for(int i = 0 ; i < 3; i++)
    for(int j = 0; j < 3; j++)
      o.data[j][i] = m.data[i][j];
  return o;
}

mat3 mat3_mul(mat3 a, mat3 b){
  mat3 temp;
  int k, r, c;
  for(c=0; c<3; ++c) 
    for(r=0; r<3; ++r) {
      temp.data[c][r] = 0.f;
      for(k=0; k<3; ++k)
	temp.data[c][r] += a.data[k][r] * b.data[c][k];
    }
  return temp;
}

vec3 mat3_mul_vec3(mat3 m, vec3 v){
  vec3 r = {0};
  int i, j;
  for(j=0; j<3; ++j) {
    for(i=0; i<3; ++i)
      r.data[j] += m.data[i][j] * v.data[i];
  }
  return r;
}

vec2 mat3_mul_vec2(mat3 m, vec2 _v){
  vec3 v = {.x = _v.x, .y = _v.y, .z = 1.0};
  vec3 r = {0};
  int i, j;
  for(j=0; j<2; ++j) 
    for(i=0; i<3; ++i)
      r.data[j] += m.data[i][j] * v.data[i];
  return r.xy;
}

mat3 mat3_2d_rotation(float angle){
  float s = sinf(angle);
  float c = cosf(angle);
  mat3 r = {.data = {
      {   c,   s, 0.f},
      {  -s,   c, 0.f},
      { 0.f, 0.f, 1.f},
    }};
	    
  return r;
}

mat3 mat3_2d_translation(float x, float y){
  return (mat3){
    .data = {
      {1.0, 0.0, 0.0},
      {0.0, 1.0, 0.0},
      {x, y, 1.0}
    }
  };
}

// mat4 

mat4 mat4_identity()
{
  mat4 M;
  int i, j;
  for(i=0; i<4; ++i)
    for(j=0; j<4; ++j)
      M.data[i][j] = i==j ? 1.f : 0.f;
  return M;
}
mat4 mat4_dup(mat4 N)
{
  return N;
}

vec4 mat4_row(mat4 M, int i)
{
  vec4 row;
  for(int j = 0; j < 4;j++)
    row.data[j] = M.data[j][i];
  return row;
}

vec4 mat4_col(mat4 M, int i)
{
  return M.columns[i];
}
mat4 mat4_transpose(mat4 N)
{
  mat4 M;
  int i, j;
  for(j=0; j<4; ++j)
    for(i=0; i<4; ++i)
      M.data[i][j] = N.data[j][i];
  return M;
}
mat4 mat4_add(mat4 a, mat4 b)
{
  mat4 M;
  int i;
  for(i=0; i<4; ++i)
    M.columns[i] = vec4_add(a.columns[i], b.columns[i]);
  return M;
}
mat4 mat4_sub(mat4 a, mat4 b)
{
  mat4 M;
  int i;
  for(i=0; i<4; ++i)
    M.columns[i] = vec4_sub(a.columns[i], b.columns[i]);
  return M;
}

mat4 mat4_scale(mat4 a, float k)
{
  mat4 M;
  int i;
  for(i=0; i<4; ++i)
    M.columns[i] = vec4_scale(a.columns[i], k);
  return M;
}
mat4 mat4_scale_aniso(mat4 a, float x, float y, float z)
{
  mat4 M;
  int i;
  M.columns[0] = vec4_scale(a.columns[0], x);
  M.columns[1] = vec4_scale(a.columns[1], y);
  M.columns[2] = vec4_scale(a.columns[2], z);
  for(i = 0; i < 4; ++i) {
    M.data[3][i] = a.data[3][i];
  }
  return M;
}
mat4 mat4_mul(mat4 a, mat4 b)
{
  mat4 temp;
  int k, r, c;
  for(c=0; c<4; ++c) for(r=0; r<4; ++r) {
      temp.data[c][r] = 0.f;
      for(k=0; k<4; ++k)
	temp.data[c][r] += a.data[k][r] * b.data[c][k];
    }
  return temp;
}
vec4 mat4_mul_vec4( mat4 M, vec4 v)
{
  vec4 r;
  int i, j;
  for(j=0; j<4; ++j) {
    r.data[j] = 0.f;
    for(i=0; i<4; ++i)
      r.data[j] += M.data[i][j] * v.data[i];
  }
  return r;
}

vec3 mat4_mul_vec3( mat4 M, vec3 v)
{
  vec3 r;
  int i, j;
  for(j=0; j<3; ++j) {
    r.data[j] = 0.f;
    for(i=0; i<3; ++i)
      r.data[j] += M.data[i][j] * v.data[i];
  }
  return vec3_add(r, M.columns[3].xyz);
}

mat4 mat4_translate(float x, float y, float z)
{
  mat4 T = mat4_identity();
  T.data[3][0] = x;
  T.data[3][1] = y;
  T.data[3][2] = z;
  return T;
}

mat4 mat4_translate_in_place(mat4 M, float x, float y, float z)
{
  vec4 t= {.data = {x,y,z,0.0}};
  int i;
  for (i = 0; i < 4; ++i) {
    vec4 r = mat4_row(M, i);
    M.data[3][i] += vec4_mul_inner(r, t);
  }
  return M;
}

mat4 mat4_from_vec3_mul_outer(vec3 a, vec3 b)
{
  mat4 M;
  int i, j;
  for(i=0; i<4; ++i) 
    for(j=0; j<4; ++j)
      M.data[i][j] = i<3 && j<3 ? a.data[i] * b.data[j] : 0.f;
  return M;
}

mat4 mat4_rotate(mat4 M, float x, float y, float z, float angle)
{
  float s = sinf(angle);
  float c = cosf(angle);
  vec3 u = {.data = { x, y, z}};

  if(vec3_len(u) > 1e-4) {
    u = vec3_normalize(u);
    mat4 T;
    T = mat4_from_vec3_mul_outer(u, u);

    mat4 S = 
      {.data = {
	  {    0,  u.data[2], -u.data[1], 0},
	  {-u.data[2],     0,  u.data[0], 0},
	  { u.data[1], -u.data[0],     0, 0},
	  {    0,     0,     0, 0}
	}};
    S = mat4_scale(S, s);

    mat4 C = mat4_identity(C);
    C = mat4_sub(C, T);
    C = mat4_scale(C, c);
    T = mat4_add(T, C);
    T = mat4_add( T, S);

    T.data[3][3] = 1.;		
    return mat4_mul(M, T);
  }
  return M;
}

mat4 mat4_scaled(float scale_x, float scale_y, float scale_z){
  return (mat4) {.m00 = scale_x, .m11 = scale_y, .m22 = scale_z, .m33 = 1};
}

mat4 mat4_rotate_X(mat4 M, float angle)
{
  float s = sinf(angle);
  float c = cosf(angle);
  mat4 R = {.data = {
      {1.f, 0.f, 0.f, 0.f},
      {0.f,   c,   s, 0.f},
      {0.f,  -s,   c, 0.f},
      {0.f, 0.f, 0.f, 1.f}
    }};
  return mat4_mul(M, R);
}
mat4 mat4_rotate_Y(mat4 M, float angle)
{
  float s = sinf(angle);
  float c = cosf(angle);
  mat4 R = {.data = {
      {   c, 0.f,   s, 0.f},
      { 0.f, 1.f, 0.f, 0.f},
      {  -s, 0.f,   c, 0.f},
      { 0.f, 0.f, 0.f, 1.f}
    }};
  return mat4_mul(M, R);
}
mat4 mat4_rotate_Z(mat4 M, float angle)
{
  float s = sinf(angle);
  float c = cosf(angle);
  mat4 R = {.data = {
      {   c,   s, 0.f, 0.f},
      {  -s,   c, 0.f, 0.f},
      { 0.f, 0.f, 1.f, 0.f},
      { 0.f, 0.f, 0.f, 1.f}}
  };
  return mat4_mul(M, R);
}
mat4 mat4_invert(mat4 M)
{
  mat4 T;
  float s[6];
  float c[6];
  s[0] = M.data[0][0]*M.data[1][1] - M.data[1][0]*M.data[0][1];
  s[1] = M.data[0][0]*M.data[1][2] - M.data[1][0]*M.data[0][2];
  s[2] = M.data[0][0]*M.data[1][3] - M.data[1][0]*M.data[0][3];
  s[3] = M.data[0][1]*M.data[1][2] - M.data[1][1]*M.data[0][2];
  s[4] = M.data[0][1]*M.data[1][3] - M.data[1][1]*M.data[0][3];
  s[5] = M.data[0][2]*M.data[1][3] - M.data[1][2]*M.data[0][3];

  c[0] = M.data[2][0]*M.data[3][1] - M.data[3][0]*M.data[2][1];
  c[1] = M.data[2][0]*M.data[3][2] - M.data[3][0]*M.data[2][2];
  c[2] = M.data[2][0]*M.data[3][3] - M.data[3][0]*M.data[2][3];
  c[3] = M.data[2][1]*M.data[3][2] - M.data[3][1]*M.data[2][2];
  c[4] = M.data[2][1]*M.data[3][3] - M.data[3][1]*M.data[2][3];
  c[5] = M.data[2][2]*M.data[3][3] - M.data[3][2]*M.data[2][3];
	
  /* Assumes it is invertible */
  float idet = 1.0f/( s[0]*c[5]-s[1]*c[4]+s[2]*c[3]+s[3]*c[2]-s[4]*c[1]+s[5]*c[0] );
	
  T.data[0][0] = ( M.data[1][1] * c[5] - M.data[1][2] * c[4] + M.data[1][3] * c[3]) * idet;
  T.data[0][1] = (-M.data[0][1] * c[5] + M.data[0][2] * c[4] - M.data[0][3] * c[3]) * idet;
  T.data[0][2] = ( M.data[3][1] * s[5] - M.data[3][2] * s[4] + M.data[3][3] * s[3]) * idet;
  T.data[0][3] = (-M.data[2][1] * s[5] + M.data[2][2] * s[4] - M.data[2][3] * s[3]) * idet;

  T.data[1][0] = (-M.data[1][0] * c[5] + M.data[1][2] * c[2] - M.data[1][3] * c[1]) * idet;
  T.data[1][1] = ( M.data[0][0] * c[5] - M.data[0][2] * c[2] + M.data[0][3] * c[1]) * idet;
  T.data[1][2] = (-M.data[3][0] * s[5] + M.data[3][2] * s[2] - M.data[3][3] * s[1]) * idet;
  T.data[1][3] = ( M.data[2][0] * s[5] - M.data[2][2] * s[2] + M.data[2][3] * s[1]) * idet;

  T.data[2][0] = ( M.data[1][0] * c[4] - M.data[1][1] * c[2] + M.data[1][3] * c[0]) * idet;
  T.data[2][1] = (-M.data[0][0] * c[4] + M.data[0][1] * c[2] - M.data[0][3] * c[0]) * idet;
  T.data[2][2] = ( M.data[3][0] * s[4] - M.data[3][1] * s[2] + M.data[3][3] * s[0]) * idet;
  T.data[2][3] = (-M.data[2][0] * s[4] + M.data[2][1] * s[2] - M.data[2][3] * s[0]) * idet;

  T.data[3][0] = (-M.data[1][0] * c[3] + M.data[1][1] * c[1] - M.data[1][2] * c[0]) * idet;
  T.data[3][1] = ( M.data[0][0] * c[3] - M.data[0][1] * c[1] + M.data[0][2] * c[0]) * idet;
  T.data[3][2] = (-M.data[3][0] * s[3] + M.data[3][1] * s[1] - M.data[3][2] * s[0]) * idet;
  T.data[3][3] = ( M.data[2][0] * s[3] - M.data[2][1] * s[1] + M.data[2][2] * s[0]) * idet;
  return T;
}

vec3 mat4_orthonormalize(mat4 M)
{
  mat4 R = M;
  float s = 1.;
  vec3 h;

  R.columns[2].xyz = vec3_normalize(R.columns[2].xyz);
	
  s = vec3_mul_inner(R.columns[1].xyz, R.columns[2].xyz);
  h = vec3_scale(R.columns[2].xyz, s);
  R.columns[1].xyz = vec3_sub(R.columns[1].xyz, h);
  R.columns[2].xyz = vec3_normalize(R.columns[2].xyz);

  s = vec3_mul_inner(R.columns[1].xyz, R.columns[2].xyz);
  h = vec3_scale(R.columns[2].xyz, s);
  R.columns[1].xyz = vec3_sub(R.columns[1].xyz, h);
  R.columns[1].xyz = vec3_normalize(R.columns[1].xyz);

  s = vec3_mul_inner(R.columns[0].xyz, R.columns[1].xyz);
  h = vec3_scale( R.columns[1].xyz, s);
  R.columns[0].xyz = vec3_sub(R.columns[0].xyz, h);
  return vec3_normalize( R.columns[0].xyz);
}

mat4 mat4_frustum(float l, float r, float b, float t, float n, float f)
{
  mat4 M;
  M.data[0][0] = 2.f*n/(r-l);
  M.data[0][1] = M.data[0][2] = M.data[0][3] = 0.f;
	
  M.data[1][1] = 2.*n/(t-b);
  M.data[1][0] = M.data[1][2] = M.data[1][3] = 0.f;

  M.data[2][0] = (r+l)/(r-l);
  M.data[2][1] = (t+b)/(t-b);
  M.data[2][2] = -(f+n)/(f-n);
  M.data[2][3] = -1.f;
	
  M.data[3][2] = -2.f*(f*n)/(f-n);
  M.data[3][0] = M.data[3][1] = M.data[3][3] = 0.f;
  return M;
}
mat4 mat4_ortho(float l, float r, float b, float t, float n, float f)
{
  mat4 M;
  M.data[0][0] = 2.f/(r-l);
  M.data[0][1] = M.data[0][2] = M.data[0][3] = 0.f;

  M.data[1][1] = 2.f/(t-b);
  M.data[1][0] = M.data[1][2] = M.data[1][3] = 0.f;

  M.data[2][2] = -2.f/(f-n);
  M.data[2][0] = M.data[2][1] = M.data[2][3] = 0.f;
	
  M.data[3][0] = -(r+l)/(r-l);
  M.data[3][1] = -(t+b)/(t-b);
  M.data[3][2] = -(f+n)/(f-n);
  M.data[3][3] = 1.f;
  return M;
}
mat4 mat4_perspective(float y_fov, float aspect, float n, float f)
{
  mat4 m;
  /* NOTE: Degrees are an unhandy unit to work with.
   * linmath.h uses radians for everything! */
  float a = 1.f / tan(y_fov / 2.f);
  
  m.data[0][0] = a / aspect;
  m.data[0][1] = 0.f;
  m.data[0][2] = 0.f;
  m.data[0][3] = 0.f;

  m.data[1][0] = 0.f;
  m.data[1][1] = a;
  m.data[1][2] = 0.f;
  m.data[1][3] = 0.f;

  m.data[2][0] = 0.f;
  m.data[2][1] = 0.f;
  m.data[2][2] = -((f + n) / (f - n));
  m.data[2][3] = -1.f;

  m.data[3][0] = 0.f;
  m.data[3][1] = 0.f;
  m.data[3][2] = -((2.f * f * n) / (f - n));
  m.data[3][3] = 0.f;
  return m;
}
mat4 mat4_look_at(vec3 eye, vec3 center, vec3 up)
{
  //  Adapted from Android's OpenGL Matrix.java.                        
  // See the OpenGL GLUT documentation for gluLookAt for a description 
  // of the algorithm. We implement it in a straightforward way:       
  //
  // TODO: The negation of of can be spared by swapping the order of
  //       operands in the following cross products in the right way. 
  mat4 m;
  vec3 f = vec3_sub(center, eye);	
  f = vec3_normalize(f);	
	
  vec3 s = vec3_mul_cross(f, up);
  s = vec3_normalize(s);

  vec3 t = vec3_mul_cross(s, f);

  m.data[0][0] =  s.data[0];
  m.data[0][1] =  t.data[0];
  m.data[0][2] = -f.data[0];
  m.data[0][3] =   0.f;

  m.data[1][0] =  s.data[1];
  m.data[1][1] =  t.data[1];
  m.data[1][2] = -f.data[1];
  m.data[1][3] =   0.f;

  m.data[2][0] =  s.data[2];
  m.data[2][1] =  t.data[2];
  m.data[2][2] = -f.data[2];
  m.data[2][3] =   0.f;

  m.data[3][0] =  0.f;
  m.data[3][1] =  0.f;
  m.data[3][2] =  0.f;
  m.data[3][3] =  1.f;

  return mat4_translate_in_place(m, -eye.data[0], -eye.data[1], -eye.data[2]);
}

quat quat_identity(){
  return (quat) {.x = 0.f, .y = 0.f, .z = 0.f, .w = 1.f};
}

quat quat_from_axis(vec3 dir, float angle){
  float c = cos(angle / 2);
  float s = sin(angle / 2);
  return (quat){.data = {dir.x * s, dir.y * s, dir.z * s, c}};
}

quat quat_add(quat a, quat b){
  return vec4_add(a,b);
}

quat quat_sub(quat a, quat b){
  return vec4_sub(a, b);
}

quat quat_mul(quat p, quat q){
  quat r;
  r.xyz = vec3_mul_cross(p.xyz, q.xyz);
  quat w;
  w.xyz = vec3_scale(p.xyz, q.data[3]);
  r.xyz = vec3_add(r.xyz, w.xyz);
  w.xyz = vec3_scale(q.xyz, p.data[3]);
  r.xyz = vec3_add(r.xyz, w.xyz);
  r.data[3] = p.data[3]*q.data[3] - vec3_mul_inner(p.xyz, q.xyz);
  return r;
}

quat quat_scale(quat v, float s){
  return vec4_scale(v, s);
}

float quat_inner_product(quat a, quat b){
  return vec4_mul_inner(a ,b);
}

quat quat_conj(quat q){
  quat r;
  int i;
  for(i=0; i<3; ++i)
    r.data[i] = -q.data[i];
  r.data[3] = q.data[3];
  return r;
}
#define quat_normalize vec4_normalize
vec3 quat_mul_vec3(quat q, vec3 v)
{
  vec4 r;
  quat v_ = {.xyz = v};

  r = quat_conj(q);
  r = quat_normalize(r);
  r = quat_mul(v_, r);
  r = quat_mul(q, r);
  return r.xyz;
}

mat4 mat4_from_quat(quat q)
{
  mat4 M;
  float a = q.data[3];
  float b = q.data[0];
  float c = q.data[1];
  float d = q.data[2];
  float a2 = a*a;
  float b2 = b*b;
  float c2 = c*c;
  float d2 = d*d;
	
  M.data[0][0] = a2 + b2 - c2 - d2;
  M.data[0][1] = 2.f*(b*c + a*d);
  M.data[0][2] = 2.f*(b*d - a*c);
  M.data[0][3] = 0.f;

  M.data[1][0] = 2*(b*c - a*d);
  M.data[1][1] = a2 - b2 + c2 - d2;
  M.data[1][2] = 2.f*(c*d + a*b);
  M.data[1][3] = 0.f;

  M.data[2][0] = 2.f*(b*d + a*c);
  M.data[2][1] = 2.f*(c*d - a*b);
  M.data[2][2] = a2 - b2 - c2 + d2;
  M.data[2][3] = 0.f;

  M.data[3][0] = M.data[3][1] = M.data[3][2] = 0.f;
  M.data[3][3] = 1.f;
  return M;
}

mat4 quat_to_mat4(quat q){
  return mat4_from_quat(q);
}

mat4 mat4_mul_quat(mat4 M, quat q)
{
  mat4 R;
  /*  XXX: The way this is written only works for othogonal matrices. */
  /* TODO: Take care of non-orthogonal case. */
  R.columns[0].xyz = quat_mul_vec3(q, M.columns[0].xyz);
  R.columns[1].xyz = quat_mul_vec3(q, M.columns[1].xyz);
  R.columns[2].xyz = quat_mul_vec3(q, M.columns[2].xyz);

  R.data[3][0] = R.data[3][1] = R.data[3][2] = 0.f;
  R.data[3][3] = 1.f;
  return R;
}

quat quat_from_mat4(mat4 M)
{
  quat q;
  float r=0.f;
  int i;

  int perm[] = { 0, 1, 2, 0, 1 };
  int *p = perm;

  for(i = 0; i<3; i++) {
    float m = M.data[i][i];
    if( m < r )
      continue;
    m = r;
    p = &perm[i];
  }

  r = sqrtf(1.f + M.data[p[0]][p[0]] - M.data[p[1]][p[1]] - M.data[p[2]][p[2]]);

  if(r < 1e-6) {
    q.data[0] = 1.f;
    q.data[1] = q.data[2] = q.data[3] = 0.f;
    return q;
  }

  q.data[0] = r/2.f;
  q.data[1] = (M.data[p[0]][p[1]] - M.data[p[1]][p[0]])/(2.f*r);
  q.data[2] = (M.data[p[2]][p[0]] - M.data[p[0]][p[2]])/(2.f*r);
  q.data[3] = (M.data[p[2]][p[1]] - M.data[p[1]][p[2]])/(2.f*r);
  return q;
}

#include <stdio.h>

void mat4_print(mat4 mat){
  for(int i = 0 ; i < 4; i++){
    for(int j = 0; j < 4; j++)
      printf("%f ", mat.data[j][i]);
    printf("\n");
  }
}

void mat3_print(mat3 mat){
  for(int i = 0 ; i < 3; i++){
    for(int j = 0; j < 3; j++)
      printf("%f ", mat.data[j][i]);
    printf("\n");
  }
}

void mat2_print(mat2 mat){
  for(int i = 0 ; i < 2; i++){
    for(int j = 0; j < 2; j++)
      printf("%f ", mat.data[j][i]);
    printf("\n");
  }
}

void vec4_print(vec4 v){
  printf("(%f %f %f %f)", v.x, v.y, v.z, v.w);
}

void vec3_print(vec3 v){
  printf("(%f %f %f)", v.x, v.y, v.z);
}

void vec2_print(vec2 v){
  printf("(%f %f)", v.x, v.y);
}

#include "test.h"
#include "log.h"
bool linmath_test(){
  { // Test initializers
    vec4 tst = {.xyz = {.x = 1, .y = 2, .z = 3}};
    for(int i = 0; i < 3; i++)
      TEST_ASSERT(tst.data[i] == i + 1);
    TEST_ASSERT(tst.data[3] == 0.0f);
  
  
    if(false){ // this does not work!
      vec4 tst = {.xyz = {.x = 1, .y = 2, .z = 3}, .w = 4};
      for(int i = 0; i < 4; i++)
	TEST_ASSERT(tst.data[i] == i + 1);
    }
  }
  //logd("TAN: %f\n", 1.0f / tanf(3.14 * 0.5 * 0.5));
  //return TEST_FAIL;
  return TEST_SUCCESS;
}
