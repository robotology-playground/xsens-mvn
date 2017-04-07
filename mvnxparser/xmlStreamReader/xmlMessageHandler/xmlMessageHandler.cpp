/*
 * Copyright: (C) 2017 iCub Facility
 * Author: Diego Ferigo
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 *
 */

/**
 * @file xmlMessageHandler.cpp
 * @brief Handle XML error with a custom format
 * @author Diego Ferigo
 * @date 06/04/2017
 */

#include "xmlMessageHandler.h"

void xmlMessageHandler::handleMessage(QtMsgType type,
                                      const QString& description,
                                      const QUrl& identifier,
                                      const QSourceLocation& sourceLocation) {
	m_messageType    = type;
	m_description    = description;
	m_sourceLocation = sourceLocation;
	printMessage();
}

void xmlMessageHandler::printMessage() const {
	qDebug() << "\t" << m_description;
	qDebug() << "\t" << m_sourceLocation;
}
