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

#include "utils.h"
#include "light.h"

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


	sf::RenderWindow window(sf::VideoMode(image->width, image->height), "My window");
	image->use_window_display(window);

	Light light(1000, 800);
	light.set_window(window);


	float angle_a = 1.537F, angle_b = 0.F, angle_c = -0.5F;
	//window.setFramerateLimit(30);
	gmtl::Matrix44f rotate;

	sf::Vector2i prev_mouse_pos;
	float angle_x = 1.542F, angle_y = 0.F;
	gmtl::Vec3f cam_direction, cam_position(0, 0, 400), target, up_direction(0, 1, 0);
	gmtl::Matrix44f camera_mat;

	double reflect_selectivity = 35, k_reflectivity = 0.6;

	light.light_pos = { 200, 200, 200 };
	light.light_target = { 0, 0, 0 };
	bool view_light = true, check_shadow = true;
	unsigned long long interval = 0;

	//model.fix_up_normals(cam_position);

	// run the program as long as the window is open
	while (window.isOpen())
	{


		// check all the window's events that were triggered since the last iteration of the loop
		sf::Event event;
		while (window.pollEvent(event))
		{
			// "close requested" event: we close the window
			if (event.type == sf::Event::Closed) {
				image.release();
				window.close();
			}
			else if (event.type == sf::Event::MouseButtonPressed) {
				prev_mouse_pos = sf::Mouse::getPosition();
			}
			else if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::Num2) {
					check_shadow = !check_shadow;
				}
			}
		}



		if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
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

		model.set_rotation(angle_a, angle_b, angle_c);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) { cam_position += cam_direction; }
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) { cam_position -= cam_direction; }
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) { cam_position += up_direction; }
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) { cam_position -= up_direction; }
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) { cam_position -= gmtl::makeCross(cam_direction, up_direction); }
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) { cam_position += gmtl::makeCross(cam_direction, up_direction); }
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) { angle_x = 1.542F; angle_y = 0.F; }
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::G)) { reflect_selectivity += 0.5; }
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::G) && sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) && reflect_selectivity > 0.5) { reflect_selectivity -= 0.2; }
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::H)) { k_reflectivity *= 1.3; }
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::H) && sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) { k_reflectivity /= 1.7; }


		cam_direction[0] = -std::cosf(angle_x);
		cam_direction[1] = -std::sinf(angle_y);
		cam_direction[2] = -std::sinf(angle_x);
		gmtl::normalize(cam_direction);
		target = cam_position + cam_direction;
		camera_mat = lookAt(cam_position, target);

		//light.light_pos = cam_position;
		//light.light_target = target;
		light.bake_light(model);
		gmtl::Matrix44f light_matrix = light.get_matrix_transforms();


		static int iterations = 0;
		auto tot_triangles = model.total_triangles();

		auto screen_complete_matrix_transforms = screen * persp * camera_mat;
		auto model_matrix = model.get_model_matrix();

#pragma omp parallel for default(none) shared( model) num_threads(8)
		for (int i = 0; i <= tot_triangles - 3; i += 3) {
			auto normal_points = model_matrix * model.get_3_triangle(i);
			auto persp_pts = screen_complete_matrix_transforms * normal_points;


			// Do the homogenous divide
			for (int column = 0; column < 9; column++) {
				persp_pts(0, column) /= persp_pts(3, column);
				persp_pts(1, column) /= persp_pts(3, column);
				persp_pts(2, column) /= persp_pts(3, column);
				persp_pts(3, column) = 1;
			}



			for (int tri_index = 0; tri_index <= 6; tri_index += 3) {
				gmtl::Point4f pt1_normal{ normal_points(0, tri_index), normal_points(1, tri_index), normal_points(2, tri_index), 1 };

				gmtl::Point4f pt1{ persp_pts(0, tri_index), persp_pts(1, tri_index), persp_pts(2, tri_index), 1 };
				gmtl::Point4f pt2{ persp_pts(0, tri_index + 1), persp_pts(1, tri_index + 1), persp_pts(2, tri_index + 1), 1 };
				gmtl::Point4f pt3{ persp_pts(0, tri_index + 2), persp_pts(1, tri_index + 2), persp_pts(2, tri_index + 2), 1 };
				if (
					between_mat(persp_pts, tri_index, *image) &&
					(pt1[2] - 0.01 < image->at(pt1[0], pt1[1]).z ||
						pt2[2] - 0.01 < image->at(pt2[0], pt2[1]).z ||
						pt3[2] - 0.01 < image->at(pt3[0], pt3[1]).z)
					) {



					gmtl::Vec3f normal_dir =  model.get_normal(i + tri_index / 3); // Automatically normalized
					gmtl::Vec3f face_position = { normal_points(0, tri_index), normal_points(1, tri_index), normal_points(2, tri_index) };
					auto intensity = std::max(gmtl::dot(gmtl::makeNormal(light.light_pos), normal_dir), 0.F) * 0.5F;
					gmtl::Vec3f incident_light = face_position - light.light_pos;
					gmtl::normalize(incident_light);
					gmtl::Vec3f face_to_camera = cam_position - face_position;
					gmtl::normalize(face_to_camera);



					gmtl::Vec3f reflected_light = incident_light - 2 * gmtl::dot(normal_dir, incident_light) * normal_dir;

					auto specular_intensity = std::powf(std::max(gmtl::dot(reflected_light, face_to_camera), 0.F), reflect_selectivity) * k_reflectivity;

					// Maybe negative?

					auto light_reference_frame = light_matrix * pt1_normal;
					light_reference_frame[0] /= light_reference_frame[3];
					light_reference_frame[1] /= light_reference_frame[3];
					light_reference_frame[2] /= light_reference_frame[3];
					light_reference_frame[3] = 1;

					float ambient_light = 0.3, red_ambience = 0;

					// Don't want this to trip. 
					if (check_shadow && light.check_closest_lit(light_reference_frame) < light_reference_frame[2] - 0.0001) {
						// Current shape is in shadow because it is further away from what's closest lit by the light.
						specular_intensity = 0;
						intensity /= 1.4;
						ambient_light = 0.2;
						//red_ambience = 0.4;
					}
					Color c = Color::clamp(specular_intensity + intensity + ambient_light + red_ambience + 0.05, specular_intensity + intensity + ambient_light, specular_intensity + intensity + ambient_light);


					image->triangle(pt1, pt2, pt3, c);

				}
			}

		}



		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) {
			float x = sf::Mouse::getPosition(window).x * image->width / window.getSize().x,
				y = sf::Mouse::getPosition(window).y * image->height / window.getSize().y;
			//std::cout << "Z buffer val: " << image->at_check(x, y).z << "\n" << angle_x << " " << angle_y << "\n";
			view_light = !view_light;
			std::cout << light.check_closest_lit({ x, y, 1, 1 }) << " " << image->at_check(x, y).z << "\n";
		}


		window.clear();
		if (view_light) {
			image->render();
		}
		else {
			light.render();
		}
		window.display();

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