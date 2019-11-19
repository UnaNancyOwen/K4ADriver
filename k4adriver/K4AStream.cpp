#include "K4AUtil.h"
#include "K4AStream.h"

#include <chrono>

namespace oni
{
    namespace driver
    {
        K4AStream::K4AStream( class K4ACapture* k4a_capture, class K4ADevice* k4a_device )
            : k4a_capture( k4a_capture ),
              k4a_device( k4a_device )
        {
            K4ALogDebug( "K4AStream::K4AStream" );
        }

        K4AStream::~K4AStream()
        {
            K4ALogDebug( "K4AStream::~K4AStream" );

            stop();
        }

        OniStatus K4AStream::start()
        {
            K4ATraceFunc( "" );

            is_running = true;
            thread = std::thread( &K4AStream::MainLoop, this );

            return ONI_STATUS_OK;
        }

        void K4AStream::stop()
        {
            K4ATraceFunc( "" );

            is_running = false;
            if( thread.joinable() ){
                thread.join();
            }
        }

        OniStatus K4AStream::setProperty( int propertyId, const void* data, int dataSize )
        {
            K4ALogDebug( "K4AStream::setProperty : %d", propertyId );

            switch( propertyId ){
                case ONI_STREAM_PROPERTY_VIDEO_MODE:
                    if( data && ( dataSize == sizeof( OniVideoMode ) ) ){
                        OniVideoMode* mode = ( OniVideoMode* )data;
                        K4ALogDebug( "set video mode: %dx%d @%d format=%d", mode->resolutionX, mode->resolutionY, mode->fps, static_cast<int>( mode->pixelFormat ) );
                        video_mode = *mode;
                        return ONI_STATUS_OK;
                    }
                    break;
                case ONI_STREAM_PROPERTY_AUTO_WHITE_BALANCE:
                    if( data && ( dataSize == sizeof( OniBool ) ) ){
                        return ONI_STATUS_OK;
                    }
                    break;
                case ONI_STREAM_PROPERTY_AUTO_EXPOSURE:
                    if( data && ( dataSize == sizeof( OniBool ) ) ){
                        return ONI_STATUS_OK;
                    }
                    break;
                default:
                    break;
            }

            return ONI_STATUS_NOT_IMPLEMENTED;
        }

        OniStatus K4AStream::getProperty( int propertyId, void* data, int* dataSize )
        {
            K4ALogDebug( "K4AStream::getProperty : %d", propertyId );

            switch( propertyId ){
                case ONI_STREAM_PROPERTY_HORIZONTAL_FOV:
                    if( data && dataSize && *dataSize == sizeof( float ) ){
                        *reinterpret_cast<float*>( data ) = horizontal_fov;
                        return ONI_STATUS_OK;
                    }
                    break;
                case ONI_STREAM_PROPERTY_VERTICAL_FOV:
                    if( data && dataSize && *dataSize == sizeof( float ) ){
                        *reinterpret_cast<float*>( data ) = vertical_fov;
                        return ONI_STATUS_OK;
                    }
                    break;
                case ONI_STREAM_PROPERTY_VIDEO_MODE:
                    if( data && dataSize && *dataSize == sizeof( OniVideoMode ) ){
                        *reinterpret_cast<OniVideoMode*>( data ) = video_mode;
                        return ONI_STATUS_OK;
                    }
                    break;
                case ONI_STREAM_PROPERTY_STRIDE:
                    if( data && dataSize && *dataSize == sizeof( int ) ){
                        *reinterpret_cast<int*>( data ) = video_mode.resolutionX * static_cast<int32_t>( bytes_per_pixel );
                        return ONI_STATUS_OK;
                    }
                    break;
                case ONI_STREAM_PROPERTY_AUTO_WHITE_BALANCE:
                    if( data && dataSize && *dataSize == sizeof( OniBool ) ){
                        *reinterpret_cast<OniBool*>( data ) = TRUE;
                        return ONI_STATUS_OK;
                    }
                    break;
                case ONI_STREAM_PROPERTY_AUTO_EXPOSURE:
                    if( data && dataSize && *dataSize == sizeof( OniBool ) ){
                        *reinterpret_cast<OniBool*>( data ) = TRUE;
                        return ONI_STATUS_OK;
                    }
                    break;
                default:
                    break;
            }

            return ONI_STATUS_NOT_IMPLEMENTED;
        }

        OniBool K4AStream::isPropertySupported( int propertyId )
        {
            K4ALogDebug( "K4AStream::isPropertySupported : %d", propertyId );

            switch( propertyId )
            {
                case ONI_STREAM_PROPERTY_HORIZONTAL_FOV:
                case ONI_STREAM_PROPERTY_VERTICAL_FOV:
                case ONI_STREAM_PROPERTY_VIDEO_MODE:
                case ONI_STREAM_PROPERTY_STRIDE:
                case ONI_STREAM_PROPERTY_AUTO_WHITE_BALANCE:
                case ONI_STREAM_PROPERTY_AUTO_EXPOSURE:
                    return true;
                default:
                    return false;
            }
        }

        K4AColorStream::K4AColorStream( class K4ACapture* k4a_capture, class K4ADevice* k4a_device )
            : K4AStream( k4a_capture, k4a_device )
        {
            K4ALogDebug( "K4AColorStream::K4AColorStream" );

            k4a::calibration calibration = k4a_device->getCalibration();

            video_mode.pixelFormat = ONI_PIXEL_FORMAT_RGB888;
            video_mode.resolutionX = calibration.color_camera_calibration.resolution_width;
            video_mode.resolutionY = calibration.color_camera_calibration.resolution_height;
            video_mode.fps = 30;

            constexpr int32_t channels = 3;
            bytes_per_pixel = sizeof( uint8_t ) * channels;

            switch( calibration.color_camera_calibration.resolution_height ){
                case  720:
                case 1080:
                case 1440:
                case 2160:
                    horizontal_fov = static_cast<float>( 90.0 * 0.01745329251994329576923690768489 );
                    vertical_fov   = static_cast<float>( 59.0 * 0.01745329251994329576923690768489 );
                    break;
                case 1536:
                case 3072:
                    horizontal_fov = static_cast<float>( 90.0 * 0.01745329251994329576923690768489 );
                    vertical_fov   = static_cast<float>( 74.3 * 0.01745329251994329576923690768489 );
                    break;
                default:
                    break;
            }
        }

        K4AColorStream::~K4AColorStream()
        {
            K4ALogDebug( "K4AColorStream::~K4AColorStream" );
        }

        void K4AColorStream::MainLoop()
        {
            K4ATraceFunc( "" );

            int32_t frame_index = 0;

            while( is_running )
            {
                std::pair<std::vector<uint8_t>, std::chrono::microseconds> data;
                bool result = k4a_capture->get_color_image( data );
                if( !result ){
                    std::this_thread::sleep_for( std::chrono::milliseconds( REQUEST_WAIT_TIME ) );
                    continue;
                }

                std::vector<uint8_t> color_image     = data.first;
                std::chrono::microseconds time_stamp = data.second;

                OniFrame* pFrame = getServices().acquireFrame();

                k4a::calibration calibration = k4a_device->getCalibration();
                const int32_t width  = calibration.color_camera_calibration.resolution_width;
                const int32_t height = calibration.color_camera_calibration.resolution_height;

                pFrame->frameIndex            = frame_index++;
                pFrame->videoMode.pixelFormat = ONI_PIXEL_FORMAT_RGB888;
                pFrame->videoMode.resolutionX = width;
                pFrame->videoMode.resolutionY = height;
                pFrame->videoMode.fps         = 30;
                pFrame->width                 = width;
                pFrame->height                = height;

                OniRGB888Pixel* pixels = reinterpret_cast<OniRGB888Pixel*>( pFrame->data );
                uint8_t* buffer = reinterpret_cast<uint8_t*>( &color_image[0] );
                constexpr int32_t channels = 4;
                for( int32_t y = 0; y < height; y++ ){
                    for( int32_t x = 0; x < width; x++ ){
                        int32_t buffer_index = y * ( width * channels ) + x * channels;
                        int32_t pixels_index = y * width + x;
                        pixels[pixels_index].b = buffer[buffer_index + 0];
                        pixels[pixels_index].g = buffer[buffer_index + 1];
                        pixels[pixels_index].r = buffer[buffer_index + 2];
                    }
                }

                pFrame->cropOriginX     = 0;
                pFrame->cropOriginY     = 0;
                pFrame->croppingEnabled = FALSE;
                pFrame->sensorType      = ONI_SENSOR_COLOR;
                pFrame->stride          = width * sizeof( OniRGB888Pixel );
                pFrame->timestamp       = time_stamp.count();

                raiseNewFrame( pFrame );
                getServices().releaseFrame( pFrame );
            }
        }

        K4ADepthStream::K4ADepthStream( class K4ACapture* k4a_capture, class K4ADevice* k4a_device )
            : K4AStream( k4a_capture, k4a_device )
        {
            K4ALogDebug( "K4ADepthStream::K4ADepthStream" );

            k4a::calibration calibration = k4a_device->getCalibration();

            video_mode.pixelFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
            video_mode.resolutionX = calibration.depth_camera_calibration.resolution_width;
            video_mode.resolutionY = calibration.depth_camera_calibration.resolution_height;
            video_mode.fps = 30;

            bytes_per_pixel = sizeof( uint16_t );

            switch( calibration.depth_mode ){
                case k4a_depth_mode_t::K4A_DEPTH_MODE_NFOV_2X2BINNED:
                case k4a_depth_mode_t::K4A_DEPTH_MODE_NFOV_UNBINNED:
                    horizontal_fov = static_cast<float>( 75.0 * 0.01745329251994329576923690768489 );
                    vertical_fov   = static_cast<float>( 65.0 * 0.01745329251994329576923690768489 );
                    break;
                case k4a_depth_mode_t::K4A_DEPTH_MODE_WFOV_2X2BINNED:
                case k4a_depth_mode_t::K4A_DEPTH_MODE_WFOV_UNBINNED:
                    horizontal_fov = static_cast<float>( 120.0 * 0.01745329251994329576923690768489 );
                    vertical_fov   = static_cast<float>( 120.0 * 0.01745329251994329576923690768489 );
                    break;
                default:
                    break;
            }
        }

        K4ADepthStream::~K4ADepthStream()
        {
            K4ALogDebug( "K4ADepthStream::~K4ADepthStream" );
        }

        void K4ADepthStream::MainLoop()
        {
            K4ATraceFunc( "" );

            int32_t frame_index = 0;

            while( is_running )
            {
                std::pair<std::vector<uint16_t>, std::chrono::microseconds> data;
                bool result = k4a_capture->get_depth_image( data );
                if( !result ){
                    std::cout << "result is false!\n";
                    std::this_thread::sleep_for( std::chrono::milliseconds( REQUEST_WAIT_TIME ) );
                    continue;
                }
                else{
                    std::cout << "result is true!!\n";
                }

                std::vector<uint16_t> depth_image    = data.first;
                std::chrono::microseconds time_stamp = data.second;

                OniFrame* pFrame = getServices().acquireFrame();

                k4a::calibration calibration = k4a_device->getCalibration();
                const int32_t width  = calibration.depth_camera_calibration.resolution_width;
                const int32_t height = calibration.depth_camera_calibration.resolution_height;

                pFrame->frameIndex            = frame_index++;
                pFrame->videoMode.pixelFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
                pFrame->videoMode.resolutionX = width;
                pFrame->videoMode.resolutionY = height;
                pFrame->videoMode.fps         = 30;
                pFrame->width                 = width;
                pFrame->height                = height;

                OniDepthPixel* pixels = reinterpret_cast<OniDepthPixel*>( pFrame->data );
                uint16_t* buffer = reinterpret_cast<uint16_t*>( &depth_image[0] );
                size_t size = depth_image.size() * sizeof( uint16_t );
                memcpy( pixels, buffer, size );

                pFrame->cropOriginX     = 0;
                pFrame->cropOriginY     = 0;
                pFrame->croppingEnabled = FALSE;
                pFrame->sensorType      = ONI_SENSOR_DEPTH;
                pFrame->stride          = width * sizeof( OniDepthPixel );
                pFrame->timestamp       = time_stamp.count();

                raiseNewFrame( pFrame );
                getServices().releaseFrame( pFrame );
            }
        }

        K4AInfraredStream::K4AInfraredStream( K4ACapture* k4a_capture, K4ADevice* k4a_device )
            : K4AStream( k4a_capture, k4a_device )
        {
            K4ALogDebug( "K4AInfraredStream::K4AInfraredStream" );

            k4a::calibration calibration = k4a_device->getCalibration();

            video_mode.pixelFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
            video_mode.resolutionX = calibration.depth_camera_calibration.resolution_width;
            video_mode.resolutionY = calibration.depth_camera_calibration.resolution_height;
            video_mode.fps = 30;

            bytes_per_pixel = sizeof( uint16_t );

            switch( calibration.depth_mode ){
                case k4a_depth_mode_t::K4A_DEPTH_MODE_NFOV_2X2BINNED:
                case k4a_depth_mode_t::K4A_DEPTH_MODE_NFOV_UNBINNED:
                    horizontal_fov = static_cast<float>( 75.0 * 0.01745329251994329576923690768489 );
                    vertical_fov   = static_cast<float>( 65.0 * 0.01745329251994329576923690768489 );
                    break;
                case k4a_depth_mode_t::K4A_DEPTH_MODE_WFOV_2X2BINNED:
                case k4a_depth_mode_t::K4A_DEPTH_MODE_WFOV_UNBINNED:
                    horizontal_fov = static_cast<float>( 120.0 * 0.01745329251994329576923690768489 );
                    vertical_fov   = static_cast<float>( 120.0 * 0.01745329251994329576923690768489 );
                    break;
                default:
                    break;
            }
        }

        K4AInfraredStream::~K4AInfraredStream()
        {
            K4ALogDebug( "K4AInfraredStream::~K4AInfraredStream" );
        }

        void K4AInfraredStream::MainLoop()
        {
            K4ATraceFunc( "" );

            int32_t frame_index = 0;

            while( is_running )
            {
                std::pair<std::vector<uint16_t>, std::chrono::microseconds> data;
                bool result = k4a_capture->get_infrared_image( data );
                if( !result ){
                    std::this_thread::sleep_for( std::chrono::milliseconds( REQUEST_WAIT_TIME ) );
                    continue;
                }

                std::vector<uint16_t> infrared_image = data.first;
                std::chrono::microseconds time_stamp = data.second;

                OniFrame* pFrame = getServices().acquireFrame();

                k4a::calibration calibration = k4a_device->getCalibration();
                const int32_t width  = calibration.depth_camera_calibration.resolution_width;
                const int32_t height = calibration.depth_camera_calibration.resolution_height;

                pFrame->frameIndex            = frame_index++;
                pFrame->videoMode.pixelFormat = ONI_PIXEL_FORMAT_GRAY16;
                pFrame->videoMode.resolutionX = width;
                pFrame->videoMode.resolutionY = height;
                pFrame->videoMode.fps         = 30;
                pFrame->width                 = width;
                pFrame->height                = height;

                OniGrayscale16Pixel* pixels = reinterpret_cast<OniGrayscale16Pixel*>( pFrame->data );
                uint16_t* buffer = reinterpret_cast<uint16_t*>( &infrared_image[0] );
                size_t size = infrared_image.size() * sizeof( uint16_t );
                memcpy( pixels, buffer, size );

                pFrame->cropOriginX     = 0;
                pFrame->cropOriginY     = 0;
                pFrame->croppingEnabled = FALSE;
                pFrame->sensorType      = ONI_SENSOR_DEPTH;
                pFrame->stride          = width * sizeof( OniGrayscale16Pixel );
                pFrame->timestamp       = time_stamp.count();

                raiseNewFrame( pFrame );
                getServices().releaseFrame( pFrame );
            }
        }
    }
}
