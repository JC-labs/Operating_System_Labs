#include "gui.hpp"
#include <QtCharts\QChart>
#include <QtCharts\QChartView>
#include <QtCharts\QSplineSeries>
#include "..\QueueingSystem\PriorityFIFO.hpp"
#include <thread>
#include <random>
gui::gui(QWidget *parent) : QWidget(parent){
	ui.setupUi(this);
	connect(ui.calculate, &QPushButton::clicked, this, &gui::calculate);
	ui.splitter->setStretchFactor(0, 1);

	av_wait = new QtCharts::QChartView();
	av_wait->setRenderHint(QPainter::Antialiasing);
	ui.average_wait_layout->addWidget(av_wait);
	ws_perc = new QtCharts::QChartView();
	ws_perc->setRenderHint(QPainter::Antialiasing);
	ui.waste_percent_layout->addWidget(ws_perc);
}
gui::~gui() { delete av_wait; delete ws_perc; }
void series_append(QtCharts::QXYSeries *series, double x, double y, double &min_y, double &max_y) {
	if (y < min_y) min_y = y;
	if (y > max_y) max_y = y;
	series->append(x, y);
}
const size_t thread_number = 4;
void gui::calculate() {
	std::mt19937_64 g((std::random_device())());
	std::uniform_int_distribution<> pt_d(ui.min_pt->value(), ui.max_pt->value()),
		pr_d(ui.min_priority->value(), ui.max_priority->value());

	auto av_wait_chart = new QtCharts::QChart();
	auto ws_perc_chart = new QtCharts::QChart();
	av_wait_chart->legend()->hide();
	ws_perc_chart->legend()->hide();
	auto av_wait_series = new QtCharts::QSplineSeries();
	auto ws_perc_series = new QtCharts::QSplineSeries();
	double min_y_aw = std::numeric_limits<double>::max(), max_y_aw = std::numeric_limits<double>::min();
	double min_y_wp = std::numeric_limits<double>::max(), max_y_wp = std::numeric_limits<double>::min();
	for (size_t i = ui.interval_min->value(); i <= ui.interval_max->value(); i += ui.interval_step->value()) {
		auto time = std::make_shared<uint64_t>(0);
		qs::FIFO_manager m(time);
		size_t wasted = 0, sum_wait = 0, counter = 0;
		size_t n = ui.tasks->value();
		while (counter < n) {
			if (*time % i == 0) m.add(std::make_shared<qs::AbstractTask>(pt_d(g)), pr_d(g));
			if (m.empty())
				wasted++;
			else {
				auto t = m.get_next();
				if (!(++(*t))) {
					sum_wait += m.finish_next()->wait_time();
					counter++;
				}
			}
			(*time)++;
		}
		series_append(av_wait_series, 1.f / i, double(sum_wait) / counter, min_y_aw, max_y_aw);
		series_append(ws_perc_series, 1.f / i, double(wasted) / (*time), min_y_wp, max_y_wp);
	}
	av_wait_chart->addSeries(av_wait_series);
	av_wait_chart->createDefaultAxes();
	av_wait_chart->axisY()->setRange(min_y_aw - 1, max_y_aw + 1);
	av_wait->setChart(av_wait_chart);

	ws_perc_chart->addSeries(ws_perc_series);
	ws_perc_chart->createDefaultAxes();
	ws_perc_chart->axisY()->setRange(min_y_wp - 1, max_y_wp + 1);
	ws_perc->setChart(ws_perc_chart);
}