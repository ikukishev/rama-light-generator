#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QUrl>
#include <QPushButton>
#include <QLabel>
#include <QIcon>
#include <QCheckBox>
#include <QProgressBar>
#include <QComboBox>
#include <QSlider>

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
       //processFinished();
    }

    //m_channelConfigurator->display();
    ui->tableWidget->setRowCount(2);

    auto button = new QPushButton("start");
    button->setIcon(QIcon(":/images/record.png"));

    auto checkBox = new QCheckBox();
    auto progressBar = new QProgressBar();
    progressBar->setMaximum(20);
    progressBar->setMinimum(0);
    progressBar->setValue(15);
    auto comboBox = new QComboBox();
    comboBox->addItem("110");
    comboBox->addItem("220");

    auto slider = new QSlider();
    slider->setMaximum(100);
    slider->setMinimum(0);
    slider->setOrientation(Qt::Horizontal);



    ui->tableWidget->setCellWidget(0,0, new QLabel("Path to file"));
    ui->tableWidget->setCellWidget(0,1, button);
    ui->tableWidget->setCellWidget(0,2, checkBox);
    ui->tableWidget->setCellWidget(0,3, progressBar);
    ui->tableWidget->setCellWidget(0,4, comboBox);
    ui->tableWidget->setCellWidget(0,5, slider);

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
