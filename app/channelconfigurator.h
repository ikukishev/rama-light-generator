#ifndef CHANNELCONFIGURATOR_H
#define CHANNELCONFIGURATOR_H

#include <QDialog>
#include <QTableWidgetItem>
#include <QAbstractButton>
#include <QString>
#include <QLabel>
#include <vector>
#include <CConfiguration.h>

namespace Ui {
class ChannelConfigurator;
}

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

    QPushButton *prepareColorButton( const QColor& defaultColor );
    QLabel *prepareUUIDLabel(const QUuid& uuid );

    bool isTableDataValid() const;

    void on_tableWidget_customContextMenuRequested(const QPoint &pos);

    void on_tableWidget_itemChanged(QTableWidgetItem *);

    void on_buttonBox_clicked(QAbstractButton *button);

private:
    Ui::ChannelConfigurator *ui;
    std::vector<Channel> m_channels;
    QString   m_commPortName;
    uint32_t  m_baudRate;
};

#endif // CHANNELCONFIGURATOR_H
