#pragma once
#include <cstdint>
#include <exception>
#include <ostream> 
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
	friend std::ostream& operator<<(std::ostream &s, VirtualPageHandle const& p);
};
template <typename unsigned_type = size_t>
constexpr unsigned_type size_mask() {
	constexpr unsigned_type ret = -1;
	return ret ^ (ret >> 1);
}
#include <vector>
#include <algorithm>
class VirtualMemoryController {
	static std::vector<std::pair<VirtualPageHandle, size_t>> m_pages;
protected:
	static auto page_fault() {
		VirtualPageHandle::AddressType counter = 0u;
		if (auto it = std::find_if(m_pages.begin(), m_pages.end(),
								   [&counter](auto &h) { counter++; return !h.first.p(); }); it != m_pages.end()) {
			it->first.p(true); it->first.n(counter - 1); return it;
		}

		VirtualPageHandle::AddressType min_i = 0u;
		VirtualPageHandle *min_p = nullptr;
		size_t min_v = std::numeric_limits<size_t>::max();
		for (VirtualPageHandle::AddressType i = 0; i < m_pages.size(); i++)
			if (auto t = m_pages.at(i); t.second < min_v) {
				min_i = i;
				min_p = &t.first;
				if (min_v = t.second == 0) break;
			}
		if (min_p) {
			swap_page(*min_p);
			clear_page(m_pages[min_i].first, min_i);
			m_pages.at(min_i).second = 0u;
			return m_pages.begin() + min_i;
		}
		throw std::exception("There should be at least one physical page.");
	}
	static void maintain_clock();
	static void access(std::pair<VirtualPageHandle, size_t> &page) {
		maintain_clock();
		page.second |= size_mask();
	}
	static void swap_page(VirtualPageHandle &page) {
		//It's just a placeholder.
	}
	static void clear_page(VirtualPageHandle &page, VirtualPageHandle::AddressType address) {
		page.n(address); page.p(true); page.r(false); page.m(false);
	}
public:
	static void pages(size_t number) { m_pages.resize(number); }
	static void read_page(VirtualPageHandle &page) {
		if (page.p())
			if (auto address = page.n(); address < m_pages.size())
				if (m_pages.at(address).first.p())
					if (m_pages.at(address).first.r())
						return access(m_pages.at(address));
					else if (m_pages.at(address).first.m()) {
						m_pages.at(address).first.m(true); page.m(true);
						return access(m_pages.at(address));
					}
		auto &temp = page_fault()->first;
		temp.r(true);
		page = temp;
	}
	static void modify_page(VirtualPageHandle &page) {
		if (page.p())
			if (auto address = page.n(); address < m_pages.size())
				if (m_pages.at(address).first.p())
					if (m_pages.at(address).first.m())
						return access(m_pages.at(address));
					else if (m_pages.at(address).first.r()) {
						m_pages.at(address).first.r(true); page.r(true);
						return access(m_pages.at(address));
					}
		auto &temp = page_fault()->first;
		temp.m(true);
		page = temp;
	}
	static auto const& pages() { return m_pages; }
};
class Process {
	using IndexType = uint16_t;
	std::vector<VirtualPageHandle> m_table;
	std::vector<IndexType> m_working_set;
public:
	Process() {}
	void generate_working_set(IndexType number = 5, IndexType left = 0,
							  IndexType right = std::numeric_limits<IndexType>::max());
	Process(IndexType number = 5, IndexType left = 0,
			IndexType right = std::numeric_limits<IndexType>::max()) { 
		generate_working_set(number, left, right); 
	}
	inline VirtualPageHandle& page(IndexType index) {
		if (index >= m_table.size())
			m_table.resize(index + 1);
		return m_table.at(index); 
	}
	inline VirtualPageHandle& working_set(IndexType index) { return page(m_working_set.at(index)); }

	inline void read_page(IndexType index) { return VirtualMemoryController::read_page(page(index)); }
	inline void modify_page(IndexType index) { return VirtualMemoryController::modify_page(page(index)); }
	inline void read_ws_page(IndexType index) { return VirtualMemoryController::read_page(working_set(index)); }
	inline void modify_ws_page(IndexType index) { return VirtualMemoryController::modify_page(working_set(index)); }

	inline auto const& operator*() const { return m_table; }
	inline auto const* operator->() const { return &m_table; }
};