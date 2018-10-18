#include <iostream>
#include <sstream>
#include <string>
#include "Filesystem.hpp"
int process() {
	Filesystem<> filesystem;
	std::string string;
	size_t size;

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
			if (!stream) throw std::exception("File name is expected.");
			stream >> string;
			filesystem.filestat(string, std::cout);
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
		} else if (string == "ls") {
			filesystem.ls(std::cout);
		} else if (string == "rmdir") {
			if (!stream) throw std::exception("Folder name is expected.");
			stream >> string;
			filesystem.rmdir(string);
		} else if (string == "truncate") {
			if (!stream) throw std::exception("File name is expected.");
			stream >> string;
			if (!stream) throw std::exception("New file size is expected.");
			stream >> size;
			filesystem.truncate(string, size);
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