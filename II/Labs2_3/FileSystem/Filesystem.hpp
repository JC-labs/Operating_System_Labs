#pragma once
#include <string>
#include <fstream>
#include <bitset>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <iterator>

#define write_f(f, v) f.write(reinterpret_cast<const char *>(&(v)), sizeof(v));
#define read_f(f, v) f.read(reinterpret_cast<char *>(&(v)), sizeof(v));

enum class Filetype {
	empty = 0,
	file = 1,
	dir = 2,
	link = 3
};
inline constexpr uint8_t operator*(Filetype t) { return uint8_t(t); }

using Address = uint32_t;

template<size_t block_size = 4096u, size_t block_number = 1024u>
struct OpenedFile {
	std::string name;
	std::vector<Address> blocks;
	Address descriptor;
};

template <size_t block_size = 4096u, size_t block_number = 1024u>
class Filesystem {
	std::fstream m_storage;

	Address m_current_directory;
public:
	const Address m_root_directory = 0u;
protected:
	void state_check() {
		if (m_storage.fail())
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
	void mark_block_empty(Address const& addr) {
		state_check();

		m_storage.seekg(4 + 4 + 1);
		char buffer[7];
		m_storage.read(buffer, 7);
		if (std::string(buffer) != "Bitset")
			throw std::exception("'/Bitset' file is corrupted. Unable to restore filesystem.");

		Address size;
		read_f(m_storage, size);
		Address current;
		for (Address i = 0; i <= addr / block_size / 8; i++)
			read_f(m_storage, current);

		char b;
		m_storage.seekg(current * block_size + addr % block_size / 8);
		b = m_storage.peek();
		if (auto t = 1 << (addr % 8); b & t) {
			b &= ~t;
			m_storage.seekp(m_storage.tellg());
			m_storage.put(b);
		} else
			throw std::exception("Block is already empty. System may have been corrupted.");
	}
	Address find_free_dir_pos(Address const& dir) {
		state_check();

		Address tmp;
		if (dir) {
			m_storage.seekg(dir + 1);
			while (m_storage.get() != '\0');
			read_f(m_storage, tmp);
		} else
			tmp = dir;

		char type;
		m_storage.seekg(tmp * block_size + 8);
		while (type = m_storage.get()) {
			while (m_storage.get() != '\0');
			Address size, temp;
			read_f(m_storage, size);
			if (type == *Filetype::file) {
				while (size--)
					read_f(m_storage, temp);
			} else if (type == *Filetype::dir) {

			} else
				throw std::exception("Unsupported or broken file.");
		}

		return Address(m_storage.tellg());
	}
	Address find_dir_pos(std::string const& path, Address const& dir) {
		state_check();
		if (path.empty())
			throw std::exception("Folder or file name is expected.");

		std::istringstream s(path);
		std::string temp;

		std::getline(s, temp, '/');

		if (temp.empty())
			throw std::exception("Empty folder name wasn't expected");

		if (temp == ".") {
			m_storage.seekg(dir * block_size + 4);
			Address tmp;
			read_f(m_storage, tmp);
			if (path.size() == temp.size())
				return tmp;
			else
				return find_dir_pos(path.substr(temp.size() + 1), tmp);
		} else if (temp == "..") {
			m_storage.seekg(dir * block_size);
			Address tmp;
			read_f(m_storage, tmp);
			m_storage.seekg(tmp + 1);
			while (m_storage.get() != '\0');
			read_f(m_storage, tmp);
			if (path.size() == temp.size())
				return tmp;
			else
				return find_dir_pos(path.substr(temp.size() + 1), tmp);
		} else {
			char type;
			m_storage.seekg(dir * block_size + 8);
			while (type = m_storage.get()) {
				std::string ttemp;
				std::getline(m_storage, ttemp, '\0');
				if (temp == ttemp)
					if (path.size() == temp.size())
						return Address(m_storage.tellg()) - Address(ttemp.size()) - 1;
					else
						if (type == *Filetype::dir) {
							Address next_dir;
							read_f(m_storage, next_dir);
							m_storage.seekg(next_dir * block_size);
							read_f(m_storage, next_dir);
							return find_dir_pos(path.substr(temp.size() + 1), next_dir);
						} else
							throw std::exception("Unsupported or broken file.");
						Address size, tmp;
						read_f(m_storage, size);
						if (type == *Filetype::file) {
							while (size--)
								read_f(m_storage, tmp);
						} else if (type == *Filetype::dir) {

						} else
							throw std::exception("Unsupported or broken file.");
			}
		}
		
		throw std::exception("No such file or directory.");
	}
	Address find(std::string const& path) {
		state_check();

		if (path.empty())
			throw std::exception("Empty path wasn't expected");
		if (path.front() == '/')
			if (path.size() == 1)
				return m_root_directory;
			else
				return find_dir_pos(path.substr(1), m_root_directory);
		else
			return find_dir_pos(path, m_current_directory);
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
	void filestat(std::string const& name, std::ostream &s) {
		state_check();

		auto pos = find(name);
		m_storage.seekg(pos - 1);
		char type;
		m_storage >> type;
		if (type != *Filetype::file)
			throw std::exception("Filename is expected.");

		std::string temp;
		std::getline(m_storage, temp, '\0');
		Address size, ttemp;
		read_f(m_storage, size);
		s << temp << '\t' << size << ": ";
		while (size--) {
			read_f(m_storage, ttemp);
			s << ttemp << ' ';
		}
		s << '\n';
	}
	void create(std::string const& name) {
		state_check();

		auto pos = find_free_dir_pos(m_current_directory);
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

		auto pos = find_free_dir_pos(m_current_directory);
		auto addr = claim_free_block();
		m_storage.seekp(addr * block_size);
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
			m_storage.seekg(temp - 1);
			char type;
			m_storage >> type;
			if (type != *Filetype::dir)
				throw std::exception("Only folder names are supported in paths.");
			while (m_storage.get() != '\0');
			Address addr;
			read_f(m_storage, addr);
			m_storage.seekg(addr * block_size);
			read_f(m_storage, addr);
			m_storage.seekg(addr);
			std::string tmp;
			char b;
			do {
				m_storage >> b;
				tmp += b;
			} while (b);
			ret = tmp.substr(0, tmp.size() - 1) + '/' + ret;
			temp = addr / block_size;
		}
		return "/" + ret;
	}
	void cd(std::string const& path) {
		m_current_directory = find(path);
		if (m_current_directory) {
			m_storage.seekg(m_current_directory - 1);
			char type;
			m_storage >> type;
			if (type != *Filetype::dir)
				throw std::exception("Folder name is expected.");
		}
	}
	void ls(std::ostream &s) {
		state_check();

		Address tmp;
		std::string temp;
		if (m_current_directory) {
			m_storage.seekg(m_current_directory);
			std::getline(m_storage, temp, '\0');
			s << "  " << temp << "/:\n";
			read_f(m_storage, tmp);
		} else {
			tmp = m_current_directory;
			s << "  /:\n";
		}

		char type;
		m_storage.seekg(tmp * block_size + 8);
		while (type = m_storage.get()) {
			std::getline(m_storage, temp, '\0');
			Address size, ttemp;
			read_f(m_storage, size);
			if (type == *Filetype::file) {
				s << "    F: " << temp << std::setw(10 - temp.size()) << '\t' << size << ": ";
				while (size--) {
					read_f(m_storage, ttemp);
					s << ttemp << ' ';
				}
				s << '\n';
			} else if (type == *Filetype::dir) {
				s << "    D: " << temp << std::setw(10 - temp.size()) << '\t' << size << "\n";
			} else
				throw std::exception("Unsupported or broken file.");
		}
	}
	void rmdir(std::string const& name) {
		state_check();

		auto pos = find(name);
		m_storage.seekg(pos - 1);
		char type;
		m_storage >> type;
		if (type != *Filetype::dir)
			throw std::exception("Folder name is expected.");

		while (m_storage.get() != '\0');
		Address folder_block;
		read_f(m_storage, folder_block);

		auto begin_pos = m_storage.tellg();
		auto begin_iterator = std::istream_iterator<char>(m_storage);
		auto end_pos = find_free_dir_pos(m_current_directory);
		m_storage.seekg(end_pos + 1);
		auto end_iterator = std::istream_iterator<char>(m_storage);

		m_storage.seekp(pos - 1);
		std::copy(begin_iterator, end_iterator, 
				  std::ostream_iterator<char>(m_storage, ""));

		mark_block_empty(folder_block);
	}
	void truncate(std::string const& filename, size_t const& new_size) {
		state_check();

		auto pos = find(filename);
		m_storage.seekg(pos - 1);
		char type;
		m_storage >> type;
		if (type != *Filetype::file)
			throw std::exception("Filename is expected.");
		while (m_storage.get() != '\0');
		
		std::vector<Address> blocks;
		Address old_size, temp, temp_size;
		read_f(m_storage, old_size);
		temp_size = old_size;
		while (temp_size--) {
			read_f(m_storage, temp);
			blocks.push_back(temp);
		}

		//Delete old descriptor.
		auto begin_pos = m_storage.tellg();
		auto begin_iterator = std::istream_iterator<char>(m_storage);
		auto end_pos = find_free_dir_pos(m_current_directory);
		m_storage.seekg(end_pos + 1);
		auto end_iterator = std::istream_iterator<char>(m_storage);

		m_storage.seekp(pos - 1);
		std::copy(begin_iterator, end_iterator,
				  std::ostream_iterator<char>(m_storage, ""));

		//Change size (claim or release blocks).
		for (int32_t i = 0; i < int32_t(new_size) - int32_t(old_size); i++)
			blocks.push_back(claim_free_block());
		for (int32_t i = 0; i < int32_t(old_size) - int32_t(new_size); i++) {
			auto tmp = blocks.back();
			blocks.pop_back();
			mark_block_empty(tmp);
		}

		//Create new descriptor.
		pos = find_free_dir_pos(m_current_directory);
		Address size = Address(blocks.size());

		m_storage.seekp(pos);
		m_storage.seekp(-1, std::fstream::cur);
		m_storage << *Filetype::file << filename << '\0';
		write_f(m_storage, size);
		for (auto it : blocks)
			write_f(m_storage, it);
	}
	OpenedFile<> open(std::string const& name) {
		state_check();

		OpenedFile<> ret;

		auto pos = find(name);
		m_storage.seekg(pos - 1);
		char type;
		m_storage >> type;
		if (type != *Filetype::file)
			throw std::exception("Filename is expected.");
		ret.descriptor = pos - 1;

		std::getline(m_storage, ret.name, '\0');
		Address size, temp;
		read_f(m_storage, size);
		while (size--) {
			read_f(m_storage, temp);
			ret.blocks.push_back(temp);
		}

		return ret;
	}
	std::string read(OpenedFile<> const& file, 
					 Address const& offset, Address const& size) {
		state_check();

		if (offset + size > file.blocks.size() * block_size)
			throw std::exception("Error : cannot read outside of file.");
		
		char *buffer;
		std::string ret;
		int32_t first_page = offset / block_size,
			last_page = (offset + size) / block_size;

		if (first_page == last_page) {
			buffer = new char[size];
			m_storage.seekg(file.blocks.at(first_page) * block_size + offset);
			m_storage.read(buffer, size);
			ret = buffer;
			delete buffer;
		} else {
			buffer = new char[block_size - offset % block_size];
			m_storage.seekg(file.blocks.at(first_page) * block_size + offset);
			m_storage.read(buffer, block_size - offset % block_size);
			ret += buffer;
			delete buffer;

			buffer = new char[block_size];
			for (auto page = first_page + 1; page <= last_page - 1; page++) {
				m_storage.seekg(file.blocks.at(page) * block_size);
				m_storage.read(buffer, block_size);
				ret += buffer;
			}
			delete buffer;

			buffer = new char[block_size - (offset + size) % block_size];
			m_storage.seekg(file.blocks.at(last_page) * block_size);
			m_storage.read(buffer, block_size - (offset + size) % block_size);
			ret += buffer;
			delete buffer;
		}

		return ret;
	}
	void write(OpenedFile<> const& file, Address const& offset, 
						  Address const& size, std::string data = "") {
		state_check();

		if (offset + size > file.blocks.size() * block_size)
			throw std::exception("Error : cannot write outside of file.");
		if (data.empty())
			data.resize(size);
		if (data.size() != size)
			throw std::exception("Size of 'data' is expected to be equal to the 'size'.");

		const char *buffer;
		int32_t first_page = offset / block_size,
			last_page = (offset + size) / block_size;

		if (first_page == last_page) {
			buffer = data.c_str();
			m_storage.seekp(file.blocks.at(first_page) * block_size + offset);
			m_storage.write(buffer, size);
		} else {
			buffer = data.substr(0, block_size - offset % block_size).c_str();
			m_storage.seekp(file.blocks.at(first_page) * block_size + offset);
			m_storage.write(buffer, block_size - offset % block_size);

			for (auto page = first_page + 1; page <= last_page - 1; page++) {
				buffer = data.substr(block_size - offset % block_size + page * block_size,
									 block_size - offset % block_size + (page + 1) * block_size).c_str();
				m_storage.seekp(file.blocks.at(page) * block_size);
				m_storage.write(buffer, block_size);
			}

			buffer = data.substr(0, data.size() - block_size + (offset + size) % block_size).c_str();
			m_storage.seekp(file.blocks.at(last_page) * block_size);
			m_storage.write(buffer, block_size - (offset + size) % block_size);
		}
	}
	void link(std::string const& linkname, std::string const& filename) {
		state_check();

		auto file = find(filename);
		m_storage.seekg(file - 1);
		char type;
		m_storage >> type;
		if (type != *Filetype::file)
			throw std::exception("Filename is expected.");

		auto pos = find_free_dir_pos(m_current_directory);
		auto addr = claim_free_block();
		Address size = 1;

		m_storage.seekp(pos);
		m_storage.seekp(-1, std::fstream::cur);
		m_storage << *Filetype::link << linkname << '\0';
		write_f(m_storage, file -= 1);
	}
	void unlink(std::string const& linkname) {
		state_check();

		auto pos = find(linkname);
		m_storage.seekg(pos - 1);
		char type;
		m_storage >> type;
		if (type != *Filetype::link)
			throw std::exception("Linkname is expected.");

		auto begin_pos = m_storage.tellg();
		auto begin_iterator = std::istream_iterator<char>(m_storage);
		auto end_pos = find_free_dir_pos(m_current_directory);
		m_storage.seekg(end_pos + 1);
		auto end_iterator = std::istream_iterator<char>(m_storage);

		m_storage.seekp(pos - 1);
		std::copy(begin_iterator, end_iterator,
				  std::ostream_iterator<char>(m_storage, ""));
	}
};