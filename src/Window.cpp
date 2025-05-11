#include "Window.hpp"

#include <curses.h>
#include <signal.h>
#include <term.h>
#include <unistd.h>

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <sstream>

namespace tw {

void windowResizeHandler(int) {
    Window& instance = Window::getInstance();
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &instance.size) != 0) throw std::runtime_error("Couldn't fetch terminal resize");
}

void Window::destructHandler(int) {
    Window& instance = Window::getInstance();
    signal(SIGWINCH, SIG_DFL);

    std::cout << "\x1b[?25h";  // Show cursor

    std::cout << "\x1b[?1003;1006l";  // Disable mouse interaction reporting

    std::cout.flush();

    putp(instance.exit_alt_screen_str);
    exit(0);
}

Window::Window() {
    std::cout << "Starting this shiiii" << std::endl;

    if (setupterm(nullptr, STDOUT_FILENO, nullptr) != OK) throw std::runtime_error("Failed to initialize terminal system");

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &this->size) != 0) throw std::runtime_error("Couldn't fetch terminal size");
    signal(SIGWINCH, windowResizeHandler);
    signal(SIGINT, destructHandler);

    const char* enter_alt_screen = tigetstr("smcup");
    this->exit_alt_screen_str = tigetstr("rmcup");

    if (!enter_alt_screen || !this->exit_alt_screen_str) throw std::runtime_error("Couldn't fetch secondary screen, maybe not available for this terminal");
    putp(enter_alt_screen);

    std::cout << "\x1b[2J";
    std::cout << "\x1b[0;0H";  // Clear screen and set cursor to start

    std::cout << "\x1b[?25l";  // Hide cursor

    std::cout << "\x1b[?1003;1006h";  // Enable mouse interaction reporting

    std::cout.flush();

    this->buffer.resize(this->size.ws_col, this->size.ws_row * 2);
}

Window::~Window() {
    destructHandler(0);
}

Window& Window::getInstance() {
    static Window singleton;
    return singleton;
}

void Window::refresh() {
    std::cout << "\x1b[2J";
    std::cout << "\x1b[0;0H";
    vec2i size = this->buffer.size();
    for (int y = 1; y < size.y; y += 2) {
        for (int x = 0; x < size.x; x++) {
            color fgColor = this->buffer.at(x, y - 1);
            color bgColor = this->buffer.at(x, y);

            // std::string fg = std::string("\x1b[38;2;") + std::to_string(fgColor.r) + ";" + std::to_string(fgColor.g) + ";" + std::to_string(fgColor.b) + "m";
            // std::string bg = std::string("\x1b[48;2;") + std::to_string(bgColor.r) + ";" + std::to_string(bgColor.g) + ";" + std::to_string(bgColor.b) + "m";

            std::ostringstream oss;
            oss << "\x1b[38;2;" << int(fgColor.r) << ";" << int(fgColor.g) << ";" << int(fgColor.b) << "m"
                << "\x1b[48;2;" << int(bgColor.r) << ";" << int(bgColor.g) << ";" << int(bgColor.b) << "m"
                << "▀";
            std::cout << oss.str();
        }
    }
    std::cout.flush();
}

void Window::putPixel(size_t x, size_t y, color color) {
    this->buffer.at(x, y) = color;
}

}  // namespace tw