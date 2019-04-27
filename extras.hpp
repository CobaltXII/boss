// Credits (can be seen in a hex editor).
static const char* credits = "BOSS text editor by Adam Sidat (CobaltXII), April of 2019";

// Barf a message and exit.
void barf(std::string error) {
	std::cout << error << std::endl;
	exit(-1);
}

#ifdef MATRIX_EFFECT
// Falling character.
struct hackerman {
	char ascii;
	int x;
	float y;
	float vy;
};
#endif

// Find the endian-ness of the system.
bool check_if_is_big_endian()
{
    union {
        Uint32 i;
        char c[4];
    } endian = {0x01020304};

    return endian.c[0] == 1;
}
bool is_big_endian = check_if_is_big_endian();