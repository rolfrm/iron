
#include <png.h>
#include <stdint.h>
#include "types.h"
#include "image.h"
#include "log.h"
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
