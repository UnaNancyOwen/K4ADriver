#pragma once

#include <iostream>

#ifndef XN_NEW
#define XN_NEW(type, arg) new type(arg)
#endif

#ifndef XN_DELETE
#define XN_DELETE(arg) delete arg
#endif

#ifndef XN_MODULE_PROPERTY_AHB
#define XN_MODULE_PROPERTY_AHB 0x1080E005
#endif

#define K4ATraceError( format, ... ) printf( "[K4A] ERROR at FILE %s LINE %d FUNC %s\n\t" format "\n", __FILE__, __LINE__, __FUNCTION__, ##  __VA_ARGS__)
#define K4ATraceFunc( format, ... )  printf( "[K4A] %s " format "\n", __FUNCTION__, ##  __VA_ARGS__)
#define K4ALogDebug( format, ... )   printf( "[K4A] " format "\n", ## __VA_ARGS__ )
