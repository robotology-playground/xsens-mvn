/*
 * Copyright: (C) 2017 iCub Facility
 * Author: Diego Ferigo
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 *
 */

/**
 * @file xmlStreamReader.cpp
 * @brief Validate and parse efficiently big XML documents
 * @author Diego Ferigo
 * @date 06/04/2017
 */

#include "xmlStreamReader.h"

xmlStreamReader::xmlStreamReader()
{
    if (QCoreApplication::instance() == nullptr) {
        dummyArgc = 1;
        dummyArgv = new char[1]{' '};
        coreApp   = new QCoreApplication(dummyArgc, &dummyArgv);
    }
}
xmlStreamReader::xmlStreamReader(string documentFile) : xmlStreamReader()
{
    setDocument(documentFile);
}

xmlStreamReader::xmlStreamReader(string documentFile, string schemaFile)
    : xmlStreamReader(documentFile)
{
    setSchema(schemaFile);
}

bool xmlStreamReader::setDocument(string documentFile)
{
    xmlFile.setFileName(QString(documentFile.c_str()));
    return xmlFile.open(QIODevice::ReadOnly);
}

bool xmlStreamReader::setSchema(string schemaFile)
{
    setXmlMessageHandler(handler);
    schemaUrl = QUrl(("file://" + schemaFile).c_str());
    return schema.load(schemaUrl);
}

void xmlStreamReader::setXmlMessageHandler(xmlMessageHandler& _handler)
{
    schema.setMessageHandler(&_handler);
}

bool xmlStreamReader::validate()
{
    QXmlSchemaValidator validator(schema);
    if (not xmlFile.isOpen())
        xmlFile.open(QIODevice::ReadOnly);
    return validator.validate(&xmlFile, schemaUrl);
}
xmlStreamReader::~xmlStreamReader()
{
    delete coreApp;
}
