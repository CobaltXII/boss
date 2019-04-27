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
}
