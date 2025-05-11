#pragma once

#include <sys/ioctl.h>

#include "Array2D.hpp"
#include "color.hpp"

namespace tw {

class Window {
   private:
    friend void windowResizeHandler(int);

    static void destructHandler(int);

    winsize size;
    Array2D<color> buffer;
    char *exit_alt_screen_str;

    Window();
    ~Window();

   public:
    static Window &getInstance();

    Window(const Window &other) = delete;
    Window &operator=(const Window &other) = delete;

    void    refresh();
    void    putPixel(size_t x, size_t y, color color);
};

}  // namespace tw