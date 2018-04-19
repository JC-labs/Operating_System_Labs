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
}
gui::~gui() { delete av_wait; }
const size_t thread_number = 4;
void gui::calculate() {
	auto av_wait_chart = new QtCharts::QChart();
	av_wait_chart->legend()->hide();
	auto av_wait_series = new QtCharts::QSplineSeries();
	std::mt19937_64 g((std::random_device())());
	std::uniform_int_distribution<> pt_d(ui.min_pt->value(), ui.max_pt->value()),
		pr_d(ui.min_priority->value(), ui.max_priority->value());
	size_t min_y = std::numeric_limits<size_t>::max(), max_y = std::numeric_limits<size_t>::min();
	for (size_t i = ui.interval_min->value(); i <= ui.interval_max->value(); i += ui.interval_step->value()) {
		auto t = pt_d(g);
		if (t < min_y) min_y = t;
		if (t > max_y) max_y = t;
		av_wait_series->append(i, t);
	}
	av_wait_chart->addSeries(av_wait_series);
	av_wait_chart->createDefaultAxes();
	av_wait_chart->axisY()->setRange(min_y - 1, max_y + 1);
	av_wait->setChart(av_wait_chart);
}