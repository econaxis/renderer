#pragma once
#include <vector>
#include <iostream>
#include <png++/png.hpp>

class Image;

// Will convert bitmap output from the Image to display onto the screen
// We can have multiple types of display, hence we can separate it out into a class.
// - ASCII display
// - Color display to PNG image file
// - xterm 256 color display

class ASCIIDisplay {
	static const char ANSII_ESC = char(27);
	static const std::string scale;
	static const int scale_size;

public:
	// Renders the image
	void render(const Image& im);
};

class PNGDisplay {
	png::image<png::rgb_pixel> pngimg;

public:
	PNGDisplay(int width = 1000, int height = 1000) : pngimg(width, height) {
		for (long i = 0; i < width * height; i++) {
			pngimg.set_pixel( i % width, (float) i / width, png::rgb_pixel(50, 50, 50));
		}
		pngimg.write("frame.png");
	};
	void render(const Image& im);
};