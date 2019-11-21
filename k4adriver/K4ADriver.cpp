#include "K4AUtil.h"
#include "K4ADriver.h"

#include <cctype>
#include <string>

namespace oni
{
    namespace driver
    {
        K4ADriver::K4ADriver( OniDriverServices* pDriverServices )
            : DriverBase( pDriverServices )
        {
            K4ALogDebug( "K4ADriver::K4ADriver" );
        }

        K4ADriver::~K4ADriver()
        {
            K4ALogDebug( "K4ADriver::~K4ADriver" );
            shutdown();
        }

        OniStatus K4ADriver::initialize( DeviceConnectedCallback connectedCallback, DeviceDisconnectedCallback disconnectedCallback, DeviceStateChangedCallback deviceStateChangedCallback, void* pCookie )
        {
            K4ATraceFunc( "" );

            OniStatus result = DriverBase::initialize( connectedCallback, disconnectedCallback, deviceStateChangedCallback, pCookie );
            if( result != ONI_STATUS_OK ){
                K4ATraceError( "DriverBase::initialize failed" );
                return result;
            }

            const int32_t device_count = k4a::device::get_installed_count();
            if( device_count == 0 ){
                K4ATraceError( "k4a::device::get_installed_count failed" );
                return ONI_STATUS_NO_DEVICE;
            }

            OniDeviceInfo info;
            strncpy_s( info.uri   , sizeof( info.uri    ), "0"         , sizeof( info.uri    ) - 1 );
            strncpy_s( info.name  , sizeof( info.name   ), "PS1080"    , sizeof( info.name   ) - 1 );
            strncpy_s( info.vendor, sizeof( info.vendor ), "PrimeSense", sizeof( info.vendor ) - 1 );
            info.usbVendorId  = 7463;
            info.usbProductId = 1537;
            deviceConnected( &info );
            deviceStateChanged( &info, 0 );

            K4ALogDebug( "K4ADriver INITIALIZED" );
            return ONI_STATUS_OK;
        }

        void K4ADriver::shutdown()
        {
            K4ATraceFunc( "" );

            if( device ){
                device.close();
            }
        }

        DeviceBase* K4ADriver::deviceOpen( const char* uri, const char* mode )
        {
            K4ATraceFunc( "uri = %s, mode = %s", uri, mode );

            try{
                if( !device ){
                    const int32_t index = std::isdigit( *uri ) ? std::stoi( uri ) : K4A_DEVICE_DEFAULT;
                    device = k4a::device::open( index );
                }
            }
            catch( const k4a::error & error ){
                K4ATraceError( "k4a::device::open failed - %s", error.what() );
                return nullptr;
            }

            return new K4ADevice( this, &device );
        }

        void K4ADriver::deviceClose( DeviceBase* pDevice )
        {
            K4ATraceFunc( "" );

            if( pDevice ){
                delete pDevice;
            }
        }

        OniStatus K4ADriver::tryDevice( const char* uri )
        {
            K4ATraceFunc( "uri = %s", uri );

            return ONI_STATUS_OK;
        }

        void* K4ADriver::enableFrameSync( StreamBase** pStreams, int streamCount )
        {
            K4ATraceFunc( "" );

            return nullptr;
        }

        void K4ADriver::disableFrameSync( void* frameSyncGroup )
        {
            K4ATraceFunc( "" );

            return;
        }
    }
}

namespace oni
{
    namespace driver
    {
        ONI_EXPORT_DRIVER( K4ADriver );
    }
}
