#pragma once
#include <shared_mutex>
namespace MyAllocator {
	namespace Additional {
		struct abstract_header {
			bool state;
			abstract_header(bool _state = false) : state(_state) {}
		};
	}
	namespace Exceptions {
		class allocation_is_not_possible : public std::exception {};
		class reallocation_has_failed : public std::exception {};
	}
	template <size_t memory_pool = 1024 * 8, typename Type = unsigned long long>
	class AbstractAllocator : public std::allocator<Type> {
	protected:
		Type *inner_memory;
		std::shared_mutex memory_mutex;
	public:
		static size_t const memory_size = memory_pool;
		AbstractAllocator() {
			std::unique_lock<std::shared_mutex>(memory_mutex);
			inner_memory = new Type[memory_pool];
		}
		~AbstractAllocator() { delete inner_memory; }
		virtual void monitor(std::function<void(bool, size_t, size_t)> monitor) abstract;

		virtual Type* allocate(size_t const n) abstract;
		virtual void deallocate(Type *ptr) abstract;
		virtual Type* reallocate(Type *&ptr, size_t const n) abstract;
	};
}