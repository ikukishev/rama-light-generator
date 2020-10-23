#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QUrl>
#include <QFileDialog>
#include <QStandardPaths>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow( parent )
    , ui( new Ui::MainWindow )
    , m_spectrograph( new Spectrograph( ) )
    , m_channelConfigurator( new ChannelConfigurator( this ) )
{
    ui->setupUi(this);
    QHeaderView * header = ui->tableWidget->horizontalHeader();
    header->setSectionResizeMode(1, QHeaderView::Stretch);
    header->setSectionResizeMode(4, QHeaderView::Stretch);
}

MainWindow::~MainWindow()
{
   delete ui;
}

const std::vector<Channel> &MainWindow::channels() const
{
   return m_channelConfigurator->channels();
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

void MainWindow::updateTable()
{
    ui->tableWidget->setRowCount(m_sequences.size());
    for ( std::size_t i = 0; i < m_sequences.size(); ++i )
    {
       auto& widgets = m_sequences[i]->getControlWidgets();
       for ( std::size_t j = 0; j < widgets.size(); ++j )
       {
           ui->tableWidget->setCellWidget( i, j, widgets[j] );
       }
    }
    qDebug() << __FUNCTION__ << "m_sequences.size() =" << m_sequences.size() << "done";
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
      if ( fileDialog.selectedFiles().count() > 0 )
      {
         for (auto file : fileDialog.selectedFiles())
         {
            qDebug() << file;
            auto sq = std::make_shared<CLightSequence>( file.toStdString(), *this );

            connect( sq.get(), &CLightSequence::deleteTriggered, [this](CLightSequence* thisObject){
               std::size_t i = 0;
               auto it = m_sequences.begin();
               for ( ; i < m_sequences.size(); ++i, ++it)
               {
                  if ( m_sequences[i].get() == thisObject )
                  {
                     m_sequences.erase( it );
                     ui->tableWidget->removeRow( i );
                     break;
                  }
               }
            });

            m_sequences.emplace_back( sq );
         }
         updateTable();
      }
   }
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
   for ( auto& channel : m_sequences )
   {
      channel->channelConfigurationUpdated();
   }
}
