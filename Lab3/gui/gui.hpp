#pragma once
#include <QtWidgets/QWidget>
#include "ui_gui.h"
namespace QtCharts { class QChartView; }
class gui : public QWidget {
	Q_OBJECT
public:
	gui(QWidget *parent = Q_NULLPTR);
	~gui();
private:
	Ui::guiClass ui;
	QtCharts::QChartView *av_wait;
	QtCharts::QChartView *ws_perc;
	QtCharts::QChartView *priority;
private slots:
	void calculate();
};