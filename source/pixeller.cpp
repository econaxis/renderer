#include <cmath>
#include <ostream>
#include "image.h"
#include <gmtl/Matrix.h>
#include <gmtl/MatrixOps.h>

#include <iomanip>

#include "sfml_header.h"

#include "obj_loader.h"

#include "utils.h"
#include "light.h"

#include "renderscene.h"
#include "camera.h"
#include <emscripten.h>
#include <cstdint>

using namespace std::chrono_literals;

RenderScene *main_scene = nullptr;
std::unique_ptr<Image> image;
Camera camera;
Model* model = new Model("/teddy.obj");
Light *light;
extern "C" int do_render();

int main(int argc, char *argv[]) {
    std::cout << "Script starting\n" << std::endl;

    std::size_t width = 1000, height = 800;
    std::string filename = "teddy.obj";

    if (argc == 4) {
        filename = argv[1];
        width = std::stoi(argv[2]);
        height = std::stoi(argv[3]);
    }
    image = std::make_unique<Image>(width, height);
    image->clear();

    gmtl::Matrix44f persp = create_perspective_transform_matrix();
    gmtl::Matrix44f screen = create_screen_matrix(image->width, image->height);

    gmtl::Matrix44f screen_persp = screen * persp;


    light = new Light(image->width, image->height);
    light->bake_light(*model);


    // Lighting constants. Changing it changes the specific properties of the object (e.g. rubber/plastic/metal/wood...)
    int specular_selectivity = 5;
    double k_reflectivity = 0.2;

    main_scene = new RenderScene{*model, *light, camera, *image, k_reflectivity, specular_selectivity, screen_persp};
    sf::Keyboard::setup_callbacks();

    return 0;
}

extern "C" {
EMSCRIPTEN_KEEPALIVE
std::uintptr_t image_data_loc() {
    return (std::uintptr_t) image->display.image_data.data();
}


EMSCRIPTEN_KEEPALIVE
int do_render() {
    main_scene->image.clear();
    gmtl::Matrix44f bogus{};
    poll_input(main_scene->image, bogus);
//    main_scene->screen_persp = screen * persp;

    main_scene->main_render_code();
    return 1;
}

EMSCRIPTEN_KEEPALIVE
void input() {
    main_scene->handle_input();
}


}