#pragma once
#include <cstdint>
#include <exception>
class VirtualPageHandle {
	using AddressType = uint32_t;
	AddressType m_address;
	const AddressType P_MASK = 0x80'00'00'00;
	const AddressType R_MASK = 0x40'00'00'00;
	const AddressType M_MASK = 0x20'00'00'00;
	const AddressType N_MASK = 0x1f'ff'ff'ff;
public:
	VirtualPageHandle() : m_address(0u) {}
	VirtualPageHandle& operator=(VirtualPageHandle const& other) { m_address = other.m_address; }
	inline bool p() const { return m_address & P_MASK; }
	inline bool r() const { return m_address & R_MASK; }
	inline bool m() const { return m_address & M_MASK; }
	inline AddressType n() const { return m_address & N_MASK; }
	inline void p(bool value) { if (value) m_address |= P_MASK; else m_address &= ~P_MASK; }
	inline void r(bool value) { if (value) m_address |= R_MASK; else m_address &= ~R_MASK; }
	inline void m(bool value) { if (value) m_address |= M_MASK; else m_address &= ~M_MASK; }
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
		if (auto it = std::find_if(m_pages.begin(), m_pages.end(), [](auto &h) { return !h.p() }); it != m_pages.end()) {
			it->p(true); return *it;
		}
		throw std::exception("Empty page wasn't found.");
		//To be extended.
	}
};
class VirtualPage : protected VirtualPageHandle {
protected:
	using VirtualPageHandle::operator=;
	void page_fault() { operator=(VirtualMemoryController::page_fault()); }
	void access() { if (p()) return; page_fault(); }
public:
	void read() { 
		access();
		if (r()) return;
		r(true);
	}
	void modify() {	
		access();
		if (m()) return;
		m(true);
	}
};
class ProcessMemoryTable {
	using IndexType = size_t;
	std::vector<VirtualPage> m_table;
	std::vector<IndexType> m_working_set;
protected:
	void generate_working_set();
public:
	ProcessMemoryTable() { generate_working_set(); }
};