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

gmtl::Matrix44f create_perspective_transform_matrix(float r = 0.8, float n = 2, float f = 700) {
    gmtl::Matrix44f persp;
    persp.set(
            n / r, 0, 0, 0,
            0, n / r, 0, 0,
            0, 0, -(f + n) / (f - n), -2 * f * n / (f - n),
            0, 0, -1, 0
    );
    return persp;
}

// Extracts a point from a 4x4 matrix given the column index.
gmtl::Point4f mat_to_point(const gmtl::Matrix<float, 4, 9> mat, int col) {
    return gmtl::Point4f {mat(0, col), mat(1, col), mat(2, col), 1};
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

    Model model("../head.obj");

    gmtl::Matrix44f persp = create_perspective_transform_matrix();
    gmtl::Matrix44f screen = create_screen_matrix(image->width, image->height);


    sf::RenderWindow window(sf::VideoMode(1, 1), "My window");
    image->use_window_display(window);
    image->display.normalize_font_scaling(window.getSize().y / height);

    Light light(image->width, image->height);
    light.bake_light(model);

    Camera camera;



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
                if (event.key.code == sf::Keyboard::Num1 && width < 2000) {
                    // Increase the size of the image. This makes the picture clearer.
                    // We limit width to around 2000 pixels because at that level too much memoy is used.
                    width *= 1.2;
                    height *= 1.2;

                    // Clears the current image data and resizes it to specific width and height.
                    image->resize(width, height);

                    // Since we changed the width/height, the screen matrix (mapping normalized device coordinates to
                    // pixels need to be changed as well.
                    screen = create_screen_matrix(image->width, image->height);

                    // Only applies if we're using SFML to render text. Useless if we render with terminal.
                    // If we increase the number of pixels (characters), then the size of each character should decrease
                    // to make the overall image size constant.
                    image->display.normalize_font_scaling((float) window.getSize().y / height);
                } else if (event.key.code == sf::Keyboard::Tilde && height > 10 && width > 10) {
                    // Decrease size of the image
                    width /= 1.2;
                    height /= 1.2;
                    image->resize(width, height);
                    screen = create_screen_matrix(image->width, image->height);
                    image->display.normalize_font_scaling((float) window.getSize().y / height);
                } else if (event.key.code == sf::Keyboard::F2) {
                    window.close();
                }
            }
        }

        // Handle keyboard input events for the frame, and if the model has rotated,
        // that means the shadow map has also changed. We need to update the light shadow
        // mapping everytime the model changes.
        if (model.check_rotated()) light.bake_light(model);

        camera.handle_keyboard_input();

        // This matrix transforms world coordinates -> camera coordinates -> perspective transform -> screen.
        gmtl::Matrix44f screen_complete_matrix_transforms = screen * persp * camera.camera_mat;


        const auto tot_triangles = model.total_triangles();

#pragma omp parallel for shared(model, tot_triangles, screen_complete_matrix_transforms)
        for (int i = 0; i <= tot_triangles - 3; i += 3) {
            // Render 3 triangles at a time. Why 3? Idk.
            auto model_coordinates = model.get_model_matrix() * model.get_3_triangle(i);
            auto persp_pts = screen_complete_matrix_transforms * model_coordinates;

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
                gmtl::Point4f pt1_world = mat_to_point(model_coordinates, tri_index);

                // Screen coordinates
                gmtl::Point4f pt1 = mat_to_point(persp_pts, tri_index);
                gmtl::Point4f pt2 = mat_to_point(persp_pts, tri_index+1);
                gmtl::Point4f pt3 = mat_to_point(persp_pts, tri_index+2);
                if (
                    // Do some simple bounds checking.
                        // Points are inside the viewport (not behind/clipped/far away)
                        between_mat(persp_pts, tri_index, *image) &&
                        // Points are in front of the current z-buffer.
                        (pt1[2] - 0.0000001F < image->at(pt1[0], pt1[1]).z ||
                         pt2[2] - 0.0000001F < image->at(pt2[0], pt2[1]).z ||
                         pt3[2] - 0.0000001F < image->at(pt3[0], pt3[1]).z)
                        ) {


                    gmtl::Vec3f normal_dir = model.get_normal(i + tri_index / 3); // Automatically normalized
                    gmtl::Vec3f face_position = {pt1_world[0], pt1_world[1], pt1_world[2]};

                    // Light intensity changes to how the plane faces the light
                    auto simple_cosine_lighting =
                            std::max(gmtl::dot(gmtl::makeNormal(light.light_pos), normal_dir), 0.F) * 0.5F;
                    gmtl::Vec3f incident_light = face_position - light.light_pos;
                    gmtl::normalize(incident_light);
                    gmtl::Vec3f face_to_camera = camera.cam_position - face_position;
                    gmtl::normalize(face_to_camera);


                    gmtl::Vec3f reflected_light =
                            incident_light - 2 * gmtl::dot(normal_dir, incident_light) * normal_dir;

                    float specular_intensity =
                            (float) std::pow(std::max(gmtl::dot(reflected_light, face_to_camera), 0.F),
                                             specular_selectivity) * k_reflectivity;
                    float ambient_light = 0.3F;

                    // First, convert point 1 world coords into the coordinates of the light reference frame.
                    auto light_reference_frame = light.get_matrix_transforms() * pt1_world;
                    light_reference_frame[0] /= light_reference_frame[3];
                    light_reference_frame[1] /= light_reference_frame[3];
                    light_reference_frame[2] /= light_reference_frame[3];
                    light_reference_frame[3] = 1;

                    // Then, we check if the point1 in light reference frame is seen by the light
                    if (light.check_closest_lit(light_reference_frame) < light_reference_frame[2] - 0.000001) {
                        // If coordinate is in light reference frame, then we decrease the lighting for it.
                        specular_intensity = 0;
                        simple_cosine_lighting /= 2;
                        ambient_light = 0.1F; // Contradicts the meaning of "ambient" light but this method looks better..
                    }
                    Color c = Color::clamp(specular_intensity + simple_cosine_lighting + ambient_light + 0.15,
                                           specular_intensity + simple_cosine_lighting + ambient_light,
                                           specular_intensity + simple_cosine_lighting + ambient_light);


                    image->triangle(pt1, pt2, pt3, c);
                }
            }
        }
        image->render();

    }


    return 0;


}

int main(int argc, char *argv[]) {
//    try {
        //obj_test();
        start(argc, argv);
//    }
//    catch (const std::exception &e) {
//        std::cout << "error: " << e.what();
//    }
}
