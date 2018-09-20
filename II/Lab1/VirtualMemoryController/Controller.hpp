#pragma once
#include <cstdint>
#include <exception>
class VirtualPageHandle {
public:
	using AddressType = uint32_t;
private:
	AddressType m_address;
	static const AddressType P_MASK = 0x80'00'00'00;
	static const AddressType R_MASK = 0x40'00'00'00;
	static const AddressType M_MASK = 0x20'00'00'00;
	static const AddressType S_MASK = 0x10'00'00'00;
	static const AddressType N_MASK = 0x0f'ff'ff'ff;
public:
	VirtualPageHandle() : m_address(0u) {}
	VirtualPageHandle& operator=(VirtualPageHandle const& other) { 
		m_address = other.m_address; return *this; 
	}
	inline bool p() const { return m_address & P_MASK; }
	inline bool r() const { return m_address & R_MASK; }
	inline bool m() const { return m_address & M_MASK; }
	inline bool s() const { return m_address & S_MASK; }
	inline AddressType n() const { return m_address & N_MASK; }
	inline void p(bool value) { if (value) m_address |= P_MASK; else m_address &= ~P_MASK; }
	inline void r(bool value) { if (value) m_address |= R_MASK; else m_address &= ~R_MASK; }
	inline void m(bool value) { if (value) m_address |= M_MASK; else m_address &= ~M_MASK; }
	inline void s(bool value) { if (value) m_address |= S_MASK; else m_address &= ~S_MASK; }
	inline void n(AddressType value) {
		if (value & ~N_MASK) throw std::exception("Address is too big.");
		m_address = (m_address & ~N_MASK) | value;
	}
};
#include <vector>
#include <algorithm>
class VirtualMemoryController {
	static std::vector<VirtualPageHandle> m_pages;
public:
	static void pages(size_t number) { m_pages.resize(number); }
	static VirtualPageHandle page_fault() {
		VirtualPageHandle::AddressType counter = 0u;
		if (auto it = std::find_if(m_pages.begin(), m_pages.end(), 
								   [&counter](auto &h) { counter++; return !h.p(); }); it != m_pages.end()) {
			it->p(true); it->n(counter); return *it;
		}
		throw std::exception("Empty page wasn't found.");
		//To be extended.
	}
};
class VirtualPage : protected VirtualPageHandle {
protected:
	using VirtualPageHandle::operator=;
	void page_fault() { operator=(VirtualMemoryController::page_fault()); }
public:
	void read() {
		if (p() && r()) return;
		if (p() && m()) return r(true);
		page_fault();
		return r(true);
	}
	void modify() {
		if (p() && m()) return;
		if (p() && r()) return m(true);
		page_fault();
		return m(true);
	}
};
class Process {
	using IndexType = uint16_t;
	std::vector<VirtualPage> m_table;
	std::vector<IndexType> m_working_set;
public:
	Process() {}
	void generate_working_set(IndexType number = 5, IndexType left = 0,
							  IndexType right = std::numeric_limits<IndexType>::max());
	Process(IndexType number = 5, IndexType left = 0,
			IndexType right = std::numeric_limits<IndexType>::max()) { 
		generate_working_set(number, left, right); 
	}
	inline VirtualPage& page(IndexType index) { 
		if (index >= m_table.size())
			m_table.resize(index + 1);
		return m_table.at(index); 
	}
	inline VirtualPage& working_set(IndexType index) { return page(m_working_set.at(index)); }
};