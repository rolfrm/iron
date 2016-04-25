
// Writes 'data' to a png file assuming it is a raw image. 'channels' should be the value 1, 3 or 4. 'bit_depth' should be 8 or 16.
void write_png_file(const char *filename, const int width, const int height, const u8 * data, int channels, int bit_depth);
