#pragma once
#include <string>

#include <fstream>
#include <sstream>


#include <iostream>
#include <limits>
#include <gmtl/gmtl.h>



struct Point {

	static gmtl::Vec3f find_normal(const Point& p1, const Point& p2, const Point& p3) {
		gmtl::Vec3f v1(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z), v2(p3.x - p1.x, p3.y - p1.y, p3.z - p1.z);
		return gmtl::makeNormal(gmtl::makeCross(v1, v2));
	}

	short x=0, y=0;
	double z=0; // Double precision used for z buffering

	// Color
	float r=0, g=0, b=0;

	template<typename T>
	Point(T x, T y, double z = std::numeric_limits<double>::max()) :x(x), y(y), z(z) {};
	Point() = default;



	static Point interp_all(const Point& p1, const Point& p2, short y) {
		if (std::abs(p2.y - p1.y) < 0.00001) {
			// Division by zero scary
			// TODO bug prone? Cannot interp for vertical line, so we just output this bogus answer.
			return Point(p1.x,  y, p1.z);
		}
		auto cache1 = y - p1.y;
		auto cache2 = p2.y - p1.y;
		auto calculated_x = cache1 * (p2.x - p1.x) / cache2 + p1.x;
		auto calculated_z = (p2.z * cache1 + p1.z * (p2.y - y)) / cache2;

		return Point((short) calculated_x, (short) y,  calculated_z);
	}

	friend bool operator<(const Point& comp1, const Point& comp2) {
		return comp1.x < comp2.x;
	}

};
class Pixel
{

	// Used for the z buffer
	static std::ofstream debug;

public:
	double z = 100000;
	float r = 0, g = 0, b = 0;

	Pixel(float r, float g, float b, double z) : r(r), g(g), b(b), z(z) {};
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

	auto get_darkness() const {
		return (r + g + b) / 3;
	}

	void replace(float _r, float _g, float _b, double _z) {
		r = _r; g = _g; b = _b; z = _z;
	}

};

