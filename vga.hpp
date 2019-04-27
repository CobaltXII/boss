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
