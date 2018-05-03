#include "gui.hpp"
#include <qspinbox.h>
#include <qglobal.h>
#include <random>
#include <thread>
void gui::restart(bool selection) { 
	stage = 0; mask = 0; 
	if (selection) 
		ui.matrix->clearSelection();
}
gui::gui(QWidget *parent) : QWidget(parent) {
	ui.setupUi(this);
	connect(ui.n, QOverload<int>::of(&QSpinBox::valueChanged), this, [&](int value) {
		ui.matrix->clear();
		ui.matrix->setRowCount(value);
		ui.matrix->setColumnCount(value);
		for (int i = 0; i < value; i++) {
			ui.matrix->setHorizontalHeaderItem(i, new QTableWidgetItem(QString::number(i)));
			for (int j = 0; j < value; j++)
				ui.matrix->setItem(i, j, new QTableWidgetItem(" "));
		}
		for (int j = 0; j < value; j++)
			ui.matrix->setVerticalHeaderItem(j, new QTableWidgetItem(QString::number(j)));
		restart();
	});
	connect(ui.matrix, &QTableWidget::itemDoubleClicked, this, [&](QTableWidgetItem *item) {
		if (item->text() == " ")
			item->setText("1");
		else if (item->text() == "1")
			item->setText(" ");
		restart();
	});
	connect(ui.randomize, &QPushButton::clicked, this, [&]() {
		std::mt19937_64 g((std::random_device())());
		std::bernoulli_distribution d(ui.percent->value());
		for (int i = 0; i < ui.matrix->rowCount(); i++) {
			ui.matrix->verticalHeaderItem(i)->setText(QString::number(i));
			for (int j = 0; j < ui.matrix->columnCount(); j++)
				ui.matrix->item(i, j)->setText(d(g) ? "1" : " ");
		}
		for (int j = 0; j < ui.matrix->columnCount(); j++)
			ui.matrix->horizontalHeaderItem(j)->setText(QString::number(j));
		restart();
	});
	connect(ui.matrix, &QTableWidget::itemSelectionChanged, this, [&]() {
		//restart(false);
	});
	connect(ui.step, &QPushButton::clicked, this, &gui::step);
	connect(ui.finish, &QPushButton::clicked, this, [&]() {
		while (mask < ui.matrix->rowCount()) step();
		ui.matrix->clearSelection();
	});
	ui.n->setValue(15);
}
void gui::step() {
	static int min_i, max_j;
	if (mask < ui.matrix->rowCount()) {
		switch (stage) {
			case 0:
			{
				min_i = 0;
				int min_v = std::numeric_limits<int>::max();
				for (int i = mask; i < ui.matrix->rowCount(); i++) {
					int sum = 0;
					for (int j = mask; j < ui.matrix->columnCount(); j++)
						if (ui.matrix->item(i, j)->text() == "1") sum++;
					if (sum != 0 && sum < min_v) {
						min_v = sum; min_i = i;
					}
				}
				ui.matrix->selectRow(min_i);
				stage++;
				break;
			}
			case 1:
			{
				QString t = ui.matrix->verticalHeaderItem(min_i)->text();
				ui.matrix->verticalHeaderItem(min_i)->setText(ui.matrix->verticalHeaderItem(mask)->text());
				ui.matrix->verticalHeaderItem(mask)->setText(t);
				for (int j = mask; j < ui.matrix->columnCount(); j++) {
					t = ui.matrix->item(min_i, j)->text();
					ui.matrix->item(min_i, j)->setText(ui.matrix->item(mask, j)->text());
					ui.matrix->item(mask, j)->setText(t);
				}
				ui.matrix->selectRow(mask);
				stage++;
				break;
			}
			case 2:
			{
				max_j = 0;
				int max_v = 0;
				for (int j = mask; j < ui.matrix->columnCount(); j++)
					if (ui.matrix->item(mask, j)->text() == "1") {
						int sum = 0;
						for (int i = mask; i < ui.matrix->rowCount(); i++)
							if (ui.matrix->item(i, j)->text() == "1") sum++;
						if (sum != 0 && sum > max_v) {
							max_v = sum; max_j = j;
						}
					}
				ui.matrix->selectColumn(max_j);
				stage++;
				break;
			}
			case 3:
			{
				QString t = ui.matrix->horizontalHeaderItem(max_j)->text();
				ui.matrix->horizontalHeaderItem(max_j)->setText(ui.matrix->horizontalHeaderItem(mask)->text());
				ui.matrix->horizontalHeaderItem(mask)->setText(t);
				for (int i = mask; i < ui.matrix->rowCount(); i++) {
					t = ui.matrix->item(i, max_j)->text();
					ui.matrix->item(i, max_j)->setText(ui.matrix->item(i, mask)->text());
					ui.matrix->item(i, mask)->setText(t);
				}
				ui.matrix->selectColumn(mask);
				stage++;
				break;
			}
			case 4:
			{
				ui.matrix->horizontalHeaderItem(mask)->setTextColor(QColor(120, 120, 120));
				ui.matrix->verticalHeaderItem(mask)->setTextColor(QColor(120, 120, 120));
				stage = 0;
				mask++;
				break;
			}
		}
	}
}