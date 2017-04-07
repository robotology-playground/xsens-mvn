/*
 * Copyright: (C) 2017 iCub Facility
 * Author: Diego Ferigo
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 *
 */

/**
 * @file xmlMessageHandler.h
 * @brief Handle XML error with a custom format
 * @author Diego Ferigo
 * @date 06/04/2017
 */

#ifndef XML_MESSAGE_HANDLER_H
#define XML_MESSAGE_HANDLER_H

#include <QtXmlPatterns>
#include <iostream>

class xmlMessageHandler : public QAbstractMessageHandler {
public:
	xmlMessageHandler() : QAbstractMessageHandler(0) {}
	QString statusMessage() const { return m_description; }
	int line() const { return m_sourceLocation.line(); }
	int column() const { return m_sourceLocation.column(); }
	void printMessage() const;

protected:
	virtual void handleMessage(QtMsgType type,
	                           const QString& description,
	                           const QUrl& identifier,
	                           const QSourceLocation& sourceLocation);

private:
	QtMsgType m_messageType;
	QString m_description;
	QSourceLocation m_sourceLocation;
};

#endif /* XML_MESSAGE_HANDLER_H */
