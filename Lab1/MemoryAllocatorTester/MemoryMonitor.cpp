#include "MemoryMonitor.hpp"
MemoryMonitor::MemoryMonitor(MyAllocator::AbstractAllocator<1024 * 256> *allocator, QWidget *parent)
	: QOpenGLWidget(parent), m_allocator(allocator) {}
MemoryMonitor::~MemoryMonitor() {}
void MemoryMonitor::initializeGL() {
	initializeOpenGLFunctions();
}
void MemoryMonitor::resizeGL(int w, int h) {
	glViewport(0, 0, w, h);
	cells = size_t(sqrtf(m_allocator->memory_size) * float(w) / h);
	glLoadIdentity();
	glOrtho(0, cells, m_allocator->memory_size / cells + 1, 0, -1, 1);
	update();
}
void color(bool state) {
	if (state)
		glColor3f(0.8, 1, 0.8);
	else
		glColor3f(1, 0.8, 0.8);
}
void MemoryMonitor::paintGL() {
	glClear(GL_COLOR_BUFFER_BIT);
	m_allocator->monitor([this](bool is_free, size_t start, size_t end) {
		size_t ex = end % cells;
		size_t ey = end / cells;

		size_t x = start % cells;
		size_t y = start / cells;
		if (y < ey) {
			glBegin(GL_TRIANGLE_FAN);
			color(is_free);
			glVertex2f(x, y);
			glVertex2f(cells, y);
			glVertex2f(cells, y + 1);
			glVertex2f(x, y + 1);
			glEnd();
			y++;
		} else {
			glBegin(GL_TRIANGLE_FAN);
			color(is_free);
			glVertex2f(x, y);
			glVertex2f(ex, y);
			glVertex2f(ex, y + 1);
			glVertex2f(x, y + 1);
			glEnd();
			return;
		}
		while (y < ey) {
			glBegin(GL_TRIANGLE_FAN);
			color(is_free);
			glVertex2f(0, y);
			glVertex2f(cells, y);
			glVertex2f(cells, y + 1);
			glVertex2f(0, y + 1);
			glEnd();
			y++;
		}
		glBegin(GL_TRIANGLE_FAN);
		color(is_free);
		glVertex2f(0, y);
		glVertex2f(ex, y);
		glVertex2f(ex, y + 1);
		glVertex2f(0, y + 1);
		glEnd();
	});
}