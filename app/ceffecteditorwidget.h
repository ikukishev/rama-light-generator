#ifndef CEFFECTEDITORWIDGET_H
#define CEFFECTEDITORWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include "timeline/CTimeLineView.h"
#include "clightsequence.h"

class CEffectEditorWidget : public QWidget
{
   Q_OBJECT
public:
   explicit CEffectEditorWidget(QWidget *parent = nullptr);



   void setCurrentSequense( std::weak_ptr<CLightSequence> sequense );

signals:


private:
   CTimeLineView* timeline = nullptr;
   QWidget* configurationArea = nullptr;
   QVBoxLayout* configurationAreaLayout = nullptr;

   QWidget* configurationWidget = nullptr;
   QUuid configurationWidgetEffectUuid;


   std::list<std::shared_ptr<QMetaObject::Connection>> m_spectrumConnections;

   std::weak_ptr<CLightSequence> currentSequense;

};

#endif // CEFFECTEDITORWIDGET_H
