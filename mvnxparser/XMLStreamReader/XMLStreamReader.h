/*
 * Copyright (C) 2018 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * GNU Lesser General Public License v2.1 or any later version.
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
    class XMLStreamReader;
}

class xmlstream::XMLStreamReader
{
private:
    std::unique_ptr<QCoreApplication> m_coreApp = nullptr;
    int m_dummyArgc;
    char* m_dummyArgv = nullptr;

protected:
    QFile m_xmlFile;
    QUrl m_schemaUrl;
    QXmlSchema m_schema;
    bool virtual parse() { return true; }

public:
    XMLStreamReader(const std::string& documentFile = {}, const std::string& schemaFile = {});
    bool setDocument(const std::string& documentFile);
    bool setSchema(const std::string& schemaFile);
    void setXmlMessageHandler(XMLMessageHandler& handler);
    bool validate();
    virtual ~XMLStreamReader();
};

#endif // XML_STREAM_READER_H
