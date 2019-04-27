// All syntax higlighting modes.
enum highlight_mode {
	hm_null,
	hm_c,
	hm_cpp
};

// All syntax highlighting mode abbreviations.
std::string highlight_mode_string[] = {
	"?",
	"c",
	"cpp"
};

// Import syntax highlighting rules for C.
namespace HI_c {
#include "syntax_c.hpp"
}

// All file extensions that use the hm_c rules.
std::string ext_hm_c[] = {
	".c"
};
