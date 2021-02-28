#pragma once
#include <string>

#include <fstream>
#include <sstream>
class Pixel
{
	double value = 0;


	static std::ofstream debug;

public:
	static void init() {
		debug.open("debug.txt");

	}

	mutable double offset = 0;
	Pixel(double v) : value(v) {};
	Pixel() = default;


	template<typename T>
	void set_darkness(T v)
	{
		if (v < 0 || v > 1)
		{
			throw std::runtime_error("invalid value");
		}
		value = static_cast<double>(v);
	}

	auto get_darkness() const {
		return value;
	}

};

