#include "Planner.hpp"
#include <qtablewidget.h>
#include <random>
Planner::Planner(int size) : m_matrix(new QTableWidget()) { resize(size); }
Planner::Planner(QTableWidget *matrix) : m_matrix(matrix) { resize(matrix->rowCount()); }
void Planner::restart(bool selection, bool colors) {
	stage = 0; mask = 0;
	if (selection)
		m_matrix->clearSelection();
	if (colors) {
		for (int i = 0; i < m_matrix->rowCount(); i++) {
			m_matrix->horizontalHeaderItem(i)->setTextColor(QColor(0, 0, 0));
			for (int j = 0; j < m_matrix->columnCount(); j++)
				m_matrix->item(i, j)->setTextColor(QColor(0, 0, 0));
		}
		for (int j = 0; j < m_matrix->columnCount(); j++)
			m_matrix->verticalHeaderItem(j)->setTextColor(QColor(0, 0, 0));
	}
}
void Planner::resize(int n) {
	m_matrix->clear();
	m_matrix->setRowCount(n);
	m_matrix->setColumnCount(n);
	for (int i = 0; i < n; i++) {
		m_matrix->setHorizontalHeaderItem(i, new QTableWidgetItem(QString::number(i)));
		for (int j = 0; j < n; j++)
			m_matrix->setItem(i, j, new QTableWidgetItem(" "));
	}
	for (int j = 0; j < n; j++)
		m_matrix->setVerticalHeaderItem(j, new QTableWidgetItem(QString::number(j)));
	restart();
}
std::mt19937_64 g((std::random_device())());
void Planner::randomize(float percent) {
	std::bernoulli_distribution d(percent);
	for (int i = 0; i < m_matrix->rowCount(); i++) {
		m_matrix->verticalHeaderItem(i)->setText(QString::number(i));
		for (int j = 0; j < m_matrix->columnCount(); j++)
			m_matrix->item(i, j)->setText(d(g) ? "1" : " ");
	}
	for (int j = 0; j < m_matrix->columnCount(); j++)
		m_matrix->horizontalHeaderItem(j)->setText(QString::number(j));
	restart();
}
bool Planner::ongoing() const {
	return mask < m_matrix->rowCount();
}
void Planner::step() {
	static int min_i, max_j;
	if (mask < m_matrix->rowCount()) {
		switch (stage) {
			case 0:
			{
				min_i = -1;
				int min_v = std::numeric_limits<int>::max();
				for (int i = mask; i < m_matrix->rowCount(); i++) {
					int sum = 0;
					for (int j = mask; j < m_matrix->columnCount(); j++)
						if (m_matrix->item(i, j)->text() == "1") sum++;
					if (sum != 0 && sum < min_v) {
						min_v = sum; min_i = i;
					}
				}
				m_matrix->selectRow(min_i);
				stage++;
				break;
			}
			case 1:
			{
				if (min_i != -1) {
					QString t = m_matrix->verticalHeaderItem(min_i)->text();
					m_matrix->verticalHeaderItem(min_i)->setText(m_matrix->verticalHeaderItem(mask)->text());
					m_matrix->verticalHeaderItem(mask)->setText(t);
					for (int j = 0; j < m_matrix->columnCount(); j++) {
						t = m_matrix->item(min_i, j)->text();
						m_matrix->item(min_i, j)->setText(m_matrix->item(mask, j)->text());
						m_matrix->item(mask, j)->setText(t);
					}
				}
				m_matrix->selectRow(mask);
				stage++;
				break;
			}
			case 2:
			{
				max_j = -1;
				int max_v = 0;
				for (int j = mask; j < m_matrix->columnCount(); j++)
					if (m_matrix->item(mask, j)->text() == "1") {
						int sum = 0;
						for (int i = mask; i < m_matrix->rowCount(); i++)
							if (m_matrix->item(i, j)->text() == "1") sum++;
						if (sum != 0 && sum > max_v) {
							max_v = sum; max_j = j;
						}
					}
				m_matrix->selectColumn(max_j);
				stage++;
				break;
			}
			case 3:
			{
				if (max_j != -1) {
					QString t = m_matrix->horizontalHeaderItem(max_j)->text();
					m_matrix->horizontalHeaderItem(max_j)->setText(m_matrix->horizontalHeaderItem(mask)->text());
					m_matrix->horizontalHeaderItem(mask)->setText(t);
					for (int i = 0; i < m_matrix->rowCount(); i++) {
						t = m_matrix->item(i, max_j)->text();
						m_matrix->item(i, max_j)->setText(m_matrix->item(i, mask)->text());
						m_matrix->item(i, mask)->setText(t);
					}
				}
				m_matrix->selectColumn(mask);
				stage++;
				break;
			}
			case 4:
			{
				m_matrix->horizontalHeaderItem(mask)->setTextColor(QColor(120, 120, 120));
				m_matrix->verticalHeaderItem(mask)->setTextColor(QColor(120, 120, 120));
				m_matrix->item(mask, mask)->setTextColor(QColor(150, 0, 0));
				stage = 0;
				mask++;
				break;
			}
		}
	}
}
void Planner::finish() {
	while (ongoing()) step();
	m_matrix->clearSelection();
}