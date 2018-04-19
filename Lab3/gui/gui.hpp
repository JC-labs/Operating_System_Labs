#pragma once
#include <QtWidgets/QWidget>
#include "ui_gui.h"
class gui : public QWidget {
	Q_OBJECT
public:
	gui(QWidget *parent = Q_NULLPTR);
private:
	Ui::guiClass ui;
};