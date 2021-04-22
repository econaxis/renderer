#pragma once

#include "obj_loader.h"
#include "light.h"
#include "camera.h"
#include "image.h"
#include <gmtl/Matrix.h>
#include <thread>
#include <chrono>

inline float pow10(float val) {
    return val * val * val * val * val * val * val * val * val * val;
};

struct RenderScene {
    Model &model;
    Light &light;
    Camera &camera;
    Image &image;
    float k_reflectivity;
    gmtl::Matrix44f screen_persp;

    std::vector<long double> times;

    void handle_input() {
        if (model.check_rotated())
            light.bake_light(model);

        camera.handle_keyboard_input();
    }

    void render_some_triangles(const gmtl::Matrix44f &screen_complete_matrix_transforms, int start, int end) {
//        std::cout<<"Thread running on "<<sched_getcpu()<<"\n";
        for (int i = start; i < end; i++) {
            auto model_coordinates = model.get_model_transformed_triangle(i);
            auto persp_pts = screen_complete_matrix_transforms * model_coordinates;

            // Do the homogenous divide
            for (int column = 0; column < 3; column++) {
                persp_pts(0, column) /= persp_pts(3, column);
                persp_pts(1, column) /= persp_pts(3, column);
                persp_pts(2, column) /= persp_pts(3, column);
                persp_pts(3, column) = 1;
            }

            // TODO: refactor this code to avoid the interconversion between Point4f <--> Point3f <--> std::vector<Point>

            // We'll just use the location of the first point for lighting and shadow calculations.
            // Approximate the shadows/lighting/reflectance that applies to point 1 also applies equally to the other points.
            gmtl::Point4f pt1_world = mat_to_point(model_coordinates, 0).to_Point4f();

            // Screen coordinates
            Point pt1 = mat_to_point(persp_pts, 0);
            Point pt2 = mat_to_point(persp_pts, 1);
            Point pt3 = mat_to_point(persp_pts, 2);


            if (
                // Do some simple bounds checking.
                // Points are inside the viewport (not behind/clipped/far away)
                    between_mat(persp_pts, image) &&
                    // Points are in front of the current z-buffer.
                    check_z_buffer(pt1, pt2, pt3, image)) {
                gmtl::Vec3f normal_dir = model.get_normal(i); // Automatically normalized
                gmtl::Vec3f face_position = {pt1_world[0], pt1_world[1], pt1_world[2]};

                gmtl::Vec3f light_angle = light.light_pos - face_position;
                // Light intensity changes to how the plane faces the light
                auto simple_cosine_lighting =
                        std::max(gmtl::dot(gmtl::makeNormal(light_angle), normal_dir) * 0.7F, 0.F);
                simple_cosine_lighting *= simple_cosine_lighting;
                gmtl::Vec3f incident_light = face_position - light.light_pos;
                gmtl::Vec3f face_to_camera = camera.cam_position - face_position;
                gmtl::normalize(incident_light);
                gmtl::normalize(face_to_camera);

                gmtl::Vec3f reflected_light =
                        incident_light - 2 * gmtl::dot(normal_dir, incident_light) * normal_dir;

                float specular_intensity_temp = std::clamp(gmtl::dot(reflected_light, face_to_camera), 0.F, 1.F);

                float specular_intensity = pow10(specular_intensity_temp) * k_reflectivity;
                float ambient_light = 0.2F;

                // First, convert point 1 world coords into the coordinates of the light reference frame.
                auto light_reference_frame = light.get_matrix_transforms() * pt1_world;

                for (int row = 0; row < 4; row++)
                    light_reference_frame[row] /= light_reference_frame[3];

                // Then, we check if the point1 in light reference frame is seen by the light
                if (light.check_closest_lit(light_reference_frame)) {
                    // If coordinate is in light reference frame, then we decrease the lighting for it.
                    specular_intensity /= 5;
                    simple_cosine_lighting /= 1.5;
                    ambient_light /= 2; // Contradicts the meaning of "ambient" light but this method looks better.
                }
                Color c = Color::clamp(specular_intensity + simple_cosine_lighting + ambient_light + 0.15,
                                       specular_intensity + simple_cosine_lighting + ambient_light,
                                       specular_intensity + simple_cosine_lighting + ambient_light);

                image.triangle(pt1, pt2, pt3, c);
            }
        }
    }

    void main_render_code() { // Handle keyboard input events for the frame, and if the model has rotated,
        // that means the shadow map has also changed. We need to update the light shadow
        // mapping everytime the model changes.
        image.clear();
        handle_input();

        // This matrix transforms world coordinates -> camera coordinates -> perspective transform -> screen.
        gmtl::Matrix44f screen_complete_matrix_transforms = screen_persp * camera.camera_mat;

        const int tot_triangles = model.total_triangles();
        const int work_interval = tot_triangles / 4;
        const int work_intervals[] = {0, work_interval, work_interval * 2, work_interval * 3, tot_triangles};

        auto start_time = std::chrono::high_resolution_clock::now();

        std::thread t1(&RenderScene::render_some_triangles, this, std::ref(screen_complete_matrix_transforms),
                       work_intervals[0], work_intervals[1]);
        std::thread t2(&RenderScene::render_some_triangles, this, std::ref(screen_complete_matrix_transforms),
                       work_intervals[1], work_intervals[2]);
        std::thread t3(&RenderScene::render_some_triangles, this, std::ref(screen_complete_matrix_transforms),
                       work_intervals[2], work_intervals[3]);
        std::thread t4(&RenderScene::render_some_triangles, this, std::ref(screen_complete_matrix_transforms),
                       work_intervals[3], work_intervals[4]);
        t1.join();
        t2.join();
        t3.join();
        t4.join();
        auto end_time = std::chrono::high_resolution_clock::now();
        times.push_back(std::chrono::duration_cast<std::chrono::duration<long double, std::milli>>(
                end_time - start_time).count());
//#pragma omp parallel for shared(model, tot_triangles, screen_complete_matrix_transforms) default(none)
        image.render();
    }

    void display_time() {
        long double a = 0;
        for (const auto &t : times) {
            a += t;
        }
        std::cout << a / times.size() << std::endl;
    }
};

