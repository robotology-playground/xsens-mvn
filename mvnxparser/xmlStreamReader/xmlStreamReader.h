/*
 * Copyright: (C) 2017 iCub Facility
 * Author: Diego Ferigo
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 *
 */

/**
 * @file xmlStreamReader.h
 * @brief Validate and parse efficiently big XML documents
 * @author Diego Ferigo
 * @date 06/04/2017
 */

#ifndef XML_STREAM_READER_H
#define XML_STREAM_READER_H

#include "xmlMessageHandler.h"
#include <QXmlSchema>
#include <QXmlSchemaValidator>
#include <QtXml>
#include <QtXmlPatterns>
#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace std;

class xmlStreamReader {
private:
	QFile xmlFile;
	QUrl schemaUrl;
	QXmlSchema schema;
	QCoreApplication* coreApp;
	int dummyArgc;
	char* dummyArgv;
	xmlMessageHandler handler;

public:
	xmlStreamReader();
	xmlStreamReader(string documentFile);
	xmlStreamReader(string documentFile, string schemaFile);
	bool setDocument(string documentFile);
	bool setSchema(string schemaFile);
	void setXmlMessageHandler(xmlMessageHandler& _handler);
	bool validate();
	virtual ~xmlStreamReader();
};

#endif
