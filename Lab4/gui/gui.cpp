#include "Planner.hpp"
#include "Tester.hpp"
#include "gui.hpp"
#include <qspinbox.h>
gui::gui(QWidget *parent) : QWidget(parent) {
	ui.setupUi(this);
	planner = new PlannerObject(ui.matrix);
	connect(ui.n, QOverload<int>::of(&QSpinBox::valueChanged), planner, &PlannerObject::resize);
	connect(ui.randomize, &QPushButton::clicked, this, [&]() { planner->randomize(ui.percent->value()); });
	//connect(ui.matrix, &QTableWidget::itemSelectionChanged, planner, &Planner::restart);
	connect(ui.step, &QPushButton::clicked, planner, &PlannerObject::step);
	connect(ui.finish, &QPushButton::clicked, planner, &PlannerObject::finish);
	connect(ui.matrix, &QTableWidget::itemDoubleClicked, this, [&](QTableWidgetItem *item) {
		if (item->text() == " ")
			item->setText("1");
		else if (item->text() == "1")
			item->setText(" ");
		planner->restart();
	});
	ui.n->setValue(15);
	connect(ui.test, &QPushButton::clicked, this, [&]() {
		new Tester(ui.from->value(), ui.to->value(), ui.percent->value());
	});
}
gui::~gui() {
	delete planner;
}