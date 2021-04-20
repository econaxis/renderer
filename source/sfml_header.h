#pragma once

#include "utils.h"

/*
 * TODO: write a custom SFML wrapper. When we port this to other platforms (e.g. Javascript),
 * we use different API's for handling system events, so we don't want to completely rewrite parts of code that call SFML directly scattered throughout.
 */


#ifndef HAS_SFML

#include <emscripten/html5.h>
#include <cstring>
#include <iostream>
#include <emscripten.h>

namespace sf::Keyboard {
    enum Key {
        Unknown = -1, //!< Unhandled key
        A = 0,        //!< The A key
        B,            //!< The B key
        C,            //!< The C key
        D,            //!< The D key
        E,            //!< The E key
        F,            //!< The F key
        G,            //!< The G key
        H,            //!< The H key
        I,            //!< The I key
        J,            //!< The J key
        K,            //!< The K key
        L,            //!< The L key
        M,            //!< The M key
        N,            //!< The N key
        O,            //!< The O key
        P,            //!< The P key
        Q,            //!< The Q key
        R,            //!< The R key
        S,            //!< The S key
        T,            //!< The T key
        U,            //!< The U key
        V,            //!< The V key
        W,            //!< The W key
        X,            //!< The X key
        Y,            //!< The Y key
        Z,            //!< The Z key
        Num0,         //!< The 0 key
        Num1,         //!< The 1 key
        Num2,         //!< The 2 key
        Num3,         //!< The 3 key
        Num4,         //!< The 4 key
        Num5,         //!< The 5 key
        Num6,         //!< The 6 key
        Num7,         //!< The 7 key
        Num8,         //!< The 8 key
        Num9,         //!< The 9 key
        Escape,       //!< The Escape key
        LControl,     //!< The left Control key
        LShift,       //!< The left Shift key
        LAlt,         //!< The left Alt key
        LSystem,      //!< The left OS specific key: window (Windows and Linux), apple (MacOS X), ...
        RControl,     //!< The right Control key
        RShift,       //!< The right Shift key
        RAlt,         //!< The right Alt key
        RSystem,      //!< The right OS specific key: window (Windows and Linux), apple (MacOS X), ...
        Menu,         //!< The Menu key
        LBracket,     //!< The [ key
        RBracket,     //!< The ] key
        Semicolon,    //!< The ; key
        Comma,        //!< The , key
        Period,       //!< The . key
        Quote,        //!< The ' key
        Slash,        //!< The / key
        Backslash,    //!< The \ key
        Tilde,        //!< The ~ key
        Equal,        //!< The = key
        Hyphen,       //!< The - key (hyphen)
        Space,        //!< The Space key
        Enter,        //!< The Enter/Return keys
        Backspace,    //!< The Backspace key
        Tab,          //!< The Tabulation key
        PageUp,       //!< The Page up key
        PageDown,     //!< The Page down key
        End,          //!< The End key
        Home,         //!< The Home key
        Insert,       //!< The Insert key
        Delete,       //!< The Delete key
        Add,          //!< The + key
        Subtract,     //!< The - key (minus, usually from numpad)
        Multiply,     //!< The * key
        Divide,       //!< The / key
        Left,         //!< Left arrow
        Right,        //!< Right arrow
        Up,           //!< Up arrow
        Down,         //!< Down arrow
        Numpad0,      //!< The numpad 0 key
        Numpad1,      //!< The numpad 1 key
        Numpad2,      //!< The numpad 2 key
        Numpad3,      //!< The numpad 3 key
        Numpad4,      //!< The numpad 4 key
        Numpad5,      //!< The numpad 5 key
        Numpad6,      //!< The numpad 6 key
        Numpad7,      //!< The numpad 7 key
        Numpad8,      //!< The numpad 8 key
        Numpad9,      //!< The numpad 9 key
        F1,           //!< The F1 key
        F2,           //!< The F2 key
        F3,           //!< The F3 key
        F4,           //!< The F4 key
        F5,           //!< The F5 key
        F6,           //!< The F6 key
        F7,           //!< The F7 key
        F8,           //!< The F8 key
        F9,           //!< The F9 key
        F10,          //!< The F10 key
        F11,          //!< The F11 key
        F12,          //!< The F12 key
        F13,          //!< The F13 key
        F14,          //!< The F14 key
        F15,          //!< The F15 key
        Pause,        //!< The Pause key

        KeyCount,     //!< Keep last -- the total number of keyboard keys

        // Deprecated values:

        Dash = Hyphen,       //!< \deprecated Use Hyphen instead
        BackSpace = Backspace,    //!< \deprecated Use Backspace instead
        BackSlash = Backslash,    //!< \deprecated Use Backslash instead
        SemiColon = Semicolon,    //!< \deprecated Use Semicolon instead
        Return = Enter         //!< \deprecated Use Enter instead
    };

    inline bool pressed[KeyCount] = {0};

    inline bool isKeyPressed(Key key) {
        return pressed[key];
    }

    inline EM_BOOL key_callback(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData) {
        std::cout<<"key: "<<keyEvent->key<<std::endl;
        if (eventType == EMSCRIPTEN_EVENT_KEYDOWN) {
            if (!strcmp(keyEvent->key, "a")) pressed[A] = true;
            if (!strcmp(keyEvent->key, "s")) pressed[S] = true;
            if (!strcmp(keyEvent->key, "d")) pressed[D] = true;
            if (!strcmp(keyEvent->key, "w")) pressed[W] = true;
            if (!strcmp(keyEvent->key, "ArrowLeft")) pressed[Left] = true;
            if (!strcmp(keyEvent->key, "ArrowRight")) pressed[Right] = true;
            if (!strcmp(keyEvent->key, "ArrowDown")) pressed[Down] = true;
            if (!strcmp(keyEvent->key, "ArrowUp")) pressed[Up] = true;
            if (!strcmp(keyEvent->key, "ArrowUp")) pressed[Up] = true;
            if (!strcmp(keyEvent->key, " ")) pressed[Space] = true;
            if (!strcmp(keyEvent->key, "q")) pressed[Q] = true;
        } else if (eventType == EMSCRIPTEN_EVENT_KEYUP) {
            if (!strcmp(keyEvent->key, "a")) pressed[A] = false;
            if (!strcmp(keyEvent->key, "s")) pressed[S] = false;
            if (!strcmp(keyEvent->key, "d")) pressed[D] = false;
            if (!strcmp(keyEvent->key, "w")) pressed[W] = false;
            if (!strcmp(keyEvent->key, "ArrowLeft")) pressed[Left] = false;
            if (!strcmp(keyEvent->key, "ArrowRight")) pressed[Right] = false;
            if (!strcmp(keyEvent->key, "ArrowDown")) pressed[Down] = false;
            if (!strcmp(keyEvent->key, "ArrowUp")) pressed[Up] = false;
            if (!strcmp(keyEvent->key, " ")) pressed[Space] = false;
            if (!strcmp(keyEvent->key, "q")) pressed[Q] = false;
        }
        return false;
    };

    inline bool currently_dragging = false;
    inline long movementX = 0, movementY = 0;

    inline EM_BOOL mouse_callback(int eventType, const EmscriptenMouseEvent *event, void *userData) {
        if (eventType == EMSCRIPTEN_EVENT_MOUSEDOWN) {
            currently_dragging = true;
        } else if (eventType == EMSCRIPTEN_EVENT_MOUSEUP) {
            currently_dragging = false;
        }

        if(currently_dragging) {
            movementX = event->movementX;
            movementY = event->movementY;
//            std::cout << "Mouse movement: " << movementX << " " << movementY << std::endl;
        }
        return false;
    };

    inline bool Dragged() {
        return sf::Keyboard::currently_dragging;
    }

}

inline void setup_callbacks() {
    std::cout<<"registering callbacks\n";
    emscripten_set_keyup_callback("canvas", nullptr, false, sf::Keyboard::key_callback);
    emscripten_set_keydown_callback("canvas", nullptr, false, sf::Keyboard::key_callback);


    emscripten_set_mousedown_callback("canvas", nullptr, false, sf::Keyboard::mouse_callback);
    emscripten_set_mouseup_callback("canvas", nullptr, false, sf::Keyboard::mouse_callback);
    emscripten_set_mousemove_callback("canvas", nullptr, false, sf::Keyboard::mouse_callback);
}

inline void poll_input(Image &image, gmtl::Matrix44f &screen) {
    // do nothing.
}

#endif


#ifdef HAS_SFML
#include <SFML/Graphics.hpp>

inline int window_width = 1000, window_height = 800;

inline void setup_callbacks() {

}

inline sf::RenderWindow& get_window() {
    static sf::RenderWindow window (sf::VideoMode(window_width, window_height), "My window");
    return window;
};
inline void poll_input(Image& image, gmtl::Matrix44f& screen) {
    // TODO: refactor screen matrix to be part of the image class.
    // check all the window's events that were triggered since the last iteration of the loop
    sf::Event event;
    sf::RenderWindow& window = get_window();
    while (window.pollEvent(event))
    {
        // "close requested" event: we close the window
        if (event.type == sf::Event::Closed)
        {
            window.close();
        }
        else if (event.type == sf::Event::KeyPressed)
        {
//            if (event.key.code == sf::Keyboard::Num1 && image.width < 2000)
//            {
//                // Increase the size of the image. This makes the picture clearer.
//                // We limit width to around 2000 pixels because at that level too much memoy is used.
//
//                // Clears the current image data and resizes it to specific width and height.
//                image.resize(image.width * 1.2, image.height * 1.2);
//
//                // Since we changed the width/height, the screen matrix (mapping normalized device coordinates to
//                // pixels need to be changed as well.
//                screen = create_screen_matrix(image.width, image.height);
//            }
//            else if (event.key.code == sf::Keyboard::Tilde && image.height > 10 && image.width > 10)
//            {
//                // Decrease size of the image
//                image.resize(image.width / 1.2, image.height / 1.2);
//                screen = create_screen_matrix(image.width, image.height);
//            }
        }
    }
}


#endif
