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

#include <png++/png.hpp>
#include <SFML/Window.hpp>



/* Jobs:
Separate input thread with std::mutex and std::thread and thread-safe queue
User Input
Camera movement
Filling in triangles
Model loading from file
Make universal container for image so we don't duplicate and use memory. e.g. both Image and Display class keeps a copy of total pixels.
*/

using namespace std::chrono_literals;



//template<typename DATA_TYPE, int ROWS, int COLS>
//std::ostream& operator<<(std::ostream& os, const gmtl::Matrix<DATA_TYPE, ROWS, COLS>& m) {
//	os << std::fixed << std::setprecision(2) << std::showpos;
//	os << "[";
//	for (int r = 0; r < ROWS; r++) {
//		os << "\n";
//		for (int c = 0; c < COLS; c++) {
//			os << m(r, c) << " ";
//		}
//	}
//	os << "]\n";
//	return os;
//}



// Begin OBJ test
#include "obj_loader.h"
int obj_test() {
	Model model;
	model.load_from_file("C:/Users/Henry/pyramid.obj");
	std::cout << model;

	return 1;
}


int start(int argc, char* argv[])
{

	Pixel::init();

	Model model;
	model.load_from_file("C:/Users/Henry/teapot.obj");


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
	float r = 1.0, n = 0.75, f = 7;
	gmtl::Matrix44f persp, trans;
	persp.set(
		n / r, 0, 0, 0,
		0, n / r, 0, 0,
		0, 0, -(f + n) / (f - n), -2 * f * n / (f - n),
		0, 0, -1, 0
	);

	trans.set(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, -5,
		0, 0, 0, 1
	);


	//std::cout << translate << cube_pts;
	//std::cout << translate * cube_pts;
	//std::cout << persp * translate * cube_pts;
	////return 0;

	sf::RenderWindow window(sf::VideoMode(800, 600), "My window");
	image->use_window_display(window);


	float angle_a = 0.F, angle_b = 0.F, angle_c = 0.F;
	window.setFramerateLimit(40);

	// run the program as long as the window is open
	while (window.isOpen())
	{

		// check all the window's events that were triggered since the last iteration of the loop
		sf::Event event;
		while (window.pollEvent(event))
		{
			// "close requested" event: we close the window
			if (event.type == sf::Event::Closed)
				window.close();

		}

		angle_a += 0.06F;
		angle_b -= 0.02F;
		angle_c += 0.05F;
		// Rendering
		float cosa = std::cos(angle_a);
		float sina = std::sin(angle_a);
		float cosb = std::cos(angle_b);
		float sinb = std::sin(angle_b);
		float cosy = std::cos(angle_c);
		float siny = std::sin(angle_c);
		gmtl::Matrix44f rotate;
		const float rot_data[] = {
			cosa * cosb, cosa*sinb*siny-sina*cosy, cosa * sinb + sina*siny, 0,
			sina * cosb, sina*sinb*siny + cosa*cosy, sina * sinb*cosy -cosa*siny, 0,
			-sinb, cosb*siny, cosb*cosy, 0,
			0, 0, 0, 1
		};

		rotate.setTranspose(rot_data);

		for (std::size_t i = 0; i < model.total_triangles(); i++) {

			auto persp_pts = persp * trans * rotate * model.get_triangle(i);


			// Do the homogenous divide
			for (int column = 0; column < 3; column++) {
				for (int row = 0; row < 4; row++) {
					persp_pts(row, column) /= persp_pts(3, column);

				}
			}

			if (persp_pts(0, 0) > 1 || persp_pts(1, 0) > 1 || persp_pts(0, 0) < -1 || persp_pts(1, 0) < -1
				|| persp_pts(0, 1) > 1 || persp_pts(1, 1) > 1 || persp_pts(0, 1) < -1 || persp_pts(1, 1) < -1
				|| persp_pts(0, 2) > 1 || persp_pts(1, 2) > 1 || persp_pts(0, 2) < -1 || persp_pts(1, 2) < -1) {
				continue;
			}

			image->triangle(
				{ persp_pts(0, 0), persp_pts(1, 0) },
				{ persp_pts(0, 1), persp_pts(1, 1) },
				{ persp_pts(0, 2), persp_pts(1, 2) }
			);
			/*image->linef({ persp_pts(0, 0), persp_pts(0, 1) }, { persp_pts(1, 0), persp_pts(1, 1) });
			image->linef({ persp_pts(0, 0), persp_pts(0, 1) }, { persp_pts(2, 0), persp_pts(2, 1) });
			image->linef({ persp_pts(1, 0), persp_pts(1, 1) }, { persp_pts(2, 0), persp_pts(2, 1) });*/

		}

		image->triangle(
			{ -0.9, -0.9 },
			{ -0.7, -0.7 },
			{ -0.95, -0.6 }
		);

		window.clear(sf::Color::Green);
		image->render();
		window.display();
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
		//obj_test();
		start(argc, argv);
	}
	catch (const std::exception& e)
	{
		std::cout << "error: " << e.what();
	}
}