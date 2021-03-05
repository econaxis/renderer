#pragma once
#include <memory>
#include "pixel.h"
#include <iostream>
#include <algorithm>
#include "display.h"

std::ostream& operator<<(std::ostream& os, std::pair<int, int> p);

// Responsible for blitting triangles/lines/pixels, antialiasing. Holds all the image data in a 1D vector. 
class Image
{
private:
	std::vector<Pixel> image;
	ASCIIDisplay display;
	PNGDisplay pngdisplay;
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
		return image[y * width + x];
	}

	void clear()
	{
		Pixel p{ 0 };
		// Clear screen white
		std::fill(image.begin(), image.end(), p);
	}

	void render()
	{
		windisplay_ptr->render(*this);
		clear();
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
		int x1 = pt1.first;
		int y1 = pt1.second;
		int x2 = pt2.first;
		int y2 = pt2.second;
		// Returns the list of pixels to fill on the line from (x1, y1) to (x2, y2)
		// Uses Bresenham's algorithm
		// TODO: antialiasing

		// Holds all the pixels on the line between pt1 and pt2.
		std::vector<std::pair<int, int>> points;



		// Handle the vertical case
		if (x1 == x2)
		{
			points.reserve(std::abs(y1 - y2));
			for (int y = std::min(y1, y2); y <= std::max(y1, y2); y++)
			{
				points.emplace_back(x1, y);
			}
			return points;
		}
		// Horizontal case
		else if (y1 == y2)
		{
			points.reserve(std::abs(x1 - x2));
			for (int x = std::min(x1, x2); x <= std::max(x1, x2); x++)
			{
				points.emplace_back(x, y1);
			}

			return points;
		}

		// Reserve in advance the Manhatten distance between pt1 and pt2. 
		// This will always overestimate the points, but it's quicker than Pythagorean theorem and prevents multiple costly allocations.
		points.reserve(3 * (std::abs(x1 - x2) + std::abs(y1 - y2)));

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
				//points.emplace_back(y + 1, x);
				//points.emplace_back(y - 1, x);
			}
			else
			{
				points.emplace_back(x, y);
				//points.emplace_back(x + 1, y);
				//points.emplace_back(x - 1, y);
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

	template<typename T>
	void line(std::pair<T, T> pt1, std::pair<T, T> pt2, float value = 1.F)
	{
		auto pts = line_points(pt1, pt2);

		for (const auto& pair : pts)
		{
			this->at(pair.first, pair.second).set_darkness(value);
		}
	}
	// Converts from floating point, -1 -> 1 coordinates to pixel coordinates
	auto pixel_coords(std::pair<float, float> in)
	{
		in.first = std::clamp(in.first, -1.F, 1.F);
		in.second = std::clamp(in.second, -1.F, 1.F);

		auto x = std::clamp((int)((in.first + 1) * width / 2), 0, width - 1);
		auto y = std::clamp((int)((in.second + 1) * height / 2), 0, height - 1);

		// Use unsigned short for memory efficiency
		return std::pair<unsigned short, unsigned short>{x, y};
	}
	void linef(std::pair<float, float> pt1, std::pair<float, float> pt2, float value = 1.F)
	{
		line(pixel_coords(pt1), pixel_coords(pt2), value);
	}


	//TODO: fill triangles horizontally for cache efficiency. Do write-up on this choice.
	// Renders a triangle given three points on the image.
	void triangle(std::pair<float, float> pt1, std::pair<float, float> pt2, std::pair<float, float> pt3, float value = 1.F)
	{

		// Convert coordinates from range (-1, 1) to (0, width)

		auto pts = std::vector{ pixel_coords(pt1),
							   pixel_coords(pt2),
							   pixel_coords(pt3) };


		std::sort(pts.begin(), pts.end(), [](const auto& t1, const auto& t2) ->bool {
			return t1.second < t2.second;
		});

		//auto base_line = line_points(pts[0], pts[2]);
		//auto bottom_line = line_points(pts[0], pts[1]);
		//auto top_line = line_points(pts[1], pts[2]);



		// Interpolator function that interpolates between two points and outputs a point in between at x value.
		auto interp = [](std::pair<short, short> p1, std::pair<short, short> p2, short y) -> std::pair<unsigned short, unsigned short> {
			if (std::abs(p2.second - p1.second) < 0.00001) {
				// Division by zero scary
				// TODO bug prone? Cannot interp for vertical line, so we just output this bogus answer.
				return std::pair{(unsigned short) p1.second, y};
			}

			return std::pair{(unsigned short) ((y - p1.second) * (p2.first - p1.first) / (p2.second - p1.second) + p1.first), y };
		};




		for (auto i = pts[0].second; i <= pts[2].second; i++) {
			auto base_pt = interp(pts[0], pts[2], i);

			if (i < pts[1].second) {
				// Left line + base line
				auto bottom_pt = interp(pts[0], pts[1], i);
				line(base_pt, bottom_pt, 0.2F);
			}
			else {
				// Right line + base line
				auto top_pt = interp(pts[1], pts[2], i);
				line(base_pt, top_pt, 0.2F);

			}

		}

		line(pts[0], pts[2], 0.8F);
		line(pts[1], pts[2], 0.8F);
		line(pts[0], pts[1], 0.8F);

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