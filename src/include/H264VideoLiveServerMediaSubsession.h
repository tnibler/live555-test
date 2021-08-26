#ifndef _H264_VIDEO_LIVE_SERVER_MEDIA_SUBSESSION_H
#define _H264_VIDEO_LIVE_SERVER_MEDIA_SUBSESSION_H

// the sources are not meant to be packages that way .. include order is essential as well as the
// guarded includes to allow for adopting to the current file structure
#include <live555/liveMedia/export.h>

#ifndef _USAGE_ENVIRONMENT_HH
#include <live555/UsageEnvironment/UsageEnvironment.hh>
#endif

#ifndef _BASIC_USAGE_ENVIRONMENT_HH
#include <live555/BasicUsageEnvironment/BasicUsageEnvironment.hh>
#endif

#ifndef _SERVER_MEDIA_SESSION_HH
#include <live555/liveMedia/ServerMediaSession.hh>
#endif
#ifndef _ON_DEMAND_SERVER_MEDIA_SUBSESSION_HH
#include <live555/liveMedia/OnDemandServerMediaSubsession.hh>
#endif
#ifndef _FILE_SERVER_MEDIA_SUBSESSION_HH
#include <live555/liveMedia/FileServerMediaSubsession.hh>
#endif

#include<mutex>
#include<set>

class H264VideoLiveServerMediaSubsession: public OnDemandServerMediaSubsession
{
public:
    static H264VideoLiveServerMediaSubsession*
    createNew(UsageEnvironment& env, Boolean reuseFirstSource, std::mutex& data_mutex,
    bool* has_data, size_t* data_size, uint8_t** data_buffer,
    std::set<FramedSource*>& sources);

    // Used to implement "getAuxSDPLine()":
    void checkForAuxSDPLine1();
    void afterPlayingDummy1();

protected:
    H264VideoLiveServerMediaSubsession(UsageEnvironment& env,
                                       Boolean reuseFirstSource, std::mutex& data_mutex,
    bool* has_data, size_t* data_size, uint8_t** data_buffer,
    std::set<FramedSource*>& sources);
    // called only by createNew();
    virtual ~H264VideoLiveServerMediaSubsession();

    void setDoneFlag() { fDoneFlag = ~0; }

protected: // redefined virtual functions
    virtual char const* getAuxSDPLine(RTPSink* rtpSink,
                                      FramedSource* inputSource);
    virtual FramedSource* createNewStreamSource(unsigned clientSessionId,
                                                unsigned& estBitrate);
    virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,
                                      unsigned char rtpPayloadTypeIfDynamic,
                                      FramedSource* inputSource);

private:
    char* fAuxSDPLine;
    char fDoneFlag; // used when setting up "fAuxSDPLine"
    RTPSink* fDummyRTPSink; // ditto
    unsigned int fStreamNum;

    std::mutex& data_mutex;
    bool* has_data;
    size_t* data_size;
    uint8_t** data_buffer;
    std::set<FramedSource*>& sources;
};

#endif