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

using namespace std;
using namespace xmlstream;
using namespace xmlstream::mvnx;

XMLContentPtrS MVNXStreamReader::getXmlTreeRoot() const
{
    return std::dynamic_pointer_cast<XMLContent>(XMLTreeRoot);
}

void MVNXStreamReader::printParsedDocument()
{
    // Initialize the XML file
    QXmlStreamReader xml;
    xmlFile.seek(0);
    xml.setDevice(&xmlFile);

    // Initialize the objects that will contain the metadata of the current
    // element
    QXmlStreamReader::TokenType tokenType;
    string tokenString;
    string ElementName;
    // string elementNS; // TODO: Namespace support
    string elementText;
    QXmlStreamAttributes elementAttributes;

    while (!xml.atEnd()) {
        xml.readNext();

        tokenType          = xml.tokenType();
        tokenString        = xml.tokenString().toStdString();
        string ElementName = xml.name().toString().toStdString();
        // string ElementNS   = xml.namespaceUri().toString().toStdString();
        string elementText = xml.text().toString().toStdString();
        elementAttributes  = xml.attributes();

        if (not xml.isCharacters()) {
            cout << tokenString << ": " << ElementName << endl;
            if (not elementAttributes.empty()) {
                for (const auto& element : elementAttributes)
                    cout << "   " << element.name().toString().toStdString()
                         << "=\"" << element.value().toString().toStdString()
                         << "\"" << endl;
            }
        }

        if (xml.isStartDocument() || xml.isEndDocument()) {
            cout << endl;
            continue;
        }

        // Print the content of the element
        if (xml.isCharacters() && not xml.isWhitespace())
            cout << tokenString << ": " << xml.text().toString().toStdString()
                 << endl;
    }
}

bool MVNXStreamReader::parse()
{
    // Initialize the XML file
    QXmlStreamReader xml;
    xmlFile.seek(0);
    xml.setDevice(&xmlFile);

    // Initialize the objects that will contain the metadata of the current
    // element
    QXmlStreamReader::TokenType tokenType;
    string tokenString;
    string ElementName;
    // string elementNS; // TODO: Namespace support
    string elementText;
    QXmlStreamAttributes elementAttributes;

    // Sequentially parse the file
    while (!xml.atEnd()) {
        xml.readNext();

        // Get the metadata of the element
        tokenType   = xml.tokenType();
        tokenString = xml.tokenString().toStdString();
        ElementName = xml.name().toString().toStdString();
        // elementNS   = xml.namespaceUri().toString().toStdString();
        elementText       = xml.text().toString().toStdString();
        elementAttributes = xml.attributes();

        // Handle the generated events
        switch (tokenType) {
            case QXmlStreamReader::StartDocument:
                break;
            case QXmlStreamReader::StartElement:
                handleStartElement(ElementName, elementAttributes);
                break;
            case QXmlStreamReader::Characters:
                handleCharacters(elementText);
                break;
            case QXmlStreamReader::Comment:
                handleComment(elementText);
                break;
            case QXmlStreamReader::EndElement:
                handleStopElement(ElementName);
                break;
            case QXmlStreamReader::EndDocument:
                XMLTreeRoot = elementsLIFO.front();
                elementsLIFO.pop_back();
                assert(elementsLIFO.empty());
                break;
            default:
                break;
        }
    }
    return false;
}

attributes_t
MVNXStreamReader::processAttributes(QXmlStreamAttributes elementAttributes)
{
    attributes_t attributes;
    if (not elementAttributes.empty()) {
        for (const auto& element : elementAttributes) {
            assert(element.name().toString().toStdString().length() != 0);
            assert(element.value().toString().toStdString().length() != 0);
            // Process the attribute
            attributes[element.name().toString().toStdString()]
                      = element.value().toString().toStdString();
        }
    }
    return attributes;
}

bool MVNXStreamReader::elementIsEnabled(string ElementName)
{
    // If the user didn't provide any configuration, all elements are enabled
    if (conf.empty()) {
        return true;
    }
    else {
        try {
            if (conf[ElementName] == true)
                return true;
        } catch (const std::out_of_range& e) {
            return false;
        };
        return false;
    }
}

void MVNXStreamReader::handleStartElement(
          string ElementName, QXmlStreamAttributes elementAttributes)
{
    if (elementIsEnabled(ElementName)) {
        // Create a new object and handle it with smart pointers
        IContentPtrS element = make_shared<XMLContent>(
                  ElementName, processAttributes(elementAttributes));
        // Push it in the buffer of pointers
        elementsLIFO.push_back(element);
    }
}

void MVNXStreamReader::handleCharacters(string elementText)
{
    IContentPtrS lastElement = elementsLIFO.back();
    assert(lastElement);
    lastElement->setText(elementText);
}

void MVNXStreamReader::handleComment(string elementText)
{
    // TODO: this is not a mvnx <comment> but a XML comment
}

void MVNXStreamReader::handleStopElement(string ElementName)
{
    if (elementIsEnabled(ElementName)) {
        if (elementsLIFO.size() > 1) {
            // Extract parent and child elements
            IContentPtrS lastElement         = elementsLIFO.back();
            IContentPtrS secondToLastElement = elementsLIFO.end()[-2];
            assert(lastElement && secondToLastElement); // Are not nullptr
            assert(lastElement->getElementName() == ElementName);

            // Assign the child the parent element
            secondToLastElement->setChild(lastElement);

            // Delete che assigned child from the buffer of the XML tree
            elementsLIFO.pop_back();
        }
    }
}

vector<XMLContentPtrS> MVNXStreamReader::findElement(string ElementName)
{
    vector<XMLContentPtrS> childElementsCast;
    assert(XMLTreeRoot); // Not nullptr
    if (XMLTreeRoot) {
        vector<IContentPtrS> childElements
                  = XMLTreeRoot->findChildElements(ElementName);
        childElementsCast.resize(childElements.size());
        // Cast to XMLContent the vector's item using a lambda expression
        transform(childElements.begin(),
                  childElements.end(),
                  childElementsCast.begin(),
                  [](IContentPtrS e) {
                      return dynamic_pointer_cast<XMLContent>(e);
                  });
    }
    return childElementsCast;
}
