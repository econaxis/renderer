#pragma once
#include <string>

#include <fstream>
#include <sstream>


#include <iostream>
#include <limits>
#include <gmtl/gmtl.h>

#include "color.h"
#include <gmtl/gmtl.h>


struct Point {
	// conversion from gmtl::Point4f
	static Point from_Point4f(const gmtl::Point4f& p4f) {
		if (p4f[3] != 1) {
			return Point(p4f[0] / p4f[3], p4f[1] / p4f[3], p4f[2] / p4f[3]);
		}
		else {
			return Point(p4f[0], p4f[1], p4f[2]);
		}
	}
    gmtl::Point4f to_Point4f() const {
	    return gmtl::Point4f{(float) x, (float) y, (float) z, 1};
	}
	short x = 0, y = 0;
	double z = 0; // Double precision used for z buffering

	// Color
	float r = 0, g = 0, b = 0;

	unsigned int face_id = 0;
	Color get_color (){
		return Color{r, g, b};
	}

	void set_color(Color c){
		r = c.r;
		g = c.g;
		b = c.b;
	}

	template<typename T>
	Point(T x, T y, double z = std::numeric_limits<double>::max()) :x(x), y(y), z(z) {};
	Point() = default;



	static Point interp_all(const Point& p1, const Point& p2, short y) {
		if (std::abs(p2.y - p1.y) < 0.00001) {
			// Division by zero scary
			// TODO bug prone? Cannot interp for vertical line, so we just output this bogus answer.
			return Point(p1.x, y, p1.z);
		}
		auto cache1 = y - p1.y;
		auto cache2 = p2.y - p1.y;
		auto calculated_x = cache1 * (p2.x - p1.x) / cache2 + p1.x;
		auto calculated_z = (p2.z * cache1 + p1.z * (p2.y - y)) / cache2;

		return Point((short)calculated_x, (short)y, calculated_z);
	}

	friend bool operator<(const Point& comp1, const Point& comp2) {
		return comp1.x < comp2.x;
	}

};

class Pixel
{

public:
	uint8_t r = 0, g = 0, b = 0, alpha = 255;

    explicit Pixel(float r, float g, float b) : r(r * 255), g(g * 255), b(b * 255){};
//    Pixel(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b){};
    explicit Pixel(const Color& p1) : r(p1.r * 255), g(p1.g * 255), b(p1.b * 255) {};
	Pixel() = default;


	template<typename T>
	void set_darkness(T v)
	{
		if (v < 0 || v > 1)
		{
			throw std::runtime_error("invalid value");
		}
		auto value = static_cast<double>(v);
		r = value;
		g = value;
		b = value;
	}

	auto set_color(float _r, float _g, float _b) {
		r = _r;
		g = _g;
		b = _b;
	}

	Color get_color() const {
		//do i need to do error handling
		return Color{(float) (r/255),(float)(g/255), (float)(b/255)};
	}

	auto get_darkness() const {
		return (r + g + b) / (255 * 3);
	}

	void replace(float _r, float _g, float _b) {
		r = _r; g = _g; b = _b;
	}

};

