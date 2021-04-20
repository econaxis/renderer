#pragma once
#include <gmtl/MatrixOps.h>
#include <iostream>
#include <gmtl/gmtl.h>
#include "image.h"
#include "obj_loader.h"
#include "utils.h"

class Light
{

	const inline static float FURTHEST_DEPTH = 100000;
	gmtl::Matrix44f perspective_matrix, screen_matrix, screen_complete_matrix_transforms;
	Image image;

	static gmtl::Matrix44f lookAt(gmtl::Vec3f &eye, gmtl::Vec3f &target, const gmtl::Vec3f &upDir = gmtl::Vec3f{0, 1, 0})
	{
		// compute the forward vector from target to eye
		gmtl::Vec3f forward = eye - target;
		gmtl::normalize(forward);

		// compute the left vector
		auto left = gmtl::makeCross(upDir, forward); // cross product
		gmtl::normalize(left);

		// recompute the orthonormal up vector
		auto up = gmtl::makeCross(forward, left); // cross product

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

public:
    const Image& get_image() {
	    return image;
	}
	gmtl::Vec3f light_pos = {-1, 1, 3}, light_target = {0, 0, 0};

	Light(std::size_t width, std::size_t height) : image(width, height)
	{
		screen_matrix.set(
			(float)width / 2, 0, 0, (float)width / 2,
			0, (float)height / 2, 0, (float)height / 2,
			0, 0, 1, 0,
			0, 0, 0, 1);

		const static float r = 0.8, n = 2, f = 700;
		perspective_matrix.set(
			n / r, 0, 0, 0,
			0, n / r, 0, 0,
			0, 0, -(f + n) / (f - n), -2 * f * n / (f - n),
			0, 0, -1, 0);

		// Set this as default position and targret.
		light_pos = {200, 200, 200};
		light_target = {0, 0, 0};
	};

	gmtl::Matrix44f get_matrix_transforms()
	{
		return screen_complete_matrix_transforms;
	}

	bool check_closest_lit(const gmtl::Point4f &point)
	{
		if (point[0] >= image.width || point[1] >= image.height || point[0] < 0 || point[1] < 0)
		{
			// 
			return true;
		}
		return image.get_z((std::size_t)point[0], (std::size_t)point[1]) < point[2] - 0.001;
	}

	void bake_light(const Model &model)
	{
		// TODO: lots of repeated code with pixeller.cpp. Have to separate into a member function of Model
		// I'm thinking "Model.render_to_image(const Image& im)"
		image.clear();
		const int tot_triangles = model.total_triangles();
		screen_complete_matrix_transforms = screen_matrix * perspective_matrix * lookAt(light_pos, light_target);
		const auto &model_matrix = model.get_model_matrix();
		for (int i = 0; i < tot_triangles; i++)
		{
			auto persp_pts = screen_complete_matrix_transforms * model.get_model_transformed_triangle(i);

			// Do the homogenous divide
			for (int column = 0; column < 3; column++)
			{
				persp_pts(0, column) /= persp_pts(3, column);
				persp_pts(1, column) /= persp_pts(3, column);
				persp_pts(2, column) /= persp_pts(3, column);
				persp_pts(3, column) = 1;
			}
			if (
				between_mat(persp_pts, image) &&
				(persp_pts(2, 0) - 0.00001 < image.get_z(persp_pts(0, 1), persp_pts(1, 1)) ||
				 persp_pts(2, 1) - 0.00001 < image.get_z(persp_pts(0, 2), persp_pts(1, 2)) ||
				 persp_pts(2, 2) - 0.00001 < image.get_z(persp_pts(0, 3), persp_pts(1, 3))))
			{
				gmtl::Point4f pt1{persp_pts(0, 0), persp_pts(1, 0), persp_pts(2, 0), 1};
				gmtl::Point4f pt2{persp_pts(0, 1), persp_pts(1, 1), persp_pts(2, 1), 1};
				gmtl::Point4f pt3{persp_pts(0, 2), persp_pts(1, 2), persp_pts(2, 2), 1};

                gmtl::Vec3f normal_dir = model.get_normal(i); // Automatically normalized

                // Light intensity changes to how the plane faces the light
                auto simple_cosine_lighting =
                        std::abs(gmtl::dot(gmtl::makeNormal(light_pos), normal_dir));

				image.triangle(pt1, pt2, pt3, Color::clamp(simple_cosine_lighting, 0.5, 0.5));
			}
		}
	}
};