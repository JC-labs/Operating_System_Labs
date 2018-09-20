#include "Controller.hpp"
#include <list>
#include <random>
int test(size_t process_number, size_t working_set_size, size_t working_set_max_index, size_t iterations) {
	static std::mt19937_64 g((std::random_device())());
	std::bernoulli_distribution ws_d(.9);
	std::uniform_int_distribution<size_t> p_d(0, process_number - 1);
	std::uniform_int_distribution<size_t> pws_d(0, working_set_size - 1);
	std::bernoulli_distribution rw_d(.5);
	std::list<Process> processes;
	for (size_t i = 0; i < process_number; i++)
		processes.push_back(Process(working_set_size, 0, working_set_max_index));

	while (iterations--) {
		for (auto &process : processes)
			if (ws_d(g))
				if (rw_d(g))
					process.working_set(pws_d(g)).read();
				else
					process.working_set(pws_d(g)).modify();
			else
				if (rw_d(g))
					process.page(p_d(g)).read();
				else
					process.page(p_d(g)).modify();
	}
	return 0;
}
int main() {
	VirtualMemoryController::pages(1024);
	return test(45, 7, 15, 1'000'000);
}