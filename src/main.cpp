#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <ncurses.h>

#include <chrono>
#include <iostream>
#include <thread>

#include "Window.hpp"

int main() {
    tw::Window &twwindow = tw::Window::getInstance();

    try {
        color red = {128, 1, 1};
        color green = {1, 128, 1};
        color blue = {1, 1, 128};

        while (1) {
            red.r++;
            green.g++;
            blue.b++;
            twwindow.putPixel(0, 0, red);
            twwindow.putPixel(0, 1, green);
            twwindow.putPixel(1, 0, blue);
            twwindow.refresh();
        }
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }
}
