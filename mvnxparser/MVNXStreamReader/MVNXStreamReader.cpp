/*
 * Copyright: (C) 2017 iCub Facility
 * Author: Diego Ferigo
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 *
 */

/**
 * @file MVNXStreamReader.cpp
 * @brief Validate and parse efficiently mvnx files
 * @author Diego Ferigo
 * @date 18/04/2017
 */

#include "MVNXStreamReader.h"
#include <iostream>

using namespace xmlstream;
using namespace xmlstream::mvnx;

XMLContentPtrS MVNXStreamReader::getXmlTreeRoot() const
{
    return std::dynamic_pointer_cast<XMLContent>(m_XMLTreeRoot);
}

void MVNXStreamReader::printParsedDocument()
{
    // Initialize the XML file
    QXmlStreamReader xml;
    m_xmlFile.seek(0);
    xml.setDevice(&m_xmlFile);

    // Initialize the objects that will contain the metadata of the current
    // element
    QXmlStreamReader::TokenType tokenType;
    std::string tokenString;
    std::string elementName;
    // string elementNS; // TODO: Namespace support
    std::string elementText;
    QXmlStreamAttributes elementAttributes;

    while (!xml.atEnd()) {
        xml.readNext();

        tokenType = xml.tokenType();
        tokenString = xml.tokenString().toStdString();
        std::string elementName = xml.name().toString().toStdString();
        // string ElementNS   = xml.namespaceUri().toString().toStdString();
        std::string elementText = xml.text().toString().toStdString();
        elementAttributes = xml.attributes();

        if (not xml.isCharacters()) {
            std::cout << tokenString << ": " << elementName << std::endl;
            if (not elementAttributes.empty()) {
                for (const auto& element : elementAttributes)
                    std::cout << "   " << element.name().toString().toStdString() << "=\""
                              << element.value().toString().toStdString() << "\""
                              << std::endl;
            }
        }

        if (xml.isStartDocument() || xml.isEndDocument()) {
            std::cout << std::endl;
            continue;
        }

        // Print the content of the element
        if (xml.isCharacters() && not xml.isWhitespace())
            std::cout << tokenString << ": " << xml.text().toString().toStdString()
                      << std::endl;
    }
}

bool MVNXStreamReader::parse()
{
    // Initialize the XML file
    QXmlStreamReader xml;
    m_xmlFile.seek(0);
    xml.setDevice(&m_xmlFile);

    // Initialize the objects that will contain the metadata of the current
    // element
    QXmlStreamReader::TokenType tokenType;
    std::string tokenString;
    std::string elementName;
    // string elementNS; // TODO: Namespace support
    std::string elementText;
    QXmlStreamAttributes elementAttributes;

    // Sequentially parse the file
    while (!xml.atEnd()) {
        xml.readNext();

        // Get the metadata of the element
        tokenType = xml.tokenType();
        tokenString = xml.tokenString().toStdString();
        elementName = xml.name().toString().toStdString();
        // elementNS   = xml.namespaceUri().toString().toStdString();
        elementText = xml.text().toString().toStdString();
        elementAttributes = xml.attributes();

        // Handle the generated events
        switch (tokenType) {
            case QXmlStreamReader::StartDocument:
                break;
            case QXmlStreamReader::StartElement:
                handleStartElement(elementName, elementAttributes);
                break;
            case QXmlStreamReader::Characters:
                handleCharacters(elementText);
                break;
            case QXmlStreamReader::Comment:
                handleComment(elementText);
                break;
            case QXmlStreamReader::EndElement:
                handleStopElement(elementName);
                break;
            case QXmlStreamReader::EndDocument:
                m_XMLTreeRoot = m_elementsLIFO.front();
                m_elementsLIFO.pop_back();
                assert(elementsLIFO.empty());
                break;
            default:
                break;
        }
    }
    return false;
}

Attributes MVNXStreamReader::processAttributes(const QXmlStreamAttributes& attributes)
{
    // The content of QXmlStreamAttributes are QStringRef.
    // We need to convert them to std::string.
    Attributes stringAttributes;
    if (!attributes.empty()) {
        for (const auto& attribute : attributes) {
            assert(attribute.name().toString().toStdString().length() != 0);
            assert(attribute.value().toString().toStdString().length() != 0);
            // Process the attribute
            stringAttributes[attribute.name().toString().toStdString()] =
                attribute.value().toString().toStdString();
        }
    }
    return stringAttributes;
}

bool MVNXStreamReader::elementIsEnabled(const xmlstream::ElementName& name)
{
    // If the user didn't provide any configuration, all elements are enabled
    if (m_conf.empty()) {
        return true;
    }

    // If the name does not match any entry in the map, return false
    if (m_conf.find(name) == m_conf.end()) {
        return false;
    }

    return m_conf[name];
}

void MVNXStreamReader::handleStartElement(const xmlstream::ElementName& name,
                                          const QXmlStreamAttributes& attributes)
{
    if (elementIsEnabled(name)) {
        // Get a weak pointer to the parent
        parent_ptr parent = nullptr;
        if (!m_elementsLIFO.empty()) {
            parent = m_elementsLIFO.back();
        }

        // Push it in the buffer of pointers
        m_elementsLIFO.emplace_back(
            std::make_shared<XMLContent>(name, processAttributes(attributes), parent));
    }
}

void MVNXStreamReader::handleCharacters(const xmlstream::ElementText& text)
{
    IContentPtrS lastElement = m_elementsLIFO.back();
    assert(lastElement);
    lastElement->setText(text);
}

void MVNXStreamReader::handleComment(const xmlstream::ElementText& /*text*/)
{
    // TODO: this is not a mvnx <comment> but a XML comment
}

void MVNXStreamReader::handleStopElement(const xmlstream::ElementName& name)
{
    if (elementIsEnabled(name)) {
        if (m_elementsLIFO.size() > 1) { // TODO: check if >=
            // Extract parent and child elements
            IContentPtrS lastElement = m_elementsLIFO.back();
            IContentPtrS secondToLastElement = m_elementsLIFO.end()[-2];
            assert(lastElement && secondToLastElement); // Are not nullptr
            assert(lastElement->getElementName() == name);

            // Assign the child the parent element
            secondToLastElement->setChild(lastElement);

            // Delete che assigned child from the buffer of the XML tree
            m_elementsLIFO.pop_back();
        }
    }
}

std::vector<XMLContentPtrS>
MVNXStreamReader::findElement(const xmlstream::ElementName& name)
{
    std::vector<XMLContentPtrS> childElementsCast;
    assert(XMLTreeRoot); // Not nullptr
    if (m_XMLTreeRoot) {
        std::vector<IContentPtrS> childElements = m_XMLTreeRoot->findChildElements(name);
        childElementsCast.resize(childElements.size());
        // Cast to XMLContent the vector's item using a lambda expression
        transform(childElements.begin(),
                  childElements.end(),
                  childElementsCast.begin(),
                  [](const IContentPtrS& e) {
                      return std::dynamic_pointer_cast<XMLContent>(e);
                  });
    }
    return childElementsCast;
}
