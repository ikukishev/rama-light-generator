#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "spectrograph.h"
#include "channelconfigurator.h"
#include "CConfiguration.h"
#include "clightsequence.h"
#include "clorserialctrl.h"

namespace Ui {
class MainWindow;
}



class QLabelEx : public QLabel
{
    Q_OBJECT
public:
    explicit QLabelEx(QWidget *parent=nullptr, Qt::WindowFlags f=Qt::WindowFlags())
        : QLabel( parent, f )
    {}
    explicit QLabelEx(const QString &text, QWidget *parent=nullptr, Qt::WindowFlags f=Qt::WindowFlags())
        : QLabel( text, parent, f )
    {}

signals:
    void clicked();

protected:

    virtual void mousePressEvent(QMouseEvent* event) override;

};



class QSliderEx : public QSlider
{
    Q_OBJECT
public:
    explicit QSliderEx(QWidget *parent = nullptr)
        : QSlider( parent )
    {}

    explicit QSliderEx(Qt::Orientation orientation, QWidget *parent = nullptr)
        : QSlider( orientation, parent )
    {}

signals:
    void clicked();

protected:

    virtual void mousePressEvent(QMouseEvent* event) override;

};



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

    void adjustSequense( std::shared_ptr<CLightSequence>& seq );

    void channelConfigurationChanged();

    void persist();

    void load();

    void updateTable();

    void sequenseDeleted(std::weak_ptr<CLightSequence> thisObject);

    void sequensePlayStarted( std::weak_ptr<CLightSequence> thisObject);

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


};

#endif // MAINWINDOW_H
