#include "image.h"

std::ostream& operator<<(std::ostream& os, std::pair<int, int> p)
{
	os << p.first << " " << p.second;
	return os;
}