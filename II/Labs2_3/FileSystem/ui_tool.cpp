#include <iostream>
#include <sstream>
#include <string>
#include "Filesystem.hpp"
int process() {
	Filesystem<> filesystem;
	std::string string;
	std::istringstream stream;
	size_t size;

	///temp
	filesystem.initialize("f");
	return 0;

	std::cout << ": ";
	while (std::getline(std::cin, string)) {
		stream.str(string);
		stream >> string;
		if (string == "exit") {
			return 0;
		} else if (string == "initialize") {
			stream >> string;
			filesystem.initialize(string);
		} else if (string == "mount") {
			stream >> string;
			filesystem.mount(string);
		} else if (string == "unmount") {
			filesystem.unmount();
		} else if (string == "filestat") {
			stream >> size;
			filesystem.filestat(size);
		} else if (string == "create") {
			stream >> string;
			filesystem.create(string);
		} else if (string == "mkdir") {
			stream >> string;
			filesystem.create(string);
		} else
			std::cout << "Unknown command.\n";
		std::cout << ": ";
	}
	return -1;
}
int main() {
	do {
		try {
			return process();
		} catch (std::exception &e) {
			std::cout << e.what() << '\n';
		}
	} while (true);
}