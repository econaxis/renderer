#pragma once
/*
 * TODO: write a custom SFML wrapper. When we port this to other platforms (e.g. Javascript),
 * we use different API's for handling system events, so we don't want to completely rewrite parts of code that call SFML directly scattered throughout.
 */


#ifdef HAS_SFML
#include <SFML/Graphics.hpp>

enum Keys {

};

bool key_pressed()
#endif
