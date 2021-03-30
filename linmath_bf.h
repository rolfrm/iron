
typedef struct {
  bf_t x, y;
}vec2bf;

void ensure_vec2bf_inited();
extern vec2bf * vec2bf_zero;
extern vec2bf * vec2bf_one;
extern vec2bf * vec2bf_half;
extern vec2bf * vec2bf_one_x;
extern vec2bf * vec2bf_one_y;
extern vec2bf * vec2bf_two;

void vec2bf_add(vec2bf * a, vec2bf * b);
void vec2bf_sub(vec2bf * a, vec2bf * b);

void vec2bf_set(vec2bf * v, f64 x, f64 y);
vec2bf * vec2bf_new(f64 x, f64 y);
vec2bf * vec2bf_new2(vec2 x);
vec2bf * vec2bf_new3(vec2bf * x);
void vec2bf_del(vec2bf * v);
void vec2bf_add(vec2bf * a, vec2bf * b);
void vec2bf_sub(vec2bf * a, vec2bf * b);
void vec2bf_mul(vec2bf * a, vec2bf * b);
// this precision seems to be important for moving between bookmarks.
// how iterating a bit it can be significantly lower than otherwise.
#define div_prec 60

void vec2bf_div2(vec2bf * c, vec2bf * a, vec2bf * b);
void vec2bf_div(vec2bf * a, vec2bf * b);
void vec2bf_inv(vec2bf * a);
bool vec2bf_cmp(vec2bf * a, vec2bf * b);
f64 vec2bf_len(vec2bf * a);
vec2 vec2bf_to_vec2(vec2bf * v);
void vec2bf_to_xy(vec2bf * v, f64 * x, f64 * y);

void vec2bf_print(vec2bf * v);
