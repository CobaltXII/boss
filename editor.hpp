// A BOSS editor.
struct editor {
	// VGA text mode dimensions.
	int vga_text_mode_x_res;
	int vga_text_mode_y_res;

	// The currently selected VGA text mode font (ASCII).
	unsigned char* vga_001 = vga_8x8;
	// The currently selected VGA text mode font's glyph dimensions.
	int vga_001_x_res = 8;
	int vga_001_y_res = 8;

	// VGA text mode buffer.
	glyph* text = NULL;

	// The current syntax highlighting mode.
	highlight_mode highlight = hm_null;

	// All of the rows currently present in the editor.
	std::vector<row> rows;

	// The scrolling offsets.
	int scroll_x = 0;
	int scroll_y = 0;

	// The cursor position.
	int cursor_x = 0;
	int cursor_y = 0;

	// The "motion tick". This is the timestamp of the last cursor movement (in
	// milliseconds since the start of the editor). The "motion tick" value is
	// used to prevent blinking while the cursor is in motion.
	Uint32 motion_tick = 0;

	// The currently opened file's filename.
	std::string filename;

	// If CTRL-B is hit, the video buffer will be saved after all events are
	// polled. The key handler will set this flag to true.
	bool save_video = false;

	#ifdef MATRIX_EFFECT
	// A vector of falling characters.
	std::vector<hackerman> hackermen;
	#endif

	// Text buffer glyph assignment function. Bounds checking is implemented in
	// this function.
	void word(int x,
			  int y,
			  glyph glyph)
	{
		if (x >= 0 && x < vga_text_mode_x_res)
		if (y >= 0 && y < vga_text_mode_y_res) {
			text[y * vga_text_mode_x_res + x] = glyph;
		}
	}

	// Default constructor.
	editor(int text_mode_x_res,
		   int text_mode_y_res)
	{
		// Set the dimensions of the editor.
		vga_text_mode_x_res = text_mode_x_res;
		vga_text_mode_y_res = text_mode_y_res;

		// Allocate the text buffer.
		text = (glyph*)malloc(
			vga_text_mode_x_res *
			vga_text_mode_y_res *
			sizeof(glyph)
		);

		if (!text) {
			barf("Could not allocate text memory.");
		}
	}

	// Rasterize the text buffer to the video buffer of a video_interface*.
	void raster(video_interface* vga);
	// Update a row.
	void update(int row_index);
	// Keypress handler.
	void key(SDL_Event e);
	// Render the current state to the text buffer.
	void render();
};