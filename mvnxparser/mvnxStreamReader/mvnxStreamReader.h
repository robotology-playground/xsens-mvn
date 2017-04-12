/*
 * Copyright: (C) 2017 iCub Facility
 * Author: Diego Ferigo
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 *
 */

/**
 * @file mvnxStreamReader.h
 * @brief Validate and parse efficiently mvnx files
 * @author Diego Ferigo
 * @date 12/04/2017
 */
#ifndef MVNX_STREAM_READER_H
#define MVNX_STREAM_READER_H

#include "mvnxDataContainers.h"
#include "xmlStreamReader.h"
#include <QXmlStreamReader>
#include <iostream>
#include <vector>

namespace mvnx_ns {

class mvnxStreamReader : public xmlStreamReader {
private:
	mvnxIContent* xmlTreeRoot;
	vector<mvnxIContent*> elementsLIFO;

public:
	mvnxStreamReader() : xmlTreeRoot(NULL){};

	// Get methods
	mvnxIContent* getXmlTreeRoot() const { return xmlTreeRoot; };
	std::vector<mvnxSubject*> getSubjects() const
	{
		mvnx* mvnxElement = static_cast<mvnx*>(xmlTreeRoot);
		return mvnxElement->getSubjects();
	};

	// Exposed API for parsing and displaying the document
	bool parse();
	void printParsedDocument();

private:
	void handleStartElement(string elementName,
	                        QXmlStreamAttributes elementAttributes);
	void handleCharacters(string elementText);
	void handleComment(string elementText);
	void handleStopElement(string elementName);
	mvnxElements_t mapStringtoElementsT(string elementLabel);
	unordered_map<string, string>
	processAttributes(QXmlStreamAttributes elementAttributes);
	mvnxIContent* mapStringtoPointer(string elementLabel,
	                                 unordered_map<string, string> attributes);
};
} // namespace mvnx_ns

#endif // MVNX_STREAM_READER_H
