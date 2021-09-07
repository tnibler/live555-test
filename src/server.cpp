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
    std::cout << std::this_thread::get_id() << std::endl;
    auto scheduler = BasicTaskScheduler::createNew();
    auto *env = BasicUsageEnvironment::createNew(*scheduler);
    auto server = RTSPServer::createNew(*env, 8001, nullptr);
    if (!server) {
        fputs("Error creating server\n", stderr);
        exit(1);
    }

    char event_loop_watch = 0;

    std::mutex buffer_mutex;
    uint8_t* buffer = nullptr;
    size_t buffer_size = 0;
    size_t data_size = 0;
    bool has_data = false;
    std::thread encoder_thread([&]()
                               {
                                   printf("Encoder thread: started\n");
                                   // std::cout << std::this_thread::get_id() << std::endl;
                                   auto encoder = new Encoder(*env);
                                   {
                                       if (!encoder->initialize())
                                       {
                                           printf("Failed initializing encoder\n");
                                           exit(1);
                                       }
                                       printf("Initialized encoder\n");

                                       if (!encoder->open_device())
                                       {
                                           printf("Failed opening device\n");
                                           exit(1);
                                       }
                                       // printf("Opened device\n");

                                       if (!encoder->init_device())
                                       {
                                           printf("Failed initing device\n");
                                           exit(1);
                                       }
                                       // printf("Initialized device\n");

                                       if (!encoder->init_mmap())
                                       {
                                           printf("Failed initing mmap\n");
                                           exit(1);
                                       }
                                       // printf("Initialized mmap\n");

                                       if (!encoder->start_capturing())
                                       {
                                           printf("Failed to start capturing\n");
                                           exit(1);
                                       }
                                       // printf("started capture\n");
                                   }

                                   printf("Encoder thread: Starting encoder loop\n");
                                   while (1)
                                   {
                                       encoder->mainloop();
                                       if (encoder->isNalsAvailableInOutputQueue())
                                       {
                                           {
                                               printf("Encoder thread: Frame available\n");
                                               std::scoped_lock<std::mutex> lk(buffer_mutex);
                                               auto nalu = encoder->getNalUnit();
                                               if (nalu->i_payload > buffer_size)
                                               {
                                                   delete[] buffer;
                                                   buffer_size = nalu->i_payload;
                                                   buffer = new uint8_t[buffer_size];
                                               }
                                               data_size = nalu->i_payload;
                                               memmove(buffer, nalu->p_payload, data_size);
                                               has_data = true;
                                           }
                                           for (auto s : LiveStreamSource::instances) {
                                               s->onFrame();
                                           }
                                       }
                                       else
                                       {
                                           // printf("No nal\n");
                                       }
                                   }
                                   printf("Exit encoder loop\n");
                               });

    const char *streamName = "live555TestStream";
    auto serverMediaSession =
        ServerMediaSession::createNew(*env, streamName, streamName, "");
    serverMediaSession->addSubsession(
        H264VideoLiveServerMediaSubsession::createNew(*env, True, buffer_mutex,
                                                      &has_data, &data_size, &buffer));
    server->addServerMediaSession(serverMediaSession);
    announceStream(server, serverMediaSession, streamName, "stream asdasd");
    while (1)
    {
        printf("Main: Starting event loop\n");
        env->taskScheduler().doEventLoop(&event_loop_watch);
        event_loop_watch = 0;
    }
    printf("Exited event loop, joining thread\n");
    encoder_thread.join();
}
