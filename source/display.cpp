#include "display.h"
#include "image.h"

// Increasing ASCII ramp of more text character.
const std::string ASCIIDisplay::scale = " .'`^\",:;Il!i><~+_-?][}{1)(|/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
const int ASCIIDisplay::scale_size = (int) ASCIIDisplay::scale.size() - 1;

std::string buffer;

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
    buffer = screen_buffer.str();
    std::cout << "STRING BUFFER ADDR: " << (void *) (buffer.c_str()) << std::endl;
    ascii_file << screen_buffer.rdbuf();
    return screen_buffer;
};


void CanvasDisplay::render(const Image &im) {
    if (image_data.size() != im.height * im.width * 4) {
        image_data.resize(im.height * im.width * 4);
    }

    const auto &pixel_data = im.get_pixels();
    image_data_loc = pixel_data.data();
//    for (std::size_t y = 0; y < im.height; y++) {
//        const std::size_t y_cache = 4 * y * im.width;
//        for (std::size_t x = 0; x < im.width; x ++) {
//            image_data[y_cache + x*4] = (uint8_t) (pixel_data[y * im.width + x].r * 255);
//            image_data[y_cache + x*4 + 1] = (uint8_t) (pixel_data[y * im.width + x].g * 255);
//            image_data[y_cache + x*4 + 2] = (uint8_t) (pixel_data[y * im.width + x].b * 255);
//            image_data[y_cache + x*4 + 3] = (uint8_t) 255;
//        }
//    }

}
//sf::Text ASCIIDisplay::render_with_gui_text(const Image& im) {
//    /*
//     * Renders all image data in 'im' to a sf::Text object (equivalent basically to a string). That sf::Text can be rendered
//     * to any window with window.draw(sf::Text).
//     *
//     * Uses ASCIIDisplay::render function to get the stream, then converts that stream to text.
//     */
//    auto text_stream = render(im);
//    text.setString(text_stream.str());
//    return text;
//}

//void PNGDisplay::render(const Image &im) {
//
//    const auto &pixel_data = im.get_pixels();
//
//    for (std::size_t y = 0; y < im.height; y++) {
//        for (std::size_t x = 0; x < im.width; x++) {
//
//            float value = pixel_data.at(y * im.width + x).get_darkness();
//
//            if (value == 0.0) {
//                continue;
//            }
//
//            png::byte rgb_value = value * 256;
//            pngimg.set_pixel(x, y, png::rgb_pixel(rgb_value, rgb_value, rgb_value));
//        }
//    }
//    pngimg.write("frame.png");
//
//    // Move the cursor back to the top left corner for the next write to happen.
//    // This doesn't clear the screen.
//
//};

#ifdef HAS_SFML
void WindowDisplay::render(const Image &im) {
    get_window().clear();
    const auto *pixel_data = im.get_pixels().data();

    texture.update(reinterpret_cast<const sf::Uint8*>(pixel_data));
    get_window().draw(sprite);
    get_window().display();
}

#endif
