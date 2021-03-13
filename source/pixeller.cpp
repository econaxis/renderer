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
#include <omp.h>


#include <atomic>


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
	model.load_from_file("C:/Users/Henry/head.obj");
	std::cout << model;

	return 1;
}

std::unique_ptr<Image> image;
bool between(float a, float max) {
	return a >= 0 && a <= max;
}

bool between_mat(const gmtl::Matrix<float, 4, 9>& persp_pts, int index) {
	const static float width = image->width;
	const static float height = image->height;
	return (
		between(persp_pts(0, index), width) &&
		between(persp_pts(1, index), height) &&
		between(persp_pts(2, index), 1) &&
		between(persp_pts(0, index + 1), width) &&
		between(persp_pts(1, index + 1), height) &&
		between(persp_pts(2, index + 1), 1) &&
		between(persp_pts(0, index + 2), width) &&
		between(persp_pts(1, index + 2), height) &&
		between(persp_pts(2, index + 2), 1));
}

bool between_mat(const gmtl::Matrix<float, 4, 3>& persp_pts) {
	const float width = image->width;
	const float height = image->height;
	return (
		between(persp_pts(0, 0), width) &&
		between(persp_pts(1, 0), height) &&
		between(persp_pts(2, 0), 1) &&
		between(persp_pts(0, 1), width) &&
		between(persp_pts(1, 1), height) &&
		between(persp_pts(2, 1), 1) &&
		between(persp_pts(0, 2), width) &&
		between(persp_pts(1, 2), height) &&
		between(persp_pts(2, 2), 1));
}


gmtl::Matrix44f lookAt(gmtl::Vec3f& eye, gmtl::Vec3f& target, const gmtl::Vec3f& upDir = gmtl::Vec3f{ 0, 1, 0 })
{
	// compute the forward vector from target to eye
	gmtl::Vec3f forward = eye - target;
	gmtl::normalize(forward);

	// compute the left vector
	auto left = gmtl::makeCross(upDir, forward); // cross product
	gmtl::normalize(left);

	// recompute the orthonormal up vector
	auto up = gmtl::makeCross(forward, left);    // cross product

	// init 4x4 matrix
	gmtl::Matrix44f matrix;


	// set rotation part, inverse rotation matrix: M^-1 = M^T for Euclidean transform
	matrix.mData[0] = left[0];
	matrix.mData[4] = left[1];
	matrix.mData[8] = left[2];
	matrix.mData[1] = up[0];
	matrix.mData[5] = up[1];
	matrix.mData[9] = up[2];
	matrix.mData[2] = forward[0];
	matrix.mData[6] = forward[1];
	matrix.mData[10] = forward[2];

	// set translation part
	matrix.mData[12] = -left[0] * eye[0] - left[1] * eye[1] - left[2] * eye[2];
	matrix.mData[13] = -up[0] * eye[0] - up[1] * eye[1] - up[2] * eye[2];
	matrix.mData[14] = -forward[0] * eye[0] - forward[1] * eye[1] - forward[2] * eye[2];

	matrix.mState = gmtl::Matrix44f::XformState::FULL;

	return matrix;
}
int start(int argc, char* argv[])
{




	if (argc == 3)
	{
		image = std::make_unique<Image>(std::stoi(argv[1]), std::stoi(argv[2]));
	}
	else
	{
		image = std::make_unique<Image>(1000, 800);
	}
	image->clear();

	Model model;
	model.load_from_file("C:/Users/henry/OneDrive - The University Of British Columbia/head.obj");

	std::ios_base::sync_with_stdio(false);


	// Right, near, far
	float r = 3, n = 7, f = 700;
	gmtl::Matrix44f persp, trans, screen;
	persp.set(
		n / r, 0, 0, 0,
		0, n / r, 0, 0,
		0, 0, -(f + n) / (f - n), -2 * f * n / (f - n),
		0, 0, -1, 0
	);

	screen.set(
		(float)image->width / 2, 0, 0, (float)image->width / 2,
		0, (float)image->height / 2, 0, (float)image->height / 2,
		0, 0, 1, 0,
		0, 0, 0, 1
	);

	gmtl::Matrix<float, 4, 3> horizon_line;
	const float hor_line_data[] = {
		f * r / n - 1, 1, -f + 1, 1,
		-f * r / n + 1, 1, -f + 1, 1,
		0, -1, -f + 1, 1
	};
	horizon_line.set(&hor_line_data[0]);

	//std::cout << translate << cube_pts;
	//std::cout << translate * cube_pts;
	//std::cout << persp * translate * cube_pts;
	////return 0;

	sf::RenderWindow window(sf::VideoMode(image->width, image->height), "My window");
	image->use_window_display(window);


	float angle_a = 3.14F, angle_b = 0.F, angle_c = -0.5F;
	//window.setFramerateLimit(30);
	gmtl::Matrix44f rotate;

	sf::Vector2i prev_mouse_pos;
	float angle_x = 4.712, angle_y = 0.F;
	gmtl::Vec3f cam_direction(0, 0, -1), cam_position(0, 0, 50), target(0, 0, 0), up_direction (0, 1, 0);
	gmtl::Matrix44f camera_mat;

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

			else if (event.type == sf::Event::MouseButtonPressed) {
				prev_mouse_pos = sf::Mouse::getPosition();
			}
			else if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::W) cam_position += cam_direction;
				if (event.key.code == sf::Keyboard::S) cam_position -= cam_direction;
				if (event.key.code == sf::Keyboard::A) cam_position -= gmtl::makeCross(cam_direction, up_direction);
				if (event.key.code == sf::Keyboard::D) cam_position += gmtl::makeCross(cam_direction, up_direction);
				if (event.key.code == sf::Keyboard::Space) prev_mouse_pos = sf::Mouse::getPosition();


				cam_direction[0] = -std::cosf(angle_x);
				cam_direction[1] = -std::sinf(angle_y);
				cam_direction[2] = -std::sinf(angle_x);
				gmtl::normalize(cam_direction);
				target = cam_position + cam_direction;

				camera_mat = lookAt(cam_position, target);

			}
		}



		if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
			angle_x += 2 * (float)(sf::Mouse::getPosition().x - prev_mouse_pos.x) / window.getSize().x;
			angle_y += 2 * (float)(sf::Mouse::getPosition().y - prev_mouse_pos.y) / window.getSize().y;
			prev_mouse_pos = sf::Mouse::getPosition();

			cam_direction[0] = -std::cosf(angle_x);
			cam_direction[1] = -std::sinf(angle_y);
			cam_direction[2] = -std::sinf(angle_x);
			gmtl::normalize(cam_direction);
			target = cam_position + cam_direction;

			camera_mat = lookAt(cam_position, target);
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
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::PageUp)) {
			angle_c -= 0.2F;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::PageDown)) {
			angle_c += 0.2F;
		}


		float cosa = std::cosf(angle_a);
		float sina = std::sinf(angle_a);
		float cosb = std::cosf(angle_b);
		float sinb = std::sinf(angle_b);
		float cosy = std::cosf(angle_c);
		float siny = std::sinf(angle_c);
		rotate.set(
			cosa * cosb, cosa * sinb * siny - sina * cosy, cosa * sinb * cosy + sina * siny, 0,
			sina * cosb, sina * sinb * siny + cosa * cosy, sina * sinb * cosy - cosa * siny, 0,
			-sinb, cosb * siny, cosb * cosy, 0,
			0, 0, 0, 1);


		static int iterations = 0;
		const auto tot_triangles = model.total_triangles();

		auto complete_matrix_transforms = persp * camera_mat * rotate;
#pragma omp parallel for default(none) shared(complete_matrix_transforms, model) num_threads(8)
		for (int i = 0; i <= tot_triangles - 3; i += 3) {

			auto model_pts = complete_matrix_transforms * model.get_3_triangle(i);
			auto persp_pts = screen * model_pts;


			// Do the homogenous divide
			for (int column = 0; column < 9; column++) {
				persp_pts(0, column) /= persp_pts(3, column);
				persp_pts(1, column) /= persp_pts(3, column);
				persp_pts(2, column) /= persp_pts(3, column);
				persp_pts(3, column) = 1;
			}


			for (int tri_index = 0; tri_index <= 6; tri_index += 3) {
				if (
					between_mat(persp_pts, tri_index) &&
					(persp_pts(2, tri_index) - 0.01 < image->at(persp_pts(0, tri_index), persp_pts(1, tri_index)).z ||
						persp_pts(2, tri_index + 1) - 0.01 < image->at(persp_pts(0, tri_index + 1), persp_pts(1, tri_index + 1)).z ||
						persp_pts(2, tri_index + 2) - 0.01 < image->at(persp_pts(0, tri_index + 2), persp_pts(1, tri_index + 2)).z)
					) {

					gmtl::Vec3f world_point1(model_pts(0, tri_index), model_pts(1, tri_index), model_pts(2, tri_index));
					gmtl::Vec3f world_point2(model_pts(0, tri_index + 1), model_pts(1, tri_index + 1), model_pts(2, tri_index + 1));
					gmtl::Vec3f world_point3(model_pts(0, tri_index + 2), model_pts(1, tri_index + 2), model_pts(2, tri_index + 2));

					auto tri_normal = gmtl::makeNormal(gmtl::makeCross(gmtl::Vec3f(world_point2 - world_point1), gmtl::Vec3f(world_point3 - world_point1)));
					const static gmtl::Vec3f light_direction = gmtl::makeNormal(gmtl::Vec3f{ -1, -1, -1 });

					auto intensity = std::abs(gmtl::dot(light_direction, tri_normal));
					Color c = { intensity * 0.8, intensity * 0.5,intensity * 0.3 };

					image->triangle(
						{ persp_pts(0, tri_index), persp_pts(1, tri_index), persp_pts(2, tri_index) },
						{ persp_pts(0, tri_index + 1), persp_pts(1, tri_index + 1), persp_pts(2, tri_index + 1) },
						{ persp_pts(0, tri_index + 2), persp_pts(1, tri_index + 2), persp_pts(2, tri_index + 2) },
						c);
				}
			}
		}

		auto hor_points = complete_matrix_transforms * horizon_line;
		for (int column = 0; column < 3; column++) {
			hor_points(0, column) /= hor_points(3, column);
			hor_points(1, column) /= hor_points(3, column);
			hor_points(2, column) /= hor_points(3, column);
			hor_points(3, column) = 1;
		}
		if (between_mat(hor_points)) {
			image->triangle(hor_points, Color{ 1.F, 0.F, 1.F });
		}



		image->render();

		if ((++iterations) % 500 == 0) {
			//std::cout << mat_time << " " << tri_time << " " << render_time << ": total time 500 iters\n";
			//std::cout << " real time: " << std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - start).count() << "\n";
			//render_time = 0;
			//tri_time = 0;
			//mat_time = 0;
			//start = std::chrono::high_resolution_clock::now();
		}
		window.display();
		//image->triangle(
		//	{ 10, 10, 1.0},
		//	{ 40, 40 , 1.0},
		//	{ 5, 60, 1.0}
		//)
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