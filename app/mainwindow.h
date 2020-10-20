#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "fft-base/qbassaudiofile.h"
#include "fft-base/spectrograph.h"
#include "channelconfigurator.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


public slots:
    void processFinished();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;
    std::shared_ptr<QBassAudioFile> m_audio_file;
    Spectrograph*           m_spectrograph;
    ChannelConfigurator* m_channelConfigurator;

};

#endif // MAINWINDOW_H
