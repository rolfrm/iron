struct fx;
typedef struct _fx {
  bf_t v;
}fx;
extern fx * fx_one, * fx_zero, * fx_half, * fx_two, * fx_minus_one;

fx * fx_new(void);
fx * fx_new1(f64 x);
fx * fx_newn(u32 count);
void fx_del(fx * f);
void fx_deln(fx * f, u32 count);

void fx_set(fx * a, f64 v);
void fx_set2(fx * a, const fx * b);
void fx_set_zero(fx * v);
int fx_isneg(fx * v);
void fx_add(fx * a, const fx * b);
void fx_sub(fx * a, const fx * b);
void fx_div(fx * a, const fx * b);
void fx_div2(fx * c,const fx * a, const fx * b);
void fx_mul(fx * a, const fx * b);

void fx_inv(fx * a);
bool fx_eq(const fx * a, const fx * b);
bool fx_gt(fx * a, fx * b);
bool fx_lt(fx * a, fx * b);

void fx_print(fx * a);
f64 fx_to_f64(fx * a);

fx * fx_deserialize(void * text, size_t s);
void * fx_serialize(fx * v, size_t * s);
/*

  fx a = fx_new(5);
  fx_add(a, 10);
  

 */
