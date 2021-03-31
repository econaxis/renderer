#pragma once
#include <SFML/Graphics.hpp>
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
	PNGDisplay(std::size_t width = 1000, std::size_t height = 1000) : pngimg(width, height) {
		for (std::size_t i = 0; i < width * height; i++) {
			pngimg.set_pixel(i % width, i / width, png::rgb_pixel(50, 50, 50));
		}
		pngimg.write("frame.png");
	};
	void render(const Image& im);
};

class WindowDisplay {
	sf::Texture texture;
	sf::Sprite sprite;
	sf::RenderWindow& window;
	std::vector<sf::Uint8> pixels;
public:
	WindowDisplay(sf::RenderWindow& window, std::size_t width = 800, std::size_t height = 600) : window(window) {
		if (!texture.create(width, height)) {
			throw std::runtime_error("texture creation failed\n");
		}
		sprite.setTexture(texture);

		//RGBA color. 4 Uint8 per pixel
		pixels.resize(width * height * 4);

		for (std::size_t i = 3; i < pixels.size(); i += 4) {
			pixels[i] = (sf::Uint8)255;
		}
	}
	void render(const Image& im);
};