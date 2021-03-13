#include "display.h"
#include <gmtl/AABox.h>
#include "image.h"
#include <omp.h>

const std::string ASCIIDisplay::scale = ".':-=+*#%B@$";
const int ASCIIDisplay::scale_size = (int) ASCIIDisplay::scale.size() - 1;

void ASCIIDisplay::render(const Image& im) {
	std::cout << ANSII_ESC << "[0;0H";
	// We push to this out_buffer, then we push out_buffer to cout then flush stream.
	// Can't control how often cout flushes, so this method gives us more control and more performance.
	std::stringstream screen_buffer;
	const auto& pixel_data = im.get_pixels();

	for (std::size_t y = 0; y < im.height; y++) {
		for (std::size_t x = 0; x < im.width; x++) {

			double value = pixel_data.at(y * im.width + x).get_darkness();

			char pixel_representation = scale.at((int)(value * scale_size));

			// Push the character that most accurately represent the value of the pixel (black/whiteness) into the screen buffer.
			screen_buffer << pixel_representation;
		}
		screen_buffer << "\n";
	}


	// Load the screen_buffer into cout.
	std::cout << screen_buffer.rdbuf() << std::flush;

	// Move the cursor back to the top left corner for the next write to happen.
	// This doesn't clear the screen.

};

void PNGDisplay::render(const Image& im) {

	const auto& pixel_data = im.get_pixels();

	for (std::size_t y = 0; y < im.height; y++) {
		for (std::size_t x = 0; x < im.width; x++) {

			float value = pixel_data.at(y * im.width + x).get_darkness();

			if (value == 0.0) {
				continue;
			}

			png::byte rgb_value = value * 256;
			pngimg.set_pixel(x, y, png::rgb_pixel(rgb_value, rgb_value, rgb_value));
		}
	}
	pngimg.write("frame.png");

	// Move the cursor back to the top left corner for the next write to happen.
	// This doesn't clear the screen.

};

void WindowDisplay::render(const Image& im) {
	const auto* pixel_data = im.get_pixels().data();
	auto* pixel_data_destination = pixels.data();
	
#pragma omp parallel for default(none) shared(pixel_data, pixel_data_destination)
	for (int y = 0; y < im.height; y++) {
		std::size_t y_cache = 4 * y * texture.getSize().x;
		for (int x = 0; x < im.width; x++) {
			y_cache += 4;
			const auto& pixel= pixel_data[y * im.width + x];
			pixel_data_destination[y_cache] = (sf::Uint8)(pixel.r * 255);
			pixel_data_destination[y_cache + 1] = (sf::Uint8)(pixel.g * 255);
			pixel_data_destination[y_cache + 2] = (sf::Uint8)(pixel.b * 255);
			// Alpha, we don't need to set.
			// pixel_data_destination[y_cache + x + 3] = 255;

		}
	}

	texture.update(&pixels[0]);
	window.draw(sprite);
}