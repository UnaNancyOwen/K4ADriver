K4ADriver
=========
This is OpenNI2 Driver for Azure Kinect.  
You can use OpenNI2 with Azure Kinect by install <code>k4adriver.dll</code> to OpenNI2 driver directory.

![image](https://user-images.githubusercontent.com/816705/69304852-09b8a400-0c1a-11ea-8b6a-8eb013b66fd8.jpg)

WIP
---
This driver is experimental implementation.  
The some features doesn't work yet.  

* Video Mode (Resolution, FPS, Pixel Format) Settings
* Multi Device Support
* NiTE2 Support

Environment
-----------
* Visual Studio 2019 / GCC 7.4 / Clang 6.0
* OpenNI2 2.2.0.33 Beta
* Azure Kinect Sensor SDK v1.3.0 (or later)
* CMake 3.15.4 (latest release is preferred)
* Intel Thread Building Blocks (latest release is preferred)

<sup>&#042; This driver requires Intel TBB only on Linux.</sup>  

License
-------
Copyright &copy; 2019 Tsukasa SUGIURA  
Distributed under the [MIT License](http://www.opensource.org/licenses/mit-license.php "MIT License | Open Source Initiative").

Contact
-------
* Tsukasa Sugiura  
    * <t.sugiura0204@gmail.com>  
    * <http://unanancyowen.com> 
