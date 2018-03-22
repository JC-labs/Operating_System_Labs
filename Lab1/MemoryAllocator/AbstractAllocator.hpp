#pragma once
namespace MyAllocator {
	namespace Additional {
		struct abstract_header {
			bool state;
			abstract_header(bool _state = false) : state(_state) {}
		};
	}
}