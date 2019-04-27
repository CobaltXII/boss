// Credits (can be seen in a hex editor).
static const char* credits = "BOSS text editor by Adam Sidat (CobaltXII), April of 2019";

// Barf a message and exit.
void barf(std::string error) {
	std::cout << error << std::endl;
	exit(-1);
}