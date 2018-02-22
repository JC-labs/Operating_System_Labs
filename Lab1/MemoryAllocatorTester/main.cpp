#include "MemoryAllocatorTester.hpp"
#include <QtWidgets/QApplication>
int main(int argc, char *argv[]) {
	QApplication a(argc, argv);
	MemoryAllocatorTester w;
	w.show();
	return a.exec();
}