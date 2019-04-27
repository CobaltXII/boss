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

	// Insert a string into this row at the specified position.
	void insert_str(unsigned int index,
					std::string element,
					unsigned char foreground = vga_gray,
					unsigned char background = vga_black)
	{
		for (unsigned int i = 0; i < element.size(); i++) {
			// Generate a glyph.
			glyph glyph = {
				element[element.size() - i - 1],
				foreground,
				background
			};

			// Insert the glyph.
			insert(begin() + index, glyph);
		}
	}

	// Split this row at a certain index, and return the right side. Discard the
	// right side from this row.
	row split(unsigned int index) {
		row right;
		for (unsigned int i = index; i < size(); i++) {
			right.push_back((*this)[i]);
		}
		erase(begin() + index, end());
		return right;
	}
};