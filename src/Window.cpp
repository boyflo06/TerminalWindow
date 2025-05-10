#include "Window.hpp"

#include <curses.h>
#include <signal.h>
#include <term.h>
#include <unistd.h>

#include <iostream>
#include <stdexcept>

namespace tw {

void windowResizeHandler(int) {
    Window& instance = Window::getInstance();
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &instance.size) != 0) throw std::runtime_error("Couldn't fetch terminal resize");
}

void Window::destructHandler(int) {
    Window& instance = Window::getInstance();
    signal(SIGWINCH, SIG_DFL);
    std::cout << "\x1b[?25h";
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
    std::cout << "\x1b[0;0H";
    std::cout << "\x1b[?25l";

    this->buffer.resize(this->size.ws_col, this->size.ws_row * 2);
}

Window::~Window() {
    destructHandler(0);
}

Window& Window::getInstance() {
    static Window singleton;
    return singleton;
}
}  // namespace tw