#include "MemoryAllocatorTester.hpp"
#include "../MemoryAllocator/MyAllocator.hpp"
#include <QtWidgets/QApplication>
int main(int argc, char *argv[]) {
	QApplication a(argc, argv);
	auto alloc = new MyAllocator::MyAllocator<1024 * 32>();
	MemoryAllocatorTester w(alloc);
	w.show();
	return a.exec();
}