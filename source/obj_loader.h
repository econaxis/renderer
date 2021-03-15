#pragma once

#include <fstream>
#include <gmtl/gmtl.h>
#include <vector>
#include <ostream>
#include <iostream>
#include <array>

#include <regex>
#include <string>
#include <ctype.h>
#include <gmtl/Matrix.h>
struct Face {
	std::array<unsigned int, 3> point_indices;
	gmtl::Vec3f normal;
	unsigned int& operator[](unsigned int index) {
		return point_indices[index];
	}
	const unsigned int operator[](unsigned int index) const {
		return point_indices[index];
	}

	const gmtl::Vec3f& get_normal() const {
		return normal;
	}

};

class Model {
	std::vector<Face> tris;
	std::vector<gmtl::Point3f> points;

	friend std::ostream& operator<<(std::ostream& os, const Model& m);

	float pos_x = 0, pos_y = 0, pos_z = 0;
	mutable bool pos_changed = true;
	gmtl::Matrix44f model_mat, rotate;
	mutable gmtl::Matrix44f total_model_mat;
	mutable bool model_changed = true;


public:

	Model() = default;
	Model(float pos_x, float pos_y) : pos_x(pos_x), pos_y(pos_y) {};

	void set_pos(float pos_x, float pos_y, float pos_z) {
		this->pos_x = pos_x;
		this->pos_y = pos_y;
		this->pos_z = pos_z;
		model_mat(0, 3) = pos_x;
		model_mat(1, 3) = pos_y;
		model_mat(2, 3) = pos_z;
		model_changed = true;
	}

	const gmtl::Matrix44f& get_model_matrix() const {
		if (model_changed) {
			total_model_mat = rotate * model_mat;
			model_changed = false;
		}
		return total_model_mat;
	}

	void set_rotation(float angle_a, float angle_b, float angle_c) {
		float cosa = std::cosf(angle_a);
		float sina = std::sinf(angle_a);
		float cosb = std::cosf(angle_b);
		float sinb = std::sinf(angle_b);
		float cosy = std::cosf(angle_c);
		float siny = std::sinf(angle_c);
		rotate.set(
			cosa * cosb, cosa * sinb * siny - sina * cosy, cosa * sinb * cosy + sina * siny, 0,
			sina * cosb, sina * sinb * siny + cosa * cosy, sina * sinb * cosy - cosa * siny, 0,
			-sinb, cosb * siny, cosb * cosy, 0,
			0, 0, 0, 1);

		model_changed = true;
	}

	void load_from_file(const std::string& filename) {
		std::ifstream file(filename);
		std::string line;

		points.reserve(50000);
		tris.reserve(100000);

		file.seekg(0, file.end);
		const int length = file.tellg();
		file.seekg(0, file.beg);
		file.clear();

		// If we are processing the beginning of file (where vertices usually are located), then reserve local_points.
		unsigned long line_counter = 0;
		std::array<std::string, 9> tokens;
		while (std::getline(file, line)) {
			if (line[0] == 'v') {
				std::size_t prev_pos = 2, new_pos = line.find_first_of(" ", prev_pos);
				float holding1 = std::stof(line.substr(prev_pos, new_pos));
				prev_pos = new_pos + 1;
				new_pos = line.find_first_of(" ", prev_pos);
				float holding2 = std::stof(line.substr(prev_pos, new_pos));
				prev_pos = new_pos + 1;
				new_pos = line.find_first_of(" ", prev_pos);
				float holding3 = std::stof(line.substr(prev_pos, new_pos));
				points.emplace_back(holding1, holding2, holding3);
			}
			else if (line[0] == 'f') {
				int tot_tokens = 0;

				std::size_t prev_pos = 2, new_pos = 2;
				while (new_pos = line.find_first_of("/ ", prev_pos), prev_pos != std::string::npos) {
					tokens[tot_tokens++] = line.substr(prev_pos, new_pos - prev_pos);
					if (new_pos == std::string::npos) {
						break;
					}
					prev_pos = new_pos + 1;
				}

				if (tot_tokens % 3 == 0) {
					tris.push_back(
						{ { (unsigned int)std::stoi(tokens[0]) - 1, (unsigned int)std::stoi(tokens[tot_tokens / 3]) - 1, (unsigned int)std::stoi(tokens[tot_tokens * 2 / 3]) - 1 } }
					);
				}
				else if (tot_tokens == 4) {
					tris.push_back(
						{ { (unsigned int)std::stoi(tokens[0]) - 1, (unsigned int)std::stoi(tokens[1]) - 1, (unsigned int)std::stoi(tokens[3]) - 1 } }
					);
					tris.push_back(
						{ { (unsigned int)std::stoi(tokens[1]) - 1, (unsigned int)std::stoi(tokens[2]) - 1, (unsigned int)std::stoi(tokens[3]) - 1 } }
					);
				}
			}


			if ((++line_counter) % 5000 == 0) {
				std::cout << "working. " << (long)file.tellg() * 100 / length << " percent\r";
			}

		}

		// Iterate through each triangle to calculate normals.
		for (int i = 0; i < tris.size(); i++) {
			auto tri = get_triangle(i);
			gmtl::Vec3f world_point1(tri(0, 0), tri(1, 0), tri(2, 0));
			gmtl::Vec3f world_point2(tri(0, 1), tri(1, 1), tri(2, 1));
			gmtl::Vec3f world_point3(tri(0, 2), tri(1, 2), tri(2, 2));

			auto tri_normal = gmtl::makeNormal(gmtl::makeCross(gmtl::Vec3f(world_point2 - world_point1), gmtl::Vec3f(world_point3 - world_point1)));



			tris[i].normal = tri_normal;
		}

	}
	gmtl::Matrix<float, 4, 3> get_triangle(int index) const {
		gmtl::Matrix<float, 4, 3> res;
		res(0, 0) = points[tris[index][0]][0];
		res(1, 0) = points[tris[index][0]][1];
		res(2, 0) = points[tris[index][0]][2];
		res(3, 0) = 1;
		res(0, 1) = points[tris[index][1]][0];
		res(1, 1) = points[tris[index][1]][1];
		res(2, 1) = points[tris[index][1]][2];
		res(3, 1) = 1;
		res(0, 2) = points[tris[index][2]][0];
		res(1, 2) = points[tris[index][2]][1];
		res(2, 2) = points[tris[index][2]][2];
		res(3, 2) = 1;
		return res;
	}

	gmtl::Vec3f get_normal(int index) const {
		return get_model_matrix() * tris[index].normal;
	}
	gmtl::Matrix<float, 4, 9> get_3_triangle(std::size_t index) const {
		gmtl::Matrix<float, 4, 9> res;
		auto* data = res.mData;
		//gmtl::Point3f* points = &points[0];
		//Face* tris = &tris[0];
		res.mState = gmtl::Matrix<float, 4, 9>::XformState::FULL;
		int tri_index0 = tris[index][0], tri_index1 = tris[index][1], tri_index2 = tris[index][2];
		data[0] = points[tri_index0][0];
		data[1] = points[tri_index0][1];
		data[2] = points[tri_index0][2];
		data[3] = 1;
		data[4] = points[tri_index1][0];
		data[5] = points[tri_index1][1];
		data[6] = points[tri_index1][2];
		data[7] = 1;
		data[8] = points[tri_index2][0];
		data[9] = points[tri_index2][1];
		data[10] = points[tri_index2][2];
		data[11] = 1;

		tri_index0 = tris[index + 1][0], tri_index1 = tris[index + 1][1], tri_index2 = tris[index + 1][2];
		data[12] = points[tri_index0][0];
		data[13] = points[tri_index0][1];
		data[14] = points[tri_index0][2];
		data[15] = 1;
		data[16] = points[tri_index1][0];
		data[17] = points[tri_index1][1];
		data[18] = points[tri_index1][2];
		data[19] = 1;
		data[20] = points[tri_index2][0];
		data[21] = points[tri_index2][1];
		data[22] = points[tri_index2][2];
		data[23] = 1;

		tri_index0 = tris[index + 2][0], tri_index1 = tris[index + 2][1], tri_index2 = tris[index + 2][2];
		data[24] = points[tri_index0][0];
		data[25] = points[tri_index0][1];
		data[26] = points[tri_index0][2];
		data[27] = 1;
		data[28] = points[tri_index1][0];
		data[29] = points[tri_index1][1];
		data[30] = points[tri_index1][2];
		data[31] = 1;
		data[32] = points[tri_index2][0];
		data[33] = points[tri_index2][1];
		data[34] = points[tri_index2][2];
		data[35] = 1;
		return res;
	}

	int total_triangles() const {
		return tris.size();
	}



};
