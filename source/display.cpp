#include "display.h"
#include "image.h"
#include <SFML/Graphics/Font.hpp>

// Increasing ASCII ramp of more text character.
const std::string ASCIIDisplay::scale = " .'`^\",:;Il!i><~+_-?][}{1)(|/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
const int ASCIIDisplay::scale_size = (int) ASCIIDisplay::scale.size() - 1;

std::stringstream ASCIIDisplay::render(const Image &im) {
    /*
     * Renders all image data in 'im' to a stringstream. Uses a grayscale ramp (ASCIIDisplay::scale) to convert analog
     * floating point color values into digital characters.
     */

    // Special ANSI Escape code to reset the cursor back to screen location 0, 0 to redraw the frame.
    // We push to this out_buffer, then we push out_buffer to cout then flush stream.
    // Can't control how often cout flushes, so this method gives us more control and more performance.
    std::stringstream screen_buffer;
    const auto &pixel_data = im.get_pixels();

    for (std::size_t y = 0; y < im.height; y++) {
        for (std::size_t x = 0; x < im.width; x++) {
            // Get a value from 0 to 1 representing the darkness of the current pixel.
            float value = pixel_data[y * im.width + x].get_darkness();

            // Convert the numerical darkness to a pixel representation. e.g. a very dark value would be presented by the high-pixel density dollar ($) sign.
            char pixel_representation = scale.at((std::size_t) (value * scale_size));

            // Push the character that most accurately represent the value of the pixel (black/whiteness) into the screen buffer.
            screen_buffer << pixel_representation;
        }
        screen_buffer << "\n";
    }
    ascii_file << screen_buffer.rdbuf();
        return screen_buffer;
};

sf::Text ASCIIDisplay::render_with_gui_text(const Image& im) {
    /*
     * Renders all image data in 'im' to a sf::Text object (equivalent basically to a string). That sf::Text can be rendered
     * to any window with window.draw(sf::Text).
     *
     * Uses ASCIIDisplay::render function to get the stream, then converts that stream to text.
     */
    auto text_stream = render(im);
    text.setString(text_stream.str());
    return text;
}

void PNGDisplay::render(const Image &im) {

    const auto &pixel_data = im.get_pixels();

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

void WindowDisplay::render(const Image &im) {
    window.clear();
    const auto *pixel_data = im.get_pixels().data();
    auto *pixel_data_destination = pixels.data();

#pragma omp parallel for default(none) shared(pixel_data, pixel_data_destination, im)
    for (std::size_t y = 0; y < im.height; y++) {
        std::size_t y_cache = 4 * y * texture.getSize().x;
        for (std::size_t x = 0; x < im.width; x++) {
            y_cache += 4;
            const auto &pixel = pixel_data[y * im.width + x];
            pixel_data_destination[y_cache] = (sf::Uint8) (pixel.r * 255);
            pixel_data_destination[y_cache + 1] = (sf::Uint8) (pixel.g * 255);
            pixel_data_destination[y_cache + 2] = (sf::Uint8) (pixel.b * 255);
            // Alpha, we don't need to set.
            // pixel_data_destination[y_cache + x + 3] = 255;

        }
    }

    texture.update(&pixels[0]);
    window.draw(sprite);
    window.display();
}
