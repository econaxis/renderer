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

#include "camera.h"


using namespace std::chrono_literals;




gmtl::Matrix44f create_screen_matrix(std::size_t pixel_width, std::size_t pixel_height) {
    gmtl::Matrix44f screen;
    screen.set(
            (float) pixel_width / 2, 0, 0, (float) pixel_width / 2,
            0, (float) pixel_height / 2, 0, (float) pixel_height / 2,
            0, 0, 1, 0,
            0, 0, 0, 1
    );
    return screen;
}

gmtl::Matrix44f create_perspective_transform_matrix (float r = 0.8, float n=2, float f = 700) {
    gmtl::Matrix44f persp;
    persp.set(
            n / r, 0, 0, 0,
            0, n / r, 0, 0,
            0, 0, -(f + n) / (f - n), -2 * f * n / (f - n),
            0, 0, -1, 0
    );
    return persp;
}



int start(int argc, char *argv[]) {
    std::unique_ptr<Image> image;

    std::size_t width = 800, height = 400;

    if (argc == 3) {
        width = std::stoi(argv[1]);
        height = std::stoi(argv[2]);
    }
    image = std::make_unique<Image>(width, height);
    image->clear();

    Model model ('../head.obj');

    gmtl::Matrix44f persp = create_perspective_transform_matrix();
    gmtl::Matrix44f screen = create_screen_matrix(image->width, image->height);

    sf::RenderWindow window(sf::VideoMode(1900, 1050), "My window");
    image->use_window_display(window);
    image->display.set_scale(window.getSize().y / height);

    Light light(image.width, image.height);
    light.bake_light(model);]
    
    Camera camera;


    float angle_x = 1.542F, angle_y = 0.F;

    // Lighting constants. Changing it changes the specific properties of the object (e.g. rubber/plastic/metal/wood...)
    int specular_selectivity = 35;
    double k_reflectivity = 0.6;



    // run the program as long as the window is open
    while (window.isOpen()) {
        // check all the window's events that were triggered since the last iteration of the loop
        sf::Event event;
        while (window.pollEvent(event)) {
            // "close requested" event: we close the window
            if (event.type == sf::Event::Closed) {
                image.release();
                window.close();
            } else if (event.type == sf::Event::KeyPressed) {
                // Hard to integrate this into classes...TODO
                // if (event.key.code == sf::Keyboard::LAlt) {
                //     prev_mouse_pos = sf::Mouse::getPosition(window);
                // }else
                if (event.key.code == sf::Keyboard::Num1 && width < 2000) {
                    // Increase the size of the image.
                    width *= 1.2;
                    height *= 1.2;
                    image->resize(width, height);
                    screen = create_screen_matrix(image->width, image->height);
                    image->display.set_scale((float) window.getSize().y / height);
                } else if (event.key.code == sf::Keyboard::Tilde && height > 10 && width > 10) {
                    // Decrease size of the image
                    width /= 1.2;
                    height /= 1.2;
                    image->resize(width, height);
                    screen = create_screen_matrix(image->width, image->height);
                    image->display.set_scale((float) window.getSize().y / height);}
            }
        }

        // Handle keyboard inputs for model, light, and camera.
        const model_did_rotate = model.check_rotated();
        if(model_did_rotate) light.bake_light();
        camera.handle_keyboard_input();

        gmtl::Matrix44f light_matrix = light.get_matrix_transforms();
        gmtl::Matrix44f screen_complete_matrix_transforms = screen * persp * camera.camera_mat;

        auto tot_triangles = model.total_triangles();
        const auto &model_matrix = model.get_model_matrix();

#pragma omp parallel for shared(model)
        for (int i = 0; i <= tot_triangles - 3; i += 3) {
            // Render 3 triangles at a time. Why 3? Idk.
            auto normal_points = model_matrix * model.get_3_triangle(i);
            auto persp_pts = screen_complete_matrix_transforms * normal_points;

            // Do the homogenous divide
            for (int column = 0; column < 9; column++) {
                persp_pts(0, column) /= persp_pts(3, column);
                persp_pts(1, column) /= persp_pts(3, column);
                persp_pts(2, column) /= persp_pts(3, column);
                persp_pts(3, column) = 1;
            }

            // Iterate through each triangle (triangle has 3 vertices, therefore increment by 3 each time). For loop runs 3 times.
            for (int tri_index = 0; tri_index <= 6; tri_index += 3) {
                // TODO: refactor this code to avoid the interconversion between Point4f <--> Point3f <--> std::vector<Point>

                // We'll just use the location of the first point for lighting and shadow calculations.
                // Approximate the shadows/lighting/reflectance that applies to point 1 also applies equally to the other points.
                gmtl::Point4f pt1_world{normal_points(0, tri_index), normal_points(1, tri_index),
                                        normal_points(2, tri_index), 1};

                gmtl::Point4f pt1{persp_pts(0, tri_index), persp_pts(1, tri_index), persp_pts(2, tri_index), 1};
                gmtl::Point4f pt2{persp_pts(0, tri_index + 1), persp_pts(1, tri_index + 1), persp_pts(2, tri_index + 1),
                                  1};
                gmtl::Point4f pt3{persp_pts(0, tri_index + 2), persp_pts(1, tri_index + 2), persp_pts(2, tri_index + 2),
                                  1};
                if (
                        between_mat(persp_pts, tri_index, *image) &&
                        (pt1[2] - 0.0000001 < image->at(pt1[0], pt1[1]).z ||
                         pt2[2] - 0.0000001 < image->at(pt2[0], pt2[1]).z ||
                         pt3[2] - 0.0000001 < image->at(pt3[0], pt3[1]).z)
                        ) {


                    gmtl::Vec3f normal_dir = model.get_normal(i + tri_index / 3); // Automatically normalized
                    gmtl::Vec3f face_position = {pt1_world[0], pt1_world[1], pt1_world[2]};

                    // Light intensity changes to how the plane faces the light
                    auto light_intensity = std::max(gmtl::dot(gmtl::makeNormal(light.light_pos), normal_dir), 0.F) * 0.5F;
                    gmtl::Vec3f incident_light = face_position - light.light_pos;
                    gmtl::normalize(incident_light);
                    gmtl::Vec3f face_to_camera = cam_position - face_position;
                    gmtl::normalize(face_to_camera);


                    gmtl::Vec3f reflected_light =
                            incident_light - 2 * gmtl::dot(normal_dir, incident_light) * normal_dir;

                    float specular_intensity =
                            (float) std::pow(std::max(gmtl::dot(reflected_light, face_to_camera), 0.F),
                                             specular_selectivity) * k_reflectivity;
                    float ambient_light = 0.3F;

                    // First, convert point 1 world coords into the coordinates of the light reference frame. 
                    auto light_reference_frame = light_matrix * pt1_world;
                    light_reference_frame[0] /= light_reference_frame[3];
                    light_reference_frame[1] /= light_reference_frame[3];
                    light_reference_frame[2] /= light_reference_frame[3];
                    light_reference_frame[3] = 1;

                    // Then, we check if the point1 in light reference frame is seen by the light
                    if (light.check_closest_lit(light_reference_frame) < light_reference_frame[2] - 0.000001) {
                        // If coordinate is in light reference frame, then we decrease the lighting for it.
                        specular_intensity = 0;
                        intensity /= 2;
                        ambient_light = 0.1F; // Contradicts the meaning of "ambient" light but this method looks better..
                    }
                    Color c = Color::clamp(specular_intensity + intensity + ambient_light + 0.15,
                                           specular_intensity + intensity + ambient_light,
                                           specular_intensity + intensity + ambient_light);


                    image->triangle(pt1, pt2, pt3, c);
                }
            }
        }
        image->render();

    }


    return 0;


}

int main(int argc, char *argv[]) {
    try {
        //obj_test();
        start(argc, argv);
    }
    catch (const std::exception &e) {
        std::cout << "error: " << e.what();
    }
}
