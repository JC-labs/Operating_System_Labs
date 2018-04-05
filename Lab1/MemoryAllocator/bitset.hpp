#pragma once
#include <cmath>
#include <exception>
namespace MyAllocator {
	namespace Additional {
		template <typename type = size_t>
		class bitset {
			type *value;
			const size_t size_of_value = sizeof(type) * 8;
			size_t size, size_of_array;
		public:
			void resize(size_t n) {
				size = n;
				if (value) delete[] value;
				if (n) {
					value = new type[
						size_of_array = (n / size_of_value + (n % size_of_value ? 1 : 0))
					];
					for (int i = 0; i < size_of_array; i++) value[i] = 0;
				} else value = nullptr;
			}
			bitset(size_t n = 0) : value(nullptr) { resize(n); }
			~bitset() { if (value) delete[] value; }
			bool operator[](size_t n) const {
				if (n > size) throw std::range_error("");
				return value[n / size_of_value] & (1 << (n % size_of_value));
			}
			void set(size_t n) {
				if (n > size) throw std::range_error("");
				value[n / size_of_value] |= (1 << (n % size_of_value));
			}
			void clear(size_t n) {
				if (n > size) throw std::range_error("");
				value[n / size_of_value] &= ~(1 << (n % size_of_value));
			}
			bool operator!() const {
				for (size_t i = 0; i < size_of_array; i++)
					if (value[i] != -1)
						return false;
				return true;
			}
			bool operator~() const {
				for (size_t i = 0; i < size_of_array; i++)
					if (value[i] != 0)
						return false;
				return true;
			}
			int free() const {
				for (size_t i = 0; i < size_of_array; i++) {
					type temp = ~value[i];
					temp &= (value[i] + 1);
					if (temp) {
						temp = std::log2(temp) + i * size_of_value;
						return temp < size ? temp : -1;
					}
				}
				return -1;
			}
		};
	}
}