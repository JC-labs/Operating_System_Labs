#pragma once
#include <qobject.h>
namespace QtCharts {
	class QChart;
	class QChartView;
	class QXYSeries;
}
class Tester : public QObject {
	Q_OBJECT
	QtCharts::QChart *chart;
	QtCharts::QChartView *view;
	QtCharts::QXYSeries *series;
public:
	Tester(int from, int to, float percent);
	~Tester();
};