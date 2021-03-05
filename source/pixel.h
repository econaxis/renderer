#pragma once
#include <string>

#include <fstream>
#include <sstream>
class Pixel
{
	double r = 0, g = 0, b = 0;

	static std::ofstream debug;

public:
	static void init() {
		debug.open("debug.txt");

	}

	mutable double offset = 0;
	Pixel(double v) : r(v), g(v), b(v) {};
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

	auto set_color(double _r, double _g, double _b) {
		r = _r;
		g = _g;
		b = _b;
	}

	auto get_darkness() const {
		return (r+g+b)/3;
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

