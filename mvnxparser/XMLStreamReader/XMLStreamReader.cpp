/*
 * Copyright: (C) 2017 iCub Facility
 * Author: Diego Ferigo
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 *
 */

/**
 * @file XMLStreamReader.cpp
 * @brief Validate and parse efficiently big XML documents
 * @author Diego Ferigo
 * @date 19/04/2017
 */

#include "XMLStreamReader.h"

using namespace std;
using namespace xmlstream;

XMLStreamReader::XMLStreamReader() : XMLStreamReader(string(), string())
{
}

XMLStreamReader::XMLStreamReader(string documentFile)
    : XMLStreamReader(documentFile, string())
{
}

XMLStreamReader::XMLStreamReader(string documentFile, string schemaFile)
{
    if (QCoreApplication::instance() == nullptr) {
        dummyArgc = 1;
        dummyArgv = new char[1]{' '};
        coreApp   = new QCoreApplication(dummyArgc, &dummyArgv);
    }
    if (!documentFile.empty()) {
        setDocument(documentFile);
    }

    if (!schemaFile.empty()) {
        setSchema(schemaFile);
    }
}

bool XMLStreamReader::setDocument(string documentFile)
{
    xmlFile.setFileName(QString(documentFile.c_str()));
    return xmlFile.open(QIODevice::ReadOnly);
}

bool XMLStreamReader::setSchema(string schemaFile)
{
    setXmlMessageHandler(handler);
    schemaUrl = QUrl(("file://" + schemaFile).c_str());
    return schema.load(schemaUrl);
}

void XMLStreamReader::setXmlMessageHandler(XMLMessageHandler& _handler)
{
    schema.setMessageHandler(&_handler);
}

bool XMLStreamReader::validate()
{
    QXmlSchemaValidator validator(schema);
    if (not xmlFile.isOpen())
        xmlFile.open(QIODevice::ReadOnly);
    return validator.validate(&xmlFile, schemaUrl);
}
XMLStreamReader::~XMLStreamReader()
{
    delete coreApp;
}
