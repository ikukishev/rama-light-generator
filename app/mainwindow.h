#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "spectrograph.h"
#include "channelconfigurator.h"
#include "CConfiguration.h"
#include "clightsequence.h"

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

private slots:

    void on_actionExit_triggered();

    void on_actionOpen_triggered();

    void on_actionSet_destination_folder_triggered();

    void on_actionSave_sequenses_configuration_triggered();

    void on_actionChannel_configuration_triggered();

private:

    void channelConfigurationChanged();

    void persist();

    void updateTable();

protected:

    virtual void closeEvent(QCloseEvent *event) override;

private:
    Ui::MainWindow *ui;
    Spectrograph*                   m_spectrograph;
    ChannelConfigurator*            m_channelConfigurator;
    std::vector<std::shared_ptr<CLightSequence>> m_sequences;

};

#endif // MAINWINDOW_H
