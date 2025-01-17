#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_ffmpegpalyer.h"

class ffmpegpalyer : public QMainWindow
{
    Q_OBJECT

public:
    ffmpegpalyer(QWidget *parent = nullptr);
    ~ffmpegpalyer();

private:
    Ui::ffmpegpalyerClass ui;
};
