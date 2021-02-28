#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <sstream>
#include <ostream>
#include <stdlib.h>
#include <thread>
#include <cassert>
#include <vector>

#include "VertexCalculator.h"

#include "image.h"
#include <gmtl/Matrix.h>
#include <gmtl/MatrixOps.h>


#include <iomanip>
using namespace std::chrono_literals;


template<typename DATA_TYPE, int ROWS, int COLS>
std::ostream& operator<<(std::ostream& os, const gmtl::Matrix<DATA_TYPE, ROWS, COLS>& m) {
	os << std::fixed << std::setprecision(2) << std::showpos;
	os << "[";
	for (int r = 0; r < ROWS; r++) {
		os << "\n";
		for (int c = 0; c < COLS; c++) {
			os << m(r, c) << " ";
		}
	}
	os << "]\n";
	return os;
}



int start(int argc, char* argv[])
{
	Pixel::init();
	std::unique_ptr<Image> image;
	if (argc == 3)
	{
		image = std::make_unique<Image>(std::stoi(argv[1]), std::stoi(argv[2]));
	}
	else
	{
		image = std::make_unique<Image>();
	}

	std::ios_base::sync_with_stdio(false);


	// Right, near, far
	double r = 0.5, n = 0.5, f = 3;
	gmtl::Matrix44d persp;
	persp.set(
		n / r, 0, 0, 0,
		0, n / r, 0, 0,
		0, 0, -(f + n) / (f - n), -2 * f * n / (f - n),
		0, 0, -1, 0
	);

	const std::vector<double> cube_pts_data{
   0 ,  0 ,  0 ,  1,
   1 ,  0 ,  0 ,  1,
   0 ,  0 ,  1 ,  1,
   1 ,  0 ,  1 ,  1,
   0 ,  1 ,  1 ,  1,
   1 ,  1 ,  1 ,  1,
   0 ,  1 ,  0 ,  1,
   1 ,  1 ,  0 ,  1,
   0 ,  0 ,  1 ,  1,
   0 ,  0 ,  0 ,  1,
   1 ,  0 ,  1 ,  1,
   1 ,  0 ,  0 ,  1,
   1 ,  1 ,  1 ,  1,
   1 ,  1 ,  0 ,  1,
   0 ,  1 ,  1 ,  1,
   0 ,  1 ,  0 ,  1,
   0 ,  0 ,  1 ,  1,
   0 ,  1 ,  1 ,  1,
   1 ,  0 ,  1 ,  1,
   1 ,  1 ,  1 ,  1,
   1 ,  0 ,  0 ,  1,
   1 ,  1 ,  0 ,  1,
   0 ,  0 ,  0 ,  1,
   0 ,  1 ,  0 ,  1
	};

	double translate_data[] = { 1, 0, 0, -0.5, 0, 1, 0, -0.5, 0, 0, -1, -2, 0, 0, 0, 1 };

	gmtl::Matrix44d translate;
	translate.setTranspose(translate_data);


	gmtl::Matrix<double, 4, 24> cube_pts;
	cube_pts.set(&(*(cube_pts_data.begin())));

	//std::cout << translate << cube_pts;
	//std::cout << translate * cube_pts;
	//std::cout << persp * translate * cube_pts;
	////return 0;

	for (double angle = 0; angle < 2 * gmtl::Math::PI; angle += 0.05) {
		gmtl::Matrix44d rotate;
		const double rot_data[] = { std::cos(angle), -std::sin(angle), 0, 0,
			std::sin(angle), std::cos(angle), 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1 };
		rotate.setTranspose(rot_data);
		auto persp_pts = persp * translate * rotate * cube_pts;
		for (int i = 0; i <= 23; i += 2) {
			double w1 = persp_pts[3][i];
			double w2 = persp_pts[3][i + 1];

			auto pt1 = std::pair<float, float>{ persp_pts[0][i] / w1, persp_pts[1][i] / w1 };
			auto pt2 = std::pair<float, float>{ persp_pts[0][i + 1] / w2, persp_pts[1][i + 1] / w2 };

			std::cout << pt1.first << " " << pt1.second << "\n" << pt2.first << " " << pt2.second << "\n\n";
			image->linef(pt1, pt2);
		}
		image->render();
		std::this_thread::sleep_for(75ms);

	}


	return 0;


	for (int i = 0; i < 50; i++)
	{
		float radian = i * 6.141F / 50.F;
		constexpr float incr = 6.141F / 3.F;

		auto pt1 = std::pair{ std::cosf(radian), std::sinf(radian) };
		auto pt2 = std::pair{ std::cosf(radian + incr), std::sinf(radian + incr) };
		auto pt3 = std::pair{ std::cosf(radian + incr * 2), std::sinf(radian + incr * 2) };

		image->triangle(pt1, pt2, pt3);
		image->render();
		std::this_thread::sleep_for(50ms);
	}

	return 0;

}

int main(int argc, char* argv[])
{
	try
	{
		start(argc, argv);
	}
	catch (const std::exception& e)
	{
		std::cout << "error: " << e.what();
	}
}