#include <cstdio>
#include <live555/liveMedia/liveMedia.hh>
#include <live555/BasicUsageEnvironment/BasicUsageEnvironment.hh>
#include "H264VideoLiveServerMediaSubsession.h"
#include <fstream>
#include <vector>

// https://stackoverflow.com/questions/19427576/live555-x264-stream-live-source-based-on-testondemandrtspserver

static void announceStream(RTSPServer* rtspServer, ServerMediaSession* sms,
			   char const* streamName, char const* inputFileName) {
  char* url = rtspServer->rtspURL(sms);
  UsageEnvironment& env = rtspServer->envir();
  env << "\n\"" << streamName << "\" stream, from the file \""
      << inputFileName << "\"\n";
  env << "Play this stream using the URL \"" << url << "\"\n";
  delete[] url;
}

int main(int argc, char** argv) {
    auto scheduler = BasicTaskScheduler::createNew();
    auto *env = BasicUsageEnvironment::createNew(*scheduler);
    auto server = RTSPServer::createNew(*env, 8001, nullptr);
    if (!server) {
        fputs("Error creating server\n", stderr);
        exit(1);
    }

    const char* streamName = "live555TestStream";
    // const char* fileName = argv[1];
    auto serverMediaSession =
     ServerMediaSession::createNew(*env, streamName, streamName, "");
    serverMediaSession->addSubsession(
        H264VideoLiveServerMediaSubsession::createNew(*env, True));
    server->addServerMediaSession(serverMediaSession);
    announceStream(server, serverMediaSession, streamName, "stream asdasd");
    env->taskScheduler().doEventLoop();
}
