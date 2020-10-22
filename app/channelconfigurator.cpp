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

const std::string configuration("channelConfiguration.json");

// JSON keys
const QString &cKeyChannels( "channels" );
const QString &cKeyLabel( "label" );
const QString &cKeyUnit( "Unit" );
const QString &cKeyChannel( "Channel" );
const QString &cKeyVoltage( "Voltage" );
const QString &cKeySpectrumBarIndex( "SpectrumBarIndex" );
const QString &cKeyMultipler( "Multipler" );



ChannelConfigurator::ChannelConfigurator(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ChannelConfigurator)
{
   ui->setupUi(this);
    connect(ui->tableWidget, &QTableWidget::customContextMenuRequested, this, &ChannelConfigurator::on_tableWidget_customContextMenuRequested);
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
    }
    setEnableOkButton(isTableDataValid());
    return static_cast< QDialog::DialogCode >( exec() );
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
                            isSchemaValid = false;
                            qWarning() << "Channel Multipler '" << cKeyMultipler << "' is wrong";
                        }
                        else
                        {
                            Multipler = jsonChannel[ cKeyMultipler ].toDouble();
                        }
                    }

                    channelsTmp.emplace_back(label, unit, ChannelNumber, Voltage, SpectrumBarIndex, Multipler);

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
        jsonChannelsArray.append(jsonObject);
    }

    QJsonObject jsonObjectChannels;
    jsonObjectChannels[ cKeyChannels ] = jsonChannelsArray;

    persistFile.write( QJsonDocument(jsonObjectChannels).toJson() );

}

void ChannelConfigurator::updateTableData()
{
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

    }
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

        channelsTmp.emplace_back( label, unit, ChannelNumber, Voltage, SpectrumBarIndex, Multipler );
        qDebug() << "label" << label
                 << "unit"<< unit
                 << "ChannelNumber" << ChannelNumber
                 << "Voltage" << Voltage
                 << "SpectrumBarIndex" << SpectrumBarIndex
                 << "Multipler" << Multipler;
    }

    m_channels = std::move(channelsTmp);

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
                isValid = false;
                qDebug() << "col:" << colIndex << "row:" << rowIndex << " is empty (nullptr)";
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
        if ( ui->tableWidget->rowCount() == 0 )
        {
            ui->tableWidget->setRowCount(1);
        }
        else
        {
            ui->tableWidget->insertRow( ui->tableWidget->currentRow() );
        }
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

void ChannelConfigurator::on_tableWidget_itemChanged( QTableWidgetItem *item )
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


