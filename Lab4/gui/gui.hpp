#pragma once
#include <QtWidgets/QWidget>
#include "ui_gui.h"
class PlannerObject;
class gui : public QWidget{	
	Q_OBJECT
public:
	gui(QWidget *parent = Q_NULLPTR);
	~gui();
protected:
	virtual void keyPressEvent(QKeyEvent *ev) override;
private:
	Ui::guiClass ui;
	PlannerObject *planner;
};