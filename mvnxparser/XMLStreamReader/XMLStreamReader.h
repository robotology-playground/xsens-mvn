/*
 * Copyright: (C) 2017 iCub Facility
 * Author: Diego Ferigo
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 *
 */

/**
 * @file XMLStreamReader.h
 * @brief Validate and parse efficiently big XML documents
 * @author Diego Ferigo
 * @date 06/04/2017
 */

#ifndef XML_STREAM_READER_H
#define XML_STREAM_READER_H

#include "XMLMessageHandler.h"
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

class XMLStreamReader {
private:
    QCoreApplication* coreApp;
    int dummyArgc;
    char* dummyArgv;

protected:
    QFile xmlFile;
    QUrl schemaUrl;
    QXmlSchema schema;
    XMLMessageHandler handler;
    bool virtual parse() { return true; };

public:
    XMLStreamReader();
    XMLStreamReader(std::string documentFile);
    XMLStreamReader(std::string documentFile, std::string schemaFile);
    bool setDocument(std::string documentFile);
    bool setSchema(std::string schemaFile);
    void setXmlMessageHandler(XMLMessageHandler& _handler);
    bool validate();
    virtual ~XMLStreamReader();
};

} // namespace xmlstream

#endif // XML_STREAM_READER_H
