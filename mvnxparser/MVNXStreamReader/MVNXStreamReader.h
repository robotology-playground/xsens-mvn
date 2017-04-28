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
#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

namespace xmlstream {
namespace mvnx {

// Simple container of the mvnx parsing configuration.
typedef std::unordered_map<std::string, bool> MVNXConfiguration;

class MVNXStreamReader : public xmlstream::XMLStreamReader {
private:
    MVNXConfiguration conf;
    IContentPtrS XMLTreeRoot;
    std::vector<IContentPtrS> elementsLIFO;

public:
    MVNXStreamReader() : XMLTreeRoot(nullptr) {}
    ~MVNXStreamReader() = default;

    // Get methods
    MVNXConfiguration getConf() const { return conf; }
    XMLContentPtrS getXmlTreeRoot() const;

    // Set methods
    void setConf(MVNXConfiguration _conf) { conf = _conf; }

    // Exposed API for parsing, displaying and handling the document
    bool parse();
    void printParsedDocument();
    std::vector<XMLContentPtrS> findElement(std::string ElementName);

private:
    void handleStartElement(std::string ElementName,
                            QXmlStreamAttributes elementAttributes);
    void handleCharacters(std::string elementText);
    void handleComment(std::string elementText);
    void handleStopElement(std::string ElementName);
    bool elementIsEnabled(std::string ElementName);
    xmlstream::attributes_t
    processAttributes(QXmlStreamAttributes elementAttributes);
};

} // namespace mvnx
} // namespace xmlstream

#endif // MVNX_STREAM_READER_H
