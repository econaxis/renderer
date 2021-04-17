#pragma once

#include <algorithm>

struct Color {
    float r = 0.0, g = 0.0, b = 0.0, opacity = 1.0F;

    static Color clamp(float r, float g, float b) {
        r = std::clamp(r, 0.F, 1.F);
        g = std::clamp(g, 0.F, 1.F);
        b = std::clamp(b, 0.F, 1.F);
        return Color{ r, g, b };
    }


    Color operator + (const Color&);
    Color operator * (float);

};

