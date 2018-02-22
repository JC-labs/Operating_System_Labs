#pragma once
#include <memory>
#include <shared_mutex>
namespace MyAllocator {
	namespace Additional {
		struct header {
			bool state;
			header **next;
			header **prev;
			header() : state(false), next(nullptr) {}
			header(header **_next, header **_prev, bool _state = false) : state(_state), next(_next), prev(_prev) {}
		};
	}
	void test();
	namespace Exceptions { class allocation_is_not_possible : public std::exception {};	}
	template <size_t memory_pool = 1024 * 8, typename Type = unsigned long long>
	class MyAllocator : public std::allocator<Type> {
		friend void test();
	private:
		Type *inner_memory;
		std::shared_mutex memory_mutex;
	protected:
		Additional::header** get_head() { return reinterpret_cast<Additional::header**>(inner_memory); }
		Additional::header** find_free(size_t const n) {
			std::shared_lock<std::shared_mutex>(memory_mutex);
			for (auto temp = reinterpret_cast<Additional::header**>(inner_memory); (*temp)->next != nullptr; temp = (*temp)->next)
				if (!(*temp)->state && (*temp)->next - temp > n)
					return temp;
			throw Exceptions::allocation_is_not_possible();
		}
	public:
		MyAllocator() {
			std::unique_lock<std::shared_mutex>(memory_mutex);
			inner_memory = new Type[memory_pool];
			Additional::header *temp_head = new Additional::header(reinterpret_cast<Additional::header**>(inner_memory + memory_pool - 1), nullptr, false),
				*temp_tail = new Additional::header(nullptr, reinterpret_cast<Additional::header**>(inner_memory), true);
			inner_memory[memory_pool - 1] = reinterpret_cast<Type>(temp_tail);
			inner_memory[0] = reinterpret_cast<Type>(temp_head);
		}
		~MyAllocator() {
			std::unique_lock<std::shared_mutex>(memory_mutex);
			auto temp = reinterpret_cast<Additional::header**>(inner_memory);
			do {
				auto clr = temp;
				temp = (*temp)->next;
				delete *clr;
			} while ((*temp)->next != nullptr);
			delete *temp;
			delete inner_memory;
		}
		Type* allocate(size_t const n) {
			auto free_space = find_free(n);
			std::unique_lock<std::shared_mutex>(memory_mutex);
			Additional::header *occupied_header = new Additional::header(nullptr, (*free_space)->prev, true);

			auto free = reinterpret_cast<Type*>(free_space) - reinterpret_cast<Type*>(&inner_memory[0]) + n;
			auto occp = reinterpret_cast<Type*>(free_space) - reinterpret_cast<Type*>(&inner_memory[0]);

			(*free_space)->prev = reinterpret_cast<Additional::header**>(inner_memory + occp);
			occupied_header->next = reinterpret_cast<Additional::header**>(inner_memory + free);
			inner_memory[free] = reinterpret_cast<Type>(*free_space);
			inner_memory[occp] = reinterpret_cast<Type>(occupied_header);
			return reinterpret_cast<Type*>(free_space) + 1;
		}
		void deallocate(Type *ptr) {

		}
		Type* reallocate(Type *ptr, size_t const n) {

		}
	};
}