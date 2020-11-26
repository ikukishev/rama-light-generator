#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "spectrograph.h"
#include "channelconfigurator.h"
#include "CConfiguration.h"
#include "clightsequence.h"
#include "clorserialctrl.h"
#include "ceffecteditorwidget.h"

namespace Ui {
class MainWindow;
}



class MainWindow : public QMainWindow
                 , public CConfigation
{
    Q_OBJECT


   enum class EMoveDirection
   {
      Up, Down
   };

public:
    explicit MainWindow( QWidget *parent = nullptr );
    ~MainWindow();

   virtual const std::vector<Channel> &channels() const override;

public slots:

private slots:

    void on_actionExit_triggered();

    void on_actionOpen_triggered();

    void on_actionSet_destination_folder_triggered();

    void on_actionSave_sequenses_configuration_triggered();

    void on_actionChannel_configuration_triggered();

    void on_actionRandom_triggered(bool checked);

    void on_actionStart_show_triggered(bool checked);

    void on_actionRepeat_triggered(bool checked);

private:

    void adjustSequense( std::shared_ptr<CLightSequence>& seq );

    void channelConfigurationChanged();

    void persist();

    void load();

    void updateTable();

    void sequenseDeleted(std::weak_ptr<CLightSequence> thisObject);

    void sequenseMove(std::weak_ptr<CLightSequence> thisObject, EMoveDirection direction);

    void sequensePlayStarted( std::weak_ptr<CLightSequence> thisObject);

    void playNext();

protected:

    virtual void closeEvent(QCloseEvent *) override;

private:
    Ui::MainWindow *ui;
    Spectrograph*                   m_spectrograph;
    ChannelConfigurator*            m_channelConfigurator;
    std::vector<std::shared_ptr<CLightSequence>> m_sequences;
    std::shared_ptr<QMetaObject::Connection> m_spectrumConnection;
    std::shared_ptr<QMetaObject::Connection> m_spectrumSpectrumIndexSelectedConnection;
    std::weak_ptr<CLightSequence>  m_current;
    CLORSerialCtrl*                m_lorCtrl;
    CEffectEditorWidget*           m_effectConfiguration;

    bool isShowStarted = false;
    bool isRepeat = false;

};

#endif // MAINWINDOW_H
