#include "Controller.hpp"
std::vector<VirtualPageHandle> VirtualMemoryController::m_pages;
#include <random>
void Process::generate_working_set(IndexType number, IndexType left, IndexType right) {
	static std::mt19937_64 g((std::random_device())());
	std::uniform_int_distribution<IndexType> d(left, right);
	while (number--)
		m_working_set.push_back(d(g));
}