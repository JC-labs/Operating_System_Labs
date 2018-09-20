#include "Controller.hpp"
#include <list>
#include <random>
#include <fstream>
#include <iomanip>
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
					process.read_ws_page(pws_d(g));
				else
					process.modify_ws_page(pws_d(g));
			else
				if (rw_d(g))
					process.read_page(p_d(g));
				else
					process.modify_page(p_d(g));
	}
	std::ofstream f;
	f.open("processes.txt");
	size_t counter = 0;
	for (auto &process : processes) {
		f << "Process " << counter++ << '\n';
		for (size_t i = 0; i < process->size(); i++)
			f << "    " << i << "    " << process->at(i) << '\n';
	}
	f.close();
	f.open("physical_memory.txt");
	counter = 0;
	for (size_t i = 0; i < VirtualMemoryController::pages().size(); i++)
		f << std::hex << std::setw(8) << std::setfill('0') << i 
			<< "    " << VirtualMemoryController::pages().at(i).first
			<< "    " << VirtualMemoryController::pages().at(i).second << '\n';
	return 0;
}
int main() {
	VirtualMemoryController::pages(1024);
	return test(45, 7, 15, 1'000'000);
}