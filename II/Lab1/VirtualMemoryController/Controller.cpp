#include "Controller.hpp"
std::vector<std::pair<VirtualPageHandle, size_t>> VirtualMemoryController::m_pages;
#include <random>
void Process::generate_working_set(IndexType number, IndexType left, IndexType right) {
	static std::mt19937_64 g((std::random_device())());
	std::uniform_int_distribution<IndexType> d(left, right);
	while (number--)
		m_working_set.push_back(d(g));
}
#include <chrono>
void VirtualMemoryController::maintain_clock() {
	static auto last_update = std::chrono::high_resolution_clock::now();
	if (auto now = std::chrono::high_resolution_clock::now(); std::chrono::duration_cast<std::chrono::microseconds>(now - last_update).count() >= 100) {
		for (auto &page : m_pages)
			page.second >>= 1;
		last_update = now;
	}
}
std::ostream& operator<<(std::ostream &s, VirtualPageHandle const& p) {
	s << std::hex << p.m_address;
	return s;
}