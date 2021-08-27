#include "H264VideoLiveServerMediaSubsession.h"
#include "LiveStreamSource.h"
#include <cstdio>
#include <live555/liveMedia/H264VideoRTPSink.hh>
#include <live555/liveMedia/H264VideoStreamDiscreteFramer.hh>
#include <live555/liveMedia/MPEG4GenericRTPSink.hh>
#include <live555/liveMedia/MPEG4VideoStreamDiscreteFramer.hh>

H264VideoLiveServerMediaSubsession *
H264VideoLiveServerMediaSubsession::createNew(UsageEnvironment &env,
                                              Boolean reuseFirstSource, std::mutex& data_mutex,
    bool* has_data, size_t* data_size, uint8_t** data_buffer,
    std::set<FramedSource*>& sources) {
  return new H264VideoLiveServerMediaSubsession(env, reuseFirstSource,
  data_mutex, has_data, data_size, data_buffer, sources);
}

H264VideoLiveServerMediaSubsession::H264VideoLiveServerMediaSubsession(UsageEnvironment &env,
                                                                       Boolean reuseFirstSource, std::mutex& data_mutex,
                                                                       bool *has_data, size_t *data_size, uint8_t **data_buffer,
                                                                       std::set<FramedSource*>& sources)
    : OnDemandServerMediaSubsession(env, reuseFirstSource),
      fAuxSDPLine(NULL), fDoneFlag(0), fDummyRTPSink(NULL),
      data_mutex(data_mutex),
      has_data(has_data),
      data_size(data_size),
      data_buffer(data_buffer),
      sources(sources)
{
}

H264VideoLiveServerMediaSubsession::~H264VideoLiveServerMediaSubsession() {
  delete[] fAuxSDPLine;
}

static void afterPlayingDummy(void *clientData) {
  H264VideoLiveServerMediaSubsession *subsess =
      (H264VideoLiveServerMediaSubsession *)clientData;
  subsess->afterPlayingDummy1();
}

void H264VideoLiveServerMediaSubsession::afterPlayingDummy1() {
  // Unschedule any pending 'checking' task:
  envir().taskScheduler().unscheduleDelayedTask(nextTask());
  // Signal the event loop that we're done:
  setDoneFlag();
}

static void checkForAuxSDPLine(void *clientData) {
  H264VideoLiveServerMediaSubsession *subsess =
      (H264VideoLiveServerMediaSubsession *)clientData;
  subsess->checkForAuxSDPLine1();
}

void H264VideoLiveServerMediaSubsession::checkForAuxSDPLine1() {
  nextTask() = NULL;

  char const *dasl;
  if (fAuxSDPLine != NULL) {
    // Signal the event loop that we're done:
    setDoneFlag();
  } else if (fDummyRTPSink != NULL &&
             (dasl = fDummyRTPSink->auxSDPLine()) != NULL) {
    fAuxSDPLine = strDup(dasl);
    fDummyRTPSink = NULL;

    // Signal the event loop that we're done:
    setDoneFlag();
  } else if (!fDoneFlag) {
    // try again after a brief delay:
    int uSecsToDelay = 100000; // 100 ms
    nextTask() = envir().taskScheduler().scheduleDelayedTask(
        uSecsToDelay, (TaskFunc *)checkForAuxSDPLine, this);
  }
}

char const *
H264VideoLiveServerMediaSubsession::getAuxSDPLine(RTPSink *rtpSink,
                                                  FramedSource *inputSource) {
  if (fAuxSDPLine != NULL)
    return fAuxSDPLine; // it's already been set up (for a previous client)

  if (fDummyRTPSink == NULL) {
    // we're not already setting it up for another, concurrent stream
    // Note: For H264 video files, the 'config' information ("profile-level-id"
    // and "sprop-parameter-sets") isn't known until we start reading the file.
    // This means that "rtpSink"s "auxSDPLine()" will be NULL initially, and we
    // need to start reading data from our file until this changes.
    fDummyRTPSink = rtpSink;

    // Start reading the file:
    fDummyRTPSink->startPlaying(*inputSource, afterPlayingDummy, this);

    // Check whether the sink's 'auxSDPLine()' is ready:
    checkForAuxSDPLine(this);
  }

  envir().taskScheduler().doEventLoop(&fDoneFlag);

  return fAuxSDPLine;
}

FramedSource *H264VideoLiveServerMediaSubsession::createNewStreamSource(
    unsigned /*clientSessionId*/, unsigned &estBitrate) {
  printf("createNewStreamSource\n");
  estBitrate = 100000; // kbps, estimate

  // Create the video source:
  LiveStreamSource *source = LiveStreamSource::createNew(envir(),
                                                         data_mutex);
  if (source == NULL)
    return NULL;
  if (!source->init())
  {
    printf("Failed to init encoder and stuff\n");
    return NULL;
  }

  // Create a framer for the Video Elementary Stream:
  auto framer = H264VideoStreamDiscreteFramer::createNew(envir(), source);
  {
    std::scoped_lock<std::mutex> lk(data_mutex);
    sources.insert(framer);
    printf("inserted Stream Source\n");
  }
  return framer;
}

RTPSink *H264VideoLiveServerMediaSubsession ::createNewRTPSink(
    Groupsock *rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic,
    FramedSource * /*inputSource*/) {
  OutPacketBuffer::increaseMaxSizeTo(5000000); // ?
  return H264VideoRTPSink::createNew(envir(), rtpGroupsock,
                                     rtpPayloadTypeIfDynamic);
}