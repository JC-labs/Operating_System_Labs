#pragma once
#include <string>
#include <fstream>
template <size_t block_size = 4096u, size_t block_number = 1024u>
class Filesystem {
	std::fstream m_storage;
public:
	void mount(std::string const& filename) {
		m_storage.open(filename.c_str(), std::fstream::in 
					   | std::fstream::out 
					   | std::fstream::binary);
		if (!m_storage)
			throw std::exception("Unable to access filesystem. Recheck the name.");
	}
	void unmount() {
		m_storage.close();
	}
};