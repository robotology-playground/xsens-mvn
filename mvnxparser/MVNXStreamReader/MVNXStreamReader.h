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
#include <memory>
#include <unordered_map>
#include <vector>

namespace xmlstream {
    namespace mvnx {
        class MVNXStreamReader;
    }
} // namespace xmlstream

// Simple container of the mvnx parsing configuration.
typedef std::string ConfigurationEntry;
typedef std::unordered_map<ConfigurationEntry, bool> MVNXConfiguration;

class xmlstream::mvnx::MVNXStreamReader : public xmlstream::XMLStreamReader
{
private:
    MVNXConfiguration m_conf;
    xmlstream::IContentPtrS m_XMLTreeRoot = nullptr;
    std::vector<xmlstream::IContentPtrS> m_elementsLIFO;

public:
    MVNXStreamReader() = default;
    virtual ~MVNXStreamReader() = default;

    // Get methods
    MVNXConfiguration getConf() const { return m_conf; }
    xmlstream::XMLContentPtrS getXmlTreeRoot() const;

    // Set methods
    void setConf(const MVNXConfiguration& conf) { m_conf = conf; }

    // Exposed API for parsing, displaying and handling the document
    bool parse() override;
    void printParsedDocument();

    std::vector<xmlstream::XMLContentPtrS>
    findElement(const xmlstream::ElementName& elementName);

private:
    void handleStartElement(const xmlstream::ElementName& name,
                            const QXmlStreamAttributes& attributes);
    void handleCharacters(const xmlstream::ElementText& text);
    void handleComment(const xmlstream::ElementText& text);
    void handleStopElement(const xmlstream::ElementName& name);
    bool elementIsEnabled(const xmlstream::ElementName& name);
    xmlstream::Attributes processAttributes(const QXmlStreamAttributes& attributes);
};

#endif // MVNX_STREAM_READER_H
