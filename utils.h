
#define STRINGIZE_DETAIL(x) #x
#define STRINGIZE(x) STRINGIZE_DETAIL(x)

#define array_element_size(array) sizeof(array[0])
#define array_count(array) (sizeof(array)/array_element_size(array))

#define UNUSED(x) (void)(x)

#define MAX(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define MIN(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

#define ABS(a) MAX(a,-a)

// gets the sign of value {-1 or 1}.
#define SIGN(x)\
  ({ __typeof__(x) _x = x; \
    _x < 0 ? -1 : 1;})


#define lambda(return_type, body_and_args) \
  ({ \
    return_type __fn__ body_and_args \
    __fn__; \
  })

// swap two variables. Each should be same type
#define SWAP(x,y)\
   { unsigned char __swap_temp[sizeof(x) == sizeof(y) ? (signed)sizeof(x) : -1]; \
     memcpy(__swap_temp, &y, sizeof(x)); \
     memcpy(&y, &x, sizeof(x)); \
     memcpy(&x, __swap_temp, sizeof(x)); \
    } 
