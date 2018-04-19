#pragma once
#include <cstdint>
#include <map>
#include <list>
#include <memory>
namespace qs {
	namespace types {
		using time_point_t = uint64_t;
		using time_period_t = time_point_t;
		using priority_t = uint16_t;
	}
	class FIFO_manager;
	class AbstractTask {
	private:
		types::time_point_t generation_time;
		types::time_period_t processing_time;
		types::time_period_t spent_time;
		types::time_point_t start_time;
		types::time_point_t finished_time;
		friend FIFO_manager;
	public:
		AbstractTask(types::time_period_t processing_time) 
			: processing_time(processing_time), spent_time(0), 
			generation_time(-1), start_time(-1), finished_time(-1) {}
	};
	class FIFO_manager {
	protected:
		std::shared_ptr<types::time_point_t> time_counter;
		std::map<types::priority_t, 
			std::list<std::shared_ptr<AbstractTask>>> queue;
	public:
		FIFO_manager(std::shared_ptr<types::time_point_t> time_counter = std::make_shared<types::time_point_t>(0)) 
			: time_counter(time_counter) {}
		void add(std::shared_ptr<AbstractTask> task, types::priority_t priority) {
			task->generation_time = *time_counter;
			if (auto list = queue.find(priority); list != queue.end())
				list->second.push_back(task);
			else
				queue.insert(std::make_pair(priority, 
											std::list<std::shared_ptr<AbstractTask>>{task}));
		}
		bool empty() { return queue.empty(); }
		std::shared_ptr<AbstractTask> get_next() { 
			queue.begin()->second.front()->start_time = *time_counter;
			return queue.begin()->second.front(); 
		}
		std::shared_ptr<AbstractTask>  finish_next() {
			auto it = queue.begin();
			auto ret = it->second.front();
			ret->finished_time = *time_counter;
			it->second.pop_front();
			if (it->second.empty()) 
				queue.erase(it);
			return ret;
		}
	};
}