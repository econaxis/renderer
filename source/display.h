#pragma once
#include <SFML/Graphics.hpp>
#include <iomanip>
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
    sf::Font font;
    sf::Text text;
public:
    float font_size_scale = 6;
    ASCIIDisplay() {
        const static std::string FONT_PATH = "/usr/share/fonts/TTF/FiraCode-Regular.ttf";
        if (!font.loadFromFile(FONT_PATH)) {
            throw std::runtime_error(FONT_PATH + " doesn't exist");
        }

        std::cout<<std::setprecision(3);
    }

    void set_scale(float scale) {
        scale /= 10;
        float font_size = 10;
        // Set font size "breakpoints" similar to CSS breakpoints.
        // When the scale gets too small, we reduce the font size (from 10) and increase the scale. 
        if (scale < 0.3) {
            scale *= 6;
            font_size = 1;
            text.setScale({scale, scale * 2.12F});
        } else if (scale < 0.5) {
            scale *= 3;
            font_size = 3;
            text.setScale({scale, scale * 1.09F});
        }else if (scale < 0.8) {
            scale *= 1.9;
            font_size = 5;
            text.setScale({scale, scale * 1.03F});

        }else {
            text.setScale({scale, scale});
        }
        text.setFont(font);
        text.setCharacterSize((unsigned int) font_size);

    }
    // Renders the image
    std::stringstream render(const Image &im);
    sf::Text render_with_gui_text(const Image &im);
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

	void draw(sf::Drawable& drawable) {
	    window.draw(drawable);
	}
};
