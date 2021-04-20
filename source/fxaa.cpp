#include <cmath>
#include "image.h"

// hmm ideally obtain the line segment and its adjacent colors
// @henry u can do that, or i can if u want
// suppose we have the line segment with pt1, pt2, and colors c1, c2 on either side


// identifying high contrast edges

// Finding luminance of c1 and c2




double t1 = 6.0 / 29.0;
double t2 = 3.0 * t1 * t1;
double t3 = t1 * t1 * t1;

double rgb2lrgb (float x){
	return x <= 0.04045 ? x / 12.92 : pow((x + 0.055) / 1.055, 2.4);
}

double xyz2lab (double t){
	return t > t3 ? pow(t, 1.0 / 3.0) : t / t2 + 4.0 / 29.0;
}

// note that values in color c are in [0,1]
double get_luminance (Color c){
	double r = rgb2lrgb(c.r);
	double g = rgb2lrgb(c.g);
	double b = rgb2lrgb(c.b);
	double y = xyz2lab((0.2225045 * r + 0.7168786 * g + 0.0606169 * b) / 1.0);

	return 116.0*y-16.0; // a value in [0,100]
}

// determine if contrast is big enough
bool is_different (Color c1, Color c2){
    double contrast_threshold = 1.05; // decrease towards 1 to increase precision, generally keep it <= 1.1
    double ratio = (get_luminance(c1) + 5.0)/(get_luminance(c2) + 5.0);
    bool b = (ratio > contrast_threshold) || (1.0/ratio > contrast_threshold);
    return b;
}

// idk now run all edges thru isDifferent() and keep the ones that return true
// in the form of pt1, pt2, c1, c2

// maybe all of this can happen in line() or something


// stores the luminance of surrounding pixels and itself
struct LumaData {
	double m, n, e, s, w, ne, se, sw, nw;
	float highest, lowest, range;
};

// collects luminance of surroudning pixels
LumaData sample(const Image& im, int i){
    // Need to get the image for light shadow mapping data as well.
    LumaData l;
    l.m = get_luminance(im[i].get_color());

    l.n = get_luminance(im[i-im.width].get_color());
    l.s = get_luminance(im[i+im.width].get_color());
    l.e = get_luminance(im[i+1].get_color());
    l.w = get_luminance(im[i-1]].get_color());

    l.n = get_luminance(im[i-im.width + 1].get_color());
    l.n = get_luminance(im[i-im.width - 1].get_color());
    l.s = get_luminance(im[i+im.width + 1].get_color());
    l.s = get_luminance(im[i+im.width - 1].get_color());
    
    l.highest = std::max(std::max(std::max(std::max(l.m, l.n), l.e), l.s), l.w);
	l.lowest = std::min(std::min(std::min(std::min(l.m, l.n), l.e), l.s), l.w);
	l.range = l.highest - l.lowest;
    
    return l;
}

// talks about the edge a point/pixel is on
struct Edge {
    bool is_horizontal;
    double pixel_factor;
};

Edge get_edge(LumaData l){
    Edge ed;
    ed.is_horizontal = get_direction(l);

    // finds pixel_factor
    double l_pos, l_pos;
    if(ed.is_horizontal) {
        ed.pixel_factor = get_pf();//.y; //TODO: GET PF
        l_pos = l.s;
        l_neg = l.n;
    } else {
        ed.pixel_factor = get_pf();//.x; //TODO: GET PF
        l_pos = l.e;
        l_neg = l.w;
    }

    double delta_pos = abs(l_pos - l.m);
    double delta_neg = abs(l_neg - l.m);

    if(delta_pos < delta_neg) {
        ed.pixel_factor = -1.0*ed.pixel_factor; // might ditch this depending on structure of points
    }

    return ed;
}


// ???
double get_pf() {return 1.0}


// ================================================
// finds how much to blend
// currently experimental
double blend_factor(LumaData ld){
    double filter = 2.0 * (ld.n + ld.e + ld.s + ld.w) + ld.ne + ld.nw + ld.se + ld.sw;
    filter *= 1.0/12.0;
    filter = saturate(filter / l.range);
    filter = smoothstep (0,1,filter);

    return filter*filter;
}

double saturate(double x){
    if(x <= 0) return 0.0;
    else if (x >= 1) return 1.0;
    else return x;
}

double smoothstep(double a, double b, double x){
    double t = saturate((x - a)/(b - a));
    return t*t*(3.0 - (2.0*t));
}

// ================================================
// finds blend direction
bool get_direction (LumaData l) {
    float horizontal =
            2.0 * abs(l.n + l.s - 2.0 * l.m) +
            abs(l.ne + l.se - 2.0 * l.e) +
            abs(l.nw + l.sw - 2.0 * l.w);
    float vertical =
            2.0 * abs(l.e + l.w - 2.0 * l.m) +
            abs(l.ne + l.nw - 2.0 * l.n) +
            abs(l.se + l.sw - 2.0 * l.s);
    return horizontal >= vertical;
}

// ================================================
// finds color of partial pixel
// are points always int coordinates
Color get_blended_color(double x, double y){
    int x0 = ceil(x);
    int y0 = ceil(y);
    float x1 = x0 - x;
    float y1 = y0 - y;
    float x2 = 1.0-x1;
    float y2 = 1.0-y1;
    // opacity - separate

    // will "average" colors by using proportion area as weight
    // algo: taking the squareroot of the average of squares
    Color val_nw, val_ne, val_sw, val_se;

    // multiplying squared colors by weight
    val_nw = square(Image.at(x0 - 1, y0 - 1).get_color())*(x1*y1);
    val_ne = square(Image.at(x0 + 1, y0 - 1).get_color())*(x2*y1);
    val_sw = square(Image.at(x0 - 1, y0 + 1).get_color())*(x1*y2);
    val_se = square(Image.at(x0 + 1, y0 + 1).get_color())*(x2*y2);

    // average time
    return squareroot(val_nw + val_ne + val_sw + val_se);
}


// Let the set of these edges be S, with each edge containing the info pt1, pt2, c1, c2

// FOR EACH object in S:
//   -  points = line_points(pt1, pt2) or something to obtain all the points we need to use
//   -  NOW IS THE EXPERIMENTAL PART I CANT EVEN TEST THIS SO LMK HOW IT GOES
//      because i'm not sure if the luminance we used here is same as the one in FXAA
//      but uh we can adjust as we go?


auto fxaa_line(Point pt1, Point pt2, Color c1, Color c2) {
    auto points = line_points(pt1, pt2); // add image to act upon
    // yo do these guys have a color, if not i'll shoot them a default color???
    // let c1 be the "left" color and c2 be the "right" color (or bottom/top, if horizontal)
    for (Point p: points){
    //    p.set_color(c1 or c2 idek); // needed if no predetermined color
        LumaData ld = sample(p);
        double b_factor = blend_factor(ld);
        Edge e = get_edge(ld);

        // blendddddddddddddddd
        // because everything at the moment is monochromatic
        // I need to do a little more math to figure what to do here. Let's save that till tmrw
        
        // it is now tomorrow
        //this chunk should assign a color to p
        Point p0 = p; // am i supposed to have a pointer thingy here
        if (e.is_horizontal) {
		    p0.y += b_factor * e.pixel_factor; //offsets vertically
	    }
	    else {
		    p0.x += b_factor * e.pixel_factor; //offsets horizontally
	    }

        // gets the color of the offset point and assigns it to the original point
        // HOLD UP POINT IS 3D?!?!?!?!?!?!?!?!?!?!
        p.set_color(get_blended_color(p0););

        // i am absolutely not sure whether this syntax is correct
    }

    return points; //this will be the vector/array/list of p's with their corresponding colors
}

//  END FOR

bool is_contrasting (double l1, double l1){
    double contrast_threshold = 1.05; // decrease towards 1 to increase precision, generally keep it <= 1.1
    double ratio = (l1 + 5.0)/(l2 + 5.0);
    bool b = (ratio > contrast_threshold) || (1.0/ratio > contrast_threshold);
    return b;
}

void fxaa(Image im) {
    std::vector<Pixel> pixels = im.get_pixels();
    Pixel p;
    double x = i % im.width;
    double y = floor(i / width);
    for(int i = 0; i < pixels.size(); i++){
        p = pixels.at(i);
        double pLum = get_direction(p.get_color());
        LumaData ld = sample(im, i);
        if (is_contrasting(ld.highest, ld.lowest)){
            double b_factor = blend_factor(ld);

            Edge e = get_edge(ld);
            if (e.is_horizontal) {
                y += b_factor * e.pixel_factor; //offsets vertically
            }
            else {
                x += b_factor * e.pixel_factor; //offsets horizontally
            }

            p.set_color(get_blended_color(x, y););
        }
    }
}


