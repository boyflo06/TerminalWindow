#include <iostream>

int main() {
	std::cout << "\x1b[?1004h";
	std::cout.flush();
	while (true) {

	}
}