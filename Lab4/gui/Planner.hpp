#pragma once
class QTableWidget;
class Planner {
	QTableWidget *m_matrix;
	int stage, mask;
public:
	Planner(int size);
	Planner(QTableWidget *matrix);
	void restart(bool selection = true, bool colors = true);
	void resize(int n);
	void randomize(float percent);
	bool ongoing() const;
	void step();
	void finish();
};
#include <qobject.h>
class PlannerObject : public QObject, protected Planner {
	Q_OBJECT
public:
	PlannerObject(QTableWidget *matrix) : Planner(matrix) {}
	Planner::restart;
public slots:
	void resize(int n) { Planner::resize(n); }
	void randomize(float percent) { Planner::randomize(percent); }
	void step() { Planner::step(); }
	void finish() { Planner::finish(); }
};