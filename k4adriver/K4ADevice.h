#pragma once

#include <k4a/k4a.hpp>
#include <Driver/OniDriverAPI.h>

#include "K4ACapture.h"
#include "K4AStream.h"

namespace oni
{
    namespace driver
    {
        class K4ADevice : public DeviceBase
        {
            public:
                K4ADevice( class K4ADriver* k4a_driver, k4a::device* device );

                virtual ~K4ADevice();

                virtual OniStatus getSensorInfoList( OniSensorInfo** pSensorInfos, int* numSensors );

                virtual StreamBase* createStream( OniSensorType sensorType );

                virtual void destroyStream( StreamBase* pStream );

                virtual OniStatus setProperty( int propertyId, const void* data, int dataSize );

                virtual OniStatus getProperty( int propertyId, void* data, int* pDataSize );

                virtual OniBool isPropertySupported( int propertyId );

                virtual OniBool isImageRegistrationModeSupported( OniImageRegistrationMode mode ) { return TRUE; };

                inline class K4ADriver*  getDriver()      { return k4a_driver;  }
                inline class K4ACapture* getCapture()     { return k4a_capture; }
                inline k4a::device*      getDevice()      { return device;      }
                inline k4a::calibration  getCalibration() { return calibration; }
                inline OniImageRegistrationMode getRegistrationMode() const { return registration_mode; }

            protected:
                K4ADevice( const K4ADevice& );
                void operator=( const K4ADevice& );

            protected:
                class K4ACapture* k4a_capture;
                class K4ADriver* k4a_driver;
                k4a::device* device;

                k4a::calibration calibration;
                std::vector<OniSensorInfo> sensors;
                OniImageRegistrationMode registration_mode;
        };
    }
}
