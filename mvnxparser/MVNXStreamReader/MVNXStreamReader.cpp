/*
 * Copyright: (C) 2018 iCub Facility
 * Author: Luca Tagliapietra, Diego Ferigo
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 *
 */

/**
 * @file MVNXStreamReader.cpp
 * @brief Validate and parse efficiently mvnx files
 * @author Luca Tagliapietra, Diego Ferigo
 * @date 18/04/2017
 */

#include "MVNXStreamReader.h"
#include <fstream>
#include <iostream>
#include <sstream>

using namespace xmlstream;
using namespace xmlstream::mvnx;

std::vector<double> stringToDoubles(const std::string& aString)
{
    std::vector<double> vec;
    std::istringstream s(aString);
    double d;
    while (s >> d) {
        vec.push_back(d);
    }
    return vec;
}

XMLContentPtrS MVNXStreamReader::getXmlTreeRoot() const
{
    return std::dynamic_pointer_cast<XMLContent>(m_XMLTreeRoot);
}

void MVNXStreamReader::printParsedDocument()
{
    // Initialize the XML file
    QXmlStreamReader xml;
    m_xmlFile.seek(0);
    xml.setDevice(&m_xmlFile);

    // Initialize the objects that will contain the metadata of the current
    // element
    QXmlStreamReader::TokenType tokenType;
    std::string tokenString;
    std::string elementName;
    // string elementNS; // TODO: Namespace support
    std::string elementText;
    QXmlStreamAttributes elementAttributes;

    while (!xml.atEnd()) {
        xml.readNext();

        tokenType = xml.tokenType();
        tokenString = xml.tokenString().toStdString();
        std::string elementName = xml.name().toString().toStdString();
        // string ElementNS   = xml.namespaceUri().toString().toStdString();
        std::string elementText = xml.text().toString().toStdString();
        elementAttributes = xml.attributes();

        if (not xml.isCharacters()) {
            std::cout << tokenString << ": " << elementName << std::endl;
            if (not elementAttributes.empty()) {
                for (const auto& element : elementAttributes)
                    std::cout << "   " << element.name().toString().toStdString() << "=\""
                              << element.value().toString().toStdString() << "\"" << std::endl;
            }
        }

        if (xml.isStartDocument() || xml.isEndDocument()) {
            std::cout << std::endl;
            continue;
        }

        // Print the content of the element
        if (xml.isCharacters() && not xml.isWhitespace())
            std::cout << tokenString << ": " << xml.text().toString().toStdString() << std::endl;
    }
}

void MVNXStreamReader::configureParser()
{
    if (m_xmlFileVersion == 3) {
        m_xmlKeysMap.at("pos") = "pos_s";
        m_xmlKeysMap.at("sensor_angular_velocity") = "sensorAngularVelocity";
        m_xmlKeysMap.at("sensor_acceleration") = "sensorAcceleration";
    } else if (m_xmlFileVersion == 4) {
        m_xmlKeysMap.at("pos") = "pos_b";
        m_xmlKeysMap.at("sensor_free_body_acceleration") = "sensorFreeAcceleration";
    } else {
        std::cerr << "Unrecognized MVNX version" << std::endl;
        exit(EXIT_FAILURE);
    }
}

bool MVNXStreamReader::parse()
{
    // Initialize the XML file
    QXmlStreamReader xml;
    m_xmlFile.seek(0);
    xml.setDevice(&m_xmlFile);

    // Initialize the objects that will contain the metadata of the current
    // element
    QXmlStreamReader::TokenType tokenType;
    std::string tokenString;
    std::string elementName;
    // string elementNS; // TODO: Namespace support
    std::string elementText;
    QXmlStreamAttributes elementAttributes;

    // Sequentially parse the file
    while (!xml.atEnd()) {
        xml.readNext();

        // Get the metadata of the element
        tokenType = xml.tokenType();
        tokenString = xml.tokenString().toStdString();
        elementName = xml.name().toString().toStdString();
        // elementNS   = xml.namespaceUri().toString().toStdString();
        elementText = xml.text().toString().toStdString();
        elementAttributes = xml.attributes();

        // Handle the generated events
        switch (tokenType) {
        case QXmlStreamReader::StartDocument:
            break;
        case QXmlStreamReader::StartElement:
            handleStartElement(elementName, elementAttributes);
            break;
        case QXmlStreamReader::Characters:
            handleCharacters(elementText);
            break;
        case QXmlStreamReader::Comment:
            handleComment(elementText);
            break;
        case QXmlStreamReader::EndElement:
            handleStopElement(elementName);
            break;
        case QXmlStreamReader::EndDocument:
            m_XMLTreeRoot = m_elementsLIFO.front();
            m_elementsLIFO.pop_back();
            m_xmlFileVersion = std::stoi(m_XMLTreeRoot->getAttribute("version"));
            configureParser();
            parseFrames();
            break;
        default:
            break;
        }
    }
    return true;
}

Attributes MVNXStreamReader::processAttributes(const QXmlStreamAttributes& attributes)
{
    // The content of QXmlStreamAttributes are QStringRef.
    // We need to convert them to std::string.
    Attributes stringAttributes;
    if (!attributes.empty()) {
        for (const auto& attribute : attributes) {
            assert(attribute.name().toString().toStdString().length() != 0);
            assert(attribute.value().toString().toStdString().length() != 0);
            // Process the attribute
            stringAttributes[attribute.name().toString().toStdString()]
                = attribute.value().toString().toStdString();
        }
    }
    return stringAttributes;
}

bool MVNXStreamReader::elementIsEnabled(const xmlstream::ElementName& name)
{
    // If the user didn't provide any configuration, all elements are enabled
    if (m_conf.empty()) {
        return true;
    }

    // If the name does not match any entry in the map, return false
    if (m_conf.find(name) == m_conf.end()) {
        return false;
    }

    return m_conf[name];
}

void MVNXStreamReader::handleStartElement(const xmlstream::ElementName& name,
                                          const QXmlStreamAttributes& attributes)
{
    if (elementIsEnabled(name)) {
        // Get a weak pointer to the parent
        parent_ptr parent = nullptr;
        if (!m_elementsLIFO.empty()) {
            parent = m_elementsLIFO.back();
        }

        // Push it in the buffer of pointers
        m_elementsLIFO.emplace_back(
            std::make_shared<XMLContent>(name, processAttributes(attributes), parent));
    }
}

void MVNXStreamReader::handleCharacters(const xmlstream::ElementText& text)
{
    IContentPtrS lastElement = m_elementsLIFO.back();
    assert(lastElement);
    lastElement->setText(text);
}

void MVNXStreamReader::handleComment(const xmlstream::ElementText& /*text*/)
{
    // TODO: this is not a mvnx <comment> but a XML comment
}

void MVNXStreamReader::handleStopElement(const xmlstream::ElementName& name)
{
    if (elementIsEnabled(name)) {
        if (m_elementsLIFO.size() > 1) { // TODO: check if >=
            // Extract parent and child elements
            IContentPtrS lastElement = m_elementsLIFO.back();
            IContentPtrS secondToLastElement = m_elementsLIFO.end()[-2];
            assert(lastElement && secondToLastElement); // Are not nullptr
            assert(lastElement->getElementName() == name);

            // Assign the child the parent element
            secondToLastElement->setChild(lastElement);

            // Delete che assigned child from the buffer of the XML tree
            m_elementsLIFO.pop_back();
        }
    }
}

const std::vector<XMLContentPtrS>
MVNXStreamReader::findElement(const xmlstream::ElementName& name) const
{
    if (!m_XMLTreeRoot) {
        return {};
    }

    std::vector<XMLContentPtrS> childElementsCast;
    std::vector<IContentPtrS> childElements = m_XMLTreeRoot->findChildElements(name);
    childElementsCast.resize(childElements.size());
    // Cast to XMLContent the vector's item using a lambda expression
    transform(childElements.begin(),
              childElements.end(),
              childElementsCast.begin(),
              [](const IContentPtrS& e) { return std::dynamic_pointer_cast<XMLContent>(e); });

    return childElementsCast;
}

const std::vector<Point> MVNXStreamReader::getPoints() const
{
    if (!m_XMLTreeRoot) {
        return {};
    }

    std::vector<Point> points;
    std::vector<XMLContentPtrS> pointVec = this->findElement(m_xmlKeysMap.at("point"));
    for (const auto& point : pointVec) {
        Point aPoint;
        aPoint.first = point->getParent()->getParent()->getAttribute("label") + ":"
                       + point->getAttribute("label");
        aPoint.second
            = stringToDoubles(point->getChildElement(m_xmlKeysMap.at("pos"))->front()->getText());
        points.push_back(aPoint);
    }
    return points;
}

const std::vector<JointInfo> MVNXStreamReader::getJointsInfo() const
{
    if (!m_XMLTreeRoot) {
        return {};
    }

    std::vector<JointInfo> jointsInfo;
    std::string mvnxDelimiter = "/";
    std::vector<XMLContentPtrS> jointsVec = this->findElement(m_xmlKeysMap.at("joint"));
    for (const auto& joint : jointsVec) {
        JointInfo aJoint;
        aJoint.front() = joint->getAttribute("label");
        std::string tmp = joint->getChildElement(m_xmlKeysMap.at("connector1"))->front()->getText();
        aJoint.at(1) = tmp.substr(0, tmp.find(mvnxDelimiter));
        tmp = joint->getChildElement(m_xmlKeysMap.at("connector2"))->front()->getText();
        aJoint.back() = tmp.substr(0, tmp.find(mvnxDelimiter));
        jointsInfo.push_back(aJoint);
    }

    return jointsInfo;
}

const std::vector<std::string> MVNXStreamReader::getNames(const std::string& attributeName) const
{
    if (!m_XMLTreeRoot) {
        return {};
    }

    std::vector<std::string> names;
    std::vector<XMLContentPtrS> elementsVec = this->findElement(attributeName);
    for (const auto& element : elementsVec) {
        names.push_back(element->getAttribute("label"));
    }
    return names;
};

bool MVNXStreamReader::fillFrameInfo(const xmlstream::XMLContentPtrS inFrame, FrameInfo& info) const
{
    if (inFrame->getElementName() != m_xmlKeysMap.at("frame")) {
        return false;
    }

    info.segmentCount = std::stoi(inFrame->getParent()->getAttribute("segmentCount"));
    info.sensorCount = std::stoi(inFrame->getParent()->getAttribute("sensorCount"));
    info.jointCount = std::stoi(inFrame->getParent()->getAttribute("jointCount"));

    info.timeFromStart = std::stoi(inFrame->getAttribute("time"));
    info.clockTime = inFrame->getAttribute("tc");
    info.clockTimems = std::stoul(inFrame->getAttribute("ms"));
    info.type = inFrame->getAttribute("type");

    // need to check since in MVNX v.4 calibration frames does not have indexes while in v.3 they
    // have negative ones
    if (!inFrame->getAttribute("index").empty()) {
        info.index = std::stoi(inFrame->getAttribute("index"));
    }

    return true;
}

bool MVNXStreamReader::parseFrame(const xmlstream::XMLContentPtrS inFrame,
                                  Frame& outFrame,
                                  const char& sep) const
{
    if (inFrame->getElementName() != m_xmlKeysMap.at("frame")) {
        return false;
    }

    // fill frame properties from frame header
    if (!fillFrameInfo(inFrame, outFrame.properties)) {
        return false;
    }

    for (const auto& children : *(inFrame->getChildElements())) {
        for (const auto& child : *(children.second)) {
            if (child->getElementName() != m_xmlKeysMap.at("contacts")) {
                outFrame.data.emplace(child->getElementName(), child->getText());
            } else {
                std::string out{};
                for (const auto& contacts : *(child->getChildElements())) {
                    for (const auto& contact : *(contacts.second)) {
                        out.append(contact->getAttribute("segment"));
                        out.append(":");
                        out.append(contact->getAttribute("point"));
                        out.append(std::string{sep});
                    }
                }
                outFrame.data.emplace(child->getElementName(), out);
            }
        }
    }
    return true;
}

void MVNXStreamReader::parseFrames()
{
    std::vector<XMLContentPtrS> frames = this->findElement(m_xmlKeysMap.at("frame"));
    for (auto& frame : frames) {
        Frame tmpFrame;
        if (!parseFrame(frame, tmpFrame)) {
            std::cerr << "Unable to parse frame " << frame->getAttribute(m_xmlKeysMap.at("index"))
                      << std::endl;
            exit(EXIT_FAILURE);
        };
        m_parsedFrames.push_back(std::make_shared<Frame>(tmpFrame));
    }
}

std::string MVNXStreamReader::getSingleDataTypeFromFrame(const std::string& label,
                                                         const Frame& frame,
                                                         const int& sampleSize,
                                                         const char& sep) const
{
    std::string out;
    if (frame.data.count(m_xmlKeysMap.at(label)) != 0
        && !frame.data.at(m_xmlKeysMap.at(label)).empty()) {
        std::string tmp = frame.data.at(m_xmlKeysMap.at(label));
        std::replace(tmp.begin(), tmp.end(), ' ', sep);
        out.append(tmp);
    } else {
        std::cerr << "Warning: attribute " << label << "not available for frame id "
                  << frame.properties.index << " leaving the field empty." << std::endl;
        for (int k = 0; k < sampleSize; ++k) {
            out.append(" " + std::string{sep});
        }
    }
    return out;
}

void MVNXStreamReader::printFrame(std::stringstream& ss,
                                  const Frame& frame,
                                  const std::vector<MVNXStreamReader::OutputDataType>& dataList,
                                  const char& sep) const
{
    for (const auto& dataType : dataList) {
        switch (dataType) {
        case LINK_POSITION:
            ss << getSingleDataTypeFromFrame(
                "link_position", frame, 3 * frame.properties.segmentCount, sep);
            break;
        case LINK_VELOCITY:
            ss << getSingleDataTypeFromFrame(
                "link_velocity", frame, 3 * frame.properties.segmentCount, sep);
            break;
        case LINK_ACCELERATION:
            ss << getSingleDataTypeFromFrame(
                "link_acceleration", frame, 3 * frame.properties.segmentCount, sep);
            break;
        case LINK_ORIENTATION:
            ss << getSingleDataTypeFromFrame(
                "link_orientation", frame, 4 * frame.properties.segmentCount, sep);
            break;
        case LINK_ANGULAR_VELOCITY:
            ss << getSingleDataTypeFromFrame(
                "link_angular_velocity", frame, 3 * frame.properties.segmentCount, sep);
            break;
        case LINK_ANGULAR_ACCELERATION:
            ss << getSingleDataTypeFromFrame(
                "link_angular_acceleration", frame, 3 * frame.properties.segmentCount, sep);
            break;
        case SENSOR_ORIENTATION:
            ss << getSingleDataTypeFromFrame(
                "sensor_orientation", frame, 4 * frame.properties.sensorCount, sep);
            break;
        case SENSOR_ANGULAR_VELOCITY:
            if (m_xmlKeysMap.at("sensor_angular_velocity").empty()) {
                std::cerr << "Output option sensor_angular_velocity not supported for the current "
                             "MVNX version"
                          << std::endl;
            } else {
                ss << getSingleDataTypeFromFrame(
                    "sensor_angular_velocity", frame, 3 * frame.properties.sensorCount, sep);
            }
            break;
        case SENSOR_ACCELERATION:
            if (m_xmlKeysMap.at("sensor_acceleration").empty()) {
                std::cerr << "Output option sensor_acceleration not supported for the current "
                             "MVNX version"
                          << std::endl;
            } else {
                ss << getSingleDataTypeFromFrame(
                    "sensor_acceleration", frame, 3 * frame.properties.sensorCount, sep);
            }
            break;
        case SENSOR_FREE_BODY_ACCELERATION:
            if (m_xmlKeysMap.at("sensor_free_body_acceleration").empty()) {
                std::cerr
                    << "Output option sensor_free_body_acceleration not supported for the current "
                       "MVNX version"
                    << std::endl;
            } else {
                ss << getSingleDataTypeFromFrame(
                    "sensor_free_body_acceleration", frame, 3 * frame.properties.sensorCount, sep);
            }
            break;
        case SENSOR_MAGNETIC_FIELD:
            ss << getSingleDataTypeFromFrame(
                "sensor_magnetic_field", frame, 3 * frame.properties.sensorCount, sep);
            break;
        case JOINT_ANGLE:
            ss << getSingleDataTypeFromFrame(
                "joint_angle", frame, 3 * frame.properties.jointCount, sep);
            break;
        case JOINT_ANGLE_XZY:
            ss << getSingleDataTypeFromFrame(
                "joint_angle_xzy", frame, 3 * frame.properties.jointCount, sep);
            break;
        case CENTER_OF_MASS:
            ss << getSingleDataTypeFromFrame("center_of_mass", frame, 3, sep);
            break;
        case CONTACTS:
            std::cerr << "TODO: Contacts are not yet supported. Ignoring for time being."
                      << std::endl;
            break;
        }
    }
    ss << std::endl;
}

void MVNXStreamReader::printCalibrationFile_LOG(const std::string& filePath, const char& sep) const
{
    std::stringstream ss;
    ss << "SegmentsList" << std::endl;
    for (auto& segment : getSegmentNames())
        ss << segment << sep;
    ss << std::endl << std::endl;

    ss << "SensorsList" << std::endl;
    for (auto& sensor : getSensorNames())
        ss << sensor << sep;
    ss << std::endl << std::endl;

    ss << "JointInfoList" << std::endl;
    ss << "JointName" << sep << "fromSegment" << sep << "toSegment" << std::endl;
    for (auto& joint : getJointsInfo())
        ss << joint.at(0) << sep << joint.at(1) << sep << joint.at(2) << std::endl;
    ss << std::endl;

    ss << "PointInfoList" << std::endl;
    ss << "Segment/PointName" << sep << "X" << sep << "Y" << sep << "Z" << std::endl;
    for (auto& point : getPoints())
        ss << point.first << sep << point.second.at(0) << sep << point.second.at(1) << sep
           << point.second.at(2) << std::endl;
    ss << std::endl;

    std::vector<std::string> segmentNames = getSegmentNames();
    ss << "FrameType" << sep;
    ss << createSingleTypeLabels(m_xmlKeysMap.at("link_position"),
                                 segmentNames,
                                 std::vector<std::string>{"X", "Y,", "Z"},
                                 sep);
    ss << createSingleTypeLabels(m_xmlKeysMap.at("link_orientation"),
                                 segmentNames,
                                 std::vector<std::string>{"W", "X", "Y,", "Z"},
                                 sep);
    ss << std::endl;

    for (int i = 0; m_parsedFrames.at(i)->properties.type != "normal" && i < m_parsedFrames.size();
         ++i) {
        ss << m_parsedFrames.at(i)->properties.type << sep;
        printFrame(ss,
                   *m_parsedFrames.at(i),
                   std::vector<MVNXStreamReader::OutputDataType>{LINK_POSITION, LINK_ORIENTATION},
                   sep);
    }

    std::ofstream myFile(filePath);
    myFile << ss.rdbuf();
    myFile.close();
}

void MVNXStreamReader::printCalibrationFile_XML(const std::string& filePath) const
{
    QFile outFile(QString(filePath.c_str()));
    outFile.open(QIODevice::WriteOnly);
    QXmlStreamWriter stream(&outFile);

    stream.setAutoFormatting(true);
    stream.writeStartDocument();

    // Retrieve and write subject element and its attributes
    const auto subject = m_XMLTreeRoot->findChildElements(m_xmlKeysMap.at("subject")).front();
    stream.writeStartElement(subject->getElementName().c_str());
    for (const auto& attr : subject->getAttributes()) {
        stream.writeAttribute(attr.first.c_str(), attr.second.c_str());
    }

    // Retrieve and write comment element
    stream.writeTextElement(
        m_xmlKeysMap.at("comment").c_str(),
        m_XMLTreeRoot->findChildElements(m_xmlKeysMap.at("comment")).front()->getText().c_str());

    // Retrieve and write segment elements and their attributes
    stream.writeStartElement(m_xmlKeysMap.at("segments").c_str());
    for (const auto& segment : m_XMLTreeRoot->findChildElements(m_xmlKeysMap.at("segment"))) {
        stream.writeStartElement(segment->getElementName().c_str());
        for (const auto& attr : segment->getAttributes()) {
            stream.writeAttribute(attr.first.c_str(), attr.second.c_str());
        }

        stream.writeStartElement(m_xmlKeysMap.at("points").c_str()); // open points tag
        for (const auto& point : segment->findChildElements(m_xmlKeysMap.at("point"))) {
            stream.writeStartElement(point->getElementName().c_str()); // open point tag
            for (const auto& attr : point->getAttributes()) {
                stream.writeAttribute(attr.first.c_str(), attr.second.c_str());
            }
            for (const auto& pos : point->findChildElements(m_xmlKeysMap.at("pos"))) {
                stream.writeTextElement(pos->getElementName().c_str(),
                                        pos->getText().c_str()); // write pos elements
            }
            stream.writeEndElement(); // close point tag
        }
        stream.writeEndElement(); // close points tag
        stream.writeEndElement(); // close segment tag
    }
    stream.writeEndElement(); // close segments tag

    // Retrieve and write sensor elements and their attributes
    stream.writeStartElement(m_xmlKeysMap.at("sensors").c_str()); // open sensors tag
    for (const auto& sensor : m_XMLTreeRoot->findChildElements(m_xmlKeysMap.at("sensor"))) {
        stream.writeStartElement(sensor->getElementName().c_str());
        for (const auto& attr : sensor->getAttributes()) {
            stream.writeAttribute(attr.first.c_str(), attr.second.c_str());
        }
        stream.writeEndElement(); // close sensor tag
    }
    stream.writeEndElement(); // close sensors tag

    // Retrieve and write joint elements and their attributes
    stream.writeStartElement(m_xmlKeysMap.at("joints").c_str()); // open joints tag
    for (const auto& joint : m_XMLTreeRoot->findChildElements(m_xmlKeysMap.at("joint"))) {
        stream.writeStartElement(joint->getElementName().c_str()); // open joint tag
        for (const auto& attr : joint->getAttributes()) {
            stream.writeAttribute(attr.first.c_str(), attr.second.c_str());
        }
        for (const auto& children : *(joint->getChildElements())) {
            for (const auto& child : *(children.second)) {
                stream.writeTextElement(child->getElementName().c_str(), child->getText().c_str());
            }
        }
        stream.writeEndElement(); // close joint tag
    }
    stream.writeEndElement(); // close joints tag

    // Retrieve and write calibration frame elements and their attributes
    stream.writeStartElement(m_xmlKeysMap.at("frames").c_str()); // open frames tag
    auto frames = m_XMLTreeRoot->findChildElements(m_xmlKeysMap.at("frames")).front();
    for (const auto& attr : frames->getAttributes()) {
        stream.writeAttribute(attr.first.c_str(), attr.second.c_str());
    }
    for (const auto& frame : frames->findChildElements(m_xmlKeysMap.at("frame"))) {
        if (frame->getAttribute("type") == "normal") {
            break;
        } else {
            stream.writeStartElement(frame->getElementName().c_str()); // open frame tag
            for (const auto& attr : frame->getAttributes()) {
                stream.writeAttribute(attr.first.c_str(), attr.second.c_str());
            }
            for (const auto& children : *(frame->getChildElements())) {
                for (const auto& child : *(children.second)) {
                    stream.writeTextElement(children.first.c_str(), child->getText().c_str());
                }
            }
            stream.writeEndElement(); // close frame tag
        }
    }
    stream.writeEndElement(); // close frames tag

    stream.writeEndDocument(); // close remaining tag, should be just subject
    outFile.close();
}

std::string MVNXStreamReader::createSingleTypeLabels(const std::string& prefix,
                                                     const std::vector<std::string>& itemLabels,
                                                     const std::vector<std::string>& postfixes,
                                                     const char& sep) const
{
    std::stringstream out;
    for (const auto& item : itemLabels) {
        for (const auto& postfix : postfixes) {
            out << prefix << ":" << item << "." << postfix << sep;
        };
    }
    return out.str();
}

void MVNXStreamReader::createLabels(std::stringstream& ss,
                                    const std::vector<MVNXStreamReader::OutputDataType>& dataList,
                                    const char& sep) const
{
    ss.clear();
    const std::vector<std::string> segmentNames = getSegmentNames();
    const std::vector<std::string> sensorNames = getSensorNames();
    const std::vector<std::string> jointNames = getJointNames();

    ss << "index" << sep << "msTime" << sep;

    for (const auto& dataType : dataList) {
        switch (dataType) {
        case LINK_POSITION:
            ss << createSingleTypeLabels(m_xmlKeysMap.at("link_position"),
                                         segmentNames,
                                         std::vector<std::string>{"X", "Y", "Z"},
                                         sep);
            break;
        case LINK_VELOCITY:
            ss << createSingleTypeLabels(m_xmlKeysMap.at("link_velocity"),
                                         segmentNames,
                                         std::vector<std::string>{"X", "Y", "Z"},
                                         sep);
            break;
        case LINK_ACCELERATION:
            ss << createSingleTypeLabels(m_xmlKeysMap.at("link_acceleration"),
                                         segmentNames,
                                         std::vector<std::string>{"X", "Y", "Z"},
                                         sep);
            break;
        case LINK_ORIENTATION:
            ss << createSingleTypeLabels(m_xmlKeysMap.at("link_orientation"),
                                         segmentNames,
                                         std::vector<std::string>{"W", "X", "Y", "Z"},
                                         sep);
            break;
        case LINK_ANGULAR_VELOCITY:
            ss << createSingleTypeLabels(m_xmlKeysMap.at("link_angular_velocity"),
                                         segmentNames,
                                         std::vector<std::string>{"X", "Y", "Z"},
                                         sep);
            break;
        case LINK_ANGULAR_ACCELERATION:
            ss << createSingleTypeLabels(m_xmlKeysMap.at("link_angular_acceleration"),
                                         segmentNames,
                                         std::vector<std::string>{"X", "Y", "Z"},
                                         sep);
            break;
        case SENSOR_ORIENTATION:
            ss << createSingleTypeLabels(m_xmlKeysMap.at("sensor_orientation"),
                                         sensorNames,
                                         std::vector<std::string>{"W", "X", "Y", "Z"},
                                         sep);
            break;
        case SENSOR_ANGULAR_VELOCITY:
            if (m_xmlKeysMap.at("sensor_angular_velocity").empty()) {
                std::cerr << "Output option sensor_angular_velocity not supported for the current "
                             "MVNX version"
                          << std::endl;
            } else {
                ss << createSingleTypeLabels(m_xmlKeysMap.at("sensor_angular_velocity"),
                                             sensorNames,
                                             std::vector<std::string>{"X", "Y", "Z"},
                                             sep);
            }
            break;
        case SENSOR_ACCELERATION:
            if (m_xmlKeysMap.at("sensor_acceleration").empty()) {
                std::cerr << "Output option sensor_acceleration not supported for the current "
                             "MVNX version"
                          << std::endl;
            } else {
                ss << createSingleTypeLabels(m_xmlKeysMap.at("sensor_acceleration"),
                                             sensorNames,
                                             std::vector<std::string>{"X", "Y", "Z"},
                                             sep);
            }
            break;
        case SENSOR_FREE_BODY_ACCELERATION:
            if (m_xmlKeysMap.at("sensor_free_body_acceleration").empty()) {
                std::cerr
                    << "Output option sensor_free_body_acceleration not supported for the current "
                       "MVNX version"
                    << std::endl;
            } else {
                ss << createSingleTypeLabels(m_xmlKeysMap.at("sensor_free_body_acceleration"),
                                             sensorNames,
                                             std::vector<std::string>{"X", "Y", "Z"},
                                             sep);
            }
            break;
        case SENSOR_MAGNETIC_FIELD:
            ss << createSingleTypeLabels(m_xmlKeysMap.at("sensor_magnetic_field"),
                                         sensorNames,
                                         std::vector<std::string>{"X", "Y", "Z"},
                                         sep);
            break;
        case JOINT_ANGLE:
            ss << createSingleTypeLabels(m_xmlKeysMap.at("joint_angle"),
                                         jointNames,
                                         std::vector<std::string>{"X", "Y", "Z"},
                                         sep);
            break;
        case JOINT_ANGLE_XZY:
            ss << createSingleTypeLabels(m_xmlKeysMap.at("joint_angle_xzy"),
                                         jointNames,
                                         std::vector<std::string>{"X", "Y", "Z"},
                                         sep);
            break;
        case CENTER_OF_MASS:
            ss << createSingleTypeLabels(m_xmlKeysMap.at("center_of_mass"),
                                         std::vector<std::string>{"com"},
                                         std::vector<std::string>{"X", "Y", "Z"},
                                         sep);
            break;
        case CONTACTS:
            std::cerr << "TODO: Contacts not yet supported. Ignoring contacs for time being."
                      << std::endl;
            break;
        }
    }
    ss << std::endl;
}

void MVNXStreamReader::printDataFile(const std::string& filePath,
                                     const std::vector<MVNXStreamReader::OutputDataType>& dataList,
                                     const char& sep) const
{
    std::stringstream ss;
    createLabels(ss, dataList, sep);

    for (unsigned long i = 0; i < m_parsedFrames.size(); ++i) {
        if (m_parsedFrames.at(i)->properties.type == "normal") {
            ss << m_parsedFrames.at(i)->properties.index << sep
               << m_parsedFrames.at(i)->properties.clockTimems << sep;
            printFrame(ss, *m_parsedFrames.at(i), dataList, sep);
        }
    }

    std::ofstream myFile(filePath);
    myFile << ss.rdbuf();
    myFile.close();
}
