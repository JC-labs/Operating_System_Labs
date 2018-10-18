#pragma once
#include <string>
#include <fstream>
#include <bitset>
#include <vector>

#define write_f(f, v) f.write(reinterpret_cast<const char *>(&v), sizeof(v));
#define read_f(f, v) f.read(reinterpret_cast<char *>(&v), sizeof(v));

enum class Filetype {
	empty = 0,
	file = 1,
	dir = 2
};
inline uint8_t operator*(Filetype t) { return uint8_t(t); }

using Address = uint32_t;

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
	Address claim_free_block() {
		state_check();

		m_storage.seekg(4 + 4 + 1);
		char buffer[7];
		m_storage.read(buffer, 7);
		if (std::string(buffer) != "Bitset")
			throw std::exception("'/Bitset' file is corrupted. Unable to restore filesystem.");

		Address size;
		read_f(m_storage, size);
		std::vector<Address> addrs(size);
		for (Address i = 0; i < size; i++)
			read_f(m_storage, addrs.at(i));

		char b;
		for (Address c = 0; c < addrs.size(); c++) {
			m_storage.seekg(addrs.at(c) * block_size);
			for (Address i = 0; i < block_size; i++) {
				b = m_storage.peek();
				if (b != 0xff)
					for (char j = 0; j < 8; j++)
						if (auto t = 1 << j; ~b & t) {
							b |= t;
							m_storage.seekp(m_storage.tellg());
							m_storage.put(b);
							return c * 8 + j;
						}
			}
		}
		throw std::exception("All of the blocks are occupied. It's impossible to allocate memory.");
	}
public:
	explicit Filesystem() : m_current_directory(m_root_directory) {
		static_assert(block_size > 16);
		static_assert(block_number > 2);
		m_storage.setstate(std::fstream::failbit);
	}
	void initialize(std::string const& filename) {
		std::ofstream tf;
		tf.open(filename, std::fstream::binary);
		if (!tf)
			throw std::exception("Storage is inaccessible.");
		
		//tf << *Filetype::dir; //root_dir type

		Address bff = 0;
		write_f(tf, bff); //root_dir's '..'
		write_f(tf, bff); //root_dir's '.'

		// file discriptor
		tf << *Filetype::file << "Bitset" << '\0'; //name
		Address bitset_size = block_number / block_size / 8 + 1;
		write_f(tf, bitset_size); //size
		for (Address i = 1; i <= bitset_size; i++)
			write_f(tf, i); //block_list

		char buffer = char(0xff);
		tf.seekp(block_size * 1);
		while (bitset_size >= 8u)
			tf.write(&buffer, 1);
		for (buffer = 1; bitset_size; bitset_size--, buffer |= buffer << 1);
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
		state_check();

		m_storage.seekg(m_current_directory * block_size + 8);
		while (m_storage.get()) {
			while (m_storage.get() != '\0');
			Address size, temp;
			read_f(m_storage, size);
			while (size--)
				read_f(m_storage, temp);
		}

		auto pos = Address(m_storage.tellg());
		auto addr = claim_free_block();
		Address size = 1;

		m_storage.seekp(pos);
		m_storage.seekp(-1, std::fstream::cur);
		m_storage << *Filetype::file << name << '\0';
		write_f(m_storage, size);
		write_f(m_storage, addr);
	}
	void mkdir(std::string const& name) {
		state_check();
		
		m_storage.seekg(m_current_directory * block_size + 8);
		while (m_storage.get()) {
			while (m_storage.get() != '\0');
			Address size, temp;
			read_f(m_storage, size);
			while (size--)
				read_f(m_storage, temp);
		}

		auto pos = Address(m_storage.tellg());
		auto addr = claim_free_block();
		m_storage.seekp(addr * block_size);
		//m_storage << *Filetype::dir; //dir type
		write_f(m_storage, pos); //dir's '..'
		write_f(m_storage, addr); //dir's '.'
		m_storage << '\0';

		m_storage.seekp(pos);
		m_storage.seekp(-1, std::fstream::cur);
		m_storage << *Filetype::dir << name << '\0';
		write_f(m_storage, addr);
	}
	std::string pwd() {
		state_check();

		std::string ret = "";
		Address temp = m_current_directory;
		while (temp != 0) {
			m_storage.seekg(temp * block_size + 1);
			Address addr;
			read_f(m_storage, addr);
			m_storage.seekg(addr);
			std::string tmp;
			char b;
			while (m_storage >> b)
				tmp += b;
			ret = tmp + '/' + ret;
			temp = addr / block_size;
		}
		return ret.empty() ? "/" : ret;
	}
};