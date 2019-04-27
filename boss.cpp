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

#include <memory>
#include <vector>
#include <string>
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
