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

	// Conversion from row to std::string.
	std::string to_string() {
		std::string str;
		for (int i = 0; i < size(); i++) {
			str += (*this)[i].ascii;
		}
		return str;
	}

	// Append a row to the end of this row.
	void append(row row) {
		for (unsigned int i = 0; i < row.size(); i++) {
			push_back(row[i]);
		}
	}
};