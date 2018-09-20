#pragma once
#include <cstdint>
class VirtualPageHandle {
	using AddressType = uint32_t;
	AddressType m_masked_address;
	const AddressType P_MASK = 0x80'00'00'00;
	const AddressType R_MASK = 0x40'00'00'00;
	const AddressType M_MASK = 0x20'00'00'00;
	const AddressType N_MASK = 0x1f'ff'ff'ff;
public:
	bool p() const { return m_masked_address & P_MASK; }
	bool r() const { return m_masked_address & R_MASK; }
	bool m() const { return m_masked_address & M_MASK; }
	AddressType n() const { return m_masked_address & N_MASK; }
	AddressType operator*() const { return n(); }
};
#include <vector>
class ProcessMemoryTable {
	using IndexType = size_t;
	std::vector<VirtualPageHandle> m_table;
	std::vector<IndexType> m_working_set;
protected:
	void generate_working_set();
public:

};
template <size_t PhysicalPageNumber = 1024>
class VirtualMemoryController {
};