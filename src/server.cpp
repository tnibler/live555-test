#include <cstdio>
#include <live555/liveMedia/liveMedia.hh>
#include <live555/BasicUsageEnvironment/BasicUsageEnvironment.hh>
#ifndef _FRAMED_SOURCE_HH
#include "live555/liveMedia/FramedSource.hh"
#endif
#include "H264VideoLiveServerMediaSubsession.h"
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>
#include "Encoder.h"
#include "LiveStreamSource.h"
#include <iostream>

// https://stackoverflow.com/questions/19427576/live555-x264-stream-live-source-based-on-testondemandrtspserver

static void announceStream(RTSPServer *rtspServer, ServerMediaSession *sms,
                           char const *streamName, char const *inputFileName)
{
    char *url = rtspServer->rtspURL(sms);
    UsageEnvironment &env = rtspServer->envir();
    env << "\n\"" << streamName << "\" stream, from the file \""
        << inputFileName << "\"\n";
    env << "Play this stream using the URL \"" << url << "\"\n";
    delete[] url;
}

void getFramesFromAllSources(void *client_data)
{
    printf("getFramesFromAllSources\n");
    auto sources = (std::set<FramedSource *> *)client_data;
    for (auto source : *sources)
    {
        if (source->isCurrentlyAwaitingData())
        {
            ((LiveStreamSource *)source)->deliverFrame();
        }
    }
}

int main(int argc, char **argv)
{
    std::cout << std::this_thread::get_id() << std::endl;
    auto scheduler = BasicTaskScheduler::createNew();
    auto *env = BasicUsageEnvironment::createNew(*scheduler);
    auto server = RTSPServer::createNew(*env, 8001, nullptr);
    if (!server)
    {
        fputs("Error creating server\n", stderr);
        exit(1);
    }

    char event_loop_watch = 0;

    std::mutex buffer_mutex;
    std::set<FramedSource *> sources;
    uint8_t *buffer = nullptr;
    size_t buffer_size = 0;
    size_t data_size = 0;
    bool has_data = false;

    EventTriggerId get_frames_event = scheduler->createEventTrigger(getFramesFromAllSources);
    std::thread encoder_thread(
        [&]()
        {
            printf("Encoder thread started\n");
            while (1)
            {
                env->taskScheduler().triggerEvent(get_frames_event, &sources);
            }
        });

    const char *streamName = "live555TestStream";
    auto serverMediaSession =
        ServerMediaSession::createNew(*env, streamName, streamName, "");
    serverMediaSession->addSubsession(
        H264VideoLiveServerMediaSubsession::createNew(*env, True, buffer_mutex,
                                                      &has_data, &data_size, &buffer, sources));
    server->addServerMediaSession(serverMediaSession);
    announceStream(server, serverMediaSession, streamName, "stream asdasd");
    while (1)
    {
        printf("Starting event loop\n");
        env->taskScheduler().doEventLoop(&event_loop_watch);
        printf("Interrupted event loop. Sources: %d\n", sources.size());
        event_loop_watch = 0;
    }
}
