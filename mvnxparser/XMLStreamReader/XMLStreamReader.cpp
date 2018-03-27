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

XMLStreamReader::XMLStreamReader(const string& documentFile, const string& schemaFile)
{
    // Instantiate QCoreApplication at the first execution
    if (!QCoreApplication::instance()) {
        m_dummyArgc = 1;
        m_dummyArgv = new char[1]{' '};
        m_coreApp = std::unique_ptr<QCoreApplication>(
            new QCoreApplication(m_dummyArgc, &m_dummyArgv));
    }

    // Set the document file (absolute path)
    if (!documentFile.empty()) {
        setDocument(documentFile);
    }

    // Set the xml schema file (absolute path)
    if (!schemaFile.empty()) {
        setSchema(schemaFile);
    }
}

bool XMLStreamReader::setDocument(const string& documentFile)
{
    m_xmlFile.setFileName(QString(documentFile.c_str()));
    return m_xmlFile.open(QIODevice::ReadOnly);
}

bool XMLStreamReader::setSchema(const string& schemaFile)
{
    m_schemaUrl = QUrl(("file://" + schemaFile).c_str());
    return m_schema.load(m_schemaUrl);
}

void XMLStreamReader::setXmlMessageHandler(XMLMessageHandler& handler)
{
    m_schema.setMessageHandler(&handler);
}

bool XMLStreamReader::validate()
{
    QXmlSchemaValidator validator(m_schema);

    if (!m_xmlFile.isOpen()) {
        m_xmlFile.open(QIODevice::ReadOnly);
    }

    return validator.validate(&m_xmlFile, m_schemaUrl);
}

XMLStreamReader::~XMLStreamReader()
{
    delete m_dummyArgv;
}
