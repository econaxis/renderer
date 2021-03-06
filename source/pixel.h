#pragma once
#include <string>

#include <fstream>
#include <sstream>


#include <iostream>
#include <limits>



auto interp = [](std::pair<double, double> p1, std::pair<double, double> p2, int x) {


};
struct Point {
	unsigned short calculated_x=0, y=0;
	double z=0; // Double precision used for z buffering

	// Color
	float r=0, g=0, b=0;

	Point(unsigned short x, unsigned short y, double z = -std::numeric_limits<double>::max()) :calculated_x(x), y(y), z(z) {};
	Point() = default;

	template<typename T, typename T1>
	static auto interp_x(std::pair<T, T1> p1, std::pair<T, T1> p2, T x) {
		if (std::abs(p2.first - p1.first) < 0.00001) {
			// Division by zero scary
			// TODO bug prone? Cannot interp for vertical line, so we just output this bogus answer.
			return std::pair<T, T1>{ x, p1.second };
		}

		auto y_val = (float) (p1.second - p2.second) / (p1.first - p2.first) * (x - p2.first) + p2.second;
		return std::pair<T, T1>{ x, y_val };
	};

	static Point interp_all(const Point& p1, const Point& p2, float y) {
		if (std::abs(p2.y - p1.y) < 0.00001) {
			// Division by zero scary
			// TODO bug prone? Cannot interp for vertical line, so we just output this bogus answer.
			return Point(p1.calculated_x, y, p1.z);
		}
		auto p1y = (float)p1.y;
		auto p1x = (float)p1.calculated_x;
		auto p1z = (float)p1.z;
		auto p2y = (float)p2.y;
		auto p2x = (float)p2.calculated_x;
		auto p2z = (float)p2.z;
		auto calculated_x = (y - p1y) * (p2x - p1x) / (p2y - p1y) + p1x;
		auto calculated_z = (p1z * (y - p1y) + p2z * (p2y - y)) / (p2y - p1y);

		if (calculated_z + 0.00001 < std::min(p1z, p2z) || calculated_z - 0.00001 > std::max(p1z, p2z)) {
			std::cout << "err";
		}
		if (calculated_x + 0.00001 < std::min(p1x, p2x) || calculated_x - 0.00001 > std::max(p1x, p2x)) {
			std::cout << "err";

		}

		return Point((unsigned short) calculated_x, y,  calculated_z);
	}


};
class Pixel
{

	// Used for the z buffer
	double z = 100000;
	static std::ofstream debug;

public:
	float r = 0, g = 0, b = 0;
	static void init() {
		debug.open("debug.txt");

	}

	mutable double offset = 0;
	Pixel(double v) : r(v), g(v), b(v) {};
	Pixel(float r, float g, float b, double z) : r(r), g(g), b(b), z(z) {};
	Pixel() = default;

	double get_z() const {
		return z;
	}

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

	auto get_r() const {
		return r;
	}

	auto get_b() const {
		return b;
	}
	auto get_g() const {
		return g;
	}


};

