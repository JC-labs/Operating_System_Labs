#pragma once
#include "AbstractAllocator.hpp"
//#include <memory>
#include <algorithm>
#include <functional>
namespace MyAllocator {
	namespace Additional {
		struct header : abstract_header {
			header **next;
			header **prev;
			header() : abstract_header(false), next(nullptr) {}
			header(header **_next, header **_prev, bool _state = false) 
				: abstract_header(_state), next(_next), prev(_prev) {}
		};
	}
	template <size_t memory_pool = 1024 * 8, typename Type = unsigned long long>
	class MyAllocator : public AbstractAllocator<memory_pool, Type> {
	protected:
		virtual Additional::header** find_free(size_t const n) {
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
			Additional::header *temp_tail = new Additional::header(nullptr, reinterpret_cast<Additional::header**>(inner_memory), true),
				*temp_head = new Additional::header(reinterpret_cast<Additional::header**>(inner_memory + memory_pool - 1), nullptr, false);
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
			delete[] inner_memory;
		}
		virtual Type* allocate(size_t const n) override {
			auto free_space = find_free(n);
			std::unique_lock<std::shared_mutex>(memory_mutex);
			Additional::header *occupied_header = new Additional::header(nullptr, (*free_space)->prev, true);
			auto shift = reinterpret_cast<Type*>(free_space) - reinterpret_cast<Type*>(&inner_memory[0]);
			(*free_space)->prev = reinterpret_cast<Additional::header**>(inner_memory + shift);
			occupied_header->next = reinterpret_cast<Additional::header**>(inner_memory + shift + n);
			inner_memory[shift + n] = reinterpret_cast<Type>(*free_space);
			inner_memory[shift] = reinterpret_cast<Type>(occupied_header);
			(*(*(*free_space)->next)->next)->prev = reinterpret_cast<Additional::header**>(inner_memory + shift + n);
			return reinterpret_cast<Type*>(free_space) + 1;
		}
		virtual void deallocate(Type *ptr) override {
			auto freed = reinterpret_cast<Additional::header**>(ptr - 1);
			std::unique_lock<std::shared_mutex>(memory_mutex);
			if (!(*(*freed)->next)->state) {
				auto temp = (*freed)->next;
				(*freed)->next = (*(*freed)->next)->next;
				(*(*freed)->next)->prev = freed;
				delete *temp;
			}
			if (!(*freed)->prev && !(*(*freed)->prev)->state) {
				(*(*freed)->next)->prev = (*freed)->prev;
				(*(*freed)->prev)->next = (*freed)->next;
				delete *freed;
			} else
				(*freed)->state = false;
		}
		virtual Type* reallocate(Type *&ptr, size_t const n) override {
			auto origin = reinterpret_cast<Additional::header**>(ptr - 1);
			auto shift = reinterpret_cast<Type*>(origin) - reinterpret_cast<Type*>(&inner_memory[0]);
			if ((*origin)->next - origin > n || (!(*(*origin)->next)->state && (*(*origin)->next)->next - origin > n)) {
				Additional::header *free_space;
				if (!(*(*origin)->next)->state)
					free_space = *(*origin)->next;
				else
					free_space = (new Additional::header((*origin)->next, origin, false));
				free_space->prev = reinterpret_cast<Additional::header**>(inner_memory + shift);
				(*origin)->next = reinterpret_cast<Additional::header**>(inner_memory + shift + n);
				inner_memory[shift + n] = reinterpret_cast<Type>(free_space);
				(*free_space->next)->prev = reinterpret_cast<Additional::header**>(inner_memory + shift + n);
				return ptr;
			} else {
				auto temp_ptr = allocate(n);
				std::move(ptr, ptr + ((*origin)->next - origin), temp_ptr);
				deallocate(ptr);
				ptr = temp_ptr;
				return temp_ptr;
			}
		}
	public:
		virtual void monitor(std::function<void(bool, size_t, size_t)> monitor) override {
			for (auto temp = reinterpret_cast<Additional::header**>(inner_memory); (*temp)->next != nullptr; temp = (*temp)->next) {
				std::shared_lock<std::shared_mutex>(memory_mutex);
				monitor((*temp)->state, static_cast<size_t>(reinterpret_cast<Additional::abstract_header**>(temp) - reinterpret_cast<Additional::abstract_header**>(inner_memory)),
						static_cast<size_t>(reinterpret_cast<Additional::abstract_header**>((*temp)->next) - reinterpret_cast<Additional::abstract_header**>(inner_memory)));
			}
		}
	};
}