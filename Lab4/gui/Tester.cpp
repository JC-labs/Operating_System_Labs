#include "Planner.hpp"
#include "Tester.hpp"
#include <QtCharts\QChart>
#include <QtCharts\QChartView>
#include <QtCharts\QSplineSeries>
#include <QtCharts\QLineSeries>
#include <chrono>
Tester::Tester(int from, int to, float percent) {
	chart = new QtCharts::QChart();
	chart->legend()->hide();

	view = new QtCharts::QChartView();
	view->setRenderHint(QPainter::Antialiasing);

	series = new QtCharts::QSplineSeries();
	Planner planner(1);
	for (int i = from; i < to; i++) {
		planner.resize(i);
		planner.randomize(percent);

		auto start_time = std::chrono::high_resolution_clock::now();
		planner.finish();
		series->append(i, (std::chrono::high_resolution_clock::now() - start_time).count());
	}

	chart->addSeries(series);
	chart->createDefaultAxes();
	//chart->axisY()->setRange(min_y_p - 0.05 * min_y_p, max_y_p + 0.05 * max_y_p);
	view->setChart(chart);
	view->show();
	view->repaint();
	view->update();
}
Tester::~Tester() {
	delete chart;
	delete view;
	delete series;
}