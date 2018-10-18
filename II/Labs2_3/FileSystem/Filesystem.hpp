#pragma once
#include <string>
#include <fstream>
#include <bitset>
#include <list>

#define write_f(f, v) f.write(reinterpret_cast<const char *>(&v), sizeof(v));

enum class Filetype {
	empty = 0,
	file = 1,
	dir = 2
};
inline uint8_t operator*(Filetype t) { return uint8_t(t); }

using Address = size_t;

struct File {
	Filetype type;
	size_t size;
	Address prev, next;
};

template <size_t block_size = 4096u, size_t block_number = 1024u>
class Filesystem {
	std::fstream m_storage;

	Address m_current_directory;
public:
	const Address m_root_directory = 0u;
protected:
	void state_check() {
		if (!m_storage)
			throw std::exception("Unable to access filesystem. Make sure it was mounted.");
	}
public:
	explicit Filesystem() : m_current_directory(m_root_directory) {
		static_assert(block_size > 16);
		static_assert(block_number > 2);
	}
	void initialize(std::string const& filename) {
		std::ofstream tf;
		tf.open(filename, std::fstream::binary);
		if (!tf)
			throw std::exception("Storage is inaccessible.");
		
		tf << *Filetype::dir; //root_dir type

		uint32_t bff = 0;
		write_f(tf, bff); //root_dir's '.'
		write_f(tf, bff); //root_dir's '..'

		// file discriptor
		tf << "Bitset\0"; //name
		uint32_t bitset_size = block_number / block_size / 8 + 1;
		write_f(tf, bitset_size); //size
		for (uint32_t i = 1; i <= bitset_size; i++)
			write_f(tf, i); //block_list

		char buffer = char(0xff);
		tf.seekp(block_size * 1);
		while (bitset_size >= 8u)
			tf.write(&buffer, 1);
		for (uint32_t i = 0; i < 8u - bitset_size - 1u; i++)
			buffer <<= 1;
		tf.write(&buffer, 1);
			   		
		tf.seekp(block_size * block_number - 1);
		tf.write("", 1);
	}
	void mount(std::string const& filename) {
		m_storage.open(filename.c_str(), std::fstream::in 
					   | std::fstream::out 
					   | std::fstream::binary);
		if (!m_storage)
			throw std::exception("Unable to access filesystem. Recheck the name.");
	}
	void unmount() {
		state_check();
		m_storage.close();
	}
	void filestat(size_t) {
		state_check();
		throw std::exception("Unimplemented feature");
	}
	void create(std::string const& name) {

	}
	void mkdir(std::string const& name) {

	}
};