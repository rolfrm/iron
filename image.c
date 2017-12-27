#include <png.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "types.h"
#include "image.h"
#include "log.h"
#include "mem.h"

void write_png_file(const char *filename, const int width, const int height, const u8 * data, int channels, int bit_depth) {

  FILE *fp = fopen(filename, "wb");
  if(!fp)
    ERROR("Could not open '%s'", filename);

  png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png) ERROR("Could not write PNG version string");

  png_infop info = png_create_info_struct(png);
  if (!info)ERROR("Could not create png struct");

  if (setjmp(png_jmpbuf(png)))
    ERROR("error writing png");

  png_init_io(png, fp);
  u8 * row_pointers[height];
  for(int i = 0; i < height; i++){
    row_pointers[i] = (u8 *) (data + i * width * channels);
  }
  int type = PNG_COLOR_TYPE_RGB;
  if(channels == 1)
    type = PNG_COLOR_TYPE_GRAY;
  if(channels == 2)
    type = PNG_COLOR_TYPE_GRAY_ALPHA;
  if(channels == 4)
    type = PNG_COLOR_TYPE_RGB_ALPHA;
  // Output is 8bit depth, RGBA format.
  png_set_IHDR(
    png,
    info,
    width, height,
    bit_depth,
    type,
    PNG_INTERLACE_NONE,
    PNG_COMPRESSION_TYPE_DEFAULT,
    PNG_FILTER_TYPE_DEFAULT
  );
  png_write_info(png, info);

  // To remove the alpha channel for PNG_COLOR_TYPE_RGB format,
  // Use png_set_filler().
  //png_set_filler(png, 0, PNG_FILLER_AFTER);

  png_write_image(png, row_pointers);
  png_write_end(png, NULL);
  fclose(fp);
}

size_t image_pixel_type_size(image_pixel_type type){
  switch(type){
  case PIXEL_RGBA:
    return 4;
  case PIXEL_RGB:
    return 3;
  case PIXEL_GRAY_ALPHA:
    return 2;
  case PIXEL_GRAY:
    return 1;
  }
  ERROR("Unknown pixel type");
  return 0;
}


image * image_new(int width, int height, image_pixel_type type){
  void * buffer = alloc0(width * height * image_pixel_type_size(type));
  image _pt = {.type = type, .width = width, .height = height, .buffer = buffer };
  return iron_clone(&_pt, sizeof(_pt));
}

void image_delete(image ** img){
  dealloc((void *) (*img)->buffer);
  dealloc(*img);
  *img = NULL;
}

void image_save(const image * img, const char * filename){
  write_png_file(filename, img->width, img->height, img->buffer, img->type, 8);
}

image * image_load(const char * filename){
  png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  ASSERT(png);
  FILE *fp = fopen(filename, "r");
  if(!fp)
    ERROR("Could not open '%s'", filename);

  png_infop info_ptr = png_create_info_struct(png);
  if (!png)
    ERROR("png_create_info_struct failed");
  
  if (setjmp(png_jmpbuf(png)))
    ERROR("Error during init_io");

  png_init_io(png, fp);
  png_set_sig_bytes(png, 0);
  png_read_info(png, info_ptr);

  int width = png_get_image_width(png, info_ptr);
  int height = png_get_image_height(png, info_ptr);
  int color_type = png_get_color_type(png, info_ptr);
  int bit_depth = png_get_bit_depth(png, info_ptr);
  ASSERT(bit_depth == 8);
  //int number_of_passes = png_set_interlace_handling(png);
  png_read_update_info(png, info_ptr);
  
  
  // read file 
  if (setjmp(png_jmpbuf(png)))
    ERROR("Error during read_image");
  u32 rowbytes = png_get_rowbytes(png,info_ptr);
  void * dataptr = alloc(rowbytes * height);
  png_bytep* row_pointers = (png_bytep*) alloc(sizeof(png_bytep) * height);
  
  for (int y=0; y<height; y++)
    row_pointers[y] = dataptr + rowbytes * y;

  png_read_image(png, row_pointers);

  fclose(fp);
  //image * img = alloc0(sizeof(image));

  image_pixel_type px_type;
  switch(color_type){
  case PNG_COLOR_TYPE_RGB:
    px_type = PIXEL_RGB;
    break;
  case PNG_COLOR_TYPE_GRAY:
    px_type = PIXEL_GRAY;
    break;
  case PNG_COLOR_TYPE_GRAY_ALPHA:
    px_type = PIXEL_GRAY_ALPHA;
    break;
  case PNG_COLOR_TYPE_RGB_ALPHA:
    px_type = PIXEL_RGBA;
    break;
  }
  
  image img = {.buffer = dataptr, .height = height, .width = width, .type = px_type};	       
  dealloc(row_pointers);

  return iron_clone(&img, sizeof(img));
}

void * image_get(image * img, int x, int y){
  int pxsize = image_pixel_type_size(img->type);
  if(x < 0 || y < 0 || x >= img->width || y >= img->height)
    return NULL;
  return img->buffer + (x + y * img->height) * pxsize;
}

void image_clear(image * img){
  int pxsize = image_pixel_type_size(img->type);
  memset(img->buffer, 0, pxsize * img->width * img->height);
}

void image_remove_alpha(image * img){
  if(img->type == PIXEL_GRAY || img->type == PIXEL_RGB)
    return;
  int pxsize = 2;
  int keepsize = 1;
  if(img->type == PIXEL_RGBA){
    pxsize = 4;
    keepsize = 3;
  }
  u64 size = img->width * img->height * image_pixel_type_size(img->type);
  u64 pxs = img->width * img->height;
  void * buffer = (void *) img->buffer;
  for(u64 i = 0; i < pxs; i+= 1){
    u64 newi = i * keepsize;
    u64 oldi = i * pxsize;
    memmove(buffer + newi, buffer + oldi, keepsize);
  }
  image newimg = {.type = keepsize == 1 ? PIXEL_GRAY : PIXEL_RGBA, .width = img->width,
		  .height = img->height, .buffer = img->buffer};
  memcpy(img, &newimg, sizeof(image));
}
