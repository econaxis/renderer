#pragma once
#include "pixel.h"
#include <iostream>
#include <algorithm>
#include "display.h"


// Responsible for blitting triangles/lines/pixels, antialiasing. Holds all the image data in a 1D vector. 
class Image
{
private:
	std::vector<Pixel> image;
	ASCIIDisplay asciidisplay;
	PNGDisplay display;

public:
	const int width, height;

	auto get_pixels() const -> const std::vector<Pixel>& {
		return image;
	}
	Image(int width = 1000, int height = 1000) : width(width), height(height), display(width, height)
	{
		image.resize((std::size_t) width * height);
		clear();
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
		// Clear screen white
		std::fill(image.begin(), image.end(), Pixel());
	}

	void render()
	{
		display.render(*this);
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


	auto line_points(std::pair<int, int> pt1, std::pair<int, int> pt2)
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

		// Reserve in advance the Manhatten distance between pt1 and pt2. 
		// This will always overestimate the points, but it's quicker than Pythagorean theorem and prevents multiple costly allocations.
		points.reserve(std::abs(x1 - x2) + std::abs(y1 - y2));

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

	void line(std::pair<int, int> pt1, std::pair<int, int> pt2)
	{
		auto pts = line_points(pt1, pt2);

		for (const auto& pair : pts)
		{
			this->at(pair.first, pair.second).set_darkness(1);
		}
	}

	void line(std::vector<std::pair<int, int>> pts, float value = 1)
	{
		for (const auto& p : pts)
		{
			this->at(p.first, p.second).set_darkness(value);
		}
	}

	// Converts from floating point, -1 -> 1 coordinates to pixel coordinates
	auto pixel_coords(std::pair<float, float> in)
	{
		in.first = std::clamp(in.first, -1.F, 1.F);
		in.second = std::clamp(in.second, -1.F, 1.F);

		auto x = std::clamp((int)((in.first + 1) * width / 2), 0, width - 1);
		auto y = std::clamp((int)((in.second + 1) * height / 2), 0, height - 1);

		return std::pair<int, int>{x, y};
	}
	void linef(std::pair<float, float> pt1, std::pair<float, float> pt2)
	{
		line(pixel_coords(pt1), pixel_coords(pt2));
	}


	//TODO: fill triangles horizontally for cache efficiency. Do write-up on this choice.
	// Renders a triangle given three points on the image.
	void triangle(std::pair<float, float> pt1, std::pair<float, float> pt2, std::pair<float, float> pt3)
	{

		// Convert coordinates from range (-1, 1) to (0, width)

		auto pts = std::vector{ pixel_coords(pt1),
							   pixel_coords(pt2),
							   pixel_coords(pt3) };


		std::sort(pts.begin(), pts.end());

		auto base_line = line_points(pts[0], pts[2]);
		auto left_line = line_points(pts[0], pts[1]);
		auto right_line = line_points(pts[1], pts[2]);


		// Interpolator function that interpolates between two points and outputs a point in between at x value.
		auto interp = [](std::pair<double, double> p1, std::pair<double, double> p2, int x) {

			//TODO: division by zero?
			auto slope = (p1.second - p2.second) / (p1.first - p2.first);
			auto y_val = slope * (x - p2.first) + p2.second;
			return std::pair<int, int>{ x,(int)y_val };
		};




		for (int i = pts[0].first; i <= pts[2].first; i++) {
			auto base_pt = interp(pts[0], pts[2], i);
			this->at(base_pt.first, base_pt.second).set_darkness(0.2);

			if (i < pts[1].first) {
				// Left line + base line
				auto left_pt = interp(pts[0], pts[1], i);
				line(base_pt, left_pt);
				this->at(left_pt.first, left_pt.second).set_darkness(0.2);
			}
			else {
				// Right line + base line
				auto right_pt = interp(pts[1], pts[2], i);
				line(base_pt, right_pt);
				this->at(right_pt.first, right_pt.second).set_darkness(0.2);
			}

		}

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