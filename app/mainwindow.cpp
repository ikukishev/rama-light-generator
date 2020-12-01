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
#include <QLabel>
#include <QScreen>
#include "constants.h"
#include "widgets/LabelEx.h"
#include "widgets/SliderEx.h"
#include "widgets/FloatSliderWidget.h"


const QString cSequenseConfigurationFileName( "sequenseConfiguration.json" );

const QString cKeyOutputDirectory( "outputDir" );
const QString cKeyPlayRandom( "isRandomPlay" );
const QString cKeySequenses( "sequenses" );


MainWindow::MainWindow( QWidget *parent )
    : QMainWindow( parent )
    , ui( new Ui::MainWindow )
    , m_spectrograph( nullptr )
    , m_channelConfigurator( new ChannelConfigurator( this ) )
    , m_spectrumConnection( nullptr )
    , m_current(  )
    , m_lorCtrl( new CLORSerialCtrl( this ) )
    , m_effectConfiguration( nullptr )
{
    std::srand(std::time(NULL));

    ui->setupUi(this);
    QHeaderView * header = ui->tableWidget->horizontalHeader();
    header->setSectionResizeMode(3, QHeaderView::Stretch);
    header->setSectionResizeMode(6, QHeaderView::Stretch);

    header = ui->tableWidget_2->horizontalHeader();
    header->setSectionResizeMode(1, QHeaderView::Stretch);
    header->setSectionResizeMode(2, QHeaderView::Stretch);

    m_spectrograph = new Spectrograph(  );

    ui->spectrographLayout->addWidget(m_spectrograph);
    m_spectrograph->show();

    load();
    m_lorCtrl->setPortParams( m_channelConfigurator->commPortName(), m_channelConfigurator->baudRate() );

    move( QGuiApplication::primaryScreen()->geometry().topLeft() );

    setWindowState(Qt::WindowMaximized);

    connect( m_channelConfigurator, &ChannelConfigurator::showStateChanged, [this]( const EShowState& showState ){
        startShowTriggered( EShowState::Active == showState );
    } );
}

MainWindow::~MainWindow()
{
    if ( nullptr != m_effectConfiguration)
    {
        delete m_effectConfiguration;
    }
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
    config[ cKeyPlayRandom ] = isPlayRandomEnabled;
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

        if ( json.contains( cKeyPlayRandom ) )
        {
            if ( !json[ cKeyPlayRandom  ].isBool() )
            {
                qWarning() << "Output directory is not a boolean type: " << cKeyPlayRandom  ;
            }
            else
            {
                isPlayRandomEnabled = json[ cKeyPlayRandom  ].toBool();
                ui->actionRandom->setChecked( isPlayRandomEnabled );
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
                        m_sequences.push_back( seqPtr );
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
    ui->tableWidget->clear();
    ui->tableWidget->setRowCount(m_sequences.size());
    for ( std::size_t i = 0; i < m_sequences.size(); ++i )
    {
       auto widgets = m_sequences[i]->getControlWidgets();
       for ( std::size_t j = 0; j < widgets.size(); ++j )
       {
           ui->tableWidget->setCellWidget( i, j, widgets[j] );
       }
    }
    qDebug() << __FUNCTION__ << "m_sequences.size() =" << m_sequences.size() << "done";
}

void MainWindow::sequenseDeleted( std::weak_ptr<CLightSequence> thisObject)
{
    auto sequense = thisObject.lock();
    if ( nullptr == sequense )
    {
        return;
    }

    std::size_t i = 0;
    auto it = m_sequences.begin();
    for ( ; i < m_sequences.size(); ++i, ++it)
    {
       if ( m_sequences[i] == sequense )
       {
          m_sequences.erase( it );
          ui->tableWidget->removeRow( i );
          break;
       }
    }

    if ( m_current.lock() == sequense )
    {
        sequense = nullptr;
        m_current.reset();
    }

    sequensePlayStarted( m_current );

}

void MainWindow::sequenseMove(std::weak_ptr<CLightSequence> thisObject, EMoveDirection direction)
{

   auto sequense = thisObject.lock();
   if ( nullptr == sequense )
   {
       return;
   }

   std::size_t i = 0;
   auto it = m_sequences.begin();
   for ( ; i < m_sequences.size(); ++i, ++it)
   {
      if ( m_sequences[i] == sequense )
      {
         break;
      }
   }

   if ( i > 0 && EMoveDirection::Up == direction )
   {
      auto old = m_sequences[ i-1 ];
      m_sequences[ i-1 ] = sequense;
      m_sequences[ i ] = old;

      updateTable();
   }
   else if ( i < m_sequences.size()-1 && EMoveDirection::Down == direction )
   {
      auto old = m_sequences[ i+1 ];
      m_sequences[ i+1 ] = sequense;
      m_sequences[ i ] = old;

      updateTable();
   }

}

void MainWindow::sequensePlayStarted(std::weak_ptr<CLightSequence> thisObject)
{
    auto currentIndex = ui->tableWidget_2->currentIndex();
    ui->tableWidget_2->clear();
    QStringList labels;
    labels << "Chnnel Name";
    labels << "Gain";
    labels << "Threshold";
    labels << "Fading";

    ui->tableWidget_2->setHorizontalHeaderLabels(labels);

    auto sequense = thisObject.lock();
    if ( nullptr == sequense )
    {
        ui->tableWidget_2->setRowCount(0);
        m_spectrumConnection = nullptr;
        return;
    }

    m_current = thisObject;
    if ( nullptr != m_effectConfiguration )
    {
        m_effectConfiguration->setCurrentSequense( thisObject );
    }
    m_lorCtrl->playStarted( thisObject );
    m_channelConfigurator->sequensePlayStarted( thisObject );

    qDebug() << __FUNCTION__ << sequense->getFileName().c_str();

    m_spectrumConnection = std::shared_ptr<QMetaObject::Connection>(
                new QMetaObject::Connection( connect(sequense.get(), &CLightSequence::positionChanged,
                                                     m_spectrograph, &Spectrograph::spectrumChanged) ),
                [](QMetaObject::Connection* con){ disconnect(*con); delete con; } );

    ui->tableWidget_2->setRowCount( m_channelConfigurator->channels().size() );
    for ( std::size_t i = 0; i < m_channelConfigurator->channels().size(); ++i )
    {
        const auto& channel = m_channelConfigurator->channels()[i];
        auto channelConfiguration = sequense->getConfiguration( channel.uuid );

        auto widgetPressed = [this, channelConfiguration, &channel]()
        {
            qDebug() << "widgetPressed " << channelConfiguration->channelUuid;

            auto setSpectrumIndex = [ channelConfiguration ]( int index )
            {
                qDebug() << channelConfiguration->channelUuid << "  index:" << index;
                channelConfiguration->setSpectrumIndex( index );
            };

            m_spectrumSpectrumIndexSelectedConnection = std::shared_ptr<QMetaObject::Connection>(
                        new QMetaObject::Connection( connect( m_spectrograph, &Spectrograph::selectedBarChanged, setSpectrumIndex) ),
                        [](QMetaObject::Connection* con){ disconnect(*con); delete con; } );

            if ( channelConfiguration->isSpectrumIndexSet() )
            {
                m_spectrograph->setBarSelected( *(channelConfiguration->spectrumIndex) );
            }
            else
            {
                m_spectrograph->setBarSelected( channel.spectrumIndex );
            }

            if ( channelConfiguration->isGainSet() )
            {
                m_spectrograph->setGain( *(channelConfiguration->gain) );
            }
            else
            {
                m_spectrograph->setGain( channel.gain );
            }

            m_spectrograph->setMinimumLevel( channelConfiguration->minimumLevel );

            if ( channelConfiguration->isFadeSet() )
            {
                m_spectrograph->setFading( *(channelConfiguration->fade) );
            }
            else
            {
                m_spectrograph->setFading( channel.fade );
            }

            ui->spectrographGroupBox->setTitle( channel.label + ", unit: " + QString::number( channel.unit ) + ", channel: " + QString::number( channel.channel ) );

        };

        auto label = new LabelEx( channel.label );
        connect( label, &LabelEx::clicked, widgetPressed );

        ui->tableWidget_2->setCellWidget( i, 0, label );


        auto gainSlider = new FloatSliderWidget( cMaxGainValue, cMinGainValue,
                                                 channelConfiguration->isGainSet() ? (*channelConfiguration->gain) : channel.gain );
        connect(gainSlider, &FloatSliderWidget::valueChanged, [channelConfiguration, this]( double value ){
            channelConfiguration->setGain( value );
            m_spectrograph->setGain( value );
        });
        connect(gainSlider, &FloatSliderWidget::clicked, widgetPressed );
        ui->tableWidget_2->setCellWidget( i, 1, gainSlider );


        auto threshHold = new FloatSliderWidget( cMaxThreshholdValue, cMinThreshholdValue, channelConfiguration->minimumLevel );
        connect( threshHold, &FloatSliderWidget::valueChanged, [channelConfiguration, this]( double value ){
            channelConfiguration->minimumLevel =  value;
            m_spectrograph->setMinimumLevel( channelConfiguration->minimumLevel );
        });
        connect( threshHold, &FloatSliderWidget::clicked, widgetPressed );
        ui->tableWidget_2->setCellWidget( i, 2, threshHold );


        auto fading = new FloatSliderWidget( cMaxFadeValue, cMinFadeValue,
                                             channelConfiguration->isFadeSet() ? (*channelConfiguration->fade) : channel.fade );
        connect(fading, &FloatSliderWidget::valueChanged, [channelConfiguration, this]( int value ){
            channelConfiguration->setFade( value );
            m_spectrograph->setFading( value );
        });
        connect(fading, &FloatSliderWidget::clicked, widgetPressed );

        ui->tableWidget_2->setCellWidget( i, 3, fading );
    }

    if ( currentIndex.row() >=0 && currentIndex.row() < ui->tableWidget_2->rowCount() )
    {
        ui->tableWidget_2->selectRow( currentIndex.row() );
        ui->tableWidget_2->scrollTo( currentIndex );
        auto widget = ui->tableWidget_2->cellWidget( currentIndex.row(), 0 );
        if ( nullptr != widget )
        {
            if ( auto label = dynamic_cast<LabelEx*>( widget ) )
            {
                label->emitClick();
            }
            else
            {
                qDebug() << " FAILED CAST ";
            }
        }
        else
        {
            qDebug() << " FAILED GET WIDGET ";
        }
    }
    else
    {
        qDebug() << " FAILED GET CURRENT ROW: " << currentIndex.row();
    }

}

void MainWindow::playNext()
{
   if ( !isShowStarted )
   {
       if ( isRepeat )
       {
           auto current = m_current.lock();
           if ( nullptr != current )
           {
               current->getAudioFile()->setPosition( 0 );
               current->getAudioFile()->play();
           }
       }
       return;
   }

   auto current = m_current.lock();
   if ( nullptr != current )
   {
      if ( QBassAudioFile::EState::Play == current->getAudioFile()->state() && !current->isGenerateStarted() )
      {
         return;
      }
   }


   std::vector<std::size_t> indexList;

   for ( std::size_t i = 0;  i < m_sequences.size(); ++i )
   {
      if ( !m_sequences[i]->isGenerateStarted() )
      {
         indexList.push_back( i );
      }
   }

   if ( isPlayRandomEnabled )
   {
      if ( !indexList.empty() )
      {
         int index = std::rand() % indexList.size();
         current = m_sequences[ indexList[ index ] ];
      }
   }
   else if ( nullptr == current )
   {
      if ( indexList.empty() )
      {
         return;
      }
      else
      {
         current = m_sequences[indexList[0]];
      }
   }
   else
   {
      std::size_t var = 0;
      for ( ; var < m_sequences.size(); ++var)
      {
         if ( m_sequences[var] == current )
         {
            break;
         }
      }

      if ( !indexList.empty() )
      {
         auto nextIndex = -1;
         for ( std::size_t index = 0; index < indexList.size(); ++index )
         {
            if ( indexList[index] > var )
            {
               nextIndex = indexList[index];
               break;
            }
         }

         if ( -1 != nextIndex )
         {
            current = m_sequences[ nextIndex ];
         }
         else
         {
            current = m_sequences[ indexList[0] ];
         }
      }
   }

   if ( nullptr != current )
   {
      current->getAudioFile()->setPosition( 0 );
       current->getAudioFile()->play();
   }
}

void MainWindow::closeEvent(QCloseEvent *)
{
   qDebug() << __FUNCTION__;
   persist();
   if ( nullptr != m_effectConfiguration )
   {
       m_effectConfiguration->close();
   }
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
         for ( auto file : fileDialog.selectedFiles() )
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
   sequensePlayStarted( m_current );
   m_lorCtrl->setPortParams( m_channelConfigurator->commPortName(), m_channelConfigurator->baudRate() );
}

void MainWindow::adjustSequense( std::shared_ptr<CLightSequence> &seq )
{
    if ( seq )
    {
        connect( seq.get(), &CLightSequence::deleteTriggered, this, &MainWindow::sequenseDeleted     );
        connect( seq.get(), &CLightSequence::playStarted,     this, &MainWindow::sequensePlayStarted );
        connect( seq.get(), &CLightSequence::moveUp,          [this](std::weak_ptr<CLightSequence> thisObject)
        {
            sequenseMove( thisObject, EMoveDirection::Up );
        });
        connect( seq.get(), &CLightSequence::moveDown,        [this](std::weak_ptr<CLightSequence> thisObject)
        {
            sequenseMove( thisObject, EMoveDirection::Down );
        });

        connect( seq.get(), &CLightSequence::playFinished,    [this](std::weak_ptr<CLightSequence> thisObject)
        {
            playNext();
        });

        connect( seq.get(), &CLightSequence::generationStarted,    [this](std::weak_ptr<CLightSequence> thisObject)
        {
            if ( m_current.lock() == thisObject.lock())
            {
               playNext();
            }
        });
    }
}



void MainWindow::on_actionRandom_triggered(bool checked)
{
   isPlayRandomEnabled = checked;
}


void MainWindow::on_actionStart_show_triggered(bool checked)
{
    startShowTriggered( checked );
}


void MainWindow::on_actionRepeat_triggered(bool checked)
{
    isRepeat = checked;
}

void MainWindow::on_actionEffect_editor_triggered(bool checked)
{
    if ( checked )
    {
        if ( nullptr == m_effectConfiguration )
        {
            m_effectConfiguration = new CEffectEditorWidget(  );
            m_effectConfiguration->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint );

            m_effectConfiguration->move( QGuiApplication::primaryScreen()->geometry().topLeft() );

            m_effectConfiguration->setWindowState( Qt::WindowMaximized );
        }
        if ( !m_current.expired() )
        {
            m_effectConfiguration->setCurrentSequense( m_current );
        }
        m_effectConfiguration->show();
    }
    else
    {
        if ( nullptr != m_effectConfiguration )
        {
            delete m_effectConfiguration;
            m_effectConfiguration = nullptr;
        }
    }
}

void MainWindow::startShowTriggered(bool active)
{
    if ( !isShowStarted )
    {
       isShowStarted = active;
       playNext();
    }
    else
    {
       isShowStarted = active;
    }
}
