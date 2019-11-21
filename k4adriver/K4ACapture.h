#pragma once

#include <thread>
#include <atomic>
#include <utility>
#include <vector>
#include <chrono>

#if __has_include(<concurrent_queue.h>)
#include <concurrent_queue.h>
#else
#include <tbb/concurrent_queue.h>
namespace concurrency = tbb;
#endif

#include <k4a/k4a.hpp>
#include <Driver/OniDriverAPI.h>

#include "K4AStream.h"

#define MAX_QUEUE_SIZE 3

namespace oni
{
    namespace driver
    {
        class K4ACapture
        {
            public:
                K4ACapture( class K4ADevice* k4a_device );

                ~K4ACapture();

                bool get_color_image( std::pair<std::vector<uint8_t>, std::chrono::microseconds>& color_data );

                bool get_depth_image( std::pair<std::vector<uint16_t>, std::chrono::microseconds>& depth_data );

                bool get_infrared_image( std::pair<std::vector<uint16_t>, std::chrono::microseconds>& infrared_data );

                void start();

                void stop();

            protected:
                K4ACapture( const K4ACapture& );
                void operator=( const K4ACapture& );

            private:
                void capture_thread();

            protected:
                class K4ADevice* k4a_device;
                k4a::device* device;
                k4a::capture capture;
                k4a::transformation transformation;
                OniImageRegistrationMode registration_mode;

                concurrency::concurrent_queue<std::pair<std::vector<uint8_t>, std::chrono::microseconds>> color_queue;
                concurrency::concurrent_queue<std::pair<std::vector<uint16_t>, std::chrono::microseconds>> depth_queue;
                concurrency::concurrent_queue<std::pair<std::vector<uint16_t>, std::chrono::microseconds>> infrared_queue;

                std::thread thread;
                std::atomic_bool is_capture;
        };
    }
}
