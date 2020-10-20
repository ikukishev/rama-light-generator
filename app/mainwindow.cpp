#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QUrl>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_audio_file( nullptr )
    , m_spectrograph( new Spectrograph( ))
    , m_channelConfigurator( new ChannelConfigurator( this ))
{
    ui->setupUi(this);

    m_audio_file = QBassAudioFile::get( "/home/ivan/Downloads/PlanetshakersOverflow.mp3" );
    if ( m_audio_file )
    {
       connect(m_audio_file.get(), SIGNAL(processFinished()), this, SLOT(processFinished()));
       connect(m_audio_file.get(), SIGNAL(positionChanged(const SpectrumData&)), m_spectrograph, SLOT(spectrumChanged( const SpectrumData&)));
       processFinished();
    }

    m_channelConfigurator->display();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::processFinished()
{
    //m_spectrograph->setParams(m_audio_file.getSpectrum()[0]->spectrum.count(), SpectrumLowFreq, SpectrumHighFreq*4);
   if (m_audio_file)
   {
      m_audio_file->play();
      m_spectrograph->show();
   }
}


void MainWindow::on_pushButton_clicked()
{
   if (m_audio_file)
      m_audio_file->stop();
}

void MainWindow::on_pushButton_2_clicked()
{
   if (m_audio_file)
      m_audio_file->play();
}
