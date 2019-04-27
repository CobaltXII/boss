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
}
