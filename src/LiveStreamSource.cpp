#include "LiveStreamSource.h"
#include <live555/groupsock/GroupsockHelper.hh>
#include <fstream>
#include <cstdio>
#include <iostream>

EventTriggerId LiveStreamSource::eventTriggerId = 0;

LiveStreamSource *LiveStreamSource::createNew(UsageEnvironment &env) {
  LiveStreamSource *newSource =
      new LiveStreamSource(env);
  return newSource;
}

LiveStreamSource::LiveStreamSource(UsageEnvironment &env)
    : FramedSource(env) {
  encoder = new Encoder(env);
}

LiveStreamSource::~LiveStreamSource() {
  cleanup(CS_CAPTURE_STOP);
  envir().taskScheduler().deleteEventTrigger(eventTriggerId);
  eventTriggerId = 0;
}

void LiveStreamSource::cleanup(clean_state_t cleanState) {
	switch(cleanState) {
	case CS_CAPTURE_STOP:
		encoder->stop_capturing();
	case CS_MUNMAP_FILE:
		encoder->uninit_device();
	case CS_CLOSE_FILE:
		encoder->close_device();
	case CS_FREE_MEM:
		encoder->uninitialize();
	case CS_UNINITIALIZED:
		break;
	}
}


bool LiveStreamSource::init() {
	if (!encoder->initialize()) {
    cleanup(CS_FREE_MEM);
    printf("Failed initializing encoder\n");
		return false;
	}

	if(!encoder->open_device()) {
    cleanup(CS_FREE_MEM);
    printf("Failed opening device\n");
		return false;
	}

	if (!encoder->init_device()) {
    cleanup(CS_CLOSE_FILE);
    printf("Failed initing device\n");
		return false;
	}

	if (!encoder->init_mmap()) {
    cleanup(CS_CLOSE_FILE);
    printf("Failed initing mmap\n");
		return false;
	}

	if(!encoder->start_capturing()) {
    cleanup(CS_MUNMAP_FILE);
    printf("Failed to start capturing\n");
		return false;
	}

	encoder->mainloop();
	if(eventTriggerId == 0) {
		eventTriggerId = envir().taskScheduler().createEventTrigger(_deliverFrame);
	}
	return true;
}

void LiveStreamSource::doGetNextFrame() {
  if (!nalQueue.empty()) {
    deliverFrame();
  }
  else {
    getFromEncoder();
    gettimeofday(&fPresentationTime, NULL);
    deliverFrame();
  }
}

void LiveStreamSource::_deliverFrame(void* clientData) {
  ((LiveStreamSource*)clientData)->deliverFrame();
}

void LiveStreamSource::deliverFrame() {
  if (!isCurrentlyAwaitingData()) {
    return;
  }

  auto nalu = nalQueue.front();
  nalQueue.pop();

  // cut off start codes 0 0 1 or 0 0 0 1
  int truncate = 0;
  if (nalu->i_payload >= 4 && nalu->p_payload[0] == 0 &&
      nalu->p_payload[1] == 0 && nalu->p_payload[2] == 0 &&
      nalu->p_payload[3] == 1) {
    truncate = 4;
  } else {
    if (nalu->i_payload >= 3 && nalu->p_payload[0] == 0 &&
        nalu->p_payload[1] == 0 && nalu->p_payload[2] == 1) {
      truncate = 3;
    }
  }

  if (nalu->i_payload - truncate > (signed int)fMaxSize) {
    fFrameSize = fMaxSize;
    fNumTruncatedBytes = nalu->i_payload - truncate - fMaxSize;
  } else {
    fFrameSize = nalu->i_payload - truncate;
  }
  memmove(fTo, nalu->p_payload + truncate, fFrameSize);
  FramedSource::afterGetting(this);
}

void LiveStreamSource::doStopGettingFrames() { fHaveStartedReading = false; }

void LiveStreamSource::getFromEncoder() {
	encoder->mainloop();
	while(encoder->isNalsAvailableInOutputQueue()) {
		auto nalu = encoder->getNalUnit();
		nalQueue.push(nalu);
	}
}