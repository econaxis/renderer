#pragma once
#include <memory>
#include "pixel.h"
#include <iostream>
#include <algorithm>
#include "display.h"

std::ostream& operator<<(std::ostream& os, std::pair<int, int> p);

struct Color {
	float r = 0.0,  g = 0.0,  b = 0.0;
};


// Responsible for blitting triangles/lines/pixels, antialiasing. Holds all the image data in a 1D vector. 
class Image
{
private:
	std::vector<Pixel> image;
	//ASCIIDisplay display;
	//PNGDisplay pngdisplay;
	std::unique_ptr<WindowDisplay> windisplay_ptr; // Deferred construction. Must use unique_ptr


public:
	const int width, height;

	auto get_pixels() const -> const std::vector<Pixel>& {
		return image;
	}
	Image(int width = 800, int height = 600) : width(width), height(height)
	{
		image.resize((std::size_t)width * height);
		clear();
	}

	void use_window_display(sf::RenderWindow& window) {
		windisplay_ptr = std::make_unique<WindowDisplay>(window, width, height);
	}
	auto& operator[](int idx)
	{
		if (idx >= height)
		{
			throw std::runtime_error("invalid index");
		}
		return image[idx];
	}

	auto& at(std::size_t x, std::size_t y)
	{
		return image.at(y * width + x);
	}

	void clear()
	{
		Pixel p{};
		// Clear screen white

		for (auto& i : image) {
			i = p;
		}
	}

	void render()
	{
		windisplay_ptr->render(*this);
		clear();
	}

	void render_no_clear() {
		windisplay_ptr->render(*this);

	}
	auto& arr()
	{
		return image;
	}

	const auto& carr() const
	{
		return image;
	}

	template<typename T>
	auto line_points(std::pair<T, T> pt1, std::pair<T, T> pt2)
	{
		unsigned short x1 = pt1.first;
		unsigned short y1 = pt1.second;
		unsigned short x2 = pt2.first;
		unsigned short y2 = pt2.second;
		// Returns the list of pixels to fill on the line from (x1, y1) to (x2, y2)
		// Uses Bresenham's algorithm
		// TODO: antialiasing

		// Holds all the pixels on the line between pt1 and pt2.
		std::vector<std::pair<unsigned short, unsigned short>> points;

		// Horizontal case
		if (y1 == y2)
		{
			points.reserve(std::abs(x1 - x2));
			for (auto x = std::min(x1, x2); x <= std::max(x1, x2); x++)
			{
				points.emplace_back(x, y1);
			}

			return points;
		}
		// Vertical case
		else if (x1 == x2)
		{
			points.reserve(std::abs(y1 - y2));
			for (int y = std::min(y1, y2); y <= std::max(y1, y2); y++)
			{
				points.emplace_back(x1, y);
			}
			return points;
		}


		// Reserve in advance the Manhatten distance between pt1 and pt2. 
		// This will always overestimate the points, but it's quicker than Pythagorean theorem and prevents multiple costly allocations.
		points.reserve(std::abs(x1 - x2) + std::abs(y1 - y2));

		// Use Bresenham's algorithm for all other lines. 
		const bool steep = (fabs(y2 - y1) > fabs(x2 - x1));
		if (steep)
		{
			std::swap(x1, y1);
			std::swap(x2, y2);
		}

		if (x1 > x2)
		{
			std::swap(x1, x2);
			std::swap(y1, y2);
		}

		const int dx = x2 - x1;
		const int dy = std::abs(y2 - y1);

		float error = dx / 2.0f;
		const int ystep = (y1 < y2) ? 1 : -1;
		int y = (int)y1;

		const int maxX = (int)x2;

		for (int x = (int)x1; x <= maxX; x++)
		{
			if (steep)
			{
				points.emplace_back(y, x);
			}
			else
			{
				points.emplace_back(x, y);
			}

			error -= dy;
			if (error < 0)
			{
				y += ystep;
				error += dx;
			}
		}

		return points;
	}



	void line(Point p1, Point p2, float value = 1.F) {
		auto pts = line_points(std::pair{ p1.calculated_x, p1.y }, std::pair{ p2.calculated_x, p2.y });

		for (const auto& p : pts) {
			if (at(p.first, p.second).get_z() < std::min(p1.z, p2.z)) {
				at(p.first, p.second) = Pixel(1.0, 0.2, 0.3, std::max(p1.z, p2.z));
			}
		}
	}
	// Converts from floating point, -1 -> 1 coordinates to pixel coordinates
	auto pixel_coords(std::pair<float, float> in)
	{
		in.first = std::clamp(in.first, -1.F, 1.F);
		in.second = std::clamp(in.second, -1.F, 1.F);

		auto x = (in.first + 1) * width / 2;
		auto y = (in.second + 1) * height / 2;

		// Use unsigned short for memory efficiency
		return std::pair<unsigned short, unsigned short>{x, y};
	}


	void horizontal_line(Point p1, Point p2, Color c) {
		if (p1.calculated_x == 0 || p2.calculated_x == 0) {
			bool dummy = false;

		}
		auto start = std::min(p1, p2, [](const auto& less, const auto& more) ->bool {
			return less.calculated_x < more.calculated_x; });
		auto end = std::max(p1, p2, [](const auto& less, const auto& more) ->bool {
			return less.calculated_x < more.calculated_x;
		});
		if (at(start.calculated_x, p1.y).get_z() > start.z) {
			// Yes, can overwrite.
			at(start.calculated_x, p1.y) = Pixel(c.r+0.1, c.g + 0.2, c.b-0.1, start.z);
		}
		if (at(end.calculated_x, p1.y).get_z() > end.z) {
			// Yes, can overwrite.
			at(end.calculated_x, p1.y) = Pixel(c.r+0.1, c.g + 0.2, c.b-0.1, end.z);
		}
		for (auto x = start.calculated_x; x <= end.calculated_x; x++)
		{
			auto interp_z = (start.z * (x - start.calculated_x) + end.z * (end.calculated_x - x)) / (end.calculated_x - start.calculated_x);
				// Check z buffer.
				if (at(x, p1.y).get_z() > interp_z) {
					// Yes, can overwrite.
					if (at(x, p1.y).r == 0.0 && at(x, p1.y).g == 0.5) {
						at(x, p1.y) = Pixel(c.r, c.g, c.b, interp_z);
					}
					else {
						at(x, p1.y) = Pixel(c.r, c.g, c.b, interp_z);
					}
				}
				else {
					//at(x, p1.y).g = 1.0;
				}
		}

	}

	//TODO: fill triangles horizontally for cache efficiency. Do write-up on this choice.
	// Renders a triangle given three points on the image.
	void triangle(Point pt1, Point pt2, Point pt3, float value = 1.F)
	{

		// Convert coordinates from range (-1, 1) to (0, width)

		auto pts = std::vector{ pt1, pt2, pt3 };


		std::sort(pts.begin(), pts.end(), [](const auto& t1, const auto& t2) ->bool {
			return t1.y < t2.y;
			});



		Point base_pt, bottom_pt, top_pt;
		for (auto y = pts[0].y; y <= pts[1].y; y++) {
			base_pt = Point::interp_all(pts[0], pts[2], y);

			// Left line + base line
			bottom_pt = Point::interp_all(pts[0], pts[1], y);
			horizontal_line(base_pt, bottom_pt, {  (1.F- (float) base_pt.z) , 0.1F, 0.1F });

		}
		for (auto y = pts[1].y; y <= pts[2].y; y++) {
			base_pt = Point::interp_all(pts[0], pts[2], y);
			// Right line + base line
			top_pt = Point::interp_all(pts[1], pts[2], y);
			horizontal_line(base_pt, top_pt, { (float) (1.0 - base_pt.z) , 0.1F, 0.1F });
		}

		//line(pts[0], pts[2], 0.8F);
		//line(pts[1], pts[2], 0.8F);
		//line(pts[0], pts[1], 0.8F);

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

	//	//for (const auto& arr : carr())
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