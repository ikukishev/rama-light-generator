#include "csequensegenerator.h"
#include "clightsequence.h"
#include <pugixml-1.10/src/pugixml.hpp>
#include <QFileInfo>
#include <QColor>
#include <QDateTime>

template< typename TIntegral >
inline TIntegral  milisecondToCentisecond(TIntegral miliseconds )
{
    return miliseconds / 10;
}

uint64_t totalCentseconds( const CLightSequence *sequense )
{
    if ( nullptr != sequense )
    {
        auto& spData = sequense->getAudioFile()->getSpectrum();
        if ( !spData.empty() )
        {
            return milisecondToCentisecond( spData.back()->position ); // + uint64_t( max * 100.0 );
        }
    }
    return 0;
}

template <typename Base, typename Derived>
struct is_base {
    constexpr static bool check(Base*)   { return true; }
    constexpr static bool check(...)     { return false; }
    enum { value = check(static_cast<Derived*>(0)) };
};

class CGeneratorNodeBase : public pugi::xml_node
{
public:
    CGeneratorNodeBase( pugi::xml_node&& node )
        : pugi::xml_node( std::move(node) )
    { }

    virtual ~CGeneratorNodeBase(){}

    template< typename T, typename... Args >
    T appendChild( Args&&... args )
    {
        static_assert ( is_base< CGeneratorNodeBase, T >::value,  "Class is not derived from CGeneratorNodeBase");
        T node( append_child(pugi::node_element), args... );
        auto* nodePtr = reinterpret_cast<CGeneratorNodeBase*>(&node);
        node.set_name( nodePtr->getName() );
        nodePtr->render();
        return node;
    }

protected:

    virtual void render() = 0;
    virtual const char* getName() = 0;
};


class CTrackChannel : public CGeneratorNodeBase
{
public:

    CTrackChannel( pugi::xml_node&& node, uint32_t& asavedIndex )
        : CGeneratorNodeBase( std::move(node) )
        , savedIndex( asavedIndex )
    {}

protected:

    virtual void render() override
    {
        append_attribute( "savedIndex" ) = savedIndex;
    }

    virtual const char* getName( ) override
    { return "channel"; }

private:

    uint32_t savedIndex;
};



class CLoopLevels : public CGeneratorNodeBase
{
public:

    CLoopLevels( pugi::xml_node&& node )
        : CGeneratorNodeBase( std::move(node) )
    {}

protected:

    virtual void render() override  { }

    virtual const char* getName( ) override
    { return "loopLevels"; }

};




class CTrackChannels : public CGeneratorNodeBase
{
public:

    CTrackChannels( pugi::xml_node&& node, const CLightSequence *asequense )
        : CGeneratorNodeBase( std::move(node) )
        , sequense( asequense )
    { }

protected:

    virtual void render() override
    {
        if ( nullptr != sequense)
        {
            const auto& channels = sequense->getGlobalConfiguration().channels();
            for ( uint32_t i = 0; i < channels.size(); ++i )
            {
                appendChild<CTrackChannel>( i );
            }
        }
    }

    virtual const char* getName( ) override
    { return "channels"; }

    const CLightSequence *sequense;

};



class CTrack : public CGeneratorNodeBase
{
public:

    CTrack( pugi::xml_node&& node, const CLightSequence *asequense )
        : CGeneratorNodeBase( std::move(node) )
        , sequense( asequense )
    { }

protected:

    virtual void render() override
    {
        auto centiSeconds = totalCentseconds( sequense );
        if ( centiSeconds > 0 )
        {
            append_attribute( "totalCentiseconds" ) = centiSeconds;
            append_attribute( "timingGrid" ) = 0;
            appendChild<CTrackChannels>( sequense );
            appendChild<CLoopLevels>(  );
        }
    }

    virtual const char* getName( ) override
    { return "track"; }

private:
    const CLightSequence *sequense;
};



class CTracks : public CGeneratorNodeBase
{
public:

    CTracks( pugi::xml_node&& node, const CLightSequence *asequense )
        : CGeneratorNodeBase( std::move(node) )
        , sequense( asequense )
    { }

protected:

    virtual void render() override
    {
        appendChild<CTrack>( sequense );
    }

    virtual const char* getName( ) override
    { return "tracks"; }

private:
    const CLightSequence *sequense;
};




class CTiming : public CGeneratorNodeBase
{
public:

    CTiming( pugi::xml_node&& node, uint64_t& acentisecond )
        : CGeneratorNodeBase( std::move(node) )
        , centisecond( acentisecond )
    {}

protected:

    virtual void render() override
    {
        append_attribute( "centisecond" ) = centisecond;
    }

    virtual const char* getName( ) override
    { return "timing"; }

private:

    uint64_t centisecond;
};




class CTimingGrid : public CGeneratorNodeBase
{
public:

    CTimingGrid( pugi::xml_node&& node, const CLightSequence *asequense )
        : CGeneratorNodeBase( std::move(node) )
        , sequense( asequense )
    { }

protected:

    virtual void render() override
    {
        auto centiSeconds = totalCentseconds( sequense );
        if ( centiSeconds > 0 )
        {
            append_attribute( "type" ) = "freeform";
            append_attribute( "saveID" ) = 0;

            appendChild<CTiming>( uint64_t(1) );

            auto& spData = sequense->getAudioFile()->getSpectrum();
            for ( auto& spectrum : spData )
            {
                appendChild<CTiming>( milisecondToCentisecond( spectrum->position ) );
            }

        }
    }

    virtual const char* getName( ) override
    { return "timingGrid"; }

private:
    const CLightSequence *sequense;
};




class CTimingGrids : public CGeneratorNodeBase
{
public:

    CTimingGrids( pugi::xml_node&& node, const CLightSequence *asequense )
        : CGeneratorNodeBase( std::move(node) )
        , sequense( asequense )
    { }

protected:

    virtual void render() override
    {
        appendChild<CTimingGrid>( sequense );
    }

    virtual const char* getName( ) override
    { return "timingGrids"; }

private:
    const CLightSequence *sequense;
};





class CEffectInten : public CGeneratorNodeBase
{
public:

    CEffectInten( pugi::xml_node&& node, uint32_t astartCentisecond, uint32_t aendCentisecond, uint32_t aintensity )
        : CGeneratorNodeBase( std::move(node) )
        , startCentisecond( astartCentisecond )
        , endCentisecond( aendCentisecond )
        , intensity( aintensity )
    { }

protected:

    virtual void render() override
    {
        append_attribute( "type" ) = "intensity";
        append_attribute( "startCentisecond" ) = startCentisecond;
        append_attribute( "endCentisecond" ) = endCentisecond;
        append_attribute( "intensity" ) = intensity;
    }

    virtual const char* getName( ) override
    { return "effect"; }

private:
    uint32_t startCentisecond;
    uint32_t endCentisecond;
    uint32_t intensity;
};





class CLMSChannel : public CGeneratorNodeBase
{
public:

    CLMSChannel( pugi::xml_node&& node
                 , const Channel& achannel
                 , const CLightSequence *asequense
                 , uint32_t& asavedIndex
                 , uint32_t& acentiseconds )
        : CGeneratorNodeBase( std::move(node) )
        , channel( achannel )
        , sequense( asequense )
        , savedIndex( asavedIndex )
        , centiseconds( acentiseconds )
    { }

protected:

    virtual void render() override
    {
        if ( nullptr == sequense)
        {
            return;
        }

        auto& spectrum = sequense->getAudioFile()->getSpectrum();

        if ( spectrum.empty() )
        {
            return;
        }

        double minimumLevel = 0.0;
        double fading = channel.fade;
        double gain = channel.gain;
        uint32_t spectrumIndex = channel.spectrumIndex;


        auto channelConfigurationPtr = sequense->getConfiguration( channel.uuid );

        if ( channelConfigurationPtr )
        {
            minimumLevel = channelConfigurationPtr->minimumLevel;

            if ( channelConfigurationPtr->isFadeSet() )
                fading = *channelConfigurationPtr->fade;

            if ( channelConfigurationPtr->isGainSet() )
                gain = *channelConfigurationPtr->gain;

            if ( channelConfigurationPtr->isSpectrumIndexSet() )
                spectrumIndex = *channelConfigurationPtr->spectrumIndex;
        }


        auto current = spectrum.begin();

        appendChild<CEffectInten>(1u, milisecondToCentisecond( (*current)->position ), 0u);

        auto prev = current;
        ++current;

        // To simulate fade effect will be used linear finction:
        // Y(x) = k*x + b

        auto getDelta = [ b = 1.0, k = (-1.0 / (1000.0 *fading)) ] ( uint64_t prev, uint64_t current ) -> double
        {
            auto dt = current - prev;
            double y = k * double(dt) + b;
            return b - y;
        };

        double currentIntensity = 0.0;

        for ( ; current != spectrum.end(); ++prev, ++current)
        {
            double intensity = (*prev)->spectrum[spectrumIndex] * gain;
            if ( intensity < minimumLevel )
            {
                intensity = 0.0;
            }

            currentIntensity -= getDelta( (*prev)->position, (*current)->position );

            if ( intensity > currentIntensity )
            {
                currentIntensity = intensity;
            }

            if ( currentIntensity > 1.0 )
            {
                currentIntensity = 1.0;
            }

            intensity = (100.0 * currentIntensity) * (channel.voltage / (220.0));

            appendChild<CEffectInten>( milisecondToCentisecond( (*prev)->position ),
                                           milisecondToCentisecond( (*current)->position ),
                                           uint32_t(intensity) );
        }

        append_attribute( "name" ) = channel.label.toStdString().c_str();
        auto color = QColor(channel.color);

        uint32_t calcColor = (uint32_t(color.blue()) & 0xFF )<<16;
        calcColor = calcColor | ((uint32_t(color.green()) & 0xFF )<<8);
        calcColor = calcColor | (uint32_t(color.red()) & 0xFF );

        append_attribute( "color" ) = calcColor; //color.red()*color.green()*color.blue();
        append_attribute( "centiseconds" ) = centiseconds;
        append_attribute( "deviceType" ) = "LOR";
        append_attribute( "unit" ) = channel.unit;
        append_attribute( "circuit" ) = channel.channel;
        append_attribute( "savedIndex" ) = savedIndex;


    }

    virtual const char* getName( ) override
    { return "channel"; }

private:
    const Channel& channel;
    const CLightSequence *sequense;
    uint32_t savedIndex;
    uint32_t centiseconds;
};





class CLMSChannels : public CGeneratorNodeBase
{
public:

    CLMSChannels( pugi::xml_node&& node, const CLightSequence *asequense )
        : CGeneratorNodeBase( std::move(node) )
        , sequense( asequense )
    { }

protected:

    virtual void render() override
    {
        if ( nullptr != sequense)
        {
            uint32_t centiSeconds = totalCentseconds( sequense );
            const auto& channels = sequense->getGlobalConfiguration().channels();
            for ( uint32_t i = 0; i < channels.size(); ++i )
            {
                const Channel& channel = channels[i];
                appendChild<CLMSChannel>( channel, sequense, i, centiSeconds );
            }
        }
    }

    virtual const char* getName( ) override
    { return "channels"; }

private:
    const CLightSequence *sequense;
};




class CLMSSequence : public CGeneratorNodeBase
{
public:

    CLMSSequence( pugi::xml_node&& node, const CLightSequence *asequense )
        : CGeneratorNodeBase( std::move(node) )
        , sequense( asequense )
    { }

    virtual void render() override
    {
        append_attribute( "saveFileVersion" ) = 14;
        append_attribute( "author" ) = "Ivan";
        append_attribute( "createdAt" ) = QDateTime::currentDateTime().toString("dd/MM/yyyy h:m:s ap").toStdString().c_str();
        append_attribute( "musicFilename" ) = QFileInfo( sequense->getFileName().c_str() ).fileName().toStdString().c_str();
        append_attribute( "videoUsage" ) = 2;

        appendChild<CLMSChannels>( sequense );
        appendChild<CTimingGrids>( sequense );
        appendChild<CTracks>( sequense );
    }

    virtual const char* getName( ) override
    { return "sequence"; }

private:
    const CLightSequence *sequense;
};




bool CSequenseGenerator::generateLms(const CLightSequence *sequense)
{
    if ( nullptr == sequense )
    {
        return false;
    }

    pugi::xml_document xml;
    CLMSSequence lms(xml.append_child( pugi::node_element ), sequense);
    lms.set_name( lms.getName() );
    lms.render();
    QString fileName = sequense->getGlobalConfiguration().getDestination()
            + "/" + QFileInfo(sequense->getFileName().c_str()).fileName() + ".lms";
    return xml.save_file( fileName.toStdString().c_str() );
}


