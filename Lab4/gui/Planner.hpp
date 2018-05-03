#pragma once
#include <qobject.h>
class QTableWidget;
class Planner : public QObject {
	Q_OBJECT
	QTableWidget *m_matrix;
	int stage, mask;
public:
	Planner(QTableWidget *matrix);
	void restart(bool selection = true, bool colors = true);
	void resize(int n);
	void randomize(float percent);
	bool ongoing() const;
	void step();
	void finish();
};