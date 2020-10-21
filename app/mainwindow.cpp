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
#include <QFileDialog>
#include <QStandardPaths>

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

    QHeaderView * header = ui->tableWidget->horizontalHeader();
    header->setSectionResizeMode(1, QHeaderView::Stretch);
    header->setSectionResizeMode(4, QHeaderView::Stretch);

    ui->tableWidget->setRowCount(2);

    //auto button = new QPushButton("start");
    //button->setIcon(QIcon(":/images/record.png"));
    //button->setIcon(style()->standardIcon(QStyle::SP_MediaSeekForward));

    //auto checkBox = new QCheckBox();
    //auto progressBar = new QProgressBar();
    //progressBar->setMaximum(20);
//    progressBar->setMinimum(0);
//    progressBar->setValue(15);
//    auto comboBox = new QComboBox();
//    comboBox->addItem("110");
//    comboBox->addItem("220");

    auto slider = new QSlider();
    slider->setMaximum(100);
    slider->setMinimum(0);
    slider->setValue(50);
    slider->setOrientation(Qt::Horizontal);
    slider->setDisabled(true);



//    ui->tableWidget->setCellWidget(0,0, new QLabel("Path to file"));
//    ui->tableWidget->setCellWidget(0,1, button);
//    ui->tableWidget->setCellWidget(0,2, checkBox);
//    ui->tableWidget->setCellWidget(0,3, progressBar);
//    ui->tableWidget->setCellWidget(0,4, comboBox);
      ui->tableWidget->setCellWidget(0,4, slider);



}

MainWindow::~MainWindow()
{
   delete ui;
}

const std::vector<Channel> &MainWindow::channels() const
{
   return m_channelConfigurator->channels();
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

void MainWindow::on_actionExit_triggered()
{
   qDebug() << __FUNCTION__;
   persist();
   QApplication::exit();
}

void MainWindow::persist()
{

}

void MainWindow::closeEvent(QCloseEvent *event)
{
   qDebug() << __FUNCTION__;
   persist();
}

void MainWindow::on_actionOpen_triggered()
{
   QFileDialog fileDialog(this);
   fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
   fileDialog.setFileMode(QFileDialog::ExistingFiles);
   fileDialog.setWindowTitle(tr("Open Files"));
   fileDialog.setNameFilters({{"*.mp3"},{"*.wav"}});
   fileDialog.setDirectory(QStandardPaths::standardLocations(QStandardPaths::MusicLocation).value(0, QDir::homePath()));
   if (fileDialog.exec() == QDialog::Accepted)
   {
      for (auto file : fileDialog.selectedFiles()) {
         qDebug() << file;
      }
   }
       //addToPlaylist(fileDialog.selectedUrls());
}

void MainWindow::on_actionSet_destination_folder_triggered()
{
   QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                               "/home",
                                               QFileDialog::ShowDirsOnly
                                               | QFileDialog::DontResolveSymlinks);
   if ( !dir.isEmpty() && dir != destinationFolder )
      destinationFolder = dir;
}

void MainWindow::on_actionSave_sequenses_configuration_triggered()
{
   qDebug() << __FUNCTION__;
   persist();
}

void MainWindow::on_actionChannel_configuration_triggered()
{
   if ( QDialog::Accepted == m_channelConfigurator->display() )
   {
      channelConfigurationChanged();
   }
}

void MainWindow::channelConfigurationChanged()
{
   qDebug() << __FUNCTION__;
}
