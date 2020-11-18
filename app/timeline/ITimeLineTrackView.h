#ifndef CTIMELINEVIEW_H
#define CTIMELINEVIEW_H

#include <memory>
#include <QObject>
#include <QPoint>

#include <QGraphicsItem>
#include <QString>
#include <QUuid>

#include "IEffectGenerator.h"


constexpr int cFieldMargin = 10;

class ITimeLineChannel;
class ITimeLineTrackView;
class IEffect;

class ITimeLineChannel
      : public QObject
      , public QGraphicsItem
{
   Q_OBJECT

   friend class IEffect;

public:
   ITimeLineChannel( QObject* parent = nullptr ) : QObject(parent)
   {};

   virtual ~ITimeLineChannel();

   virtual QColor color() const = 0;
   virtual ITimeLineTrackView* timeLinePtr() const = 0;

   virtual const QString& uuid() const = 0;
   virtual const QString& label() const = 0;
   std::list<IEffect *> effects() const;

   void removeEffect( IEffect * effect );

   void updateEffectPositions();

signals:
   void effectRemoved( ITimeLineChannel* tlChannel, QUuid uuid );
   void effectAdded( ITimeLineChannel* tlChannel, IEffect* effect );
   void effectChanged( ITimeLineChannel* tlChannel, IEffect* effect );
   void effectSelected( ITimeLineChannel* tlChannel, IEffect* effect );
   void effectDoubleClick( ITimeLineChannel* tlChannel, IEffect* effect );

protected:
   void effectChangedEvent( IEffect* effect );
   void effectSelectedEvent( IEffect* effect );

};


class IEffect : public QObject
             , public QGraphicsItem
{
   Q_OBJECT
public:

   IEffect( std::shared_ptr<IEffectGenerator> effectGenerator, QObject * parent = nullptr )
      : QObject( parent )
      , m_effectGenerator( effectGenerator )
   {}

   virtual ~IEffect() = default;

   const QString& effectNameLabel() const {  return m_effectGenerator->effectNameLabel();  }
   void setEffectNameLabel(const QString &ef) {  m_effectGenerator->setEffectNameLabel( ef );  }

   int64_t effectDuration() const         {  return m_effectGenerator->effectDuration();  }
   void setEffectDuration(const int64_t &eD);

   int64_t effectStartPosition() const    {  return m_effectGenerator->effectStartPosition();  }
   void setEffectStartPosition(const int64_t &SP)  {  m_effectGenerator->setEffectStartPosition( SP );  }

   ITimeLineChannel *getChannel() const;

   void updatePosition();

   const QUuid& getUuid() const  { return   m_effectGenerator->getUuid(); }

   std::shared_ptr<IEffectGenerator> getEffectGenerator() const;

signals:
   void clicked( ITimeLineChannel *channel, IEffect* track );

protected:
   void effectChanged();
   void effectsSelected();

private:
   std::shared_ptr<IEffectGenerator> m_effectGenerator;
};


class ITimeLineTrackView
{
public:
   ITimeLineTrackView() = default;
   virtual ~ITimeLineTrackView() = default;

   virtual uint32_t channelHeight() const = 0;
   virtual uint64_t channelLabelWidth() const = 0;

   virtual int64_t compositionPosition() const = 0;
   virtual int64_t compositionDuration() const = 0;

   virtual qreal    convertPositionToSceneX() const = 0;
   virtual qreal    convertPositionToSceneX( int64_t position ) const = 0;
   virtual int64_t convertSceneXToPosition( qreal x ) const = 0;

   virtual ITimeLineChannel* getNeiborChannel( ITimeLineChannel* channel, int offsetIndex ) const = 0;

};


#endif
