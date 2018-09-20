#include "gui.hpp"
#include <QtCharts\QChart>
#include <QtCharts\QChartView>
#include <QtCharts\QSplineSeries>
#include <QtCharts\QLineSeries>
#include "..\QueueingSystem\PriorityFIFO.hpp"
#include <thread>
#include <random>
#include <vector>
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
	priority = new QtCharts::QChartView();
	priority->setRenderHint(QPainter::Antialiasing);
	ui.priority_layout->addWidget(priority);

	calculate();
}
gui::~gui() { delete av_wait; delete ws_perc; delete priority; }
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
	auto av_wait_series = new QtCharts::QLineSeries();
	auto ws_perc_series = new QtCharts::QLineSeries();
	double min_y_aw = std::numeric_limits<double>::max(), max_y_aw = std::numeric_limits<double>::min();
	double min_y_wp = std::numeric_limits<double>::max(), max_y_wp = std::numeric_limits<double>::min();
	size_t n = ui.tasks->value();
	for (double i = ui.intensity_min->value(); i <= ui.intensity_max->value(); i *= ui.intensity_coeff->value()) {
		auto time = std::make_shared<uint64_t>(0);
		qs::FIFO_manager m(time);
		size_t wasted = 0, sum_wait = 0, counter = 0;
		while (counter < n) {
			if (i >= 1) 
				for (auto j = 0; j < size_t(i); j++)
					m.add(std::make_shared<qs::AbstractTask>(pt_d(g)), pr_d(g));
			else
				if (*time % size_t(1.f / i) == 0)
					m.add(std::make_shared<qs::AbstractTask>(pt_d(g)), pr_d(g));
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
		series_append(av_wait_series, i, double(sum_wait) / counter, min_y_aw, max_y_aw);
		series_append(ws_perc_series, i, double(wasted) / (*time), min_y_wp, max_y_wp);
	}
	av_wait_chart->addSeries(av_wait_series);
	av_wait_chart->createDefaultAxes();
	av_wait_chart->axisY()->setRange(min_y_aw - 0.05 * min_y_aw, max_y_aw + 0.05 * max_y_aw);
	av_wait->setChart(av_wait_chart);

	ws_perc_chart->addSeries(ws_perc_series);
	ws_perc_chart->createDefaultAxes();
	ws_perc_chart->axisY()->setRange(min_y_wp - 0.05 * min_y_wp, max_y_wp + 0.05 * max_y_wp);
	ws_perc->setChart(ws_perc_chart);

	auto priority_chart = new QtCharts::QChart();
	priority_chart->legend()->hide();
	auto priority_series = new QtCharts::QLineSeries();
	double min_y_p = std::numeric_limits<double>::max(), max_y_p = std::numeric_limits<double>::min();
	auto time = std::make_shared<uint64_t>(0);
	qs::FIFO_manager m(time);
	size_t counter = 0;
	auto shift = ui.min_priority->value();
	auto coeff = ui.max_priority->value() - shift;
	n *= coeff;
	std::vector<std::pair<uint64_t, uint64_t>> sum_wait;
	sum_wait.resize(coeff);
	double intensity = (ui.intensity_max->value() + ui.intensity_min->value()) / 2.0;
	if (!intensity) intensity = 1.0;
	while (counter < n) {
		if (intensity >= 1)
			for (auto j = 0; j < size_t(intensity); j++)
				m.add(std::make_shared<qs::AbstractTask>(pt_d(g)), pr_d(g));
		else
			if (*time % size_t(1.f / intensity) == 0)
				m.add(std::make_shared<qs::AbstractTask>(pt_d(g)), pr_d(g));
		if (!m.empty()) {
			auto t = m.get_next_pair();
			if (!(++(*t.second))) {
				sum_wait[t.first - shift].first++;
				sum_wait[t.first - shift].second += m.finish_next()->wait_time();
				counter++;
			}
		}
		(*time)++;
	}
	bool out = false;
	double out_value;
	for (size_t i = 0; i < coeff; i++) {
		if (!out && sum_wait[i].first == 0) { out = true; out_value = max_y_p + 0.05 * max_y_p; }
		series_append(priority_series, i + shift, out ? out_value : double(sum_wait[i].second) / sum_wait[i].first, min_y_p, max_y_p);
	}
	priority_chart->addSeries(priority_series);
	priority_chart->createDefaultAxes();
	priority_chart->axisY()->setRange(min_y_p - 0.05 * min_y_p, max_y_p + 0.05 * max_y_p);
	priority->setChart(priority_chart);
}