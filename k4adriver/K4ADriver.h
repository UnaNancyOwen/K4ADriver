#pragma once

#include <k4a/k4a.hpp>
#include <Driver/OniDriverAPI.h>

#include "K4ADevice.h"

namespace oni
{
    namespace driver
    {
        class K4ADriver : public DriverBase
        {
            public:
                K4ADriver( OniDriverServices* pDriverServices );

                virtual ~K4ADriver();

                virtual OniStatus initialize( DeviceConnectedCallback connectedCallback, DeviceDisconnectedCallback disconnectedCallback, DeviceStateChangedCallback deviceStateChangedCallback, void* pCookie );

                virtual void shutdown();

                virtual DeviceBase* deviceOpen( const char* uri, const char* mode );

                virtual void deviceClose( DeviceBase* pDevice );

                virtual OniStatus tryDevice( const char* uri );

                virtual void* enableFrameSync( StreamBase** pStreams, int streamCount );

                virtual void disableFrameSync( void* frameSyncGroup );

            protected:
                K4ADriver( const K4ADriver& );
                void operator=( const K4ADriver& );

            protected:
                k4a::device device;
        };
    }
}
