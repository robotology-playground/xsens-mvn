/*
 * Copyright: (C) 2017 iCub Facility
 * Author: Diego Ferigo
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 *
 */

/**
 * @file mvnxStreamReader.h
 * @brief Validate and parse efficiently mvnx files
 * @author Diego Ferigo
 * @date 12/04/2017
 */
#ifndef MVNX_STREAM_READER_H
#define MVNX_STREAM_READER_H

#include "xmlDataContainers.h"
#include "xmlStreamReader.h"
#include <QXmlStreamReader>
#include <iostream>
#include <unordered_map>
#include <vector>

namespace xmlstream {
namespace mvnx {

// Simple container of the mvnx parsing configuration.
typedef std::unordered_map<std::string, bool> mvnxConf;

class mvnxStreamReader : public xmlstream::xmlStreamReader {
private:
    mvnxConf conf;
    xmlstream::xmlContent* xmlTreeRoot;
    std::vector<xmlstream::xmlContent*> elementsLIFO;

public:
    mvnxStreamReader() : xmlTreeRoot(nullptr){};

    // Get methods
    mvnxConf getConf() const { return conf; };
    xmlstream::xmlContent* getXmlTreeRoot() const { return xmlTreeRoot; };

    // Set methods
    void setConf(mvnxConf _conf) { conf = _conf; };

    // Exposed API for parsing, displaying and handling the document
    bool parse();
    void printParsedDocument();
    std::vector<xmlstream::xmlContent*> findElement(std::string elementName);

private:
    void handleStartElement(std::string elementName,
                            QXmlStreamAttributes elementAttributes);
    void handleCharacters(std::string elementText);
    void handleComment(std::string elementText);
    void handleStopElement(std::string elementName);
    bool elementIsEnabled(std::string elementName);
    xmlstream::attributes_t
    processAttributes(QXmlStreamAttributes elementAttributes);
};

} // namespace mvnx
} // namespace xmlstream

#endif // MVNX_STREAM_READER_H
