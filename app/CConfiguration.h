#ifndef CCONFIGURATION_H
#define CCONFIGURATION_H

#include <QString>
#include <vector>
#include <QUuid>
#include <QObject>

class Channel
{
public:
    Channel(const QString& alabel,
        const uint32_t& aunit,
        const uint32_t& achannel,
        const uint32_t& avoltage,
        const uint32_t& aspectrumIndex,
        const double& again,
        const double& afade,
        const QString& acolor,
        const QUuid& auuid)
        : label( alabel )
        , unit( aunit )
        , channel( achannel )
        , voltage( avoltage )
        , spectrumIndex( aspectrumIndex )
        , gain( again )
        , fade( afade )
        , color( acolor )
        , uuid( auuid )
    {}
    QString label;
    uint32_t unit;
    uint32_t channel;
    uint32_t voltage;
    uint32_t spectrumIndex;
    double gain;
    double fade;
    QString color;
    QUuid uuid;
};


class CConfigation
{
public:

   virtual ~CConfigation() = default;

   const QString& getDestination() const
   {
      return destinationFolder;
   }

   virtual const std::vector<Channel>& channels() const = 0;


   bool hasChannel( const QUuid &uuid ) const
   {
      bool result = false;
      for ( auto& cn : channels() )
      {
         if ( uuid == cn.uuid )
         {
            result = true;
            break;
         }
      }
      return result;
   }

protected:
   QString   destinationFolder;
   bool isPlayRandomEnabled = false;

};

#endif // CCONFIGURATIONINTERFACE_H
