/*
 * Copyright: (C) 2017 iCub Facility
 * Author: Diego Ferigo
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 *
 */

/**
 * @file mvnxStreamReader.cpp
 * @brief Validate and parse efficiently mvnx files
 * @author Diego Ferigo
 * @date 12/04/2017
 */

#include "mvnxStreamReader.h"

using namespace mvnx_ns;
using namespace std;

void mvnxStreamReader::printParsedDocument()
{
	// Initialize the XML file
	QXmlStreamReader xml;
	xmlFile.seek(0);
	xml.setDevice(&xmlFile);

	// Initialize the objects that will contain the metadata of the current
	// element
	QXmlStreamReader::TokenType tokenType;
	string tokenString;
	string elementName;
	// string elementNS; // TODO: Namespace support
	string elementText;
	QXmlStreamAttributes elementAttributes;

	while (!xml.atEnd()) {
		xml.readNext();

		tokenType          = xml.tokenType();
		tokenString        = xml.tokenString().toStdString();
		string elementName = xml.name().toString().toStdString();
		// string ElementNS   = xml.namespaceUri().toString().toStdString();
		string elementText = xml.text().toString().toStdString();
		elementAttributes  = xml.attributes();

		if (not xml.isCharacters()) {
			cout << tokenString << ": " << elementName << endl;
			if (not elementAttributes.empty()) {
				for (auto element : elementAttributes)
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

bool mvnxStreamReader::parse()
{
	// Initialize the XML file
	QXmlStreamReader xml;
	xmlFile.seek(0);
	xml.setDevice(&xmlFile);

	// Initialize the objects that will contain the metadata of the current
	// element
	QXmlStreamReader::TokenType tokenType;
	string tokenString;
	string elementName;
	// string elementNS; // TODO: Namespace support
	string elementText;
	QXmlStreamAttributes elementAttributes;

	// Sequentially parse the file
	while (!xml.atEnd()) {
		xml.readNext();

		// Get the metadata of the element
		tokenType   = xml.tokenType();
		tokenString = xml.tokenString().toStdString();
		elementName = xml.name().toString().toStdString();
		// elementNS   = xml.namespaceUri().toString().toStdString();
		elementText       = xml.text().toString().toStdString();
		elementAttributes = xml.attributes();

		// Handle the generated events
		switch (tokenType) {
			case QXmlStreamReader::StartDocument:
				// TODO
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
				xmlTreeRoot = dynamic_cast<xmlContent*>(elementsLIFO.front());
				break;
			default:
				break;
		}
	}
	return false;
}

attributes_t
mvnxStreamReader::processAttributes(QXmlStreamAttributes elementAttributes)
{
	attributes_t attributes;
	if (not elementAttributes.empty()) {
		for (auto element : elementAttributes) {
			attributes[element.name().toString().toStdString()]
			          = element.value().toString().toStdString();
		}
	}
	return attributes;
}

void mvnxStreamReader::handleStartElement(
          string elementName, QXmlStreamAttributes elementAttributes)
{
	// Create a new element object
	xmlContent* element = new xmlContent(elementName,
	                                     processAttributes(elementAttributes));

	// Push it in the buffer
	elementsLIFO.push_back(element);
}

void mvnxStreamReader::handleCharacters(string elementText)
{
	xmlContent* lastElement = elementsLIFO.back();
	assert(lastElement != NULL);
	lastElement->setText(elementText);
}

void mvnxStreamReader::handleComment(string elementText)
{
	// TODO: this is not a mvnx <comment> but a XML comment
}

void mvnxStreamReader::handleStopElement(string elementName)
{
	if (elementsLIFO.size() > 1) {
		// Extract parent and child elements
		IContent* lastElement = dynamic_cast<IContent*>(elementsLIFO.back());
		IContent* secondToLastElement
		          = dynamic_cast<IContent*>(elementsLIFO.end()[-2]);
		assert(lastElement != NULL && secondToLastElement != NULL);
		assert(lastElement->getElementName() == elementName);

		// Assign the child the parent element
		secondToLastElement->setChild(lastElement);

		// Delete che assigned child from the buffer of the XML tree
		elementsLIFO.pop_back();
	}
}

vector<xmlContent*> mvnxStreamReader::findElement(string elementName)
{
	vector<xmlContent*> childElementsCasted;
	if (xmlTreeRoot != NULL) {
		vector<IContent*> childElements
		          = xmlTreeRoot->findChildElements(elementName);
		childElementsCasted.resize(childElements.size());
		// Cast to xmlContent the vector's item using a lambda expression
		transform(childElements.begin(), childElements.end(),
		          childElementsCasted.begin(),
		          [](IContent* e) { return dynamic_cast<xmlContent*>(e); });
	}
	return childElementsCasted;
}