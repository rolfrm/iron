#include "full.h"
#include "utf8.h"
size_t codepoint_to_utf8(u32 codepoint, char * out, size_t maxlen){
  char data[4];
  u32 c = codepoint;
  char * b = data;
  if (c<0x80) *b++=c;
  else if (c<0x800) *b++=192+c/64, *b++=128+c%64;  
  else if (c-0xd800u <0x800) return 0;
  else if (c<0x10000) *b++=224+c/4096, *b++=128+c/64%64, *b++=128+c%64;
  else if (c<0x110000) *b++=240+c/262144, *b++=128+c/4096%64, *b++=128+c/64%64, *b++=128+c%64;
  else return 0;
  
  size_t l = b - data;
  if(maxlen < l ) return 0;
  if(out != NULL)
    memcpy(out, data, l);
  return l;		    
}

u32 utf8_to_codepoint(const char * _txt, size_t * out_len){
  u8 * txt = (u8 *) _txt;
  *out_len = 0;
  if(*txt < 0x80){
    *out_len = 1;
    return *txt;
  }
  int len = 0;
  u32 codept = 0;
  u32 offset = 0;
  if(*txt > 0b11110000){
    len = 4;
    codept = *txt & 0b00000111;
    offset = 0x10000;
  }else if(*txt > 0b11100000){
    len = 3;
    codept = *txt & 0b00001111;
    offset = 0xFFFF;
  }else if(*txt > 0b11000000){
    len = 2;
    offset = 0x0080;
    codept = (*txt & 0b00011111);
  }
  *out_len = (size_t)len;
  for(int i = 1; i < len; i++)
    codept = (codept << 6) | (txt[i] & 0b00111111);
  
  UNUSED(offset);
  return codept;
}
