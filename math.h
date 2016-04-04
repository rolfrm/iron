
extern double PI;

extern const f32 f32_infinity;
extern const f32 f32_negative_infinity;
extern const f64 f64_infinity;
extern const f64 f64_negative_infinity;

// returns the 1-based index of the highest bit of x.
u8 hibit(u64 x);

// returns true if two floats are equal given 'prec' precission.
int feq(float a, float b, float prec);

