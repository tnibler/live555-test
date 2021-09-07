#ifndef _LIVE_STREAM_SOURCE_H
#define _LIVE_STREAM_SOURCE_H

#include <queue>
#include <mutex>
#include <set>
#include "Encoder.h"

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

class LiveStreamSource: public FramedSource
{
public:
    static LiveStreamSource* createNew(UsageEnvironment& env, std::mutex& data_mutex,
    bool* has_data, size_t* data_size, uint8_t** data_buffer);
    // "preferredFrameSize" == 0 means 'no preference'
    // "playTimePerFrame" is in microseconds
    bool init();
    void onFrame();
    static std::vector<LiveStreamSource*> instances;

protected:
    LiveStreamSource(UsageEnvironment& env, std::mutex& data_mutex,
    bool* has_data, size_t* data_size, uint8_t** data_buffer);
    // called only by createNew()

    virtual ~LiveStreamSource();

private:
    // redefined virtual functions:
    virtual void doGetNextFrame();
    virtual void doStopGettingFrames();
    void deliverFrame();
    static void doGetNextFrame0(void* clientData) { ((LiveStreamSource*)clientData)->doGetNextFrame(); };


private:
    Boolean fHaveStartedReading;
    Boolean fNeedIFrame;

    EventTriggerId event_trigger_id;
    std::mutex& data_mutex;
    bool* has_data;
    size_t* data_size;
    uint8_t** data_buffer = nullptr;
};

#endif