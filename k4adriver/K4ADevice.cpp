#include "K4AUtil.h"
#include "K4ADevice.h"

namespace oni
{
    namespace driver
    {
        K4ADevice::K4ADevice( class K4ADriver* k4a_driver, k4a::device* device )
            : k4a_driver( k4a_driver ),
              k4a_capture( nullptr ),
              device( device ),
              registration_mode( ONI_IMAGE_REGISTRATION_OFF )
        {
            K4ALogDebug( "K4ADevice::K4ADevice" );

            k4a_device_configuration_t device_configuration = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
            device_configuration.color_format               = k4a_image_format_t::K4A_IMAGE_FORMAT_COLOR_BGRA32;
            device_configuration.color_resolution           = k4a_color_resolution_t::K4A_COLOR_RESOLUTION_720P;
            device_configuration.depth_mode                 = k4a_depth_mode_t::K4A_DEPTH_MODE_NFOV_UNBINNED;
            device_configuration.synchronized_images_only   = true;
            device_configuration.wired_sync_mode            = k4a_wired_sync_mode_t::K4A_WIRED_SYNC_MODE_STANDALONE;
            device->start_cameras( &device_configuration );

            calibration = device->get_calibration( device_configuration.depth_mode, device_configuration.color_resolution );

            OniSensorInfo color_sensor;
            color_sensor.pSupportedVideoModes                = new OniVideoMode[1];
            color_sensor.sensorType                          = ONI_SENSOR_COLOR;
            color_sensor.numSupportedVideoModes              = 1;
            color_sensor.pSupportedVideoModes[0].pixelFormat = ONI_PIXEL_FORMAT_RGB888;
            color_sensor.pSupportedVideoModes[0].fps         = 30;
            color_sensor.pSupportedVideoModes[0].resolutionX = calibration.color_camera_calibration.resolution_width;
            color_sensor.pSupportedVideoModes[0].resolutionY = calibration.color_camera_calibration.resolution_height;
            sensors.push_back( color_sensor );

            OniSensorInfo depth_sensor;
            depth_sensor.pSupportedVideoModes                = new OniVideoMode[1];
            depth_sensor.sensorType                          = ONI_SENSOR_DEPTH;
            depth_sensor.numSupportedVideoModes              = 1;
            depth_sensor.pSupportedVideoModes[0].pixelFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
            depth_sensor.pSupportedVideoModes[0].fps         = 30;
            depth_sensor.pSupportedVideoModes[0].resolutionX = calibration.depth_camera_calibration.resolution_width;
            depth_sensor.pSupportedVideoModes[0].resolutionY = calibration.depth_camera_calibration.resolution_height;
            sensors.push_back( depth_sensor );

            OniSensorInfo infrared_sensor;
            infrared_sensor.pSupportedVideoModes                = new OniVideoMode[1];
            infrared_sensor.sensorType                          = ONI_SENSOR_IR;
            infrared_sensor.numSupportedVideoModes              = 1;
            infrared_sensor.pSupportedVideoModes[0].pixelFormat = ONI_PIXEL_FORMAT_GRAY16;
            infrared_sensor.pSupportedVideoModes[0].fps         = 30;
            infrared_sensor.pSupportedVideoModes[0].resolutionX = calibration.depth_camera_calibration.resolution_width;
            infrared_sensor.pSupportedVideoModes[0].resolutionY = calibration.depth_camera_calibration.resolution_height;
            sensors.push_back( infrared_sensor );
        }

        K4ADevice::~K4ADevice()
        {
            K4ALogDebug( "K4ADevice::~K4ADevice" );

            if( k4a_capture ){
                delete k4a_capture;
            }

            if( device ){
                device->stop_cameras();
            }
        }

        OniStatus K4ADevice::getSensorInfoList( OniSensorInfo** pSensorInfos, int* numSensors )
        {
            K4ATraceFunc( "" );

            *pSensorInfos = &sensors[0];
            *numSensors   = static_cast<int32_t>( sensors.size() );

            return ONI_STATUS_OK;
        }

        StreamBase* K4ADevice::createStream( OniSensorType sensorType )
        {
            K4ATraceFunc( "sensor type = %d", sensorType );

            if( !k4a_capture ){
                k4a_capture = new K4ACapture( this );
            }

            switch( sensorType ){
                case ONI_SENSOR_COLOR:
                    return new K4AColorStream( this );
                case ONI_SENSOR_DEPTH:
                    return new K4ADepthStream( this );
                case ONI_SENSOR_IR:
                    return new K4AInfraredStream( this );
                default:
                    return nullptr;
            }
        }

        void K4ADevice::destroyStream( StreamBase* pStream )
        {
            K4ATraceFunc( "" );

            if( !pStream ){
                return;
            }

            delete pStream;
        }

        OniStatus K4ADevice::setProperty( int propertyId, const void* data, int dataSize )
        {
            K4ATraceFunc( "K4ADevice::setProperty : %d", propertyId );

            switch( propertyId )
            {
                case ONI_DEVICE_PROPERTY_IMAGE_REGISTRATION:
                    if( data && ( dataSize == sizeof( OniImageRegistrationMode ) ) )
                    {
                        registration_mode = *reinterpret_cast<OniImageRegistrationMode*>( const_cast<void*>( data ) );
                        K4ALogDebug( "set registration mode: %d", registration_mode );
                        return ONI_STATUS_OK;
                    }
                    break;
                case ONI_DEVICE_PROPERTY_PLAYBACK_SPEED:
                    if( data && ( dataSize == sizeof( float ) ) ){
                        return ONI_STATUS_OK;
                    }
                    break;
                case ONI_DEVICE_PROPERTY_PLAYBACK_REPEAT_ENABLED:
                    if( data && ( dataSize == sizeof( OniBool ) ) ){
                        return ONI_STATUS_OK;
                    }
                    break;
                default:
                    return ONI_STATUS_NOT_SUPPORTED;
            }

            return ONI_STATUS_ERROR;
        }

        OniStatus K4ADevice::getProperty( int propertyId, void* data, int* pDataSize )
        {
            K4ALogDebug( "K4ADevice::getProperty : %d", propertyId );

            switch( propertyId ){
                case ONI_DEVICE_PROPERTY_SERIAL_NUMBER:
                    if( data && pDataSize && *pDataSize > 0 ){
                        const std::string serial_number = device->get_serialnum();
                        const int32_t n = snprintf( reinterpret_cast<char*>( data ), *pDataSize - 1, "%s", serial_number.c_str() );
                        *pDataSize = n + 1;
                        return ONI_STATUS_OK;
                    }
                    break;
                case ONI_DEVICE_PROPERTY_IMAGE_REGISTRATION:
                    if( data && pDataSize && *pDataSize == sizeof( OniImageRegistrationMode ) ){
                        *reinterpret_cast<OniImageRegistrationMode*>( data ) = registration_mode;
                        return ONI_STATUS_OK;
                    }
                    break;
                case ONI_DEVICE_PROPERTY_PLAYBACK_SPEED:
                    if( data && pDataSize && *pDataSize == sizeof( float ) ){
                        *reinterpret_cast<float*>( data ) = 0.0f;
                        return ONI_STATUS_OK;
                    }
                    break;
                case ONI_DEVICE_PROPERTY_PLAYBACK_REPEAT_ENABLED:
                    if( data && pDataSize && *pDataSize == sizeof( OniBool ) ){
                        *reinterpret_cast<OniBool*>( data ) = TRUE;
                        return ONI_STATUS_OK;
                    }
                    break;
                #ifdef XN_MODULE_PROPERTY_AHB
                // Hack NiTE2 (Refer to RealSense SDK)
                case XN_MODULE_PROPERTY_AHB:
                    if( data && pDataSize && *pDataSize == sizeof( uint8_t ) * 12 ){
                        const uint8_t hack[] = { 0x40, 0x0, 0x0, 0x28, 0x6A, 0x26, 0x54, 0x4F, 0xFF, 0xFF, 0xFF, 0xFF };
                        memcpy( data, hack, sizeof( hack ) );
                        return ONI_STATUS_OK;
                    }
                    break;
                #endif
                default:
                    return ONI_STATUS_NOT_SUPPORTED;
            }

            return ONI_STATUS_ERROR;
        }

        OniBool K4ADevice::isPropertySupported( int propertyId )
        {
            K4ALogDebug( "K4ADevice::isPropertySupported : %d", propertyId );

            switch( propertyId )
            {
                case ONI_DEVICE_PROPERTY_SERIAL_NUMBER:
                case ONI_DEVICE_PROPERTY_IMAGE_REGISTRATION:
                case ONI_DEVICE_PROPERTY_PLAYBACK_SPEED:
                case ONI_DEVICE_PROPERTY_PLAYBACK_REPEAT_ENABLED:
                case XN_MODULE_PROPERTY_AHB:
                    return TRUE;
                default:
                    return FALSE;
            }
        }
    }
}
