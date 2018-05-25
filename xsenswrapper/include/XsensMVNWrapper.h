/*
 * Copyright(C) 2016-2017 iCub Facility
 * Authors: Francesco Romano, Luca Tagliapietra
 * CopyPolicy : Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 */

#ifndef YARP_XSENSMVNWRAPPER_H
#define YARP_XSENSMVNWRAPPER_H

#include <yarp/dev/DeviceDriver.h>
#include <yarp/dev/PreciselyTimed.h>
#include <yarp/dev/Wrapper.h>

namespace yarp {
    namespace dev {
        class XsensMVNWrapper;
    }

    namespace os {
        template <typename type>
        class BufferedPort;
        class Port;
    } // namespace os
} // namespace yarp

namespace xsens {
    class XsensSegmentsFrame;
    class XsensSensorsFrame;
} // namespace xsens

class yarp::dev::XsensMVNWrapper
    : public yarp::dev::DeviceDriver
    , public yarp::dev::IPreciselyTimed
    , public yarp::dev::IWrapper
    , public yarp::dev::IMultipleWrapper
{

    class XsensMVNWrapperPrivate;
    XsensMVNWrapperPrivate* m_pimpl;
    friend class XsensMVNWrapperPrivate;

    // explicitly define the ports we open
    yarp::os::BufferedPort<xsens::XsensSegmentsFrame>* m_segmentsOutputPort;
    yarp::os::BufferedPort<xsens::XsensSensorsFrame>* m_sensorsOutputPort;
    yarp::os::Port* m_commandPort; // this implements the RPC thrift/XsensDriverService service

public:
    XsensMVNWrapper();
    virtual ~XsensMVNWrapper();

    // DeviceDriver interface
    bool open(yarp::os::Searchable& config);
    bool close();

    // IPreciselyTimed interface
    virtual yarp::os::Stamp getLastInputStamp();

    // IWrapper interface
    virtual bool attach(yarp::dev::PolyDriver* poly);
    virtual bool detach();

    // IMultipleWrapper interface
    virtual bool attachAll(const yarp::dev::PolyDriverList&);
    virtual bool detachAll();
};

#endif // end of YARP_XSENSMVNWRAPPER_H
