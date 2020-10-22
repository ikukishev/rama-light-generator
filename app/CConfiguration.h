#ifndef CCONFIGURATION_H
#define CCONFIGURATION_H

#include <QString>
#include <vector>

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

class CConfigation
{

public:

   virtual ~CConfigation() = default;

   const QString& getDestination() const
   {
      return destinationFolder;
   }

   virtual const std::vector<Channel>& channels() const = 0;

protected:
   QString   destinationFolder;

};

#endif // CCONFIGURATIONINTERFACE_H