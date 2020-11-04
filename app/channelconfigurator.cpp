#include "channelconfigurator.h"
#include "ui_channelconfigurator.h"
#include <QStringList>
#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMenu>
#include <QColorDialog>
#include <QPushButton>

const std::string configuration("channelConfiguration.json");

// JSON keys
const QString cKeyChannels( "channels" );
const QString cKeyLabel( "label" );
const QString cKeyUnit( "Unit" );
const QString cKeyChannel( "Channel" );
const QString cKeyVoltage( "Voltage" );
const QString cKeySpectrumBarIndex( "SpectrumBarIndex" );
const QString cKeyMultipler( "Multipler" );
const QString cKeyColor( "Color" );
const QString cKeyUUID( "uuid" );
const QString cKeyPortName( "commPortName" );
const QString cKeyPortBaudRate( "commPortBaudRate" );

const uint32_t cDefaultBaudRate( 115200 );



ChannelConfigurator::ChannelConfigurator(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ChannelConfigurator)
    , m_commPortName()
    , m_baudRate( cDefaultBaudRate )
{
   ui->setupUi(this);
    connect(ui->tableWidget, &QTableWidget::customContextMenuRequested, this, &ChannelConfigurator::on_tableWidget_customContextMenuRequested);
    QHeaderView * header = ui->tableWidget->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::Stretch);
    header->setSectionResizeMode(7, QHeaderView::Stretch);
   load();
}

ChannelConfigurator::~ChannelConfigurator()
{
    delete ui;
}

QDialog::DialogCode ChannelConfigurator::display()
{
    updateTableData();
    if ( ui->tableWidget->rowCount() == 0)
    {
        ui->tableWidget->setRowCount(1);
        ui->tableWidget->setCellWidget(0, 6, prepareColorButton( QColorConstants::Red ));
        ui->tableWidget->setCellWidget(0, 7, prepareUUIDLabel( QUuid::createUuid() ));
    }
    setEnableOkButton(isTableDataValid());
    auto result = static_cast< QDialog::DialogCode >( exec() );

//    for ( int rowIndex = 0; rowIndex < (int)m_channels.size(); ++rowIndex)
//    {
//       auto button = ui->tableWidget->cellWidget(rowIndex, 6);
//       if ( button)
//       {
//          delete button;
//       }
//       auto label = ui->tableWidget->cellWidget(rowIndex, 7);
//       if ( label )
//       {
//          delete label;
//       }

//    }

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

                    double Multipler;
                    if ( jsonChannel.contains( cKeyMultipler ) )
                    {
                        if ( !jsonChannel[ cKeyMultipler ].isDouble() )
                        {
                            Multipler = 1.0;
                            qWarning() << "Channel Multipler '" << cKeyMultipler << "' is wrong";
                        }
                        else
                        {
                            Multipler = jsonChannel[ cKeyMultipler ].toDouble();
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

                    channelsTmp.emplace_back(label, unit, ChannelNumber, Voltage, SpectrumBarIndex, Multipler, color, uuid);

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
        jsonObject[ cKeyMultipler ] = channel.multipler;
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
   labels << "Multipler";
   labels << "Color";
   labels << "UUID";

    ui->tableWidget->setHorizontalHeaderLabels(labels);
    ui->tableWidget->setRowCount(m_channels.size());

    for ( int rowIndex = 0; rowIndex < (int)m_channels.size(); ++rowIndex)
    {
        ui->tableWidget->setItem(rowIndex, 0, new QTableWidgetItem( m_channels[rowIndex].label) );

        if (m_channels[rowIndex].unit > 0)
            ui->tableWidget->setItem(rowIndex, 1, new QTableWidgetItem( QString::number(m_channels[rowIndex].unit)) );

        if (m_channels[rowIndex].channel > 0)
            ui->tableWidget->setItem(rowIndex, 2, new QTableWidgetItem( QString::number(m_channels[rowIndex].channel)) );

        if (m_channels[rowIndex].voltage > 0)
            ui->tableWidget->setItem(rowIndex, 3, new QTableWidgetItem( QString::number(m_channels[rowIndex].voltage)) );

        if (m_channels[rowIndex].spectrumIndex > 0)
            ui->tableWidget->setItem(rowIndex, 4, new QTableWidgetItem( QString::number(m_channels[rowIndex].spectrumIndex)) );

        qDebug() << "rowIndex:" << rowIndex << "Multipler:" << m_channels[rowIndex].multipler;
        ui->tableWidget->setItem(rowIndex, 5, new QTableWidgetItem( QString::number(m_channels[rowIndex].multipler)) );

        ui->tableWidget->setCellWidget( rowIndex, 6, prepareColorButton( m_channels[rowIndex].color ) );
        ui->tableWidget->setCellWidget( rowIndex, 7, prepareUUIDLabel( m_channels[rowIndex].uuid ) );

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

    auto getDouble = [ this, &getText ] ( int rowIndex, int colIndex, double& val) -> bool
    {
        QString  text;
        bool result = false;
        if ( !getText( rowIndex, colIndex, text ) )
        {
            return result;
        }

        double value = text.toDouble( &result );
        if ( result )
        {
            val = value;
        }
        else
        {
            QMessageBox::warning( this, "Warning", QString("col: ").append( QString::number(colIndex) )
                                  .append(" row: ").append(  QString::number( rowIndex )  )
                                  .append(" wrong float value. must be > 0")  );
        }
        return result;
    };

    for ( int rowIndex = 0; rowIndex < ui->tableWidget->rowCount(); ++rowIndex)
    {

        QString  label;
        if ( !getText( rowIndex, 0, label) )
        {
            continue;
        }

        uint32_t unit;
        if ( !getInt( rowIndex, 1, unit) )
        {
            continue;
        }

        uint32_t ChannelNumber;
        if ( !getInt( rowIndex, 2, ChannelNumber) )
        {
            continue;
        }

        uint32_t Voltage;
        if ( !getInt( rowIndex, 3, Voltage) )
        {
            continue;
        }

        uint32_t SpectrumBarIndex;
        if ( !getInt( rowIndex, 4, SpectrumBarIndex) )
        {
            continue;
        }

        double   Multipler;
        if ( !getDouble( rowIndex, 5, Multipler) )
        {
            continue;
        }

        QPushButton* buttonPtr = (QPushButton*)ui->tableWidget->cellWidget(rowIndex, 6);
        auto color = buttonPtr->text();

        QLabel* labelPtr = (QLabel*)ui->tableWidget->cellWidget(rowIndex, 7);
        auto uuid = labelPtr->text();

        channelsTmp.emplace_back( label, unit, ChannelNumber, Voltage, SpectrumBarIndex, Multipler, color, uuid );
        qDebug() << "label" << label
                 << "unit"<< unit
                 << "ChannelNumber" << ChannelNumber
                 << "Voltage" << Voltage
                 << "SpectrumBarIndex" << SpectrumBarIndex
                 << "Multipler" << Multipler
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
        if ( QDialogButtonBox::AcceptRole == ui->buttonBox->buttonRole(button))
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
            case 0: break;
            case 5:
            {
                bool isMultiplerValid = false;
                double mult = item.toDouble( &isMultiplerValid );
                if ( !isMultiplerValid || !(mult > 0.0) )
                {
                    isValid = false;
                    qDebug() << "col:" << colIndex << "row:" << rowIndex << " Multipler value must be greater that '0'";
                }
                break;
            }
            case 6: break;
            case 7: break;

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
        ui->tableWidget->setCellWidget(index, 6, prepareColorButton( QColorConstants::Red ));
        ui->tableWidget->setCellWidget(index, 7, prepareUUIDLabel( QUuid::createUuid() ));
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
            auto button = ui->tableWidget->cellWidget(index, 6);
            if ( button)
            {
               delete button;
            }
            auto label = ui->tableWidget->cellWidget(index, 7);
            if ( label )
            {
               delete label;
            }
            ui->tableWidget->removeRow( ui->tableWidget->currentRow() );
        }
        setEnableOkButton(isTableDataValid());
    });

    QMenu menu(this);
    menu.addAction(newAct);
    menu.addAction(delAct);

    menu.exec( this->pos() + pos );
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


