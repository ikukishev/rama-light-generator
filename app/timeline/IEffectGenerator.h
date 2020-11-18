#ifndef IEFFECTGENERATOR_H
#define IEFFECTGENERATOR_H

#include <QString>
#include <QUuid>
#include <QJsonObject>
#include <map>
#include <memory>
#include <QWidget>
#include <QDebug>

class IEffectGenerator;

class IEffectGeneratorFactory
{
public:

   IEffectGeneratorFactory( const QString& atype );

   virtual ~IEffectGeneratorFactory() = default;

   const QString& type() const { return m_type; }

   virtual std::shared_ptr< IEffectGenerator > create() = 0;
   virtual std::shared_ptr< IEffectGenerator > create( const QUuid& uuid ) = 0;

   static std::shared_ptr<IEffectGeneratorFactory> get( const QString& atype );

   static std::shared_ptr< IEffectGenerator > create( const QJsonObject& object );

   static std::map<QString, std::shared_ptr<IEffectGeneratorFactory> >& getGeneratorFactories();

private:
   QString m_type;

};


class IEffectGenerator
{
   friend class IEffectGeneratorFactory;
public:

   IEffectGenerator( IEffectGeneratorFactory& afactory );
   IEffectGenerator( IEffectGeneratorFactory& afactory, const QUuid& uuid );

   virtual ~IEffectGenerator() = default;

   const QString& effectNameLabel() const {  return m_effectNameLabel;  }
   void setEffectNameLabel( const QString &ef ) {  m_effectNameLabel = ef;  }

   int64_t effectDuration() const         {  return m_effectDuration;  }
   void setEffectDuration( const int64_t &eD ) { m_effectDuration = eD; }

   int64_t effectStartPosition() const    {  return m_effectStartPosition;  }
   void setEffectStartPosition( const int64_t &SP )  {  m_effectStartPosition = SP;  }

   const QUuid&   getUuid() const {  return m_uuid;  }

   const QString& type() const {  return m_factory.type();  }

   QJsonObject toJson() const ;

   double generate( int64_t position, const std::vector<float>& fft );

   QWidget* configurationWidget( QWidget* parent );

   bool isPositionActive( int64_t position ) const;

protected:
   virtual QJsonObject toJsonParameters() const = 0;
   virtual bool parseParameters( const QJsonObject& parameters ) = 0;
   virtual double calculateIntensity( int64_t position, const std::vector<float>& fft ) = 0;
   virtual QWidget* buildWidget( QWidget* parent ) = 0;

private:
   IEffectGeneratorFactory& m_factory;

   QUuid   m_uuid;
   QString m_effectNameLabel;
   int64_t m_effectDuration;
   int64_t m_effectStartPosition;

};


#define DECLARE_EFFECT_FACTORY( effectName, className ) \
   class CEffectGeneratorFactory##effectName: public IEffectGeneratorFactory { \
   public: \
      CEffectGeneratorFactory##effectName(): IEffectGeneratorFactory( #effectName ) {}\
      virtual std::shared_ptr< IEffectGenerator > create() override { auto ptr = std::make_shared<className>( *this ); qDebug() << "create" << type() << "ptr:" << (void*)ptr.get(); return ptr; } \
      virtual std::shared_ptr< IEffectGenerator > create( const QUuid& uuid ) override { return std::make_shared<className>( *this, uuid ); } \
   };\
   static CEffectGeneratorFactory##effectName s_factory_##effectName;


#endif // IEFFECTGENERATOR_H