#ifndef LABELEX_H
#define LABELEX_H

#include <QLabel>

class LabelEx : public QLabel
{
    Q_OBJECT
public:
    explicit LabelEx(QWidget *parent=nullptr, Qt::WindowFlags f=Qt::WindowFlags())
        : QLabel( parent, f )
    {}
    explicit LabelEx(const QString &text, QWidget *parent=nullptr, Qt::WindowFlags f=Qt::WindowFlags())
        : QLabel( text, parent, f )
    {}

signals:
    void clicked();

protected:

    virtual void mousePressEvent(QMouseEvent* event) override;

};


#endif // LABELEX_H
