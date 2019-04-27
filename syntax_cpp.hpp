// The EOF character.
const int t_EOF = -1;

// A stream reader.
struct stream_reader {
	// Underlying buffer.
	row buffer;

	// Position in the underlying buffer.
	int pos = 0;

	// Default constructor.
	stream_reader(row text = row("")) {
		buffer = text;
	}

	// Peek the next character.
	int peek() {
		return buffer[pos].ascii;
	}

	// Peek the second next character.
	int peek_two() {
		if (pos + 1 >= int(buffer.size())) {
			return t_EOF;
		}
		return buffer[pos + 1].ascii;
	}

	// Rewind the reader to the beginning.
	void rewind() {
		pos = 0;
	}

	// Consume the next character in the string.
	int consume() {
		return buffer[pos++].ascii;
	}

	// Check if the end-of-file has been reached.
	bool eof() {
		return pos >= int(buffer.size());
	}
};

// All token types.
enum token_type {
	tk_eof,
	tk_keyword,
	tk_identifier,
	tk_constant,
	tk_string,
	tk_special,
	tk_operator,
	tk_comment,
	tk_directive,
	tk_macro,
	tk_whitespace
};

// Mappings from token_type to vga_color.
vga_color token_to_color[] = {
	vga_gray,
	vga_white,
	vga_gray,
	vga_blue,
	vga_blue,
	vga_dark_green,
	vga_dark_cyan,
	vga_red,
	vga_cyan,
	vga_dark_cyan,
	vga_gray
};

// All token types as strings.
std::string token_type_str[] = {
	"tk_eof",
	"tk_keyword",
	"tk_identifier",
	"tk_constant",
	"tk_string",
	"tk_special",
	"tk_operator",
	"tk_comment",
	"tk_directive",
	"tk_macro",
	"tk_whitespace"
};

// A token.
struct token {
	token_type type;
	std::string text;
};

// A tokenizer.
struct tokenizer {
	// Underlying stream reader.
	stream_reader reader;

	// If this tokenizer found an unclosed multiline comment (missing the
	// two-character end sequence), the 'open' flag will be set.
	bool open = false;

	// If the first token found by this tokenizer was a preprocessor directive,
	// this flag will be set.
	bool directive = false;

	// Default constructor.
	tokenizer(row text) {
		reader = stream_reader(text);
	}

	// Checks if a string is a keyword.
	bool tis_keyword(std::string s) {
		return s == "alignas" ||
			   s == "alignof" ||
			   s == "and" ||
			   s == "and_eq" ||
			   s == "asm" ||
			   s == "atomic_cancel" ||
			   s == "atomic_commit" ||
			   s == "atomic_noexcept" ||
			   s == "auto" ||
			   s == "bitand" ||
			   s == "bitor" ||
			   s == "bool" ||
			   s == "break" ||
			   s == "case" ||
			   s == "catch" ||
			   s == "char" ||
			   s == "char8_t" ||
			   s == "char16_t" ||
			   s == "char32_t" ||
			   s == "class" ||
			   s == "compl" ||
			   s == "concept" ||
			   s == "const" ||
			   s == "consteval" ||
			   s == "constexpr" ||
			   s == "const_cast" ||
			   s == "continue" ||
			   s == "co_await" ||
			   s == "co_return" ||
			   s == "co_yield" ||
			   s == "decltype" ||
			   s == "default" ||
			   s == "delete" ||
			   s == "do" ||
			   s == "double" ||
			   s == "dynamic_cast" ||
			   s == "else" ||
			   s == "enum" ||
			   s == "explicit" ||
			   s == "export" ||
			   s == "extern" ||
			   s == "false" ||
			   s == "float" ||
			   s == "for" ||
			   s == "friend" ||
			   s == "goto" ||
			   s == "if" ||
			   s == "inline" ||
			   s == "int" ||
			   s == "long" ||
			   s == "mutable" ||
			   s == "namespace" ||
			   s == "new" ||
			   s == "noexcept" ||
			   s == "not" ||
			   s == "not_eq" ||
			   s == "nullptr" ||
			   s == "operator" ||
			   s == "or" ||
			   s == "or_eq" ||
			   s == "private" ||
			   s == "protected" ||
			   s == "public" ||
			   s == "reflexpr" ||
			   s == "register" ||
			   s == "reinterpret_cast" ||
			   s == "requires" ||
			   s == "return" ||
			   s == "short" ||
			   s == "signed" ||
			   s == "sizeof" ||
			   s == "static" ||
			   s == "static_assert" ||
			   s == "static_cast" ||
			   s == "struct" ||
			   s == "switch" ||
			   s == "synchronized" ||
			   s == "template" ||
			   s == "this" ||
			   s == "thread_local" ||
			   s == "throw" ||
			   s == "true" ||
			   s == "try" ||
			   s == "typedef" ||
			   s == "typeid" ||
			   s == "typename" ||
			   s == "union" ||
			   s == "unsigned" ||
			   s == "using(1)" ||
			   s == "virtual" ||
			   s == "void" ||
			   s == "volatile" ||
			   s == "wchar_t" ||
			   s == "while" ||
			   s == "xor" ||
			   s == "xor_eq";
	}

	// Checks if a character is a digit or decimal point.
	static bool tis_digit(int c) {
		return (c >= '0' && c <= '9') || 
			   c == '.';
	}

	// Checks if a character is a numerical character.
	static bool tis_number(int c) {
		return (c >= '0' && c <= '9') ||
			   (c >= 'a' && c <= 'f') ||
			   (c >= 'A' && c <= 'F') ||
			   (c == 'u' || c == 'U') ||
			   (c == 'l' || c == 'L') ||
			   (c == 'x' || c == 'X') ||
			   (c == '+' || c == '-') ||
			   c == '.';
	}

	// Checks if a character can be the first character of an identifier
	// token.
	static bool tis_id_start(int c) {
		return (c >= 'a' && c <= 'z') ||
			   (c >= 'A' && c <= 'Z') ||
			   c == '_';
	}

	// Checks if a character can be contained in an identifier token (but not
	// the first character).
	static bool tis_id(int c) {
		return (c >= 'a' && c <= 'z') ||
			   (c >= 'A' && c <= 'Z') ||
			   (c >= '0' && c <= '9') ||
			   c == '_';
	}

	// Checks if a character is an operator character.
	static bool tis_op(int c) {
		return c == '+' || c == '-' || c == '*' || c == '/' ||
			   c == '%' || c == '=' || c == '!' || c == '>' ||
			   c == '<' || c == '&' || c == '|' || c == '^' ||
			   c == '~' || c == '?' || c == ':' || c == '.';
	}

	// Checks if a character is a special character.
	static bool tis_special(int c) {
		return c == '[' || c == ']' ||
			   c == '{' || c == '}' ||
			   c == '(' || c == ')' ||
			   c == ';' || c == ',';
	}

	// Checks if a character is whitespace.
	static bool tis_space(int c) {
		return c == ' ' || c == '\t' || c == '\n';
	}

	// Consume while the predicate function returns true.
	std::string read_while(bool (*predicate)(int)) {
		std::string str;
		while (!reader.eof() && predicate(reader.peek())) {
			str += reader.consume();
		}
		return str;
	}

	// Read a (numerical) constant.
	token read_numerical() {
		return {
			tk_constant,
			read_while(tis_number)
		};
	}

	// Read an identifier (or keyword).
	token read_identifier() {
		std::string identifier = read_while(tis_id);
		if (tis_keyword(identifier)) {
			return {
				tk_keyword,
				identifier
			};
		} else {
			return {
				tk_identifier,
				identifier
			};
		}
	}

	// Read a special character.
	token read_special() {
		std::string str;
		str += reader.consume();
		return {
			tk_special,
			str
		};
	}

	// Read a string surrounded by a quotation character while handling escape
	// codes.
	token read_string(int quotation) {
		bool escaped = false;
		std::string str = "";
		str += quotation;
		reader.consume();
		while (!reader.eof()) {
			int c = reader.consume();
			str += c;
			if (escaped) {
				escaped = false;
			}
			else if (c == '\\') {
				// Handling the escape character (\).
				escaped = true;
			}
			else if (c == quotation) {
				break;
			}
		}
		return {
			tk_string,
			str
		};
	}

	// Read an inline comment.
	token read_inline_comment() {
		std::string str = read_while(
			[](int c) -> bool {
				return c != '\n';
			}
		);
		str += reader.consume();
		return {
			tk_comment,
			str
		};
	}

	// Read a multiline comment.
	token read_multiline_comment() {
		bool closed = false;
		std::string str;
		while (!reader.eof()) {
			int c = reader.peek();
			if (c == '*') {
				int c = reader.peek_two();
				if (c == '/') {
					closed = true;
					str += reader.consume();
					str += reader.consume();
					break;
				}
			}
			str += reader.consume();
		}
		if (!closed) {
			open = true;
		}
		return {
			tk_comment,
			str
		};
	}

	// Get the next token.
	token next(bool open) {
		// Check for the EOF.
		if (reader.eof()) {
			return {
				tk_eof,
				""
			};
		}

		// Continue parsing open multiline comments.
		if (reader.pos == 0 && open) {
			return read_multiline_comment();
		}

		// Peek the next character.
		int c = reader.peek();

		// Check for preprocessor directives.
		if (reader.pos == 0) {
			std::string str = read_while(tis_space);
			if (reader.peek() != '#') {
				reader.rewind();
			} else {
				directive = true;
				str += reader.consume();
				str += read_while(tis_space);
				str += read_while(tis_id);
				return {
					tk_directive,
					str
				};
			}
		}

		// Check for text following a preprocessor directive.
		if (directive) {
			return {
				tk_macro,
				read_while(
					[](int c) -> bool {
						return c != '\n';
					}
				)
			};
		}

		// Check for whitespace.
		if (tis_space(c)) {
			return {
				tk_whitespace,
				read_while(tis_space)
			};
		}

		// Check for comments.
		if (c == '/') {
			int c = reader.peek_two();
			if (c == '/') {
				return read_inline_comment();
			} else if (c == '*') {
				return read_multiline_comment();
			}
		}

		// Check for string literals.
		if (c == '"' || c == '\'') {
			return read_string(c);
		}

		// Check for identifiers (or keywords).
		if (tis_id_start(c)) {
			return read_identifier();
		}

		// Check for special characters.
		if (tis_special(c)) {
			return read_special();
		}

		// Check for operator characters.
		if (tis_op(c)) {
			return {
				tk_operator,
				read_while(tis_op)
			};
		}

		// Check for numerical constants.
		if (tis_digit(c)) {
			return read_numerical();
		}

		// Something weird is going on.
		return {
			tk_eof,
			""
		};
	}
};