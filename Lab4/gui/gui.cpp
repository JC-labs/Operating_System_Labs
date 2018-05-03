#include "gui.hpp"
#include <qspinbox.h>
#include "Planner.hpp"
gui::~gui() {
	delete planner;
}
gui::gui(QWidget *parent) : QWidget(parent) {
	ui.setupUi(this);
	planner = new Planner(ui.matrix);
	connect(ui.n, QOverload<int>::of(&QSpinBox::valueChanged), planner, &Planner::resize);
	connect(ui.randomize, &QPushButton::clicked, this, [&]() { planner->randomize(ui.percent->value()); });
	//connect(ui.matrix, &QTableWidget::itemSelectionChanged, planner, &Planner::restart);
	connect(ui.step, &QPushButton::clicked, planner, &Planner::step);
	connect(ui.finish, &QPushButton::clicked, planner, &Planner::finish);
	connect(ui.matrix, &QTableWidget::itemDoubleClicked, this, [&](QTableWidgetItem *item) {
		if (item->text() == " ")
			item->setText("1");
		else if (item->text() == "1")
			item->setText(" ");
		planner->restart();
	});
	ui.n->setValue(15);
	connect(ui.test, &QPushButton::clicked, this, [&]() {

	});
}