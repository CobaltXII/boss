// A row of glyphs/characters.
class row: public std::vector<glyph> {
public:
	// If this row contains an unclosed multiline comment (missing the
	// two-character end sequence), the 'open' flag will be set.
	bool open = false;
};