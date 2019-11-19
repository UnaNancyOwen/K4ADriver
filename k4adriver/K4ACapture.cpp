#include "K4AUtil.h"
#include "K4ACapture.h"

namespace oni
{
    namespace driver
    {
        K4ACapture::K4ACapture( k4a::device* device )
            : device( device )
        {
            K4ALogDebug( "K4ACapture::K4ACapture" );

            start();
        }

        K4ACapture::~K4ACapture()
        {
            K4ALogDebug( "K4ACapture::~K4ACapture" );

            stop();
        }

        void K4ACapture::start()
        {
            K4ATraceFunc( "" );

            is_capture = true;

            thread = std::thread( &K4ACapture::capture_thread, this );
        }

        void K4ACapture::stop()
        {
            K4ATraceFunc( "" );

            is_capture = false;

            if( thread.joinable() ){
                thread.join();
            }
        }

        bool K4ACapture::get_color_image( std::pair<std::vector<uint8_t>, std::chrono::microseconds>& color_data )
        {
            return color_queue.try_pop( color_data );
        }

        bool K4ACapture::get_depth_image( std::pair<std::vector<uint16_t>, std::chrono::microseconds>& depth_data )
        {
            return depth_queue.try_pop( depth_data );
        }

        bool K4ACapture::get_infrared_image( std::pair<std::vector<uint16_t>, std::chrono::microseconds>& infrared_data )
        {
            return infrared_queue.try_pop( infrared_data );
        }

        void K4ACapture::capture_thread()
        {
            K4ATraceFunc( "" );

            while( is_capture ){
                bool result = device->get_capture( &capture, std::chrono::milliseconds( K4A_WAIT_INFINITE ) );
                if( !result ){
                    capture.reset();
                    continue;
                }

                {
                    if( color_queue.unsafe_size() > MAX_QUEUE_SIZE ){
                        std::pair<std::vector<uint8_t>, std::chrono::microseconds> data;
                        color_queue.try_pop( data );
                    }

                    std::vector<uint8_t> buffer;
                    std::chrono::microseconds time_stamp;
                    k4a::image image = capture.get_color_image();
                    if( image ){
                        buffer.assign( image.get_buffer(), image.get_buffer() + image.get_size() );
                        time_stamp = image.get_device_timestamp();
                    }
                    image.reset();

                    color_queue.push( std::make_pair( buffer, time_stamp ) );
                }

                {
                    if( depth_queue.unsafe_size() > MAX_QUEUE_SIZE ){
                        std::pair<std::vector<uint16_t>, std::chrono::microseconds> data;
                        depth_queue.try_pop( data );
                        std::pair<std::vector<uint16_t>, std::chrono::microseconds>().swap( data );
                    }

                    std::vector<uint16_t> buffer;
                    std::chrono::microseconds time_stamp;
                    k4a::image image = capture.get_depth_image();
                    if( image ){
                        buffer.assign( reinterpret_cast<uint16_t*>( image.get_buffer() ), reinterpret_cast<uint16_t*>( image.get_buffer() + image.get_size() ) );
                        time_stamp = image.get_device_timestamp();
                    }
                    image.reset();

                    depth_queue.push( std::make_pair( buffer, time_stamp ) );
                }

                {
                    if( infrared_queue.unsafe_size() > MAX_QUEUE_SIZE ){
                        std::pair<std::vector<uint16_t>, std::chrono::microseconds> data;
                        infrared_queue.try_pop( data );
                        std::pair<std::vector<uint16_t>, std::chrono::microseconds>().swap( data );
                    }

                    std::vector<uint16_t> buffer;
                    std::chrono::microseconds time_stamp;
                    k4a::image image = capture.get_ir_image();
                    if( image ){
                        buffer.assign( reinterpret_cast<uint16_t*>( image.get_buffer() ), reinterpret_cast<uint16_t*>( image.get_buffer() + image.get_size() ) );
                        time_stamp = image.get_device_timestamp();
                    }
                    image.reset();

                    infrared_queue.push( std::make_pair( buffer, time_stamp ) );
                }

                capture.reset();
            }
        }
    }
}
