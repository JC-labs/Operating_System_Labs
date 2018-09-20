#pragma once
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include "..\MemoryAllocator\AbstractAllocator.hpp"
class MemoryMonitor : public QOpenGLWidget, protected QOpenGLFunctions {
	Q_OBJECT
public:
	MemoryMonitor(MyAllocator::AbstractAllocator<1024 * 256> *allocator, QWidget *parent = nullptr);
	~MemoryMonitor();
protected:
	MyAllocator::AbstractAllocator<1024 * 256> *m_allocator;
	size_t cells;
	virtual void initializeGL() override;
	virtual void resizeGL(int w, int h) override;
	virtual void paintGL() override;
};