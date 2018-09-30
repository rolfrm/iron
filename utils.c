#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "types.h"
#include "utils.h"

void print_raw(void * data, size_t size){
  u8 * bytes = data;
  for(size_t i = 0; i < size;i++,bytes++)
    printf("%X", *bytes);
}
