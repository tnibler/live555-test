#include "LiveStreamSource.h"
#include <live555/groupsock/GroupsockHelper.hh>
#include <fstream>
#include <cstdio>
#include <iostream>
#include <thread>
#include <algorithm>

std::vector<LiveStreamSource*> LiveStreamSource::instances = std::vector<LiveStreamSource*>();

LiveStreamSource *LiveStreamSource::createNew(UsageEnvironment &env, std::mutex& data_mutex,
    bool* has_data, size_t* data_size, uint8_t** data_buffer) {
  LiveStreamSource *newSource =
      new LiveStreamSource(env, data_mutex, has_data, data_size, data_buffer);
  return newSource;
}

LiveStreamSource::LiveStreamSource(UsageEnvironment &env, std::mutex& data_mutex,
    bool* has_data, size_t* data_size, uint8_t** data_buffer)
    : FramedSource(env),
    data_mutex(data_mutex),
    has_data(has_data),
    data_size(data_size),
    data_buffer(data_buffer) {
  printf("LiveStreamSource\n");
  event_trigger_id = envir().taskScheduler().createEventTrigger(LiveStreamSource::doGetNextFrame0);
  instances.push_back(this);
}

LiveStreamSource::~LiveStreamSource() {
  printf("~LiveStreamSource\n");
  std::vector<LiveStreamSource*>::iterator pos = std::find(instances.begin(), instances.end(), this);
  if (pos != instances.end()) {
    instances.erase(pos);
  }
}

bool LiveStreamSource::init() {
	return true;
}

void LiveStreamSource::doGetNextFrame() {
  printf("doGetNextFrame\n");
  deliverFrame();
}

void LiveStreamSource::onFrame() {
  envir().taskScheduler().triggerEvent(event_trigger_id, this);
}

void LiveStreamSource::deliverFrame() {
  if (!isCurrentlyAwaitingData()) {
    printf("deliverFrame: not awaiting data \n");
    return;
  }
  printf("deliverFrame\n");
  {
    std::scoped_lock<std::mutex> lk(data_mutex);
    if (!*has_data || !*data_buffer) {
      printf("deliverFrame: no data\n");
      return;
    }

    // cut off start codes 0 0 1 or 0 0 0 1
    size_t truncate = 0;
    uint8_t *buffer = *data_buffer;
    if (*data_size >= 4 && buffer[0] == 0 &&
        buffer[1] == 0 && buffer[2] == 0 &&
        buffer[3] == 1) {
      truncate = 4;
    }
    else if (*data_size >= 3 && buffer[0] == 0 &&
          buffer[1] == 0 && buffer[2] == 1) {
        truncate = 3;
    }

    if (*data_size - truncate > fMaxSize) {
      fFrameSize = fMaxSize;
      fNumTruncatedBytes = *data_size - truncate - fMaxSize;
    }
    else {
      fFrameSize = *data_size - truncate;
    }
    memmove(fTo, buffer + truncate, fFrameSize);
    *has_data = false;
    printf("deliverFrame: Wrote frame\n");
  }
  gettimeofday(&fPresentationTime, NULL);
  nextTask() = envir().taskScheduler().scheduleDelayedTask(0,
                                                           (TaskFunc *)FramedSource::afterGetting, this);
  // FramedSource::afterGetting(this);
  printf("deliverFrame: return\n");
}

void LiveStreamSource::doStopGettingFrames() { fHaveStartedReading = false; }