#pragma once

#include <gmtl/gmtl.h>
#include "pixel.h"

class Image;


bool between(float a, float max);

bool between_mat(const gmtl::Matrix<float, 4, 3> &persp_pts, const Image &im);

bool check_z_buffer(Point &pt1, Point &pt2, Point &pt3, const Image &image);

gmtl::Matrix44f create_screen_matrix(std::size_t pixel_width, std::size_t pixel_height);

gmtl::Matrix44f create_perspective_transform_matrix(float r = 45, float b = 30, float n = 125, float f = 5000);

// Extracts a point from a 4x4 matrix given the column index.
Point mat_to_point(const gmtl::Matrix<float, 4, 3> &mat, int col);
