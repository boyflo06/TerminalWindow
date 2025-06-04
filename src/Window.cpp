#include "Window.hpp"

#include <curses.h>
#include <signal.h>
#include <term.h>
#include <unistd.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <fstream>

namespace tw {

void windowResizeHandler(int) {
    Window& instance = Window::getInstance();
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &instance.size) != 0) throw std::runtime_error("Couldn't fetch terminal resize");
}

void Window::destructHandler(int) {
    Window& instance = Window::getInstance();
    signal(SIGWINCH, SIG_DFL);

	tcsetattr(STDIN_FILENO, TCSANOW, &instance.orig_termios);

    std::cout << "\x1b[?25h";  // Show cursor

    std::cout << "\x1b[?1003;1006l";  // Disable mouse interaction reporting

	std::cout << "\x1b[?1004l";

    std::cout.flush();

    putp(instance.exit_alt_screen_str);
    exit(0);
}

std::ifstream getStatStream(pid_t pid) {
	std::string statPath = "/proc/" + std::to_string(pid) + "/stat";

	if (access(statPath.c_str(), R_OK) != 0) throw std::runtime_error("proc stat file not found for pid or not readable " + std::to_string(pid));

	std::ifstream file = std::ifstream(statPath);

	if (file.fail()) throw std::runtime_error("couldn't open proc stat file " + std::to_string(pid));

	return file;
}

pid_t getTermPid(std::ifstream &file) {
	std::string trashbin, termPidStr;

	file >> trashbin;
	file >> trashbin;
	file >> trashbin;
	file >> termPidStr;

	return std::stoi(termPidStr);
}

static ::Window getXWindow(Display *display) {
	pid_t sh_pid = getppid();
	std::ifstream sh_stat = getStatStream(sh_pid);
    pid_t terminal_pid = getTermPid(sh_stat);
    
    // Try the WINDOWID environment variable first (most reliable)
    /* const char* windowid_env = getenv("WINDOWID");
    if (windowid_env) {
        std::cout << "Found WINDOWID environment variable: " << windowid_env << std::endl;
    } else {
        std::cout << "WINDOWID environment variable not set, searching windows..." << std::endl;
    } */
    
    // Get _NET_CLIENT_LIST property from root window
    Atom prop = XInternAtom(display, "_NET_CLIENT_LIST", False);
    Atom actual_type;
    int actual_format;
    unsigned long num_items, bytes_after;
    unsigned char* data = NULL;
    
    XGetWindowProperty(
        display, DefaultRootWindow(display), prop, 0, ~0L, 
        False, XA_WINDOW, &actual_type, &actual_format, 
        &num_items, &bytes_after, &data
    );
    
    
    if (data) {
        ::Window* windows = (::Window*)data;
        Atom pid_atom = XInternAtom(display, "_NET_WM_PID", False);
        
        for (unsigned long i = 0; i < num_items; i++) {
            Atom actual_type_pid;
            int actual_format_pid;
            unsigned long num_items_pid, bytes_after_pid;
            unsigned char* pid_data = NULL;
            
            ::Window window = windows[i];
            
            XGetWindowProperty(
                display, window, pid_atom, 0, 1, False, XA_CARDINAL,
                &actual_type_pid, &actual_format_pid, &num_items_pid,
                &bytes_after_pid, &pid_data
            );
            
            if (pid_data) {
                pid_t window_pid = *(pid_t*)pid_data;
                
				if (window_pid == terminal_pid) {
					XFree(data);
					return window;
				}
                
                XFree(pid_data);
            }
        }
        
        XFree(data);
    }
	return 0;
}

Window::Window() {
	//X11 Setup

	this->xDisplay = XOpenDisplay(NULL);
	if (!this->xDisplay) throw std::runtime_error("Couldn't open X11 display (connection failed)");

	this->xWindow = getXWindow(this->xDisplay);
	if (!this->xWindow) throw std::runtime_error("Couldn't fetch terminal window. Please run this in a standalone terminal!");
	std::cout << "window id: " << this->xWindow << std::endl;

	::Window root = DefaultRootWindow(xDisplay);

	XGrabKey(this->xDisplay, AnyKey, 0, root, True, GrabModeAsync, GrabModeAsync);

	XSelectInput(this->xDisplay, root, KeyPressMask | KeyReleaseMask /* |    // Keyboard events
                 ButtonPressMask | ButtonReleaseMask | // Mouse button events
                 PointerMotionMask */);
    
    XEvent event;
    KeySym keysym;
    char buffer[10];
    while (true) {
        if (XPending(this->xDisplay)) {
            if (XPending(this->xDisplay)) {
            XNextEvent(this->xDisplay, &event);
            
            if (event.type == KeyPress || event.type == KeyRelease) {
                // Convert keycode to keysym
                keysym = XLookupKeysym(&event.xkey, 0);
                
                // Get key string representation
                XLookupString(&event.xkey, buffer, sizeof(buffer), NULL, NULL);
                
                // Print event details
                std::cout << (event.type == KeyPress ? "Key Press: " : "Key Release: ");
                if (event.xkey.state & ShiftMask) std::cout << "Shift+";
                if (event.xkey.state & ControlMask) std::cout << "Ctrl+";
                if (event.xkey.state & Mod1Mask) std::cout << "Alt+";
                if (event.xkey.state & Mod4Mask) std::cout << "Super+";
                std::cout << XKeysymToString(keysym) << " (KeyCode: " << event.xkey.keycode << ")" 
                          << std::endl;
                
                // Detect Ctrl+C specifically to demonstrate handling special keys
                if (event.type == KeyPress && keysym == XK_c && 
                    (event.xkey.state & ControlMask) && !(event.xkey.state & ~ControlMask)) {
                    std::cout << "Ctrl+C detected - you can also exit with this" << std::endl;
                }
            }
        } else {
            // No events, sleep briefly to avoid high CPU usage
            usleep(10000);  // 10ms
        }
        }
    }
	//Term Setup

	termios raw;
	tcgetattr(STDIN_FILENO, &this->orig_termios);

	raw = this->orig_termios;
	raw.c_lflag &= ~(ECHO | ICANON);
	tcsetattr(STDIN_FILENO, TCSANOW, &raw);

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

	std::cout << "\x1b[?1004h";
	this->focused = true;

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