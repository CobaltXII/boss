// A VGA glyph represents a foreground color, background color, and a ASCII
// character code.
struct glyph {
	char ascii;
	unsigned char fg;
	unsigned char bg;
};