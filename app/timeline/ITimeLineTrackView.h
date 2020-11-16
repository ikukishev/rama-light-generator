#ifndef CTIMELINEVIEW_H
#define CTIMELINEVIEW_H

#include <memory>
#include <QObject>
#include <QPoint>

#include <QGraphicsItem>
#include <QString>
#include <QUuid>

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
   ITimeLineChannel( QObject* parent = nullptr ) : QObject(parent) {};
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

   IEffect( const QUuid& uuid, QObject * parent = nullptr )
      : QObject( parent )
      , m_uuid( uuid )
   {}

   IEffect( QObject * parent = nullptr );

   virtual ~IEffect() = default;

   const QString& effectNameLabel() const {  return m_effectNameLabel;  }
   void setEffectNameLabel(const QString &ef) {  m_effectNameLabel = ef;  }

   int64_t effectDuration() const         {  return m_effectDuration;  }
   void setEffectDuration(const int64_t &eD);

   int64_t effectStartPosition() const    {  return m_effectStartPosition;  }
   void setEffectStartPosition(const int64_t &SP)  {  m_effectStartPosition = SP;  }

   ITimeLineChannel *getChannel() const;

   void updatePosition();

   const QUuid& getUuid() const;

signals:
   void clicked( ITimeLineChannel *channel, IEffect* track );

protected:
   void effectChanged();
   void effectsSelected();

private:
   QUuid   m_uuid;
   QString m_effectNameLabel;
   int64_t m_effectDuration;
   int64_t m_effectStartPosition;
};


class IEffectFactory
{
public:
   IEffectFactory();
   virtual ~IEffectFactory() = default;

   virtual const QString& menuLabel() const = 0;

   virtual IEffect* create(ITimeLineChannel* parent, u_int64_t position) = 0;

   static const std::list<IEffectFactory *> &trackFactories();

private:
   static std::list<IEffectFactory *> &initTrackFactories();
   static std::shared_ptr<std::list< IEffectFactory* >> sTrackFactories;

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


using TimeLineTrackVievPtr = std::shared_ptr<ITimeLineTrackView>;

#endif
