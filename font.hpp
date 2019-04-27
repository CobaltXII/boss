// Decompress a VGA text mode font.
unsigned char* decompress(unsigned char* font,
						  int glyph_x_res,
						  int glyph_y_res)
{
	// Allocate memory for the uncompressed font.
	unsigned char* out = (unsigned char*)malloc(
		glyph_x_res *
		glyph_y_res *
		256
	);

	if (!out) {
		barf("Could not allocate font memory.");
	}
	
	// Calculate the length (in octets) of the font based on the dimensions of
	// each glyph.
	int font_length = (
		glyph_x_res *
		glyph_y_res *
		32
	);
	
	// Decompress the font.
	for (int i = 0; i < font_length; i++) {
		for (int j = 0; j != 8; j++) {
			// Little-endian machines will load fonts horizontally flipped as
			// the font is compiled as big-endian.
			#ifdef __LITTLE_ENDIAN__
			out[8 * i + (7 - j)] = (font[i] >> j) & 1;
			#else
			// If the machine is not little-endian (or the __LITTLE_ENDIAN__)
			// flag is not set, the font will be read normally.
			out[8 * i + j] = (font[i] >> j) & 1;
			#endif
		}
	}
	
	return out;
}
