/*
 * Copyright: (C) 2017 iCub Facility
 * Author: Diego Ferigo
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 *
 */

/**
 * @file MVNXStreamReader.h
 * @brief Validate and parse efficiently mvnx files
 * @author Diego Ferigo
 * @date 18/04/2017
 */
#ifndef MVNX_STREAM_READER_H
#define MVNX_STREAM_READER_H

#include "XMLDataContainers.h"
#include "XMLStreamReader.h"

#include <QXmlStreamReader>
#include <array>
#include <unordered_map>

namespace xmlstream {
namespace mvnx {
const int INVALID_FRAME_INDEX = -10;
class MVNXStreamReader;
} // namespace mvnx
} // namespace xmlstream

// Simple container of the mvnx parsing configuration.
using Point = std::pair<std::string, std::vector<double>>;
using JointInfo = std::array<std::string, 3>;
using ConfigurationEntry = std::string;
using MVNXConfiguration = std::unordered_map<ConfigurationEntry, bool>;

struct FrameInfo {
    int segmentCount = -1;
    int sensorCount = -1;
    int jointCount = -1;
    int timeFromStart = -1;
    int index = xmlstream::mvnx::INVALID_FRAME_INDEX;
    unsigned long long clockTimems = 0;
    std::string clockTime = "";
    std::string type = "";
};

struct Frame {
    // Frame properties
    FrameInfo properties;
    std::map<std::string, std::string> data;
    std::vector<std::string> contacts;
};

class xmlstream::mvnx::MVNXStreamReader : public xmlstream::XMLStreamReader {
private:
    int m_xmlFileVersion = -1;

    int m_nSensors = -1;
    int m_nSegments = -1;
    int m_nJoints = -1;

    MVNXConfiguration m_conf; // TODO: REMOVE not supported anymore
    xmlstream::IContentPtrS m_XMLTreeRoot = nullptr;
    std::vector<xmlstream::IContentPtrS> m_elementsLIFO;

    std::vector<std::shared_ptr<Frame>> m_parsedFrames;

    std::map<std::string, std::string> m_xmlKeysMap{};

public:
    enum OutputDataType {
        LINK_POSITION,
        LINK_VELOCITY,
        LINK_ACCELERATION,
        LINK_ORIENTATION,
        LINK_ANGULAR_VELOCITY,
        LINK_ANGULAR_ACCELERATION,
        SENSOR_ORIENTATION,
        SENSOR_ANGULAR_VELOCITY,
        SENSOR_ACCELERATION,
        SENSOR_FREE_BODY_ACCELERATION,
        SENSOR_MAGNETIC_FIELD,
        JOINT_ANGLE,
        JOINT_ANGLE_XZY,
        CENTER_OF_MASS,
        CONTACTS // TODO: not yet supported
    };

    MVNXStreamReader();
    virtual ~MVNXStreamReader() = default;

    // Get methods
    MVNXConfiguration getConf() const { return m_conf; } // TODO: TO REMOVE
    xmlstream::XMLContentPtrS getXmlTreeRoot() const;

    // Set methods
    void setConf(const MVNXConfiguration& conf) { m_conf = conf; } // TODO: TO REMOVE

    // Exposed API for parsing, displaying and handling the document
    bool parse() override;
    void printParsedDocument(); // TODO: decide if moving into XML class

    const std::vector<xmlstream::XMLContentPtrS>
    findElement(const xmlstream::ElementName& elementName) const;

    // Helper functions
    const std::vector<Point> getPoints() const;
    const std::vector<JointInfo> getJointsInfo() const;

    const std::vector<std::string> getJointNames() const
    {
        return getNames(m_xmlKeysMap.at("joint"));
    }
    const std::vector<std::string> getSensorNames() const
    {
        return getNames(m_xmlKeysMap.at("sensor"));
    }
    const std::vector<std::string> getPointNames() const
    {
        return getNames(m_xmlKeysMap.at("point"));
    }
    const std::vector<std::string> getSegmentNames() const
    {
        return getNames(m_xmlKeysMap.at("segment"));
    }

    void printCalibrationFile_LOG(const std::string& filePath, const char& sep = '\t') const;
    void printCalibrationFile_XML(const std::string& filePath) const;
    void printDataFile(const std::string& filePath,
                       const std::vector<MVNXStreamReader::OutputDataType>& dataList,
                       const char& sep = '\t') const;

private:
    void handleStartElement(const xmlstream::ElementName& name,
                            const QXmlStreamAttributes& attributes);
    void handleCharacters(const xmlstream::ElementText& text);
    void handleComment(const xmlstream::ElementText& text);
    void handleStopElement(const xmlstream::ElementName& name);
    bool elementIsEnabled(const xmlstream::ElementName& name);
    xmlstream::Attributes processAttributes(const QXmlStreamAttributes& attributes);

    void configureParser();

    const std::vector<std::string> getNames(const std::string& attributeName) const;

    bool fillFrameInfo(const xmlstream::XMLContentPtrS inFrame, FrameInfo& info) const;
    bool parseFrame(const xmlstream::XMLContentPtrS inFrame,
                    Frame& outFrame,
                    const char& sep = '\t') const;
    void parseFrames();

    void printFrame(std::stringstream& out,
                    const Frame& frame,
                    const std::vector<MVNXStreamReader::OutputDataType>& dataList,
                    const char& sep = '\t') const;
    void createLabels(std::stringstream& ss,
                      const std::vector<MVNXStreamReader::OutputDataType>& dataList,
                      const char& sep = '\t') const;

    std::string createSingleTypeLabels(const std::string& prefix,
                                       const std::vector<std::string>& itemLabels,
                                       const std::vector<std::string>& postfixes,
                                       const char& sep = '\t') const;

    std::string getSingleDataTypeFromFrame(const std::string& label,
                                           const Frame& frame,
                                           const int& sampleSize,
                                           const char& sep = '\t') const;
};

#endif // MVNX_STREAM_READER_H
