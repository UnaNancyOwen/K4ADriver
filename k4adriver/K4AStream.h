#pragma once

#include <atomic>
#include <thread>

#include <k4a/k4a.hpp>
#include <Driver/OniDriverAPI.h>

#include "K4ADevice.h"

#define REQUEST_WAIT_TIME 5

namespace oni
{
    namespace driver
    {
        class K4AStream : public StreamBase
        {
            public:
                K4AStream( class K4ADevice* k4a_device );

                virtual ~K4AStream();

                virtual OniStatus start();

                virtual void stop();

                virtual OniStatus setProperty( int propertyId, const void* data, int dataSize );

                virtual OniStatus getProperty( int propertyId, void* data, int* pDataSize );

                virtual OniBool isPropertySupported( int propertyId );

                virtual OniStatus convertDepthToColorCoordinates( StreamBase* colorStream, int depthX, int depthY, OniDepthPixel depthZ, int* pColorX, int* pColorY );

                virtual void MainLoop() = 0;

            protected:
                K4AStream( const K4AStream& );
                void operator=( const K4AStream& );

                void capture_thread( void* param )
                {
                    K4AStream* stream = reinterpret_cast<K4AStream*>( param );
                    stream->MainLoop();
                }

            protected:
                class K4ADevice* k4a_device;
                class K4ACapture* k4a_capture;

                std::atomic_bool is_running;
                std::thread thread;

                OniImageRegistrationMode registration_mode;
                OniVideoMode video_mode;
                size_t bytes_per_pixel;
                float horizontal_fov;
                float vertical_fov;
        };

        class K4AColorStream : public K4AStream
        {
        public:
            K4AColorStream( class K4ADevice* k4a_device );

            virtual ~K4AColorStream();

            void MainLoop();
        };

        class K4ADepthStream : public K4AStream
        {
            public:
                K4ADepthStream( class K4ADevice* k4a_device );

                virtual ~K4ADepthStream();

                void MainLoop();
        };

        class K4AInfraredStream : public K4AStream
        {
        public:
            K4AInfraredStream( class K4ADevice* k4a_device );

            virtual ~K4AInfraredStream();

            void MainLoop();
        };
    }
}
