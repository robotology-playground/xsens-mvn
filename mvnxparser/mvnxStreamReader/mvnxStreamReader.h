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

namespace mvnx_ns {

// Simple container of the mvnx parsing configuration.
typedef unordered_map<string, bool> mvnxConf;

class mvnxStreamReader : public xmlStreamReader {
private:
    mvnxConf conf;
    xmlContent* xmlTreeRoot;
    vector<xmlContent*> elementsLIFO;

public:
    mvnxStreamReader() : xmlTreeRoot(nullptr){};

    // Get methods
    mvnxConf getConf() const { return conf; };
    xmlContent* getXmlTreeRoot() const { return xmlTreeRoot; };

    // Set methods
    void setConf(mvnxConf _conf) { conf = _conf; };

    // Exposed API for parsing, displaying and handling the document
    bool parse();
    void printParsedDocument();
    vector<xmlContent*> findElement(string elementName);

private:
    void handleStartElement(string elementName,
                            QXmlStreamAttributes elementAttributes);
    void handleCharacters(string elementText);
    void handleComment(string elementText);
    void handleStopElement(string elementName);
    bool elementIsEnabled(string elementName);
    attributes_t processAttributes(QXmlStreamAttributes elementAttributes);
};
} // namespace mvnx_ns

#endif // MVNX_STREAM_READER_H
