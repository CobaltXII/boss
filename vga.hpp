// VGA tones.
unsigned char vga_tone[] = {
	0x00, 0x55, 0xAA, 0xFF
};

// Conversion from VGA color combination to 32-bit ARGB8888 color.
Uint32 make_vga_argb8888(unsigned char v,
						 unsigned char g,
						 unsigned char a)
{
	return vga_tone[v] << 16 |
		   vga_tone[g] << 8 |
		   vga_tone[a];
}

// All 16 VGA color combinations converted into 32-bit ARGB colors.
Uint32 vga_argb8888[] = {
	make_vga_argb8888(0, 0, 0),
	make_vga_argb8888(0, 0, 2),
	make_vga_argb8888(0, 2, 0),
	make_vga_argb8888(0, 2, 2),
	make_vga_argb8888(2, 0, 0),
	make_vga_argb8888(2, 0, 2),
	make_vga_argb8888(2, 1, 0),
	make_vga_argb8888(2, 2, 2),
	make_vga_argb8888(1, 1, 1),
	make_vga_argb8888(1, 1, 3),
	make_vga_argb8888(1, 3, 1),
	make_vga_argb8888(1, 3, 3),
	make_vga_argb8888(3, 1, 1),
	make_vga_argb8888(3, 1, 3),
	make_vga_argb8888(3, 3, 1),
	make_vga_argb8888(3, 3, 3),
};

// The English names of all 16 VGA color combinations.
enum vga_color {
	vga_black,
	vga_dark_blue,
	vga_dark_green,
	vga_dark_cyan,
	vga_dark_red,
	vga_dark_pink,
	vga_dark_yellow,
	vga_gray,
	vga_dark_gray,
	vga_blue,
	vga_green,
	vga_cyan,
	vga_red,
	vga_pink,
	vga_yellow,
	vga_white
};