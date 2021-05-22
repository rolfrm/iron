
typedef struct {
  fx * x, *y;
}vec2bf;

extern bf_context_t bf_ctx;
void ensure_vec2bf_inited(void);
extern const vec2bf * vec2bf_zero;
extern const vec2bf * vec2bf_one;
extern const vec2bf * vec2bf_minus_one;
extern const vec2bf * vec2bf_half;
extern const vec2bf * vec2bf_one_x;
extern const vec2bf * vec2bf_one_y;
extern const vec2bf * vec2bf_two;

void vec2bf_set(vec2bf * v, f64 x, f64 y);
void vec2bf_set2(vec2bf * v, const vec2bf * other);
vec2bf * vec2bf_new(f64 x, f64 y);
vec2bf * vec2bf_new2(vec2 x);
vec2bf * vec2bf_new3(const vec2bf * x);
void vec2bf_del(vec2bf * v);
vec2bf * vec2bf_clone(const vec2bf * x);
void vec2bf_clear(vec2bf * a);

void vec2bf_add(vec2bf * a, const vec2bf * b);
void vec2bf_add_fx(vec2bf * a, const fx * b);
void vec2bf_sub(vec2bf * a, const vec2bf * b);
void vec2bf_sub_fx(vec2bf * a, const fx * b);
void vec2bf_mul(vec2bf * a, const vec2bf * b);
void vec2bf_mul_fx(vec2bf * a, const fx * b);
void vec2bf_scale_f32(vec2bf * a, f32 s);
// this precision seems to be important for moving between bookmarks.
// how iterating a bit it can be significantly lower than otherwise.
#define div_prec 60

void vec2bf_div2(vec2bf * c, const vec2bf * a, const vec2bf * b);
void vec2bf_div(vec2bf * a, const vec2bf * b);
void vec2bf_div_fx(vec2bf * a, const fx * b);
void vec2bf_inv(vec2bf * a);
bool vec2bf_cmp(const vec2bf * a, const vec2bf * b);
f64 vec2bf_len(const vec2bf * a);
vec2 vec2bf_to_vec2(const vec2bf * v);
void vec2bf_to_xy(const vec2bf * v, f64 * x, f64 * y);

void vec2bf_print(const vec2bf * v);
