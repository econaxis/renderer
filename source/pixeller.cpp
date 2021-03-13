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

#include "obj_loader.h"

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


std::unique_ptr<Image> image;
bool between(float a, float max) {
	return a > 0 && a < max;
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

	Model model, model1;
	model.load_from_file("C:/Users/henry/OneDrive - The University Of British Columbia/head.obj");

	model1 = model;
	model1.set_pos(0.5, 0.5, 0);

	// Right, near, far
	float r = 0.8, n = 2, f = 700;
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


	float angle_a = 1.537F, angle_b = 0.F, angle_c = -0.5F;
	//window.setFramerateLimit(30);
	gmtl::Matrix44f rotate;

	sf::Vector2i prev_mouse_pos;
	float angle_x = 1.542F, angle_y = 0.F;
	gmtl::Vec3f cam_direction, cam_position(0, 0, 30), target, up_direction(0, 1, 0);
	gmtl::Matrix44f camera_mat;

	double reflectivity = 3;

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
		}



		if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
			angle_x -= 2 * (float)(sf::Mouse::getPosition().x - prev_mouse_pos.x) / window.getSize().x;
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
			angle_b -= 0.02F;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
			angle_b += 0.02F;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
			angle_a += 0.02F;
		}if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
			angle_a -= 0.02F;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::PageUp)) {
			angle_c -= 0.02F;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::PageDown)) {
			angle_c += 0.02F;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) { cam_position += cam_direction; }
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) { cam_position -= cam_direction; }
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) { cam_position += up_direction; }
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) { cam_position -= up_direction; }
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) { cam_position -= gmtl::makeCross(cam_direction, up_direction); }
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) { cam_position += gmtl::makeCross(cam_direction, up_direction); }
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) { angle_x = 1.542F; angle_y = 0.F; }
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::G)) { reflectivity += 0.1; }
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::G) && sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) && reflectivity > 0.5) { reflectivity -= 0.2; }


		cam_direction[0] = -std::cosf(angle_x);
		cam_direction[1] = -std::sinf(angle_y);
		cam_direction[2] = -std::sinf(angle_x);
		gmtl::normalize(cam_direction);
		target = cam_position + cam_direction;
		camera_mat = lookAt(cam_position, target);

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
		auto tot_triangles = model.total_triangles();

		auto complete_matrix_transforms = persp * camera_mat * rotate * model.get_model_matrix();
		auto screen_complete_matrix_transforms = screen * complete_matrix_transforms;
#pragma omp parallel for default(none) shared(complete_matrix_transforms, model) num_threads(8)
		for (int i = 0; i <= tot_triangles - 3; i += 3) {
			auto normal_points = model.get_3_triangle(i);
			auto persp_pts = screen_complete_matrix_transforms * normal_points;


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


					const static gmtl::Vec3f light_direction = gmtl::makeNormal(gmtl::Vec3f{ -1, -1, -1 });

					gmtl::Vec3f face_position = rotate * gmtl::Vec3f{ normal_points(0, tri_index), normal_points(1, tri_index), normal_points(2, tri_index) };
					gmtl::Vec3f incident_light = face_position - light_direction;
					gmtl::normalize(incident_light);
					gmtl::Vec3f normal_dir = rotate * model.get_normal(i + tri_index / 3); // Automatically normalized
					gmtl::Vec3f face_to_camera = cam_position - face_position;
					gmtl::normalize(face_to_camera);


					if (gmtl::dot(normal_dir, face_to_camera) < 0) {
						normal_dir *= -1;
					}

					gmtl::Vec3f reflected_light = incident_light - 2 * gmtl::dot(normal_dir, incident_light) * normal_dir;

					if (gmtl::dot(reflected_light, face_to_camera) < 0) {
						reflected_light *= -1;
					}
					auto specular_intensity = std::powf(std::max(gmtl::dot(reflected_light, face_to_camera), 0.F), reflectivity) * 0.5F;

					// Maybe negative?
					auto intensity = std::max(gmtl::dot(light_direction, normal_dir), 0.F) * 0.6F;

					Color c =  Color::clamp(specular_intensity  + intensity * 0.2 , specular_intensity  + intensity * 0.2 + 0.1 , specular_intensity  + intensity * 0.2  + 0.3);

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

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) {
			auto x = sf::Mouse::getPosition(window).x * image->width / window.getSize().x,
				y = sf::Mouse::getPosition(window).y * image->height / window.getSize().y;
			std::cout << "Z buffer val: " << image->at_check(x, y).z << "\n" << angle_x << " " << angle_y << "\n";
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