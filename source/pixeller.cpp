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
	model.load_from_file("C:/Users/Henry/teddy.obj");


	std::unique_ptr<Image> image;
	if (argc == 3)
	{
		image = std::make_unique<Image>(std::stoi(argv[1]), std::stoi(argv[2]));
	}
	else
	{
		image = std::make_unique<Image>(1000, 800);
	}
	image->clear();

	std::ios_base::sync_with_stdio(false);


	// Right, near, far
	float r = 5, n = 6, f = 80;
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
		0, 0, 1, -40,
		0, 0, 0, 1
	);


	//std::cout << translate << cube_pts;
	//std::cout << translate * cube_pts;
	//std::cout << persp * translate * cube_pts;
	////return 0;

	sf::RenderWindow window(sf::VideoMode(image->width, image->height), "My window");
	image->use_window_display(window);


	float angle_a = 3.14F, angle_b = 0.F, angle_c = -0.5F;
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

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
			angle_b -= 0.2F;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
			angle_b += 0.2F;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
			angle_a += 0.2F;
		}if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
			angle_a -= 0.2F;
		}

		//angle_a += 0.05F;
		//angle_c += 0.075F;
		// Rendering
		float cosa = std::cosf(angle_a);
		float sina = std::sinf(angle_a);
		float cosb = std::cosf(angle_b);
		float sinb = std::sinf(angle_b);
		float cosy = std::cosf(angle_c);
		float siny = std::sinf(angle_c);
		gmtl::Matrix44f rotate;
		const float rot_data[] = {
			cosa * cosb, cosa * sinb * siny - sina * cosy, cosa * sinb + sina * siny, 0,
			sina * cosb, sina * sinb * siny + cosa * cosy, sina * sinb * cosy - cosa * siny, 0,
			-sinb, cosb * siny, cosb * cosy, 0,
			0, 0, 0, 1
		};

		rotate.setTranspose(rot_data);

		const auto tot_triangles = model.total_triangles();
		for (unsigned int i = 0; i < tot_triangles; i++) {

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

			Point p1((persp_pts(0, 0) + 1) * image->width / 2, (persp_pts(1, 0) + 1) * image->height / 2, persp_pts(2, 0));
			Point p2((persp_pts(0, 1) + 1) * image->width / 2, (persp_pts(1, 1) + 1) * image->height / 2, persp_pts(2, 1));
			Point p3((persp_pts(0, 2) + 1) * image->width / 2, (persp_pts(1, 2) + 1) * image->height / 2, persp_pts(2, 2));

			image->triangle(p1, p2, p3);


			static int counter = 0;
		
		
			/*image->linef({ persp_pts(0, 0), persp_pts(0, 1) }, { persp_pts(1, 0), persp_pts(1, 1) });
			image->linef({ persp_pts(0, 0), persp_pts(0, 1) }, { persp_pts(2, 0), persp_pts(2, 1) });
			image->linef({ persp_pts(1, 0), persp_pts(1, 1) }, { persp_pts(2, 0), persp_pts(2, 1) });*/

		}
		window.clear(sf::Color::Green);
		image->render();
		window.display();
		//image->triangle(
		//	{ 10, 10, 1.0},
		//	{ 40, 40 , 1.0},
		//	{ 5, 60, 1.0}
		//);
		//std::cout << "rendered one frame\n";

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