#pragma once

#include <gmtl/gmtl.h>

#include "image.h"

inline bool between(float a, float max) {
	return a > 0 && a < max;
}


inline bool between_mat(const gmtl::Matrix<float, 4, 9>& persp_pts, int index, const Image& im) {
	const static float width = im.width;
	const static float height = im.height;
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
