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

#include "xmlDataContainers.h"
#include "xmlStreamReader.h"
#include <QXmlStreamReader>
#include <iostream>
#include <vector>

namespace mvnx_ns {

class mvnxStreamReader : public xmlStreamReader {
private:
	xmlContent* xmlTreeRoot;
	vector<xmlContent*> elementsLIFO;

public:
	mvnxStreamReader() : xmlTreeRoot(NULL){};

	// Get methods
	xmlContent* getXmlTreeRoot() const { return xmlTreeRoot; };

	// Exposed API for parsing and displaying the document
	bool parse();
	void printParsedDocument();
	vector<xmlContent*> findElement(string elementName);

private:
	void handleStartElement(string elementName,
	                        QXmlStreamAttributes elementAttributes);
	void handleCharacters(string elementText);
	void handleComment(string elementText);
	void handleStopElement(string elementName);
	attributes_t processAttributes(QXmlStreamAttributes elementAttributes);
};
} // namespace mvnx_ns

#endif // MVNX_STREAM_READER_H
