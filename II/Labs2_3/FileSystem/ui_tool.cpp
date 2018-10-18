#include <iostream>
#include <sstream>
#include <string>
#include "Filesystem.hpp"
int process() {
	Filesystem<> filesystem;
	std::string string;

	std::cout << ": ";
	while (std::getline(std::cin, string)) {
		std::istringstream stream(string);
		stream >> string;
		if (string == "exit") {
			return 0;
		} else if (string == "initialize") {
			if (!stream) throw std::exception("File name is expected.");
			stream >> string;
			filesystem.initialize(string);
		} else if (string == "mount") {
			if (!stream) throw std::exception("File name is expected.");
			stream >> string;
			filesystem.mount(string);
		} else if (string == "unmount") {
			filesystem.unmount();
		} else if (string == "filestat") {
			//if (!stream) throw std::exception("File name is expected.");
			//stream >> size;
			//filesystem.filestat(size);
		} else if (string == "create") {
			if (!stream) throw std::exception("File name is expected.");
			stream >> string;
			filesystem.create(string);
		} else if (string == "mkdir") {
			if (!stream) throw std::exception("Folder name is expected.");
			stream >> string;
			filesystem.mkdir(string);
		} else if (string == "pwd") {
			std::cout << filesystem.pwd() << '\n';
		} else if (string == "cd") {
			if (!stream) throw std::exception("Folder path is expected.");
			std::getline(stream, string);
			filesystem.cd(string.substr(1));
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