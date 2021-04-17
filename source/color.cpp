#include "color.h"
#include <cmath>

Color Color::operator + (const Color& c){
    return Color{r+c.r, g+c.g, b+c.b};
}

Color Color::operator * (float f){
    return Color{f*r, f*g, f*b};
}

Color square(Color c){
    return Color{c.r*c.r, c.g*c.g, c.b*c.b};
}

Color squareroot(Color c){
    return Color{sqrtf(c.r), sqrtf(c.g), sqrtf(c.b)};
}
