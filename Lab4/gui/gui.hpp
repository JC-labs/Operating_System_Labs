#pragma once
#include <QtWidgets/QWidget>
#include "ui_gui.h"
class Planner;
class gui : public QWidget{	
	Q_OBJECT
public:
	gui(QWidget *parent = Q_NULLPTR);
	~gui();
private:
	Ui::guiClass ui;
	Planner *planner;
};