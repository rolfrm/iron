// Based on linmath.h
// https://github.com/datenwolf/linmath.h

typedef float v4sf __attribute__ ((vector_size (16)));
typedef float v2sf __attribute__ ((vector_size (8)));
typedef float v3sf __attribute__ ((vector_size (16)));

typedef struct{
  union {
    struct{
      float x,y;
    };
    float data[2];
    v2sf sse;
  };
}vec2;

typedef struct{
  union {
    struct{
      float x,y,z;
    };
    float data[3];
    vec2 xy;
    v3sf sse;
  };

}vec3;

typedef struct{
  union {
    struct{
      float x,y,z,w;
    };
    float data[4];
    vec3 xyz;
    v4sf sse;
  };
}vec4;

#define LINMATH_H_OP(n,name, op)			\
  vec##n vec##n##_##name (vec##n a, vec##n const b);

#define LINMATH_H_DEFINE_VEC(n)					\
  LINMATH_H_OP(n,add,+)						\
  LINMATH_H_OP(n,sub,-)						\
  LINMATH_H_OP(n,mul,*)						\
  LINMATH_H_OP(n,div,/)						\
  vec##n vec##n##_scale(vec##n v, float s);			\
  float vec##n##_mul_inner(vec##n a, vec##n b);			\
  float vec##n##_sqlen(vec##n v);				\
  float vec##n##_len(vec##n v);					\
  vec##n vec##n##_normalize(vec##n v);				\
  bool vec##n##_compare(vec##n v1, vec##n v2, float eps);	\
  
LINMATH_H_DEFINE_VEC(2)
LINMATH_H_DEFINE_VEC(3)
LINMATH_H_DEFINE_VEC(4)

vec2 vec2mk(float x, float y);
vec3 vec3mk(float x, float y, float z);
vec4 vec4mk(float x, float y, float z, float w);
vec2 vec2_new(float x, float y);
vec2 vec2_new1(float xy);
// element-wise min
vec2 vec2_min(vec2 a, vec2 b);
// element-wise max
vec2 vec2_max(vec2 a, vec2 b);
extern const vec2 vec2_infinity;
extern const vec2 vec2_zero;
extern const vec2 vec2_half;
extern const vec2 vec2_one;


vec3 vec3_new(float x, float y, float z);
vec3 vec3_new1(float v);
vec3 vec3_min(vec3 a, vec3 b);
vec3 vec3_max(vec3 a, vec3 b);
vec3 vec3_abs(vec3 a);
vec3 vec3_apply(vec3 a, float (*f)(float v));
float vec3_min_element(vec3 a);
float vec3_max_element(vec3 a);
extern const vec3 vec3_infinity;
extern const vec3 vec3_zero;
extern const vec3 vec3_half;
extern const vec3 vec3_one;
vec4 vec4_new(float x, float y, float z, float w);
vec2 vec2_round(vec2 v);
vec3 vec3_mul_cross(vec3 const a, vec3 const b);
vec3 vec3_reflect(vec3 const v, vec3 const n);
vec3 vec3_less(vec3 a, vec3 b);
vec3 vec3_gt(vec3 a, vec3 b);
vec3 vec3_gteq(vec3 a, vec3 b);
extern const vec4 vec4_infinity;
extern const vec4 vec4_zero;
extern const vec4 vec4_half;
extern const vec4 vec4_one;

bool vec3_eq(vec3 a, vec3 b);
vec4 vec4_mul_cross(vec4 a, vec4 b);
vec4 vec4_reflect(vec4 v, vec4 n);

typedef struct{
  union{
    struct{
      float 
	m00, m01,
	m10, m11;
    };
    vec2 columns[2];
    float data[2][2];
  };
}mat2;

mat2 mat2_identity();
mat2 mat2_rotation(float angle);
vec2 mat2_mul_vec2(mat2 m, vec2 v);
mat2 mat2_mul(mat2 a, mat2 b);
typedef struct{
  union{
    struct{
      float 
	m00, m01, m02, 
	m10, m11, m12, 
	m20, m21, m22;
    };
    vec3 columns[3];
    float data[3][3];
  };
}mat3;

mat3 mat3_identity();
vec3 mat3_col(mat3 m, int i);
vec3 mat3_row(mat3 m, int i);
mat3 mat3_transpose(mat3 m);
mat3 mat3_mul(mat3 a, mat3 b);
vec3 mat3_mul_vec3(mat3 m, vec3 v);
vec2 mat3_mul_vec2(mat3 m, vec2 v);
mat3 mat3_2d_rotation(float angle);
mat3 mat3_2d_translation(float x, float y);

typedef struct{
  union{
    struct{
      float 
	m00, m01, m02, m03,  // column, row
	m10, m11, m12, m13,
	m20, m21, m22, m23,
	m30, m31, m32, m33;
    };
    vec4 columns[4];
    float data[4][4];
  };
}mat4;

mat4 mat4_identity();
mat4 mat4_dup(mat4 N);
vec4 mat4_row(mat4 M, int i);
vec4 mat4_col(mat4 M, int i);
mat4 mat4_transpose(mat4 N);
mat4 mat4_add(mat4 a, mat4 b);
mat4 mat4_sub(mat4 a, mat4 b);
mat4 mat4_scale(mat4 a, float k);
mat4 mat4_scaled(float scale_x, float scale_y, float scale_z);
mat4 mat4_scale_transform(float x, float y, float z);
mat4 mat4_scale_aniso(mat4 a, float x, float y, float z);
mat4 mat4_mul(mat4 a, mat4 b);
vec4 mat4_mul_vec4( mat4 M, vec4 v);

//Assumes that mat4 is an affine transform.
vec3 mat4_mul_vec3( mat4 M, vec3 v);
mat4 mat4_translate(float x, float y, float z);
mat4 mat4_translate_in_place(mat4 M, float x, float y, float z);
mat4 mat4_from_vec3_mul_outer(vec3 a, vec3 b);
mat4 mat4_rotate(mat4 M, float x, float y, float z, float angle);
mat4 mat4_rotate_X(mat4 M, float angle);
mat4 mat4_rotate_Y(mat4 M, float angle);
mat4 mat4_rotate_Z(mat4 M, float angle);
mat4 mat4_invert(mat4 M);
vec3 mat4_orthonormalize(mat4 M);
mat4 mat4_frustum(float l, float r, float b, float t, float n, float f);
mat4 mat4_ortho(float l, float r, float b, float t, float n, float f);
mat4 mat4_perspective(float y_fov, float aspect, float n, float f);
mat4 mat4_look_at(vec3 eye, vec3 center, vec3 up);

typedef vec4 quat;
quat quat_identity();
quat quat_from_axis(vec3 dir, float angle);
quat quat_add(quat a, quat b);
quat quat_sub(quat a, quat b);
quat quat_mul(quat p, quat q);
quat quat_scale(quat v, float s);
float quat_inner_product(quat a, quat b);
quat quat_conj(quat q);
#define quat_normalize vec4_normalize
vec3 quat_mul_vec3(quat q, vec3 v);
mat4 mat4_from_quat(quat q);
mat4 quat_to_mat4(quat q);
mat4 mat4_mul_quat(mat4 M, quat q);
quat quat_from_mat4(mat4 M);

void mat4_print(mat4 mat);
void mat3_print(mat3 mat);
void mat2_print(mat2 mat);
void vec4_print(vec4 v);
void vec3_print(vec3 v);
void vec2_print(vec2 v);

bool linmath_test();
