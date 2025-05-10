#pragma once

#include <sys/ioctl.h>

#include "Array2D.hpp"

namespace tw {

class Window {
   private:
    friend void windowResizeHandler(int);

    static void destructHandler(int);

    winsize size;
    Array2D<wchar_t> buffer;
    char *exit_alt_screen_str;

    Window();
    ~Window();

   public:
    static Window &getInstance();

    Window(const Window &other) = delete;
    Window &operator=(const Window &other) = delete;
};

}  // namespace tw