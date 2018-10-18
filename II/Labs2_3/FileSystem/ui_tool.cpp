#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include "Filesystem.hpp"
int process() {
	Filesystem<> filesystem;
	std::string string;
	size_t size;

	std::map<size_t, OpenedFile<>> files;
	static size_t counter = 0;

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
		} else if (string == "open") {
			if (!stream) throw std::exception("Filename is expected.");
			stream >> string;
			files.insert(std::pair(counter++, filesystem.open(string)));
			std::cout << "File '" << files.at(counter - 1).name
				<< "' was opened with fd: " << counter - 1 << '\n';
		} else if (string == "close") {
			if (!stream) throw std::exception("fd is expected.");
			stream >> size;
			files.erase(size);
		} else if (string == "read") {
			Address offset, index;
			if (!stream) throw std::exception("fd is expected.");
			stream >> index;
			if (!stream) throw std::exception("offset is expected.");
			stream >> offset;
			if (!stream) throw std::exception("size is expected.");
			stream >> size;
			if (files.find(index) == files.end()) 
				throw std::exception("File with the fd is not open.");
			std::cout << "  Content:\n" <<
				filesystem.read(files.at(index), offset, Address(size));
		} else if (string == "write") {
			Address offset, index;
			if (!stream) throw std::exception("fd is expected.");
			stream >> index;
			if (!stream) throw std::exception("offset is expected.");
			stream >> offset;
			if (!stream) throw std::exception("size is expected.");
			stream >> size;
			if (files.find(index) == files.end())
				throw std::exception("File with the fd is not open.");
			filesystem.write(files.at(index), offset, Address(size));
		} else if (string == "link") {
			std::string file;
			if (!stream) throw std::exception("Link name is expected.");
			stream >> string;
			if (!stream) throw std::exception("Filename is expected.");
			stream >> file;
			filesystem.link(string, file);
		} else if (string == "unlink") {
			if (!stream) throw std::exception("Link name is expected.");
			stream >> string;
			filesystem.unlink(string);
		} else if (string == "symlink") {
			std::string file;
			if (!stream) throw std::exception("Link name is expected.");
			stream >> string;
			if (!stream) throw std::exception("Filename is expected.");
			stream >> file;
			filesystem.symlink(string, file);
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