#pragma once
#include "MyAllocator.hpp"
#include <unordered_map>
#include "bitset.hpp"
#include <cassert>
namespace MyAllocator {
	namespace Additional {
		enum class slab_type {
			//undefined = -1,
			free = 0,
			partiated = 1,
			whole = 2,
			continuation = 3
		};
		template <size_t slab_size> struct slab_header {
			slab_type type;
			bitset<> parts;
			slab_header(slab_type _type = slab_type::free/*undefined*/) 
				: type(_type) {}
			void make_partiated(size_t const part_size, bool set_first_part = false) {
				type = slab_type::partiated;
				parts.resize(slab_size / part_size + 
					(slab_size % part_size ? 1 : 0));
				if (set_first_part) parts.set(0);
			}
			int find_free() const {
				if (!parts) return -1;
				else return parts.free();
			}
		};
	}
	template <size_t memory_pool = 1024 * 32, 
		size_t slab_size = 1024, 
		typename Type = unsigned long long>
	class MySlabAllocator 
		: public MyAllocator<memory_pool, Type> {

		static_assert(memory_pool % slab_size == 0, 
					  "Stabs cannot be evenly distributed.");
	protected:
		Additional::slab_header<slab_size> slabs[memory_pool / slab_size];
		std::unordered_multimap<size_t, size_t> partial_slabs;
	protected:
		virtual Additional::header** find_free(size_t const n) override {
			assert(false, "find free is not to be used in SlabAllocator");
			return nullptr;
		}
		virtual size_t find_n_free_slabs(size_t const n) const {
			std::shared_lock<std::shared_mutex>(memory_mutex);
			size_t concurrent_free_slabs = 0;
			for (size_t i = 0; i < memory_pool / slab_size; i++) {
				if (concurrent_free_slabs >= n)
					return i - concurrent_free_slabs;
				if (slabs[i].type == Additional::slab_type::free)
					concurrent_free_slabs++;
				else
					concurrent_free_slabs = 0;
			}
		}
	public:
		MySlabAllocator() {
			std::unique_lock<std::shared_mutex>(memory_mutex);
			inner_memory = new Type[memory_pool];
		}
		virtual ~MySlabAllocator() override { delete inner_memory; }

		virtual Type* allocate(size_t const n) override {
			std::unique_lock<std::shared_mutex>(memory_mutex);
			if (n > slab_size * 2) {
				if (auto size = n / slab_size + (n % slab_size ? 1 : 0); size > 0) {
					auto ret_index = find_n_free_slabs(size);
					slabs[ret_index].type = Additional::slab_type::whole;
					for (auto i = ret_index + 1; i < ret_index + size; i++)
						slabs[i].type = Additional::slab_type::continuation;
					return reinterpret_cast<Type*>(inner_memory + ret_index * slab_size);
				} else
					throw Exceptions::allocation_is_not_possible();
			} else {
				auto range = partial_slabs.equal_range(n);
				for (auto it = range.first; it != range.second; it++) {
					if (auto index = slabs[it->second].find_free(); index != -1) {
						slabs[it->second].parts.set(index);
						return reinterpret_cast<Type*>(
							inner_memory + slab_size * it->second + index * it->first);
					}
				}

				auto ret_index = find_n_free_slabs(1);
				partial_slabs.insert(std::make_pair(n, ret_index));
				slabs[ret_index].make_partiated(n, true);
				return reinterpret_cast<Type*>(inner_memory + slab_size * ret_index);
			}
			throw Exceptions::allocation_is_not_possible();
		}
		virtual void deallocate(Type *ptr) override {
		
		}
		virtual Type* reallocate(Type *ptr, size_t const n) override {
			return nullptr; 
		}
	};
}