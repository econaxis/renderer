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
typedef std::array<unsigned int, 3> Triangle;

class Model {
	std::vector<Triangle> tris;
	std::vector<gmtl::Point3f> points;
	std::vector<std::array<std::string, 9>> debug;

	friend std::ostream& operator<<(std::ostream& os, const Model& m);


public:
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
			}


			if ((++line_counter) % 5000 == 0) {
				std::cout << "working. " << (long)file.tellg() * 100 / length << " percent\r";
			}

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

	gmtl::Matrix<float, 4, 9> get_3_triangle(std::size_t index) const {
		gmtl::Matrix<float, 4, 9> res;
		auto* data = res.mData;
		//gmtl::Point3f* points = &points[0];
		//Triangle* tris = &tris[0];
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

	unsigned int total_triangles() const {
		return tris.size();
	}
};

std::ostream& operator<<(std::ostream& os, const Model& m) {
	os << "Vertices:\n";

	for (const gmtl::Point3f& p : m.points) {
		os << p[0] << " " << p[1] << " " << p[2] << "\n";
	}

	os << "Faces:\n";
	for (const Triangle& f : m.tris) {
		os << f.at(0) << " " << f.at(1) << " " << f.at(2) << "\n";

	}
	return os;
}