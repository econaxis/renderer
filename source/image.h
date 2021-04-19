#pragma once

#include <memory>
#include "pixel.h"
#include <iostream>
#include <algorithm>
#include "display.h"
#include <array>
#include <chrono>
#include <thread>
#include <gmtl/gmtl.h>
#include <cmath>
#include <thread>
#include "color.h"

// Responsible for blitting triangles/lines/pixels, antialiasing. Holds all the image data in a 1D vector.
class Image {
private:
    std::vector<Pixel> image;
    std::vector<double> z_buffer;
    std::vector<unsigned int> face_id;
public:
#ifdef HAS_SFML
    WindowDisplay display;
#else
    CanvasDisplay display;
#endif
    std::size_t width, height;

    const auto &get_pixels() const {
        return image;
    }

    double get_z(std::size_t x, std::size_t y) const {
        return z_buffer[y * width + x];
    }

    void set_z(std::size_t x, std::size_t y, double new_val) {
        z_buffer[y * width + x] = new_val;
    }

    Image(std::size_t width, std::size_t height) : width(width), height(height), display(width, height) {
        // In case we somehow overflow, make the image vector 1 bigger.
        image.resize(width * height + 1);
        z_buffer.resize(width * height + 1, 100000000000);
        face_id.resize(width * height + 1);
        clear();
    }

    void resize(std::size_t new_width, std::size_t new_height) {
        width = new_width;
        height = new_height;
        image.resize(width * height + 1);
        z_buffer.resize(width * height + 1, 100000000000);
        face_id.resize(width * height + 1);

        clear();
        std::cout << "Resized to " << new_width << " " << new_height << "\n";
    }

    auto &operator[](std::size_t idx) {
        if (idx >= height) {
            throw std::runtime_error("invalid index");
        }
        return image[idx];
    }

    auto &at(std::size_t x, std::size_t y) {
        return image[y * width + x];
    }

    const auto &at(std::size_t x, std::size_t y) const {
        return image[y * width + x];
    }

    void clear() {
        std::fill(image.begin(), image.end(), Pixel{});
        std::fill(z_buffer.begin(), z_buffer.end(), 100000000000);
    }

    void render() {
        display.render(*this);
    }

    template<typename T>
    auto line_points(std::pair<T, T> pt1, std::pair<T, T> pt2) {
        T x1 = pt1.first;
        T y1 = pt1.second;
        T x2 = pt2.first;
        T y2 = pt2.second;
        // Returns the list of pixels to fill on the line from (x1, y1) to (x2, y2)
        // Uses Bresenham's algorithm
        // TODO: antialiasing

        // Holds all the pixels on the line between pt1 and pt2.
        std::vector<std::pair<short, short>> points;

        // Horizontal case
        if (y1 == y2) {
            points.reserve(std::abs(x1 - x2));
            for (auto x = std::min(x1, x2); x <= std::max(x1, x2); x++) {
                points.emplace_back(x, y1);
            }

            return points;
        }
            // Vertical case
        else if (x1 == x2) {
            points.reserve(std::abs(y1 - y2));
            for (int y = std::min(y1, y2); y <= std::max(y1, y2); y++) {
                points.emplace_back(x1, y);
            }
            return points;
        }

        // Reserve in advance the Manhatten distance between pt1 and pt2.
        // This will always overestimate the points, but it's quicker than Pythagorean theorem and prevents multiple costly allocations.
        points.reserve(std::abs(x1 - x2) + std::abs(y1 - y2));

        // Use Bresenham's algorithm for all other lines.
        const bool steep = (fabs(y2 - y1) > fabs(x2 - x1));
        if (steep) {
            std::swap(x1, y1);
            std::swap(x2, y2);
        }

        if (x1 > x2) {
            std::swap(x1, x2);
            std::swap(y1, y2);
        }

        const int dx = x2 - x1;
        const int dy = std::abs(y2 - y1);

        float error = dx / 2.0f;
        const int ystep = (y1 < y2) ? 1 : -1;
        int y = (int) y1;

        const int maxX = (int) x2;

        for (int x = (int) x1; x <= maxX; x++) {
            if (steep) {
                points.emplace_back(y, x);
            } else {
                points.emplace_back(x, y);
            }

            error -= dy;
            if (error < 0) {
                y += ystep;
                error += dx;
            }
        }

        return points;
    }

    void set_face_id(std::size_t x, std::size_t y, unsigned int val_face_id) {
        face_id[y * width + x] = val_face_id;
    }

    unsigned int get_face_id(std::size_t x, std::size_t y) const {
        return face_id[y * width + x];
    }

    void horizontal_line(Point p1, Point p2, const Color &c, unsigned int face_id = 0) {
        const auto &start = std::min(p1, p2);
        const auto &end = std::max(p1, p2);

        double interp_z = start.z;
        double incr = (end.z - start.z) / (end.x - start.x);
        for (auto x = start.x; x < end.x; x++) {
            interp_z += incr;
            auto &target_pixel = at(x, p1.y);

            // Check z buffer.
            if (get_z(x, p1.y) >= interp_z) {
                // Yes, can overwrite.
                target_pixel = Pixel{c};
                set_z(x, p1.y, interp_z);
                set_face_id(x, p1.y, face_id);
            }
        }
    }

    //TODO: fill triangles horizontally for cache efficiency. Do write-up on this choice.
    // Renders a triangle given three points on the image.
    void triangle(Point pt1, Point pt2, Point pt3, Color c) {
        auto face_idval = pt1.face_id;

        auto pts = std::array<Point, 3>{pt1, pt2, pt3};

        std::sort(pts.begin(), pts.end(), [](const auto &t1, const auto &t2) -> bool {
            return t1.y < t2.y;
        });


        Point base_pt, bottom_pt, top_pt;
        for (short y = pts[0].y; y <= pts[1].y; y++) {
            base_pt = Point::interp_all(pts[0], pts[2], y);
            // Left line + base line
            bottom_pt = Point::interp_all(pts[0], pts[1], y);
            horizontal_line(base_pt, bottom_pt, c, face_idval);
        }
        for (short y = pts[1].y; y <= pts[2].y; y++) {
            base_pt = Point::interp_all(pts[0], pts[2], y);
            // Right line + base line
            top_pt = Point::interp_all(pts[1], pts[2], y);
            horizontal_line(base_pt, top_pt, c, face_idval);
        }
    }

    void triangle(const gmtl::Point4f &pt1, const gmtl::Point4f &pt2, const gmtl::Point4f &pt3, Color c) {
        /*
         * Similar to triangle method, except it takes in 3 gmtl::Point4f points and a color.
         */
        triangle(Point::from_Point4f(pt1), Point::from_Point4f(pt2), Point::from_Point4f(pt3), c);
    }

    //void print() const
    //{
    //	std::stringstream buffer;

    //	for (int h = 0; h < height; h++) {
    //		for (int w = 0; w < width; w++) {
    //			image[h * height + w].render(buffer);
    //		}
    //		buffer << "\n";
    //	}

    //	//for (const auto& arr : arr())
    //	//{
    //	//	auto last_offset = 0;
    //	//	for (const auto& i : arr)
    //	//	{
    //	//		i.offset += last_offset;
    //	//		auto offset = i.render(buffer);

    //	//		i.offset = 0;
    //	//	}
    //	//	buffer << '\n';
    //	//}

    //	std::cout << buffer.rdbuf() << std::flush;
    //}
};
