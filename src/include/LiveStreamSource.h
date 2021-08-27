#ifndef _LIVE_STREAM_SOURCE_H
#define _LIVE_STREAM_SOURCE_H

#include <queue>
#include <mutex>
#include "Encoder.h"
#include "H264VideoLiveServerMediaSubsession.h"

// the sources are not meant to be packages that way .. include order is essential as well as the
// guarded includes to allow for adopting to the current file structure
#include <live555/liveMedia/export.h>

#ifndef _USAGE_ENVIRONMENT_HH
#include <live555/UsageEnvironment/UsageEnvironment.hh>
#endif

#ifndef _BASIC_USAGE_ENVIRONMENT_HH
#include <live555/BasicUsageEnvironment/BasicUsageEnvironment.hh>
#endif

#ifndef _FRAMED_SOURCE_HH
#include "live555/liveMedia/FramedSource.hh"
#endif

#include <set>

class LiveStreamSource : public FramedSource
{
public:
    static LiveStreamSource *createNew(UsageEnvironment &env, std::mutex &data_mutex);
    // "preferredFrameSize" == 0 means 'no preference'
    // "playTimePerFrame" is in microseconds
    bool init();
    void deliverFrame();
    virtual void doGetNextFrame();

protected:
    LiveStreamSource(UsageEnvironment &env, std::mutex &data_mutex);
    // called only by createNew()

    virtual ~LiveStreamSource();

private:
    // redefined virtual functions:
    virtual void doStopGettingFrames();

private:
    Boolean fHaveStartedReading;
    Boolean fNeedIFrame;

    std::mutex &data_mutex;
};

#endif