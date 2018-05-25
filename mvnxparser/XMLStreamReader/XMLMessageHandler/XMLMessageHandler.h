/*
 * Copyright (C) 2018 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * GNU Lesser General Public License v2.1 or any later version.
 */

#ifndef XML_MESSAGE_HANDLER_H
#define XML_MESSAGE_HANDLER_H

#include <QtXmlPatterns>
#include <iostream>

class XMLMessageHandler : public QAbstractMessageHandler
{
public:
    XMLMessageHandler();

    QString statusMessage() const { return m_description; }
    long long int line() const { return m_sourceLocation.line(); }
    long long int column() const { return m_sourceLocation.column(); }
    void printMessage() const;

protected:
    void handleMessage(QtMsgType type,
                       const QString& description,
                       const QUrl& identifier,
                       const QSourceLocation& sourceLocation) override;

private:
    QtMsgType m_messageType;
    QString m_description;
    QSourceLocation m_sourceLocation;
};

#endif /* XML_MESSAGE_HANDLER_H */
