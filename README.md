Yarp drivers for the Xsens MVN system.
==========================================
[![Build Status](https://travis-ci.org/robotology-playground/xsens-mvn.svg?branch=master)](https://travis-ci.org/robotology-playground/xsens-mvn)


### Dependencies
- Xsens MVN SDK (4.2)
- YARP >= 2.3.68

### Architecture
The driver is implemented with two devices (currently available only on Windows due to restrictions of the Xsens SDK):
- `xsens_mvn`: is the proper device. It is responsible to open the xsens xme.dll and connect to the suit.
It implements a `yarp::dev::IHumanSkeleton` interface.
- `xsens_mvn_wrapper`: it opens an output streaming port and an rpc port for commands. Data are streamed periodically.
Furthermore an additional driver is provided, which is the `xsens_mvn_remote`. This driver can be used on any YARP-supported platform and it is responsible of connecting to the `xsens_mvn_wrapper` via YARP ports and exposing a software interface to the `yarp::dev::IHumanSkeleton` YARP interface.

### Development & License
When you develop on this project rememeber to check the following:
- the correct USB Dongle (the one with the DEV license) must be plugged into the computer
- at least xme.dll, xsensdeviceapi.dll and xmedef.xsb must be found by the executable. 
  - regarding the last file, by default it must be in the working directory. It is possible to call in the code the function `xmeSetPaths` to set, respectively:
  
  > the path to the xmedef.xsb file, also used for the .mvnc files, the path where props.xsb can be found, the path where you want the xme.log to be put and a Boolean value if you want to remove the old xme.log (if you specified a new path). Specifying empty strings will use the default path for those options.
  
Furthermore:
- connect the suit to the router
- connect the router to the pc (the router creates a subnet with the pc)

On Release, remember to connect to the computer the USB dongle with the Redistributable license on it.
