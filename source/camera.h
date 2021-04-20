#pragma once

#include "sfml_header.h"

#include <gmtl/Matrix.h>


struct Camera {
    float angle_x = 1.542F, angle_y = 0.F;
    gmtl::Vec3f cam_position, up_direction, target, cam_direction;
    gmtl::Matrix44f camera_mat;

    Camera() : cam_position(750, 750, 750), up_direction(0, 1, 0), target(0, 0, 0) {
        camera_mat = lookAt(cam_position, target);
        cam_direction = cam_position - target;
        gmtl::normalize(cam_direction);

        angle_x = 0.39398;
        angle_y = 0.6969;

        std::cout << "angle x: " << angle_x << " angle_y:" << angle_y << std::endl;
        std::cout << "cam_direction: " << cam_direction << std::endl;
        reprocess_camera_mat();

    }

    void handle_keyboard_input() {
        bool cam_changed = false;
        // Camera angle changing not supported yet.
        if (sf::Keyboard::Dragged()) {
            angle_x -= sf::Keyboard::movementX / 300.F;
            angle_y += sf::Keyboard::movementY / 100.F;
            cam_changed = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            cam_position += cam_direction * 10.F;
            cam_changed = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            cam_position -= cam_direction * 10.F;
            cam_changed = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
            cam_position += up_direction * 10.F;
            cam_changed = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
            cam_position -= up_direction * 10.F;
            cam_changed = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            cam_position -= gmtl::makeCross(cam_direction, up_direction) * 10.F;
            cam_changed = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            cam_position += gmtl::makeCross(cam_direction, up_direction) * 10.F;
            cam_changed = true;
        }

        if (cam_changed) {
            reprocess_camera_mat();
//            std::cout<<"Cam position: "<<cam_position<<"; camera target: "<<target<<std::endl;
        }
    }

    void reprocess_camera_mat() {
        cam_direction[0] = -std::cos(angle_x);
        cam_direction[1] = -std::sin(angle_y);
        cam_direction[2] = -std::sin(angle_x);
        gmtl::normalize(cam_direction);

        target = cam_position + cam_direction * 10.F;
        camera_mat = lookAt(cam_position, target);
//        std::cout<<"cam position: "<<cam_position<<" cam target"<<target<<" cam direction: "<<cam_direction
//            <<" "<<angle_x<<" "<<angle_y<<std::endl;
    }

    gmtl::Matrix44f lookAt(gmtl::Vec3f &eye, gmtl::Vec3f &target, const gmtl::Vec3f &upDir = gmtl::Vec3f{0, 1, 0}) {
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
};