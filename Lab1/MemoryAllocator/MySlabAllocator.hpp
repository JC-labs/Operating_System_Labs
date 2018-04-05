#pragma once
#include "AbstractAllocator.hpp"
#include "bitset.hpp"
#include <unordered_map>
namespace MyAllocator {
	namespace Additional {
		enum class slab_state {
			//undefined = -1,
			free = 0,
			partiated = 1,
			whole = 2,
			continuation = 3
		};
		template <size_t slab_size> struct slab_header {
			slab_state state;
			size_t part_size;
			bitset<> parts;
			slab_header(slab_state _type = slab_state::free/*undefined*/) 
				: state(_type) {}
			void make_partiated(size_t const _part_size, bool set_first_part = false) {
				part_size = _part_size;
				state = slab_state::partiated;
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
		: public AbstractAllocator<memory_pool, Type> {

		static_assert(memory_pool % slab_size == 0, 
					  "Stabs cannot be evenly distributed.");
	protected:
		Additional::slab_header<slab_size> slabs[memory_pool / slab_size];
		std::unordered_multimap<size_t, size_t> partial_slabs;
	protected:
		virtual size_t find_n_free_slabs(size_t const n) const {
			std::shared_lock<std::shared_mutex>(memory_mutex);
			size_t concurrent_free_slabs = 0;
			for (size_t i = 0; i < memory_pool / slab_size; i++) {
				if (concurrent_free_slabs >= n)
					return i - concurrent_free_slabs;
				if (slabs[i].state == Additional::slab_state::free)
					concurrent_free_slabs++;
				else
					concurrent_free_slabs = 0;
			}
		}
	public:
		using AbstractAllocator::AbstractAllocator;

		virtual Type* allocate(size_t const n) override {
			std::unique_lock<std::shared_mutex>(memory_mutex);
			if (n > slab_size * 2) {
				if (auto size = n / slab_size + (n % slab_size ? 1 : 0); size > 0) {
					auto ret_index = find_n_free_slabs(size);
					slabs[ret_index].state = Additional::slab_state::whole;
					for (auto i = ret_index + 1; i < ret_index + size; i++)
						slabs[i].state = Additional::slab_state::continuation;
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
			auto a = ptr - inner_memory;
			auto page = (ptr - inner_memory) / slab_size;
			auto shift = (ptr - inner_memory) % slab_size / slabs[page].part_size;
			if (shift == 0 && slabs[page].state == Additional::slab_state::whole) 
			{
				do
					slabs[page].state = Additional::slab_state::free;
				while (slabs[++page].state == Additional::slab_state::continuation);
			} else {
				slabs[page].parts.clear(shift);
				if (~slabs[page].parts) {
					auto[first, last] = partial_slabs.equal_range(slabs[page].part_size);
					for (auto it = first; it != last; it++)
						if (it->second == page) {
							partial_slabs.erase(it); break;
						}
					slabs[page].state = Additional::slab_state::free;
				}
			}
		}
		virtual Type* reallocate(Type *&ptr, size_t const n) override {
			auto ret = allocate(n);
			auto page = (ptr - inner_memory) / slab_size;
			size_t size = 0;
			if (auto shift = (ptr - inner_memory) % slab_size / slabs[page].part_size;
				shift == 0 && slabs[page].state == Additional::slab_state::whole) 
			{
				do size++; while (slabs[++page].state == Additional::slab_state::continuation);
				size *= slab_size;
			} else
				size = slabs[page].part_size;
			size = std::min(n, size);
			std::move(ptr, ptr + size, ret);
			deallocate(ptr);
			ptr = ret;
			return ret;
		}
		virtual Additional::abstract_header** get_head() override {
			
		}
		virtual void monitor(std::function<void(Additional::abstract_header const**, Additional::abstract_header const**)> monitor) override {

		}
	};
}