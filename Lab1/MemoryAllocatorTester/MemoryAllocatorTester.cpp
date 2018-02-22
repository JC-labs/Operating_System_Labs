#include "MemoryAllocatorTester.hpp"
#include "MemoryMonitor.hpp"
#include <QPushButton>
#include <QSpinBox>
MemoryAllocatorTester::MemoryAllocatorTester(QWidget *parent) : QWidget(parent) {
	ui.setupUi(this);
	ui.monitor_layout->addWidget(new MemoryMonitor(m_allocator = new MyAllocator::MyAllocator<1024 * 16>()));

	connect(ui.simulate, &QPushButton::clicked, this, &MemoryAllocatorTester::simulate);
	connect<void(QSpinBox::*)(int)>(ui.users, &QSpinBox::valueChanged, [this](int value) {
		while (ui.user_list->count() < value)
			ui.user_list->addItem("-");		
	});
	ui.users->setValue(25);
}
MemoryAllocatorTester::~MemoryAllocatorTester() { delete m_allocator; }
#include <random>
#include <QMessageBox>
std::mt19937 g((std::random_device())());
std::bernoulli_distribution b_d;
void MemoryAllocatorTester::simulate() {
	std::uniform_int_distribution<> u_d(0, ui.users->value() - 1);
	std::uniform_int_distribution<> m_d(0, 1024 - 1);
	try {
		for (size_t i = 0; i < ui.steps->value(); i++) {
			auto u = u_d(g);
			auto m = m_d(g);
			if (ui.user_list->item(u)->text() == "-") {
				ui.user_list->item(u)->setText(QString::number(reinterpret_cast<size_t>(m_allocator->allocate(m)), 16) + 
											   "    -    " + QString::number(m, 16) + "(" + QString::number(m, 10) + ")");
			} else {
				auto txt = ui.user_list->item(u)->text();
				auto t = txt.split(" ").at(0).toLongLong(nullptr, 16);
				auto address = reinterpret_cast<unsigned long long*>(t);
				if (b_d(g))
					ui.user_list->item(u)->setText(QString::number(reinterpret_cast<size_t>(m_allocator->reallocate(address, m)), 16) +
												   "    -    " + QString::number(m, 16) + "(" + QString::number(m, 10) + ")");
				else {
					m_allocator->deallocate(address);
					ui.user_list->item(u)->setText("-");
				}
			}
		}
	} catch (MyAllocator::Exceptions::allocation_is_not_possible e) {
		QMessageBox(QMessageBox::Critical, "Memory allocation isn't possible", "Seems like there is no free memory available. Try cleaning unneeded one before new allocations.",
					QMessageBox::NoButton, this).exec();
	}
	ui.monitor_layout->itemAt(0)->widget()->update();
}