// A row of glyphs/characters.
class row: public std::vector<glyph> {
public:
	// If this row contains an unclosed multiline comment (missing the
	// two-character end sequence), the 'open' flag will be set.
	bool open = false;

	// Conversion from std::string to row.
	row(std::string text = "",
		unsigned char fg = vga_gray,
		unsigned char bg = vga_black)
	{
		for (unsigned int i = 0; i < text.length(); i++) {
			push_back({text[i], fg, bg});
		}
	}
};