#include "utils.h"
#include "image.h"
#include "pixel.h"

bool between(float a, float max) {
    return a > 0 && a < max;
}

bool between_mat(const gmtl::Matrix<float, 4, 3> &persp_pts, const Image &im) {
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

bool check_z_buffer(Point &pt1, Point &pt2, Point &pt3, const Image &image) {
    return pt1.z - 0.001F < image.get_z(pt1.x, pt1.y) ||
           pt2.z - 0.001F < image.get_z(pt2.x, pt2.y) ||
           pt3.z - 0.001F < image.get_z(pt3.x, pt3.y);
}

gmtl::Matrix44f create_screen_matrix(std::size_t pixel_width, std::size_t pixel_height) {
    gmtl::Matrix44f screen;
    screen.set(
            (float) pixel_width / 2, 0, 0, (float) pixel_width / 2,
            0, (float) pixel_height / 2, 0, (float) pixel_height / 2,
            0, 0, 1, 0,
            0, 0, 0, 1);
    return screen;
}

gmtl::Matrix44f create_perspective_transform_matrix(float r, float b, float n, float f) {
    gmtl::Matrix44f persp;
    persp.set(
            n / r, 0, 0, 0,
            0, n / b, 0, 0,
            0, 0, -(f + n) / (f - n), -2 * f * n / (f - n),
            0, 0, -1, 0);
    return persp;
}

// Extracts a point from a 4x4 matrix given the column index.
Point mat_to_point(const gmtl::Matrix<float, 4, 3> &mat, int col) {
    return Point{mat(0, col), mat(1, col), (double) mat(2, col)};
}
