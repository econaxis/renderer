#pragma once
#include <vector>
#include <iostream>

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