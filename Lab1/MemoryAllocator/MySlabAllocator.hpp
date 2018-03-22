#pragma once
#include "MyAllocator.hpp"
namespace MyAllocator {
	namespace Additional {
		struct slab_header {

		};
	}
	template <size_t memory_pool = 1024 * 32, size_t slab_size = 1024, typename Type = unsigned long long>
	class MySlabAllocator : public MyAllocator<memory_pool, Type> {
		static_assert(memory_pool % slab_size == 0, "Stabs cannot be evenly distributed.");
	protected:
		Additional::slab_header slabs[memory_pool / slab_size];
	protected:
		virtual Additional::header** find_free(size_t const n) override { return nullptr; }
	public:
		MySlabAllocator() {}
		virtual ~MySlabAllocator() override {}

		virtual Type* allocate(size_t const n) override { return nullptr; }
		virtual void deallocate(Type *ptr) override {}
		virtual Type* reallocate(Type *ptr, size_t const n) override { return nullptr; }
	};
}