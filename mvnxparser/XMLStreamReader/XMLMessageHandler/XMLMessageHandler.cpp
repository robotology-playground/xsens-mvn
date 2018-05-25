/*
 * Copyright (C) 2018 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * GNU Lesser General Public License v2.1 or any later version.
 */

#include "XMLMessageHandler.h"

XMLMessageHandler::XMLMessageHandler()
    : QAbstractMessageHandler(0)
{}

void XMLMessageHandler::handleMessage(QtMsgType type,
                                      const QString& description,
                                      const QUrl& /*identifier*/,
                                      const QSourceLocation& sourceLocation)
{
    m_messageType = type;
    m_description = description;
    m_sourceLocation = sourceLocation;
    printMessage();
}

void XMLMessageHandler::printMessage() const
{
    qDebug() << "\t" << m_description;
    qDebug() << "\t" << m_sourceLocation;
}
