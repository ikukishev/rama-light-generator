#ifndef CHANNELCONFIGURATOR_H
#define CHANNELCONFIGURATOR_H

#include <QDialog>
#include <QTableWidgetItem>
#include <QAbstractButton>
#include <QString>
#include <vector>

namespace Ui {
class ChannelConfigurator;
}

class Channel
{
public:
    Channel(const QString& alabel,
        const uint32_t& aunit,
        const uint32_t& achannel,
        const uint32_t& avoltage,
        const uint32_t& aspectrumIndex,
        const double& amultipler)
        : label( alabel )
        , unit( aunit )
        , channel( achannel )
        , voltage( avoltage )
        , spectrumIndex( aspectrumIndex )
        , multipler( amultipler )
    {}
    QString label;
    uint32_t unit;
    uint32_t channel;
    uint32_t voltage;
    uint32_t spectrumIndex;
    double multipler;
};

class ChannelConfigurator : public QDialog
{
    Q_OBJECT

public:
    explicit ChannelConfigurator(QWidget *parent = nullptr);
    ~ChannelConfigurator();

    DialogCode display();
    const std::vector<Channel>& channels() const { return m_channels; };

private slots:

    void load();
    void persist();
    void updateTableData();
    void updateChannelsValue();
    void setEnableOkButton(bool isEnabled);

    bool isTableDataValid() const;

    void on_tableWidget_customContextMenuRequested(const QPoint &pos);

    void on_tableWidget_itemChanged(QTableWidgetItem *item);

    void on_buttonBox_clicked(QAbstractButton *button);

private:
    Ui::ChannelConfigurator *ui;
    std::vector<Channel> m_channels;
};

#endif // CHANNELCONFIGURATOR_H
