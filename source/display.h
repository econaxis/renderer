#pragma once

#include <iomanip>
#include <vector>
#include <iostream>
#include <fstream>

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
    std::ofstream ascii_file;
public:
    ASCIIDisplay() : ascii_file("/home/henry/renderer/display.txt"){
//        const static std::string FONT_PATH = "/usr/share/fonts/TTF/FiraCode-Regular.ttf";
//        if (!font.loadFromFile(FONT_PATH)) {
//            std::cout<<"Font couldn't be found. Reverting to console output.";
//        } else {
//            font_loaded = true;
//        }

        std::cout<<std::setprecision(2);
    }

//    void normalize_font_scaling(float scale) {
//        scale /= 10;
//        float font_size = 10;
//        // Set font size "breakpoints" similar to CSS breakpoints.
//        // When the scale gets too small, we reduce the font size (from 10) and increase the scale.
//        if (scale < 0.3) {
//            scale *= 6;
//            font_size = 1;
//            text.setScale({scale, scale * 2.12F});
//        } else if (scale < 0.5) {
//            scale *= 3;
//            font_size = 3;
//            text.setScale({scale, scale * 1.09F});
//        }else if (scale < 0.8) {
//            scale *= 1.9;
//            font_size = 5;
//            text.setScale({scale, scale * 1.03F});
//
//        }else {
//            text.setScale({scale, scale});
//        }
//        text.setFont(font);
//        text.setCharacterSize((unsigned int) font_size);
//
//    }
    std::stringstream render(const Image &im); // Renders the image as a stringstream, from which we push to std::cout
};


class CanvasDisplay {
public:
    CanvasDisplay(std::size_t width, std::size_t height) {};
    const void* image_data_loc;
    std::vector<uint8_t> image_data;

    void render(const Image& im);
};

//class PNGDisplay {
//	png::image<png::rgb_pixel> pngimg;
//
//public:
//	PNGDisplay(std::size_t width = 1000, std::size_t height = 1000) : pngimg(width, height) {
//		for (std::size_t i = 0; i < width * height; i++) {
//			pngimg.set_pixel(i % width, i / width, png::rgb_pixel(50, 50, 50));
//		}
//		pngimg.write("frame.png");
//	};
//	void render(const Image& im);
//};

#ifdef HAS_SFML
#include <SFML/Graphics.hpp>
#include "sfml_header.h"
class WindowDisplay {
	sf::Texture texture;
	sf::Sprite sprite;
public:
	WindowDisplay(std::size_t width = 1000, std::size_t height = 800) {
		if (!texture.create(width, height)) {
			std::cout<<"texture creation failed\n";
		}
		sprite.setTexture(texture);
	}
	void render(const Image& im);
    ~WindowDisplay() {
        sf::Uint8 bogus[] = {4, 2};
        texture.update(bogus);
        sprite.setTexture(texture);
    }
//	void draw(sf::Drawable& drawable) {
//	    window.draw(drawable);
//	}
};
#endif