/*
 * Copyright: (C) 2017 iCub Facility
 * Author: Diego Ferigo
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 *
 */

/**
 * @file xmlStreamReader.h
 * @brief Validate and parse efficiently big XML documents
 * @author Diego Ferigo
 * @date 06/04/2017
 */

#ifndef XML_STREAM_READER_H
#define XML_STREAM_READER_H

#include "xmlMessageHandler.h"
#include <QXmlSchema>
#include <QXmlSchemaValidator>
#include <QtXml>
#include <QtXmlPatterns>
#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace xmlstream {

class xmlStreamReader {
private:
    QCoreApplication* coreApp;
    int dummyArgc;
    char* dummyArgv;

protected:
    QFile xmlFile;
    QUrl schemaUrl;
    QXmlSchema schema;
    xmlMessageHandler handler;
    bool virtual parse() { return true; };

public:
    xmlStreamReader();
    xmlStreamReader(std::string documentFile);
    xmlStreamReader(std::string documentFile, std::string schemaFile);
    bool setDocument(std::string documentFile);
    bool setSchema(std::string schemaFile);
    void setXmlMessageHandler(xmlMessageHandler& _handler);
    bool validate();
    virtual ~xmlStreamReader();
};

} // namespace xmlstream

#endif // XML_STREAM_READER_H
