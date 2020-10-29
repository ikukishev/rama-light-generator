#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QUrl>
#include <QFileDialog>
#include <QStandardPaths>
#include <algorithm>
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonDocument>


const QString cSequenseConfigurationFileName( "sequenseConfiguration.json" );

const QString cKeyOutputDirectory( "outputDir" );
const QString cKeySequenses( "sequenses" );


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
    load();
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
    qDebug() << "MainWindow::persist()";
    QFile persistFile( cSequenseConfigurationFileName );
    if ( !persistFile.open( QIODevice::WriteOnly ) )
    {
        QMessageBox::warning( this, "Warning", QString("Couldn't write configuration to file: ") + cSequenseConfigurationFileName );
        return ;
    }

    QJsonArray sequenseArray;

    for ( const auto& seq : m_sequences )
    {
        sequenseArray.append(seq->serialize());
    }

    QJsonObject config;
    config[ cKeyOutputDirectory ] = destinationFolder;
    config[ cKeySequenses ] = sequenseArray;

    persistFile.write( QJsonDocument(config).toJson() );

}

void MainWindow::load()
{
    QFile persistFile( cSequenseConfigurationFileName );
    if ( persistFile.exists() )
    {
        if (!persistFile.open(QIODevice::ReadOnly))
        {
            QMessageBox::warning( this, "Warning", QString("Couldn't open sequense configuration file: ") + cSequenseConfigurationFileName );
            return ;
        }

        QByteArray saveData = persistFile.readAll();
        persistFile.close();

        QJsonDocument loadDoc( QJsonDocument::fromJson(saveData) );
        const QJsonObject &json = loadDoc.object();

        if ( json.contains( cKeyOutputDirectory ) )
        {
            if ( !json[ cKeyOutputDirectory ].isString() )
            {
                qWarning() << "Output directory is not a string: " << cKeyOutputDirectory ;
            }
            else
            {
                destinationFolder = json[ cKeyOutputDirectory ].toString();
            }
        }

        if (json.contains( cKeySequenses ))
        {
            QJsonArray seqJson( json[ cKeySequenses ].toArray() );
            m_sequences.reserve(seqJson.size());
            qDebug() << "Json sequenses count: " << seqJson.size();

            for ( const auto& seq : seqJson )
            {
                if ( seq.isObject() )
                {
                    std::shared_ptr<CLightSequence> seqPtr = CLightSequence::fromJson( seq.toObject(), *this );
                    if ( seqPtr )
                    {
                        adjustSequense( seqPtr );
                        m_sequences.push_back(seqPtr);
                    }
                }
                else
                {
                    qWarning() << "sequense is a JSON object";
                }
            }

            qDebug() << "sequences count: " << m_sequences.size();

            channelConfigurationChanged();
            updateTable();
        }

    }
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

void MainWindow::sequenseDeleted(CLightSequence *thisObject)
{
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
}

void MainWindow::sequensePlayStarted(CLightSequence *thisObject)
{
    qDebug() << __FUNCTION__ << thisObject->getFileName().c_str();
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

            adjustSequense(sq);

            m_sequences.emplace_back( sq );
         }
         channelConfigurationChanged();
         updateTable();
      }
   }
}

void MainWindow::on_actionSet_destination_folder_triggered()
{
   QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                               destinationFolder,
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

void MainWindow::adjustSequense( std::shared_ptr<CLightSequence> &seq )
{
    if ( seq )
    {
        connect( seq.get(), &CLightSequence::deleteTriggered, this, &MainWindow::sequenseDeleted);
        connect( seq.get(), &CLightSequence::playStarted, this, &MainWindow::sequensePlayStarted );
    }
}
