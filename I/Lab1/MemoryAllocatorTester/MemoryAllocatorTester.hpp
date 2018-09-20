#pragma once
#include <QtWidgets/QWidget>
#include "ui_MemoryAllocatorTester.h"
#include "..\MemoryAllocator\AbstractAllocator.hpp"
class MemoryAllocatorTester : public QWidget {
	Q_OBJECT
public:
	MemoryAllocatorTester(MyAllocator::AbstractAllocator<1024 * 256> *alloc, QWidget *parent = Q_NULLPTR);
	~MemoryAllocatorTester();
private:
	Ui::MemoryAllocatorTesterClass ui;
	MyAllocator::AbstractAllocator<1024 * 256> *m_allocator;
protected slots:
	void simulate();
};