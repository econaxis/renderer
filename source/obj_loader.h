#pragma once

#include <fstream>
#include <gmtl/gmtl.h>
#include <vector>
#include <ostream>
#include <iostream>
#include <array>

typedef std::array<unsigned int, 3> Triangle;

class Model {
	std::vector<Triangle> tris;
	std::vector<gmtl::Point3f> points;

	friend std::ostream& operator<<(std::ostream& os, const Model& m);

public:
	void load_from_file(const std::string& filename) {
		std::fstream file(filename);
		std::string line, temp;
		double holding1, holding2, holding3;

		while (std::getline(file, line)) {
			std::stringstream iss(line);
			iss >> temp;
			iss >> holding1 >> holding2 >> holding3;
			if (temp == "v") {
				// Vertex
				points.emplace_back(holding1, holding2, holding3);
			}
			else if (temp == "f") {

				Triangle temp{ holding1-1, holding2-1, holding3-1 };
				tris.push_back(std::move(temp));
			}

		}
	}

	gmtl::Matrix<float, 4, 3> get_triangle(unsigned int index) const {
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

	std::size_t total_triangles() const {
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