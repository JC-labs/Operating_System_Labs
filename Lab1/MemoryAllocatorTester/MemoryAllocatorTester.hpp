#pragma once
#include <QtWidgets/QWidget>
#include "ui_MemoryAllocatorTester.h"
#include "..\MemoryAllocator\MyAllocator.hpp"
class MemoryAllocatorTester : public QWidget {
	Q_OBJECT
public:
	MemoryAllocatorTester(MyAllocator::MyAllocator<1024 * 32> *alloc, QWidget *parent = Q_NULLPTR);
	~MemoryAllocatorTester();
private:
	Ui::MemoryAllocatorTesterClass ui;
	MyAllocator::MyAllocator<1024 * 32> *m_allocator;
protected slots:
	void simulate();
};