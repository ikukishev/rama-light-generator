#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qbassaudiofile.h"
#include "spectrograph.h"
#include "channelconfigurator.h"
#include "CConfiguration.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
                 , public CConfigation
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

   virtual const std::vector<Channel> &channels() const override;

public slots:
    void processFinished();

private slots:

    void on_actionExit_triggered();

    void on_actionOpen_triggered();

    void on_actionSet_destination_folder_triggered();

    void on_actionSave_sequenses_configuration_triggered();

    void on_actionChannel_configuration_triggered();

private:

    void channelConfigurationChanged();

    void persist();

protected:

    virtual void closeEvent(QCloseEvent *event) override;

private:
    Ui::MainWindow *ui;
    std::shared_ptr<QBassAudioFile> m_audio_file;
    Spectrograph*                   m_spectrograph;
    ChannelConfigurator*            m_channelConfigurator;

};

#endif // MAINWINDOW_H
