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
	connect(ui.key_lock, &QPushButton::clicked, this, [&](bool checked) {
		if (checked)
			grabKeyboard();
		else
			releaseKeyboard();
	});
}
gui::~gui() {
	delete planner;
}
#include <QKeyEvent>
void gui::keyPressEvent(QKeyEvent *ev) {
	switch (ev->key()) {
		case Qt::Key_S:
			planner->step();
			ev->accept();
			break;
		case Qt::Key_R:
			planner->randomize(ui.percent->value());
			ev->accept();
			break;
		case Qt::Key_F:
			planner->finish();
			ev->accept();
			break;
		case Qt::Key_T:
			new Tester(ui.from->value(), ui.to->value(), ui.percent->value());
			ev->accept();
			break;
		default:
			ev->ignore();
			break;
	}
}