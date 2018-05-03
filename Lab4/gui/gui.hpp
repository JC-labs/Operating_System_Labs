#pragma once
#include <QtWidgets/QWidget>
#include "ui_gui.h"
class gui : public QWidget{	
	Q_OBJECT
public:
	gui(QWidget *parent = Q_NULLPTR);
protected:
	void restart(bool selection = true);
private:
	Ui::guiClass ui;
	int stage, mask;
protected slots:
	void step();
};