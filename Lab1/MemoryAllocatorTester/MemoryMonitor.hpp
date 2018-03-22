#pragma once
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include "..\MemoryAllocator\MyAllocator.hpp"
class MemoryMonitor : public QOpenGLWidget, protected QOpenGLFunctions {
	Q_OBJECT
public:
	MemoryMonitor(MyAllocator::MyAllocator<1024 * 32> *allocator, QWidget *parent = nullptr);
	~MemoryMonitor();
protected:
	MyAllocator::MyAllocator<1024 * 32> *m_allocator;
	size_t cells;
	virtual void initializeGL() override;
	virtual void resizeGL(int w, int h) override;
	virtual void paintGL() override;
};