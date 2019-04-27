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

// 8x8 VGA compressed text mode font (ASCII).
unsigned char cmp_vga_8x8[] = {
#include "8x8.inc"
};

// 8x8 VGA uncompressed text mode font (ASCII).
unsigned char* vga_8x8 = decompress(cmp_vga_8x8, 8, 8);

// 8x10 VGA compressed text mode font (ASCII).
unsigned char cmp_vga_8x10[] = {
#include "8x10.inc"
};

// 8x10 VGA uncompressed text mode font (ASCII).
unsigned char* vga_8x10 = decompress(cmp_vga_8x10, 8, 10);

// 8x12 VGA compressed text mode font (ASCII).
unsigned char cmp_vga_8x12[] = {
#include "8x12.inc"
};

// 8x12 VGA uncompressed text mode font (ASCII).
unsigned char* vga_8x12 = decompress(cmp_vga_8x12, 8, 12);

// 8x14 VGA compressed text mode font (ASCII).
unsigned char cmp_vga_8x14[] = {
#include "8x14.inc"
};

// 8x14 VGA uncompressed text mode font (ASCII).
unsigned char* vga_8x14 = decompress(cmp_vga_8x14, 8, 14);

// 8x15 VGA compressed text mode font (ASCII).
unsigned char cmp_vga_8x15[] = {
#include "8x15.inc"
};

// 8x15 VGA uncompressed text mode font (ASCII).
unsigned char* vga_8x15 = decompress(cmp_vga_8x15, 8, 15);

// 8x16 VGA compressed text mode font (ASCII).
unsigned char cmp_vga_8x16[] = {
#include "8x16.inc"
};

// 8x16 VGA uncompressed text mode font (ASCII).
unsigned char* vga_8x16 = decompress(cmp_vga_8x16, 8, 16);

// 8x32 VGA compressed text mode font (ASCII).
unsigned char cmp_vga_8x32[] = {
#include "8x32.inc"
};

// 8x32 VGA uncompressed text mode font (ASCII).
unsigned char* vga_8x32 = decompress(cmp_vga_8x32, 8, 32);