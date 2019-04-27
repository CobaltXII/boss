/*
 * #############################################################################
 * #############################################################################
 *
 * BOSS text editor by Adam Sidat (CobaltXII), April of 2019
 *
 * #############################################################################
 * #############################################################################
 *
 */

#include <ctime>
#include <memory>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <iostream>

#include <SDL.h>

#include "extras.hpp"
#include "mario.hpp"
#include "glyph.hpp"
#include "video.hpp"
#include "font.hpp"
#include "vga.hpp"
#include "row.hpp"

#include "syntax.hpp"
#include "editor.hpp"

// Rasterize the text buffer to the video buffer of a video_interface*.
void editor::raster(video_interface* vga) {
	// Calculate the glyph size (in bits).
	int glyph_size = (
		vga_001_x_res *
		vga_001_y_res
	);
	
	for (int i = 0; i < vga_text_mode_x_res; i++)
	for (int j = 0; j < vga_text_mode_y_res; j++) {
		// Unpack the current glyph.
		glyph glyph = text[
			j * vga_text_mode_x_res + i
		];
		
		// Fetch the VGA colors.
		Uint32 u32_fg = vga_argb8888[glyph.fg];
		Uint32 u32_bg = vga_argb8888[glyph.bg];
		
		// Fetch the ASCII code.
		unsigned char ascii = glyph.ascii;

		// Fetch the glyph font pointer.
		unsigned char* font = vga_001 + glyph_size * ascii;

		// Rasterize the glyph.
		for (int x = 0; x < vga_001_x_res; x++)
		for (int y = 0; y < vga_001_y_res; y++) {
			if (font[y * vga_001_x_res + x]) {
				vga->set(
					i * vga_001_x_res + x,
					j * vga_001_y_res + y,
					u32_fg
				);
			} else {
				vga->set(
					i * vga_001_x_res + x,
					j * vga_001_y_res + y,
					u32_bg
				);
			}
		}
	}

	// Draw Mario (8x16 mode only).
	int mario_x = (SDL_GetTicks() / 15) % (vga->x_res + 32) - 16;
	if (vga_001_y_res == 16) {
		for (int x = 0; x < 16; x++) {
			for (int y = 0; y < 16; y++) {
				if (SDL_GetTicks() % 200 >= 100) {
					if (mario[y][x] == '#') {
						vga->set_safe(
							x + mario_x, y,
							vga_argb8888[vga_dark_gray]
						);
					}
				} else {
					if (mario[y][x + 16] == '#') {
						vga->set_safe(
							x + mario_x, y,
							vga_argb8888[vga_dark_gray]
						);
					}
				}
			}
		}
	}
}

// Update a row.
void editor::update(int row_index) {
	// Reject non-existant rows.
	if (row_index < 0) {
		return;
	} else if (row_index >= rows.size()) {
		return;
	}

	// Do syntax highlighting.
	if (highlight == hm_c) {
		// Find out if the upper row is open.
		bool upper_open = false;
		if (row_index - 1 >= 0) {
			upper_open = rows[row_index - 1].open;
		}

		unsigned int i = 0;
		HI_c::tokenizer tokenizer(rows[row_index]);
		for (;;) {
			// Fetch the next token.
			HI_c::token token = tokenizer.next(upper_open);
			// Break if the end-of-file was encountered.
			if (token.type == HI_c::tk_eof) {
				break;
			}
			// Color the segment of the row represented by the last fetched
			// token.
			vga_color color = HI_c::token_to_color[token.type];
			for (unsigned int j = 0; j < token.text.length(); j++) {
				if (i < rows[row_index].size()) {
					rows[row_index][i++].fg = color;
				}
			}
		}

		// Remember if the row was closed or open before doing syntax
		// highlighting.
		bool open = rows[row_index].open;

		// Mark the row as open if the tokenizer was marked as open.
		rows[row_index].open = tokenizer.open;

		// If the row's length is zero, set the row's state to to the state of
		// the upper row.
		if (rows[row_index].size() == 0) {
			rows[row_index].open = upper_open;
		}

		// If the row was closed or opened, update the following row.
		if (rows[row_index].open != open) {
			update(row_index + 1);
		}
	} else if (highlight == hm_cpp) {
		// Find out if the upper row is open.
		bool upper_open = false;
		if (row_index - 1 >= 0) {
			upper_open = rows[row_index - 1].open;
		}

		unsigned int i = 0;
		HI_cpp::tokenizer tokenizer(rows[row_index]);
		for (;;) {
			// Fetch the next token.
			HI_cpp::token token = tokenizer.next(upper_open);
			// Break if the end-of-file was encountered.
			if (token.type == HI_cpp::tk_eof) {
				break;
			}
			// Color the segment of the row represented by the last fetched
			// token.
			vga_color color = HI_cpp::token_to_color[token.type];
			for (unsigned int j = 0; j < token.text.length(); j++) {
				if (i < rows[row_index].size()) {
					rows[row_index][i++].fg = color;
				}
			}
		}

		// Remember if the row was closed or open before doing syntax
		// highlighting.
		bool open = rows[row_index].open;

		// Mark the row as open if the tokenizer was marked as open.
		rows[row_index].open = tokenizer.open;

		// If the row's length is zero, set the row's state to to the state of
		// the upper row.
		if (rows[row_index].size() == 0) {
			rows[row_index].open = upper_open;
		}

		// If the row was closed or opened, update the following row.
		if (rows[row_index].open != open) {
			update(row_index + 1);
		}
	}
}

// Keypress handler.
void editor::key(SDL_Event e) {
	if (e.type == SDL_KEYDOWN) {
		SDL_Keycode key = e.key.keysym.sym;

		// Handle keys with a left-control modifier.
		if (e.key.keysym.mod == KMOD_LCTRL) {
			bool realloc_text = true;
			if (key == SDLK_1) {
				// Switch text mode font to 8x8.
				if (vga_001_y_res != 8) {
					// Resize the text buffer.
					float resize = 8.0f / float(vga_001_y_res);
					vga_text_mode_y_res /= resize;
					// Set the font.
					vga_001 = vga_8x8;
					vga_001_x_res = 8;
					vga_001_y_res = 8;
				}
			} else if (key == SDLK_2) {
				// Switch text mode font to 8x16.
				if (vga_001_y_res != 16) {
					// Resize the text buffer.
					float resize = 16.0f / float(vga_001_y_res);
					vga_text_mode_y_res /= resize;
					// Set the font.
					vga_001 = vga_8x16;
					vga_001_x_res = 8;
					vga_001_y_res = 16;
				}
			} else if (key == SDLK_3) {
				// Switch text mode font to 8x32.
				if (vga_001_y_res != 32) {
					// Resize the text buffer.
					float resize = 32.0f / float(vga_001_y_res);
					vga_text_mode_y_res /= resize;
					// Set the font.
					vga_001 = vga_8x32;
					vga_001_x_res = 8;
					vga_001_y_res = 32;
				}
			} else {
				realloc_text = false;
			}

			if (key == SDLK_v) {
				// Paste text.
				std::string text = SDL_GetClipboardText();
				std::stringstream text_stream(text);
				std::vector<std::string> lines;
				std::string line;
				while (std::getline(text_stream, line)) {
					lines.push_back(line);
				}
				if (lines.size() == 1) {
					rows[cursor_y].insert_str(cursor_x, lines[0]);
					cursor_x += text.size();
				} else if (lines.size() > 1) {
					rows[cursor_y].insert_str(cursor_x, lines[0]);
					for (int i = 1; i < lines.size(); i++) {
						rows.insert(rows.begin() + ++cursor_y, row(lines[i]));
						update(cursor_y);
					}
					cursor_x = rows[cursor_y].size();
				}
			} else if (key == SDLK_s) {
				// Save the file.
				std::ofstream file(filename);
				if (file.is_open()) {
					for (int i = 0; i < rows.size(); i++) {
						file << rows[i].to_string();
						if (i < rows.size() - 1) {
							file << '\n';
						}
					}
					file.close();
				}
			} else if (key == SDLK_b) {
				// Save the video buffer.
				save_video = true;
			}

			if (realloc_text) {
				// Reallocate the text buffer.
				free(text);
				text = (glyph*)malloc(
					vga_text_mode_x_res *
					vga_text_mode_y_res *
					sizeof(glyph)
				);
			}

			// Scroll up if the cursor is above the viewport.
			if (cursor_y < scroll_y) {
				while (cursor_y < scroll_y) {
					scroll_y--;
				}
			}

			// Scroll down if the cursor is below the viewport.
			if (cursor_y + 2 > vga_text_mode_y_res + scroll_y) {
				while (cursor_y + 2 > vga_text_mode_y_res + scroll_y) {
					scroll_y++;
				}
			}
		}

		// Handle SDLK_BACKSPACE.
		if (key == SDLK_BACKSPACE) {
			if (rows[cursor_y].size() < 1 && cursor_y > 0) {
				// Line is empty, so remove the line, and move the cursor to the
				// end of the upper line.
				rows.erase(rows.begin() + cursor_y--);
				cursor_x = rows[cursor_y].size();
			} else {
				// Line is not empty.
				if (cursor_x == 0) {
					// Cursor is on the first character, so remove the line, add
					// it to the end of the upper line and move the cursor to
					// the previous end of the upper line.
					if (cursor_y <= 0) {
						return;
					}
					cursor_x = rows[cursor_y - 1].size();
					rows[cursor_y - 1].append(rows[cursor_y]);
					rows.erase(rows.begin() + cursor_y);
					cursor_y--;
				} else {
					// Cursor is not on the first character, so remove the
					// character before the cursor, and move the cursor to the
					// left.
					rows[cursor_y].erase(rows[cursor_y].begin() + cursor_x - 1);
					cursor_x--;
				}
			}
			// Scroll up if the cursor is above the viewport.
			if (cursor_y < scroll_y) {
				scroll_y--;
			}
		}

		// Handle SDLK_RETURN.
		else if (key == SDLK_RETURN) {
			if (cursor_x == 0) {
				// Cursor is at the start of the row. Create a new row above the
				// cursor and move the cursor down.
				rows.insert(rows.begin() + cursor_y++, row());
			} else if (cursor_x == rows[cursor_y].size()) {
				// Cursor is at the end of the row. Create a new row below the
				// cursor and move the cursor down.
				rows.insert(rows.begin() + ++cursor_y, row());
			} else {
				// Cursor is somewhere inside the row. Split the row and move
				// the right half to another line (below the current line). Then
				// remove the right half from the current line. Finally, move
				// the cursor to the start of the first line.
				row row_right = rows[cursor_y].split(cursor_x);
				rows.insert(rows.begin() + ++cursor_y, row_right);
				cursor_x = 0;
			}
			// Scroll down if the cursor is below the viewport.
			if (cursor_y + 2 > vga_text_mode_y_res + scroll_y) {
				scroll_y++;
			}
		}

		// Handle SDLK_TAB.
		else if (key == SDLK_TAB) {
			rows[cursor_y].insert_str(cursor_x++, "\t");
		}

		// Handle SDLK_LEFT.
		else if (key == SDLK_LEFT) {
			cursor_x--;
			if (cursor_x == -1) {
				if (cursor_y != 0) {
					cursor_x = rows[--cursor_y].size();
				} else {
					cursor_x = 0;
				}
			}
			// Scroll up if the cursor is above the viewport.
			if (cursor_y < scroll_y) {
				scroll_y--;
			}
		}
		// Handle SDLK_RIGHT.
		else if (key == SDLK_RIGHT) {
			cursor_x++;
			if (cursor_x > rows[cursor_y].size()) {
				if (cursor_y + 1 < rows.size()) {
					cursor_x = 0;
					cursor_y++;
				}
			}
			// Scroll down if the cursor is below the viewport.
			if (cursor_y + 2 > vga_text_mode_y_res + scroll_y) {
				scroll_y++;
			}
		}

		// Handle SDLK_UP.
		else if (key == SDLK_UP) {
			cursor_y--;
			// Scroll up if the cursor is above the viewport.
			if (cursor_y < scroll_y) {
				scroll_y--;
			}
		}
		// Handle SDLK_DOWN.
		else if (key == SDLK_DOWN) {
			cursor_y++;
			// Scroll down if the cursor is below the viewport.
			if (cursor_y + 2 > vga_text_mode_y_res + scroll_y) {
				scroll_y++;
			}
		}

		else {
			return;
		}
	}
	else if (e.type == SDL_TEXTINPUT) {
		// Insert the inputted text into the current row at the current position
		// of the cursor, and move the cursor to the right.
		rows[cursor_y].insert_str(cursor_x, e.text.text);
		cursor_x += strlen(e.text.text);
	}
	else {
		return;
	}

	// Clamp cursor_x and cursor_y.
	if (cursor_y < 0) {
		cursor_y = 0;
	} else if (cursor_y >= rows.size()) {
		cursor_y = rows.size() - 1;
	}

	if (cursor_x > rows[cursor_y].size()) {
		cursor_x = rows[cursor_y].size();
	}
	if (cursor_x < 0) {
		cursor_x = 0;
	}

	// Clamp scroll_x and scroll_y.
	if (scroll_x < 0) {
		scroll_x = 0;
	}
	if (scroll_y < 0) {
		scroll_y = 0;
	}

	// Update the current, upper, and lower rows.
	update(cursor_y);
	update(cursor_y - 1);
	update(cursor_y + 1);

	// Set the motion tick value to the current tick to prevent blinking for the
	// next few frames.
	motion_tick = SDL_GetTicks();
}

// Render the current state to the text buffer.
void editor::render() {
	// Calculate the length of the text buffer.
	int text_length = (
		vga_text_mode_x_res *
		vga_text_mode_y_res *
		sizeof(glyph)
	);

	// Clear the text buffer.
	memset(text, 0, text_length);

	// Store the printer head's Y position.
	int y = scroll_y;
	// Print all of the rows to the text buffer.
	for (unsigned int j = scroll_y; j < rows.size(); j++) {
		// Store the printer head's X position.
		int x = 8;

		// Fetch the current row.
		row row = rows[j];
		// Print the current row to the text buffer.
		for (unsigned int i = 0; i < row.size(); i++) {
			// Fetch the current glyph.
			glyph glyph = row[i];
			// Handle tabs.
			if (glyph.ascii == '\t') {
				x = (x / 4) * 4 + 4;
				continue;
			}
			// Handle regular characters.
			word(
				x - scroll_x,
				y - scroll_y + 1,
				glyph
			);
			// Increment the printer head's X position.
			x++;
		}

		// Print the filler for the right-aligned line number.
		for (int i = 0; i < 8; i++) {
			word(i, y - scroll_y + 1, {' ', vga_gray, vga_black});
		}

		// Print the right-aligned line number.
		std::string line_number = std::to_string(j + 1);
		for (unsigned int i = 0; i < line_number.size(); i++) {
			glyph glyph = {
				line_number[i],
				vga_gray,
				vga_black
			};
			word(7 - line_number.size() + i, y - scroll_y + 1, glyph);
		}

		// Increment the printer head's Y position.
		y++;

		// Break if the printer head is outside the viewport.
		if (y - scroll_y + 1 >= vga_text_mode_y_res) {
			break;
		}
	}

	// Draw the cursor if the blink timer allows it.
	if ((SDL_GetTicks() - motion_tick) % 1000 < 500) {
		// Find the real cursor X position. The cursor_x variable cannot be
		// relied on because of wide characters (like tabs).
		int real_cursor_x = 0;
		for (int i = 0; i < rows[cursor_y].size() && i < cursor_x; i++) {
			if (rows[cursor_y][i].ascii == '\t') {
				real_cursor_x = (real_cursor_x / 4) * 4 + 4;
			} else {
				real_cursor_x++;
			}
		}

		// Draw the cursor.
		word(
			real_cursor_x - scroll_x + 8,
			cursor_y - scroll_y + 1,
			{
				-37,
				vga_gray,
				vga_black
			}
		);
	}

	// Print the status bar background.
	for (unsigned int i = 0; i < vga_text_mode_x_res; i++) {
		word(i, 0, {' ', vga_black, vga_gray});
	}

	// Print the filename.
	for (unsigned int i = 0; i < filename.size(); i++) {
		word(i + 8, 0, {filename[i], vga_black, vga_gray});
	}

	// Print the syntax type abbreviation.
	std::string syntax = "(" + highlight_mode_string[highlight] + ")";
	for (unsigned int i = 0; i < syntax.size(); i++) {
		word(i + 9 + filename.size(), 0, {syntax[i], vga_black, vga_gray});
	}

	// Print the line and column numbers.
	std::stringstream status_stream;
	status_stream << "Ln " << cursor_y + 1 << "/" << rows.size() << ", ";
	status_stream << "Col " << cursor_x + 1;
	std::string status = status_stream.str();
	for (unsigned int i = 0; i < status.size(); i++) {
		glyph glyph = {
			status[i],
			vga_black,
			vga_gray
		};
		word(vga_text_mode_x_res - 8 - status.size() + i, 0, glyph);
	}

	#ifdef MATRIX_EFFECT
	// Print the falling characters.
	for (int i = 0; i < hackermen.size(); i++) {
		hackerman hacker = hackermen[i];
		int x = hacker.x;
		int y = int(hacker.y) % vga_text_mode_y_res;
		// Print the falling character.
		if ((text[y * vga_text_mode_x_res + x].ascii == ' ' ||
			text[y * vga_text_mode_x_res + x].ascii == 0) &&
			text[y * vga_text_mode_x_res + x].bg == vga_black)
		{
			word(x, y, {hacker.ascii, vga_dark_gray, vga_black});
		}
	}
	#endif
}

// Entry point.
int main(int argc, char** argv) {
	// Print the credits (for reference purposes).
	std::cout << credits << std::endl;

	// Parse command line arguments.
	if (argc != 2) {
		std::cout << "Usage: " << argv[0] << " <file>" << std::endl;
		exit(-1);
	}

	// Create an editor.
	editor boss = editor(90, 100);

	// Parse the filename.
	boss.filename = std::string(argv[1]);
	// Find the syntax highlighting mode by comparing the end of the
	// filename to many common file extensions.
	for (int i = 0; i < sizeof(ext_hm_c) / sizeof(ext_hm_c[0]); i++) {
		std::string suffix = ext_hm_c[i];
		if (suffix.size() > boss.filename.size()) {
			continue;
		}
		bool is_match = std::equal(
			suffix.rbegin(),
			suffix.rend(),
			boss.filename.rbegin()
		);
		if (is_match) {
			boss.highlight = hm_c;
		}
	}
	for (int i = 0; i < sizeof(ext_hm_cpp) / sizeof(ext_hm_cpp[0]); i++) {
		std::string suffix = ext_hm_cpp[i];
		if (suffix.size() > boss.filename.size()) {
			continue;
		}
		bool is_match = std::equal(
			suffix.rbegin(),
			suffix.rend(),
			boss.filename.rbegin()
		);
		if (is_match) {
			boss.highlight = hm_cpp;
		}
	}

	load_file:
	// Load a file (or start an empty file).
	std::ifstream file(argv[1]);
	// Verify that the file is open.
	if (file.is_open()) {
		// Load the file line by line.
		std::string line;
		while (std::getline(file, line)) {
			boss.rows.push_back(row(line));
			// Calculate the actual length of the line.
			unsigned int length = 0;
			for (int i = 0; i < line.length(); i++) {
				if (line[i] == '\t') {
					length = (length / 4) * 4 + 4;
				} else {
					length++;
				}
			}
			// Resize the window to fit the row.
			if (length + 10 > boss.vga_text_mode_x_res) {
				boss.vga_text_mode_x_res = length + 10;
			}
		}
	} else {
		// Create a new file.
		std::ofstream file(argv[1]);
		file << std::endl;
		file.close();
		// Load the newly created file.
		goto load_file;
	}
	
	// Create a video_interface.
	video_interface adapter = video_interface(
		"BOSS",
		boss.vga_text_mode_x_res * boss.vga_001_x_res,
		boss.vga_text_mode_y_res * boss.vga_001_y_res
	);

	// Update all rows.
	for (unsigned int i = 0; i < boss.rows.size(); i++) {
		boss.update(i);
	}

	// Reset the text buffer.
	boss.render();

	#ifdef MATRIX_EFFECT
	// Generate the falling characters.
	for (int i = 0; i < 128; i++) {
		boss.hackermen.push_back({
			char((unsigned char)(rand() % 256)),
			rand() % boss.vga_text_mode_x_res,
			float(rand() % boss.vga_text_mode_y_res),
			float(rand()) / float(RAND_MAX) * 0.1f + 0.1f
		});
	}
	#endif
	
	// Start the VGA text mode emulator.
	boss.raster(&adapter);
	adapter.push();
	
	// Run the VGA text mode emulator.
	for (;;) {
		// Poll input.
		SDL_Event e;
		while (SDL_PollEvent(&e) == SDL_TRUE) {
			// Quit abruptly when requested.
			if (e.type == SDL_QUIT) {
				adapter.quit();
			} else if (e.type == SDL_KEYDOWN) {
				boss.key(e);
			} else if (e.type == SDL_TEXTINPUT) {
				boss.key(e);
			}
		}
		// Render the current state to the text buffer.
		boss.render();
		// Rasterize the text mode buffer to the video buffer.
		boss.raster(&adapter);
		// Save the video buffer, if requested.
		if (boss.save_video) {
			boss.save_video = false;
			// Get the current timestamp.
			auto t = std::time(nullptr);
			auto tm = *std::localtime(&t);
			// Generate a filename with the current timestamp.
			std::stringstream filename;
			filename << "export_" << std::put_time(&tm, "%d-%m-%Y-%H-%M-%S");
			// Save the video buffer as an image.
			adapter.save_bmp("export_" + filename.str() + ".bmp");
		}
		// Push the video buffer to the video card.
		adapter.push();

		#ifdef MATRIX_EFFECT
		// Update the falling characters.
		for (int i = 0; i < boss.hackermen.size(); i++) {
			hackerman& hacker = boss.hackermen[i];
			hacker.ascii = (hacker.ascii + 1) % 256;
			hacker.y += hacker.vy;
		}
		#endif
	}
	
	return 0;
}