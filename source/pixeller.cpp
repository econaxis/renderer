#include <cmath>
#include <iostream>
#include <ostream>
#include <vector>
#include "image.h"
#include <gmtl/Matrix.h>
#include <gmtl/MatrixOps.h>




#include <iomanip>

#include <SFML/Window.hpp>

#include "obj_loader.h"


#include "utils.h"
#include "light.h"


using namespace std::chrono_literals;





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

	Model model, model1;
	model.load_from_file("../head.obj");



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
	gmtl::Matrix44f camera_mat, screen_complete_matrix_transforms;

	int specular_selectivity = 35;
	double k_reflectivity = 0.6;

	light.light_pos = { 200, 200, 200 };
	light.light_target = { 0, 0, 0 };
	bool view_light = false, check_shadow = true;
	bool model_rotated = true, cam_changed = true;


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
				else if (event.key.code == sf::Keyboard::LAlt) {
					prev_mouse_pos = sf::Mouse::getPosition(window);
				}
				else if (event.key.code == sf::Keyboard::Num1) {
					//std::cout << "Z buffer val: " << image->at_check(x, y).z << "\n" << angle_x << " " << angle_y << "\n";
					view_light = !view_light;
				}
			}
		}




		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
			angle_b -= 0.02F; model_rotated = true;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
			angle_b += 0.02F; model_rotated = true;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
			angle_a += 0.02F; model_rotated = true;
		}if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
			angle_a -= 0.02F; model_rotated = true;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::PageUp)) {
			angle_c -= 0.02F; model_rotated = true;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::PageDown)) {
			angle_c += 0.02F; model_rotated = true;
		}
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt)) {
            angle_x -= 2 * (float)(sf::Mouse::getPosition(window).x - prev_mouse_pos.x) / window.getSize().x;
            angle_y += 2 * (float)(sf::Mouse::getPosition(window).y - prev_mouse_pos.y) / window.getSize().y;
            prev_mouse_pos = sf::Mouse::getPosition(window);
            cam_changed = true;
        }
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) { cam_position += cam_direction; cam_changed = true; }
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) { cam_position -= cam_direction; cam_changed = true; }
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) { cam_position += up_direction; cam_changed = true; }
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) { cam_position -= up_direction; cam_changed = true; }
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) { cam_position -= gmtl::makeCross(cam_direction, up_direction); cam_changed = true; }
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) { cam_position += gmtl::makeCross(cam_direction, up_direction); cam_changed = true; }
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) { angle_x = 1.542F; angle_y = 0.F; cam_changed = true; }

		if (model_rotated || cam_changed || view_light) {
			if (model_rotated) {
				model.set_rotation(angle_a, angle_b, angle_c);
				model_rotated = false;
			}
			if (cam_changed) {
				cam_direction[0] = -std::cos(angle_x);
				cam_direction[1] = -std::sin(angle_y);
				cam_direction[2] = -std::sin(angle_x);
				gmtl::normalize(cam_direction);
				target = cam_position + cam_direction;
				camera_mat = lookAt(cam_position, target);
				cam_changed = false;
			}
			light.bake_light(model);
		}

		gmtl::Matrix44f light_matrix = light.get_matrix_transforms();


		auto tot_triangles = model.total_triangles();

		screen_complete_matrix_transforms = screen * persp * camera_mat;
		const auto& model_matrix = model.get_model_matrix();

#pragma omp parallel for shared(model) num_threads(8)
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
				gmtl::Point4f pt1_world{ normal_points(0, tri_index), normal_points(1, tri_index), normal_points(2, tri_index), 1 };

				gmtl::Point4f pt1{ persp_pts(0, tri_index), persp_pts(1, tri_index), persp_pts(2, tri_index), 1 };
				gmtl::Point4f pt2{ persp_pts(0, tri_index + 1), persp_pts(1, tri_index + 1), persp_pts(2, tri_index + 1), 1 };
				gmtl::Point4f pt3{ persp_pts(0, tri_index + 2), persp_pts(1, tri_index + 2), persp_pts(2, tri_index + 2), 1 };
				if (
					between_mat(persp_pts, tri_index, *image) &&
					(pt1[2] - 0.01 < image->at(pt1[0], pt1[1]).z ||
						pt2[2] - 0.01 < image->at(pt2[0], pt2[1]).z ||
						pt3[2] - 0.01 < image->at(pt3[0], pt3[1]).z)
					) {



					gmtl::Vec3f normal_dir = model.get_normal(i + tri_index / 3); // Automatically normalized
					gmtl::Vec3f face_position = { pt1_world[0], pt1_world[1], pt1_world[2] };
					auto intensity = std::max(gmtl::dot(gmtl::makeNormal(light.light_pos), normal_dir), 0.F) * 0.5F;
					gmtl::Vec3f incident_light = face_position - light.light_pos;
					gmtl::normalize(incident_light);
					gmtl::Vec3f face_to_camera = cam_position - face_position;
					gmtl::normalize(face_to_camera);



					gmtl::Vec3f reflected_light = incident_light - 2 * gmtl::dot(normal_dir, incident_light) * normal_dir;

					float specular_intensity =  (float) std::pow(std::max(gmtl::dot(reflected_light, face_to_camera), 0.F), specular_selectivity) * k_reflectivity;
					float ambient_light = 0.3;

					// Maybe negative?

					auto light_reference_frame = light_matrix * pt1_world;
					light_reference_frame[0] /= light_reference_frame[3];
					light_reference_frame[1] /= light_reference_frame[3];
					light_reference_frame[2] /= light_reference_frame[3];
					light_reference_frame[3] = 1;


					// Don't want this to trip. 
					if (check_shadow && light.check_closest_lit(light_reference_frame) < light_reference_frame[2] - 0.0001) {
						// Current shape is in shadow because it is further away from what's closest lit by the light.
						specular_intensity = 0;
						intensity /= 2;
						ambient_light = 0.1F;
						//red_ambience = 0.4;
					}
					Color c = Color::clamp(specular_intensity + intensity + ambient_light +  0.15, specular_intensity + intensity + ambient_light, specular_intensity + intensity + ambient_light);


					image->triangle(pt1, pt2, pt3, c);

				}
			}

		}






		window.clear();
		if (!view_light) {
			image->render();
		}
		else {
			light.render();

			cam_changed = true;
			model_rotated = true;
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