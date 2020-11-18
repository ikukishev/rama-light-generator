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
#include "effects/CEffectFade.h"
#include "effects/CEffectIntensity.h"


const QString cSequenseConfigurationFileName( "sequenseConfiguration.json" );

const QString cKeyOutputDirectory( "outputDir" );
const QString cKeySequenses( "sequenses" );


void QLabelEx::mousePressEvent(QMouseEvent *event)
{
    emit clicked();
    QLabel::mousePressEvent(event);
}

void QSliderEx::mousePressEvent(QMouseEvent *event)
{
    emit clicked();
    QSlider::mousePressEvent(event);
}

MainWindow::MainWindow( QWidget *parent )
    : QMainWindow( parent )
    , ui( new Ui::MainWindow )
    , m_spectrograph( nullptr )
    , m_channelConfigurator( new ChannelConfigurator( this ) )
    , m_spectrumConnection( nullptr )
    , m_current(  )
    , m_lorCtrl( new CLORSerialCtrl( this ) )
    , m_effectConfiguration( new CEffectEditorWidget( ) )
{
    ui->setupUi(this);
    QHeaderView * header = ui->tableWidget->horizontalHeader();
    header->setSectionResizeMode(1, QHeaderView::Stretch);
    header->setSectionResizeMode(4, QHeaderView::Stretch);

    header = ui->tableWidget_2->horizontalHeader();
    header->setSectionResizeMode(1, QHeaderView::Stretch);
    header->setSectionResizeMode(2, QHeaderView::Stretch);

    m_spectrograph = new Spectrograph(  );

    ui->horizontalLayout_3->addWidget(m_spectrograph);
    m_spectrograph->show();

    auto Intensity = std::make_shared< CEffectGeneratorFactoryIntensity >();
    IEffectGeneratorFactory::getGeneratorFactories()[ Intensity->type() ] = Intensity;
    auto Fade = std::make_shared< CEffectGeneratorFactoryFade >();
    IEffectGeneratorFactory::getGeneratorFactories()[ Fade->type() ] = Fade;

    auto inten = std::make_shared<CEffectInten>( *Intensity.get() );;
    inten->setEffectStartPosition(0);
    inten->setEffectDuration(50001);
    inten->generate(0, std::vector<float>());
    inten->configurationWidget( nullptr) ;

    load();
    m_lorCtrl->setPortParams( m_channelConfigurator->commPortName(), m_channelConfigurator->baudRate() );

    QScreen * primaryScreen = QGuiApplication::primaryScreen();
    QScreen * secondaryScreen = nullptr;
    auto scList = QGuiApplication::screens();
    if ( scList.size() > 1)
    {
       for ( auto screen : scList )
       {
          if ( screen != primaryScreen )
          {
             secondaryScreen = screen;
             break;
          }
       }
    }
    else
    {
       secondaryScreen = primaryScreen;
    }

    move( primaryScreen->geometry().topLeft() );

    setWindowState(Qt::WindowMaximized);
    m_effectConfiguration->setWindowState( Qt::WindowMaximized );
    m_effectConfiguration->move( secondaryScreen->geometry().topLeft() );
    m_effectConfiguration->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint );;
    m_effectConfiguration->show();


}

MainWindow::~MainWindow()
{
   delete m_effectConfiguration;
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

void MainWindow::sequensePlayStarted(std::weak_ptr<CLightSequence> thisObject)
{

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
    m_effectConfiguration->setCurrentSequense( thisObject );
    m_lorCtrl->playStarted( thisObject );

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

        auto widgetPressed = [this, channelConfiguration]()
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
                m_spectrograph->setBarSelected( Spectrograph::NullIndex );
            }

            if ( channelConfiguration->isGainSet() )
            {
                m_spectrograph->setGain( *(channelConfiguration->gain) );
            }
            else
            {
                m_spectrograph->setGain( Spectrograph::NoGain );
            }

            m_spectrograph->setMinimumLevel( channelConfiguration->minimumLevel );
            m_spectrograph->setFading( channelConfiguration->fading );
        };

        auto label = new QLabelEx( channel.label );
        connect( label, &QLabelEx::clicked, widgetPressed );

        ui->tableWidget_2->setCellWidget( i, 0, label );

        auto gainSlider = new QSliderEx( Qt::Horizontal );
        gainSlider->setMaximum( 100 );
        gainSlider->setMinimum( 0 );
        if ( channelConfiguration->isGainSet())
        {
            gainSlider->setValue( 10.0 * (*channelConfiguration->gain) );
        }
        else
        {
            gainSlider->setValue( channel.multipler * 10.0 );
        }
        connect(gainSlider, &QSliderEx::valueChanged, [channelConfiguration, this]( int value ){
            channelConfiguration->setGain( double(value)/10 );
            m_spectrograph->setGain( *(channelConfiguration->gain) );
        });
        connect(gainSlider, &QSliderEx::clicked, widgetPressed );

        ui->tableWidget_2->setCellWidget( i, 1, gainSlider );


        auto threshHold = new QSliderEx( Qt::Horizontal );
        threshHold->setMaximum( 100 );
        threshHold->setMinimum( 0 );

        threshHold->setValue( 100.0 * (channelConfiguration->minimumLevel) );

        connect(threshHold, &QSliderEx::valueChanged, [channelConfiguration, this]( int value ){
            channelConfiguration->minimumLevel =  double(value)/100.0;
            m_spectrograph->setMinimumLevel( channelConfiguration->minimumLevel );
        });
        connect(threshHold, &QSliderEx::clicked, widgetPressed );

        ui->tableWidget_2->setCellWidget( i, 2, threshHold );

        auto fading = new QSliderEx( Qt::Horizontal );
        fading->setMaximum( 100 );
        fading->setMinimum( 0 );

        fading->setValue( 10.0 * (channelConfiguration->fading ) );

        connect(fading, &QSliderEx::valueChanged, [channelConfiguration, this]( int value ){
            channelConfiguration->fading =  double(value)/10.0;
            m_spectrograph->setFading( channelConfiguration->fading );
        });
        connect(fading, &QSliderEx::clicked, widgetPressed );

        ui->tableWidget_2->setCellWidget( i, 3, fading );

    }

}

void MainWindow::closeEvent(QCloseEvent *)
{
   qDebug() << __FUNCTION__;
   persist();
   m_effectConfiguration->close();
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
    }
}


