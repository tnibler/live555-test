#ifndef _LIVE_STREAM_SOURCE_H
#define _LIVE_STREAM_SOURCE_H

#include <queue>
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

#include<vector>

typedef enum {
CS_UNINITIALIZED,
CS_FREE_MEM,
CS_CLOSE_FILE,
CS_MUNMAP_FILE,
CS_CAPTURE_STOP
} clean_state_t;

class LiveStreamSource: public FramedSource
{
public:
    static LiveStreamSource* createNew(UsageEnvironment& env);
    // "preferredFrameSize" == 0 means 'no preference'
    // "playTimePerFrame" is in microseconds
    bool init();
    static EventTriggerId eventTriggerId;

protected:
    LiveStreamSource(UsageEnvironment& env);
    // called only by createNew()

    virtual ~LiveStreamSource();

private:
    // redefined virtual functions:
    virtual void doGetNextFrame();
    virtual void doStopGettingFrames();

    void cleanup(clean_state_t);
    void getFromEncoder();
    static void _deliverFrame(void*);
    void deliverFrame();

private:
    Boolean fHaveStartedReading;
    Boolean fNeedIFrame;

    std::queue<x264_nal_t*> nalQueue;
    Encoder* encoder;
};

#endif