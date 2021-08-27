#include "LiveStreamSource.h"
#include <live555/groupsock/GroupsockHelper.hh>
#include <fstream>
#include <cstdio>
#include <iostream>
#include <thread>

LiveStreamSource *LiveStreamSource::createNew(UsageEnvironment &env, std::mutex &data_mutex)
{
  LiveStreamSource *newSource =
      new LiveStreamSource(env, data_mutex);
  return newSource;
}

LiveStreamSource::LiveStreamSource(UsageEnvironment &env, std::mutex &data_mutex)
    : FramedSource(env),
      data_mutex(data_mutex)
{
}

LiveStreamSource::~LiveStreamSource()
{
  printf("LiveStreamSource destructor\n");
}

bool LiveStreamSource::init()
{
  return true;
}

void LiveStreamSource::doGetNextFrame()
{
  deliverFrame();
}

void LiveStreamSource::deliverFrame()
{
  printf("doGetNextFrame: deliverFrame \n");
  if (!isCurrentlyAwaitingData())
  {
    printf("deliverFrame: not awaiting data \n");
    return;
  }
  {
    printf("livesource locking mutex\n");
    std::scoped_lock<std::mutex> lk(data_mutex);
    printf("livesource unlocking mutex\n");
  }
  return;
  // nextTask() = envir().taskScheduler().scheduleDelayedTask(0,
  //                                                          (TaskFunc *)FramedSource::afterGetting, this);
  // FramedSource::afterGetting(this);
}

void LiveStreamSource::doStopGettingFrames() { fHaveStartedReading = false; }