#include "channelconfigurator.h"
#include "ui_channelconfigurator.h"
#include <QStringList>
#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QComboBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMenu>
#include <QColorDialog>
#include <QPushButton>
#include "widgets/FloatSliderWidget.h"
#include "constants.h"
#include "clightsequence.h"
#include "spectrograph.h"

const std::string configuration("channelConfiguration.json");

// JSON keys
const QString cKeyChannels( "channels" );
const QString cKeyLabel( "label" );
const QString cKeyUnit( "Unit" );
const QString cKeyChannel( "Channel" );
const QString cKeyVoltage( "Voltage" );
const QString cKeySpectrumBarIndex( "SpectrumBarIndex" );
const QString cKeyGain( "Gain" );
const QString cKeyFade( "Fade" );
const QString cKeyColor( "Color" );
const QString cKeyUUID( "uuid" );
const QString cKeyPortName( "commPortName" );
const QString cKeyPortBaudRate( "commPortBaudRate" );

const uint32_t cDefaultBaudRate( 115200 );

constexpr int cColumnIndexLabel   = 0;
constexpr int cColumnIndexUnit    = 1;
constexpr int cColumnIndexChannel = 2;
constexpr int cColumnIndexVoltage = 3;
constexpr int cColumnIndexSpectrumBarIndex = 4;
constexpr int cColumnIndexGain    = 5;
constexpr int cColumnIndexFade = 6;
constexpr int cColumnIndexColor = 7;
constexpr int cColumnIndexUuid = 8;



ChannelConfigurator::ChannelConfigurator(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ChannelConfigurator)
    , m_commPortName()
    , m_baudRate( cDefaultBaudRate )
{
   ui->setupUi(this);
    connect(ui->tableWidget, &QTableWidget::customContextMenuRequested, this, &ChannelConfigurator::on_tableWidget_customContextMenuRequested);

    ui->tableWidget->setColumnCount(9);
    QHeaderView * header = ui->tableWidget->horizontalHeader();

    header->setSectionResizeMode( cColumnIndexLabel, QHeaderView::Stretch);
    header->setSectionResizeMode( cColumnIndexUuid, QHeaderView::Stretch);
    spectrograph = new Spectrograph( );
    spectrograph->setMinimumHeight( 300 );
    connect( spectrograph, &Spectrograph::selectedBarChanged, [this](int index)
    {
        if ( isDisplayed )
        {
            auto currentIndex = ui->tableWidget->currentRow();
            if ( currentIndex >=0 && currentIndex < ui->tableWidget->rowCount() )
            {
                auto spectrumIndexWidget = ui->tableWidget->cellWidget( currentIndex, cColumnIndexSpectrumBarIndex );
                if ( auto combo = dynamic_cast< QComboBox* >( spectrumIndexWidget ) )
                {
                    if ( index >= 0 )
                    {
                        combo->setCurrentIndex(index);
                    }
                }
            }
        }
    } );

    ui->verticalLayout_3->addWidget( spectrograph );
   load();
}

ChannelConfigurator::~ChannelConfigurator()
{
    delete ui;
}

QDialog::DialogCode ChannelConfigurator::display()
{
    updateTableData();
    if ( ui->tableWidget->rowCount() == 0 )
    {
        ui->tableWidget->setRowCount( 1 );

        ui->tableWidget->setItem(       0, cColumnIndexVoltage, new QTableWidgetItem( QString::number(220)) );
        ui->tableWidget->setCellWidget( 0, cColumnIndexColor, prepareColorButton( QColorConstants::Red ));
        ui->tableWidget->setCellWidget( 0, cColumnIndexSpectrumBarIndex, prepareSpectrumCombo( cDefaultSpectrumIndex ));
        ui->tableWidget->setCellWidget( 0, cColumnIndexUuid, prepareUUIDLabel( QUuid::createUuid() ));
        ui->tableWidget->setCellWidget( 0, cColumnIndexGain, new FloatSliderWidget( cMaxGainValue, cMinGainValue, cDefaultGainValue ) );
        ui->tableWidget->setCellWidget( 0, cColumnIndexFade, new FloatSliderWidget( cMaxFadeValue, cMinFadeValue, cDefaultFadeValue ) );
    }
    setEnableOkButton(isTableDataValid());

    isDisplayed = true;
    if ( auto ptr = m_sequense.lock() )
    {
        m_spectrographConnection = connect( ptr.get(), &CLightSequence::positionChanged,
                                                this, &ChannelConfigurator::positionChanged );
    }
    auto result = static_cast< QDialog::DialogCode >( exec() );
    isDisplayed = false;

    return result;
}

void ChannelConfigurator::load()
{
    QFile persistFile( configuration.c_str() ); //configuration.c_str() );
    if ( persistFile.exists() )
    {
        if (!persistFile.open(QIODevice::ReadOnly))
        {
            QMessageBox::warning( this, "Warning", QString("Couldn't open channel configuration file: ") + configuration.c_str() );
            return ;
        }

        QByteArray saveData = persistFile.readAll();
        persistFile.close();

        QJsonDocument loadDoc( QJsonDocument::fromJson(saveData) );
        const QJsonObject &json = loadDoc.object();

        bool isSchemaValid = true;
        if (json.contains( cKeyChannels ))
        {
            QJsonValue channelsValue = json[ cKeyChannels ];
            QJsonArray channelsJson( channelsValue.toArray() );
            std::vector<Channel> channelsTmp;
            channelsTmp.reserve(channelsJson.size());
            qDebug() << "Json channels count: " << channelsJson.size();

            for ( const auto& jsonChannelValue : channelsJson )
            {
                if ( jsonChannelValue.isObject() )
                {
                    const QJsonObject &jsonChannel = jsonChannelValue.toObject();
                    QString label;
                    if ( jsonChannel.contains( cKeyLabel ) )
                    {
                        if ( !jsonChannel[ cKeyLabel ].isString() )
                        {
                            isSchemaValid = false;
                            qWarning() << "Channel '" << cKeyLabel << "' is wrong";
                        }
                        else
                        {
                            label = jsonChannel[ cKeyLabel ].toString();
                        }
                    }

                    uint32_t unit;
                    if ( jsonChannel.contains( cKeyUnit ) )
                    {
                        unit = jsonChannel[ cKeyUnit ].toInt(0);
                        if ( 0 == unit )
                        {
                            isSchemaValid = false;
                            qWarning() << "Unit number '" << cKeyUnit << "' is wrong";
                        }
                    }

                    uint32_t ChannelNumber;
                    if ( jsonChannel.contains( cKeyChannel) )
                    {
                        ChannelNumber = jsonChannel[ cKeyChannel ].toInt(0);
                        if ( 0 == ChannelNumber )
                        {
                            isSchemaValid = false;
                            qWarning() << "Channel number '" << cKeyChannel << "' is wrong";
                        }
                    }

                    uint32_t Voltage;
                    if ( jsonChannel.contains( cKeyVoltage ) )
                    {
                        Voltage = jsonChannel[ cKeyVoltage ].toInt(0);
                        if ( 0 == Voltage )
                        {
                            isSchemaValid = false;
                            qWarning() << "Voltage number '" << cKeyVoltage << "' is wrong";
                        }
                    }

                    uint32_t SpectrumBarIndex;
                    if ( jsonChannel.contains( cKeySpectrumBarIndex ) )
                    {
                        SpectrumBarIndex = jsonChannel[ cKeySpectrumBarIndex ].toInt(0);
                        if ( 0 == SpectrumBarIndex )
                        {
                            isSchemaValid = false;
                            qWarning() << "Spectrum bar index number '" << cKeySpectrumBarIndex << "' is wrong";
                        }
                    }

                    double Gain;
                    if ( jsonChannel.contains( cKeyGain ) )
                    {
                        if ( !jsonChannel[ cKeyGain ].isDouble() )
                        {
                            Gain = 1.0;
                            qWarning() << "Channel Multipler '" << cKeyGain << "' is wrong";
                        }
                        else
                        {
                            Gain = jsonChannel[ cKeyGain ].toDouble();
                        }
                    }


                    double Fade;
                    if ( jsonChannel.contains( cKeyFade ) )
                    {
                        if ( !jsonChannel[ cKeyFade ].isDouble() )
                        {
                            Fade = 1.0;
                            qWarning() << "Channel Fade '" << cKeyFade << "' is wrong";
                        }
                        else
                        {
                            Fade = jsonChannel[ cKeyFade ].toDouble();
                        }
                    }

                    QString color;
                    if ( jsonChannel.contains( cKeyColor ) )
                    {
                        if ( !jsonChannel[ cKeyColor ].isString() )
                        {
                            isSchemaValid = false;
                            qWarning() << "Channel '" << cKeyColor << "' is wrong";
                        }
                        else
                        {
                            color = jsonChannel[ cKeyColor ].toString();
                        }
                    }

                    QUuid uuid;
                    if ( jsonChannel.contains( cKeyUUID ) )
                    {
                        if ( !jsonChannel[ cKeyUUID ].isString() )
                        {
                            uuid = QUuid::createUuid();
                            qWarning() << "Channel '" << cKeyUUID << "' is wrong, new generated"<< uuid;
                        }
                        else
                        {
                            uuid = jsonChannel[ cKeyUUID ].toString();
                        }
                    }

                    channelsTmp.emplace_back(label, unit, ChannelNumber, Voltage, SpectrumBarIndex, Gain, Fade, color, uuid);

                }
                else
                {
                    qDebug() << "channel is not object";
                    isSchemaValid = false;
                }
            }

            m_channels = std::move(channelsTmp);

            qDebug() << "channels count: " << m_channels.size();

        }

        if ( json.contains( cKeyPortName ) )
        {
            if ( !json[ cKeyPortName ].isString() )
            {
                qWarning() << "Channel '" << cKeyPortName << "' is wrong";
            }
            else
            {
                m_commPortName = json[ cKeyPortName ].toString();
            }
        }

        if ( json.contains( cKeyPortBaudRate ) )
        {
            m_baudRate = json[ cKeyPortBaudRate ].toInt( cDefaultBaudRate );
        }


        if ( !isSchemaValid )
        {
            QMessageBox::warning( this, "Warning", QString("Invalid configuration file schema: ") + configuration.c_str() );
        }

    }
    else
    {
        qWarning() << "File not exist:" << configuration.c_str();
    }
}

void ChannelConfigurator::persist()
{
    qDebug() << "Accep role found persist()";
    QFile persistFile(configuration.c_str());
    if (!persistFile.open(QIODevice::WriteOnly))
    {
        QMessageBox::warning( this, "Warning", QString("Couldn't write channel configuration to file: ") + configuration.c_str() );
        return ;
    }

    QJsonArray jsonChannelsArray;

    for ( const auto& channel : m_channels )
    {
        QJsonObject jsonObject;
        jsonObject[ cKeyLabel ] = channel.label;
        jsonObject[ cKeyUnit ] = static_cast<int>(channel.unit);
        jsonObject[ cKeyChannel ] = static_cast<int>(channel.channel);
        jsonObject[ cKeyVoltage ] = static_cast<int>(channel.voltage);
        jsonObject[ cKeySpectrumBarIndex ] = static_cast<int>(channel.spectrumIndex);
        jsonObject[ cKeyGain ] = channel.gain;
        jsonObject[ cKeyFade ] = channel.fade;
        jsonObject[ cKeyColor ] = channel.color;
        jsonObject[ cKeyUUID ] = channel.uuid.toString();
        jsonChannelsArray.append(jsonObject);
    }

    QJsonObject jsonObject;
    jsonObject[ cKeyPortName ] = m_commPortName;
    jsonObject[ cKeyPortBaudRate ] = static_cast<int>(m_baudRate);
    jsonObject[ cKeyChannels ] = jsonChannelsArray;

    persistFile.write( QJsonDocument(jsonObject).toJson() );

}

void ChannelConfigurator::updateTableData()
{
   ui->tableWidget->clear();
   QStringList labels;
   labels << "Label";
   labels << "Unit";
   labels << "Channel";
   labels << "Voltage";
   labels << "SpectrumBar index";
   labels << "Gain";
   labels << "Fade";
   labels << "Color";
   labels << "UUID";

    ui->tableWidget->setHorizontalHeaderLabels(labels);
    ui->tableWidget->setRowCount(m_channels.size());

    for ( int rowIndex = 0; rowIndex < (int)m_channels.size(); ++rowIndex)
    {
        ui->tableWidget->setItem(rowIndex, cColumnIndexLabel, new QTableWidgetItem( m_channels[rowIndex].label) );

        if (m_channels[rowIndex].unit > 0)
            ui->tableWidget->setItem(rowIndex, cColumnIndexUnit, new QTableWidgetItem( QString::number(m_channels[rowIndex].unit)) );

        if (m_channels[rowIndex].channel > 0)
            ui->tableWidget->setItem(rowIndex, cColumnIndexChannel, new QTableWidgetItem( QString::number(m_channels[rowIndex].channel)) );

        if (m_channels[rowIndex].voltage > 0)
        {
            ui->tableWidget->setItem(rowIndex, cColumnIndexVoltage, new QTableWidgetItem( QString::number(m_channels[rowIndex].voltage)) );
        }
        else
        {
            ui->tableWidget->setItem(rowIndex, cColumnIndexVoltage, new QTableWidgetItem( QString::number(220) ) );
        }

        ui->tableWidget->setCellWidget( rowIndex, cColumnIndexSpectrumBarIndex, prepareSpectrumCombo( m_channels[rowIndex].spectrumIndex ) );
        ui->tableWidget->setCellWidget( rowIndex, cColumnIndexGain, new FloatSliderWidget( cMaxGainValue, cMinGainValue, m_channels[rowIndex].gain ) );
        ui->tableWidget->setCellWidget( rowIndex, cColumnIndexFade, new FloatSliderWidget( cMaxFadeValue, cMinFadeValue, m_channels[rowIndex].fade ) );
        ui->tableWidget->setCellWidget( rowIndex, cColumnIndexColor, prepareColorButton( m_channels[rowIndex].color ) );
        ui->tableWidget->setCellWidget( rowIndex, cColumnIndexUuid, prepareUUIDLabel( m_channels[rowIndex].uuid ) );

    }

    ui->commBaudrate->setText( QString::number( m_baudRate ) );
    ui->commPortNameEdit->setText( m_commPortName );
}

void ChannelConfigurator::updateChannelsValue()
{
    std::vector<Channel> channelsTmp;
    channelsTmp.reserve( ui->tableWidget->rowCount() );

    auto getText = [this]( int rowIndex, int colIndex, QString& text) -> bool
    {
        auto itemPtr = ui->tableWidget->item( rowIndex, colIndex );

        bool isEmpty = true;
        if ( nullptr != itemPtr )
        {
            auto t = itemPtr->text().trimmed();
            if ( t.size() > 0 )
            {
                text = t;
                isEmpty = false;
            }
        }
        if ( isEmpty )
        {
            QMessageBox::warning( this, "Warning", QString("col: ").append( QString::number(colIndex) )
                                  .append(" row: ").append( QString::number( rowIndex ) )
                                  .append(" is empty (nullptr) getText")  );
        }
        return !isEmpty;
    };

    auto getInt = [ this, &getText ] ( int rowIndex, int colIndex, uint32_t& val) -> bool
    {
        QString  text;
        bool result = false;
        if ( !getText( rowIndex, colIndex, text ) )
        {
            return result;
        }

        int value = text.toInt( &result );
        if ( result )
        {
            val = static_cast<uint32_t>( value );
        }
        else
        {
            QMessageBox::warning( this, "Warning", QString("col: ").append( QString::number(colIndex) )
                                  .append(" row: ").append(  QString::number( rowIndex )  )
                                  .append(" wrong integer number. must be > 0")  );
        }
        return result;
    };

    for ( int rowIndex = 0; rowIndex < ui->tableWidget->rowCount(); ++rowIndex)
    {

        QString  label;
        if ( !getText( rowIndex, cColumnIndexLabel, label) )
        {
            continue;
        }

        uint32_t unit;
        if ( !getInt( rowIndex, cColumnIndexUnit, unit) )
        {
            continue;
        }

        uint32_t ChannelNumber;
        if ( !getInt( rowIndex, cColumnIndexChannel, ChannelNumber) )
        {
            continue;
        }

        uint32_t Voltage;
        if ( !getInt( rowIndex, cColumnIndexVoltage, Voltage) )
        {
            continue;
        }

        QComboBox* combo = (QComboBox*)ui->tableWidget->cellWidget(rowIndex, cColumnIndexSpectrumBarIndex);
        uint32_t SpectrumBarIndex = combo->currentIndex();

        FloatSliderWidget* gainSlider = (FloatSliderWidget*)ui->tableWidget->cellWidget(rowIndex, cColumnIndexGain);
        double   Gain = gainSlider->value();

        FloatSliderWidget* fadeSlider = (FloatSliderWidget*)ui->tableWidget->cellWidget(rowIndex, cColumnIndexFade);
        double   Fade = fadeSlider->value();

        QPushButton* buttonPtr = (QPushButton*)ui->tableWidget->cellWidget(rowIndex, cColumnIndexColor);
        auto color = buttonPtr->text();

        QLabel* labelPtr = (QLabel*)ui->tableWidget->cellWidget(rowIndex, cColumnIndexUuid);
        auto uuid = labelPtr->text();

        channelsTmp.emplace_back( label, unit, ChannelNumber, Voltage, SpectrumBarIndex, Gain, Fade, color, uuid );
        qDebug() << "label" << label
                 << "unit"<< unit
                 << "ChannelNumber" << ChannelNumber
                 << "Voltage" << Voltage
                 << "SpectrumBarIndex" << SpectrumBarIndex
                 << "Gain" << Gain
                 << "Fade" << Fade
                 << "Color" << color
                 << "Uuid" << uuid;
    }

    m_channels = std::move(channelsTmp);

    m_commPortName = ui->commPortNameEdit->text();
    bool isSucess = true;
    m_baudRate = ui->commBaudrate->text().toInt(&isSucess);
    if ( !isSucess )
    {
        m_baudRate = cDefaultBaudRate;
    }

}

void ChannelConfigurator::setEnableOkButton(bool isEnabled)
{
    for ( auto button : ui->buttonBox->buttons() )
    {
        if ( QDialogButtonBox::AcceptRole == ui->buttonBox->buttonRole(button) )
        {
            button->setEnabled( isEnabled );
            qDebug() << "Ok button => " << (isEnabled?"enabled":"disabled");
            break;
        }
    }
}

QPushButton *ChannelConfigurator::prepareColorButton(const QColor &defaultColor)
{
   auto selectColorButton = new QPushButton();
   auto setColor = [selectColorButton](const QColor& color) {
      QString qss = QString( "background-color: %1" ).arg( color.name() );
      selectColorButton->setStyleSheet( qss );
      selectColorButton->setText( color.name() );
   };

   setColor( defaultColor );

   connect( selectColorButton, &QPushButton::clicked, [this, selectColorButton, setColor](){
       QColor color = QColorDialog::getColor(QColor(selectColorButton->text()), this );
       if( color.isValid() )
       {
          setColor( color );
       }
   });
   return selectColorButton;
}

QComboBox *ChannelConfigurator::prepareSpectrumCombo(int defaultValue)
{
   QComboBox *combo = new QComboBox();

   int hzPerFft = double(cMaxFrequensy)/cFFTSize;
   for ( int fftIndex = 0; fftIndex < cFFTSize; ++fftIndex )
   {
      int bbondary = fftIndex*hzPerFft;
      int tbondary = bbondary+hzPerFft;
      QString item = QString::number(fftIndex) + " (" +
            QString::number(bbondary) + " - " + QString::number(tbondary)  + ")";
      combo->addItem(item);
   }

   if ( defaultValue >=0 || defaultValue < cFFTSize )
   {
      combo->setCurrentIndex( defaultValue );
   }
   else
   {
      combo->setCurrentIndex( cDefaultSpectrumIndex );
   }

   return combo;
}

QLabel *ChannelConfigurator::prepareUUIDLabel(const QUuid &uuid)
{
   return new QLabel(uuid.toString());
}

bool ChannelConfigurator::isTableDataValid() const
{
    bool isValid = true;

    for ( int rowIndex = 0; rowIndex < ui->tableWidget->rowCount(); ++rowIndex)
    {
        for ( int colIndex = 0; colIndex < ui->tableWidget->columnCount(); ++colIndex)
        {
            auto itemPtr = ui->tableWidget->item( rowIndex, colIndex );

            if ( nullptr == itemPtr )
            {
                auto widgetPtr = ui->tableWidget->cellWidget(rowIndex, colIndex);
                if ( nullptr == widgetPtr )
                {
                   isValid = false;
                   qDebug() << "col:" << colIndex << "row:" << rowIndex << " is empty (nullptr)";
                }
                continue;
            }

            auto item = itemPtr->text().trimmed();

            if ( item.size()<=0 )
            {
                isValid = false;
                qDebug() << "col:" << colIndex << "row:" << rowIndex << " is empty";
                continue;
            }

            switch (colIndex)
            {
            case cColumnIndexLabel: break;
            case cColumnIndexFade: break;
            case cColumnIndexGain: break;
            case cColumnIndexColor: break;
            case cColumnIndexUuid: break;
            case cColumnIndexSpectrumBarIndex: break;

            default:
            {
                bool isIntValid = false;
                int val = item.toInt(&isIntValid);
                if ( !isIntValid || !(val > 0) )
                {
                    isValid = false;
                    qDebug() << "col:" << colIndex << "row:" << rowIndex << " Any integer value must be greater that '0'";
                }
                break;
            }

            }
        }
    }

    return isValid;
}

void ChannelConfigurator::on_tableWidget_customContextMenuRequested( const QPoint &pos )
{

    qDebug() << "Test Test";
    QAction* newAct = new QAction(tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Insert a new row"));

    connect(newAct, &QAction::triggered, [this]()
    {
       int index = 0;
        if ( ui->tableWidget->rowCount() == 0 )
        {
           ui->tableWidget->setRowCount(1);
        }
        else
        {
           index = ui->tableWidget->currentRow();
           ui->tableWidget->insertRow( index );
        }
        ui->tableWidget->setItem(       index, cColumnIndexVoltage, new QTableWidgetItem( QString::number(220)) );
        ui->tableWidget->setCellWidget( index, cColumnIndexColor, prepareColorButton( QColorConstants::Red ));
        ui->tableWidget->setCellWidget( index, cColumnIndexSpectrumBarIndex, prepareSpectrumCombo( cDefaultSpectrumIndex ));
        ui->tableWidget->setCellWidget( index, cColumnIndexUuid, prepareUUIDLabel( QUuid::createUuid() ));
        ui->tableWidget->setCellWidget( index, cColumnIndexGain, new FloatSliderWidget( cMaxGainValue, cMinGainValue, cDefaultGainValue ) );
        ui->tableWidget->setCellWidget( index, cColumnIndexFade, new FloatSliderWidget( cMaxFadeValue, cMinFadeValue, cDefaultFadeValue ) );
        setEnableOkButton(isTableDataValid());
    });

    QAction* delAct = new QAction(tr("&Delete"), this);
    delAct->setShortcuts(QKeySequence::Delete);
    delAct->setStatusTip(tr("Delete row"));

    connect( delAct, &QAction::triggered, [ this ]() {

        if ( ui->tableWidget->rowCount() > 0
             && ui->tableWidget->currentRow() >= 0
             && ui->tableWidget->currentRow() < ui->tableWidget->rowCount() )
        {
            qDebug() << "Delete";
            int index = ui->tableWidget->currentRow();

            auto widget = ui->tableWidget->cellWidget(index, cColumnIndexColor);
            if ( widget ) { delete widget; }

            widget = ui->tableWidget->cellWidget(index, cColumnIndexSpectrumBarIndex);
            if ( widget ) { delete widget; }

            widget = ui->tableWidget->cellWidget(index, cColumnIndexUuid);
            if ( widget ) { delete widget; }

            widget = ui->tableWidget->cellWidget(index, cColumnIndexGain);
            if ( widget ) { delete widget; }

            widget = ui->tableWidget->cellWidget(index, cColumnIndexFade);
            if ( widget ) { delete widget; }

            ui->tableWidget->removeRow( ui->tableWidget->currentRow() );
        }
        setEnableOkButton(isTableDataValid());
    });

    QMenu menu(this);
    menu.addAction(newAct);
    menu.addAction(delAct);

    menu.exec( QCursor::pos() );
    delete newAct;
    delete delAct;

}

void ChannelConfigurator::on_tableWidget_itemChanged( QTableWidgetItem * )
{
    setEnableOkButton(isTableDataValid());
}

void ChannelConfigurator::on_buttonBox_clicked(QAbstractButton *button)
{
    if ( QDialogButtonBox::AcceptRole == ui->buttonBox->buttonRole(button))
    {
        if ( isTableDataValid() )
        {
            updateChannelsValue();
            persist();
        }
    }
}

uint32_t ChannelConfigurator::baudRate() const
{
    return m_baudRate;
}

void ChannelConfigurator::sequensePlayStarted(std::weak_ptr<CLightSequence> sequense)
{
    m_sequense = std::move(sequense);

    if ( auto ptr = m_sequense.lock() )
    {
        if ( isDisplayed )
        {
            m_spectrographConnection = connect( ptr.get(), &CLightSequence::positionChanged,
                                                this, &ChannelConfigurator::positionChanged );
        }
    }
    else
    {
        disconnect(m_spectrographConnection);
    }
}

void ChannelConfigurator::positionChanged(const SpectrumData &spectrum)
{
    if ( isDisplayed )
    {
        auto currentIndex = ui->tableWidget->currentRow();
        if ( currentIndex >=0 && currentIndex < ui->tableWidget->rowCount() )
        {

            auto spectrumIndexWidget = ui->tableWidget->cellWidget( currentIndex, cColumnIndexSpectrumBarIndex );
            if ( auto combo = dynamic_cast< QComboBox* >( spectrumIndexWidget ) )
            {
                auto index = combo->currentIndex();
                if ( index > 0 )
                {
                    spectrograph->setBarSelected(index);
                }
            }

            auto gainWidget = ui->tableWidget->cellWidget( currentIndex, cColumnIndexGain );
            if ( auto gain = dynamic_cast< FloatSliderWidget* >( gainWidget ) )
            {
                spectrograph->setGain(gain->value());
            }

            auto fadeWidget = ui->tableWidget->cellWidget( currentIndex, cColumnIndexFade );
            if ( auto fade = dynamic_cast< FloatSliderWidget* >( fadeWidget ) )
            {
                spectrograph->setFading(fade->value());
            }

        }

        spectrograph->spectrumChanged(spectrum);
    }
    else
    {
        disconnect(m_spectrographConnection);
    }
}

const QString &ChannelConfigurator::commPortName() const
{
    return m_commPortName;
}
