#pragma once

#include <gmtl/gmtl.h>

#include "image.h"

inline bool between(float a, float max)
{
	return a > 0 && a < max;
}

inline bool between_mat(const gmtl::Matrix<float, 4, 3> &persp_pts, const Image &im)
{
	/**
	 * Checks if the triangle persp_pts is viewable inside the frame of Image im
	 */
	const float width = im.width;
	const float height = im.height;
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

inline bool check_z_buffer(const gmtl::Point4f &pt1, const gmtl::Point4f &pt2, const gmtl::Point4f &pt3, const Image &image)
{
	return pt1[2] - 0.0000001F < image.at(pt1[0], pt1[1]).z ||
		   pt2[2] - 0.0000001F < image.at(pt2[0], pt2[1]).z ||
		   pt3[2] - 0.0000001F < image.at(pt3[0], pt3[1]).z;
}

inline gmtl::Matrix44f create_screen_matrix(std::size_t pixel_width, std::size_t pixel_height)
{
    gmtl::Matrix44f screen;
    screen.set(
        (float)pixel_width / 2, 0, 0, (float)pixel_width / 2,
        0, (float)pixel_height / 2, 0, (float)pixel_height / 2,
        0, 0, 1, 0,
        0, 0, 0, 1);
    return screen;
}
inline gmtl::Matrix44f create_perspective_transform_matrix(float r = 0.5, float n = 1, float f = 700)
{
    gmtl::Matrix44f persp;
    persp.set(
        n / r, 0, 0, 0,
        0, n / r, 0, 0,
        0, 0, -(f + n) / (f - n), -2 * f * n / (f - n),
        0, 0, -1, 0);
    return persp;
}

// Extracts a point from a 4x4 matrix given the column index.
inline gmtl::Point4f mat_to_point(const gmtl::Matrix<float, 4, 3> mat, int col)
{
    return gmtl::Point4f{mat(0, col), mat(1, col), mat(2, col), 1};
}
