#include "MemoryAllocatorTester.hpp"
#include "..\MemoryAllocator\MySlabAllocator.hpp"
#include <QtWidgets/QApplication>
int main(int argc, char *argv[]) {
	QApplication a(argc, argv);
	auto alloc = new MyAllocator::MySlabAllocator<1024 * 32>();
	auto temp0 = alloc->allocate(512);
	auto temp1 = alloc->allocate(512);
	auto temp2 = alloc->allocate(512);
	auto temp3 = alloc->allocate(512);
	//MemoryAllocatorTester w(alloc);
	//w.show();
	return a.exec();
}