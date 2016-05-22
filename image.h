
// Writes 'data' to a png file assuming it is a raw image. 'channels' should be the value 1, 3 or 4. 'bit_depth' should be 8 or 16.
void write_png_file(const char *filename, const int width, const int height, const u8 * data, int channels, int bit_depth);

typedef enum{
  PIXEL_RGBA = 4,
  PIXEL_RGB = 3
}image_pixel_type;

typedef struct
{
  const image_pixel_type type;
   void * const buffer;
  const int width, height;
}image;

image * image_new(int width, int height, image_pixel_type type);
void image_delete(image ** img);
void image_save(const image * img, const char * filename);
image * image_load(const char * filename);
void * image_get(image * img, int x, int y);
void image_clear(image * img);
