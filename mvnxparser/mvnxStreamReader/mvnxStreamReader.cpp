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
				xmlTreeRoot = elementsLIFO.front();
				break;
			default:
				break;
		}
	}
	return false;
}

unordered_map<string, string>
mvnxStreamReader::processAttributes(QXmlStreamAttributes elementAttributes)
{
	unordered_map<string, string> attributes;
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
	// If the element has no implementation, skip it
	if (mapStringtoElementsT(elementName) != NONE) {
		// Generate a polymorphic pointer to the correct type of element
		mvnxIContent* element = mapStringtoPointer(
		          elementName, processAttributes(elementAttributes));

		// Put its pointer in the buffer of the XML tree
		if (element != NULL) {
			elementsLIFO.push_back(element);
			assert(element == elementsLIFO.back());
		}
	}
}

void mvnxStreamReader::handleCharacters(string elementText)
{
	mvnxIContent* lastElement = elementsLIFO.back();
	assert(lastElement != NULL);
	lastElement->setText(elementText);
}

void mvnxStreamReader::handleComment(string elementText)
{
	// TODO: this is not a mvnx <comment> but a XML comment
}

void mvnxStreamReader::handleStopElement(string elementName)
{
	// If the element has no implementation, skip it
	if (mapStringtoElementsT(elementName) != NONE) {
		// Check that a parent exists in the buffer of the XML tree
		if (elementsLIFO.size() > 1) {
			// Extract parent and child elements
			mvnxIContent* lastElement         = elementsLIFO.back();
			mvnxIContent* secondToLastElement = elementsLIFO.end()[-2];
			assert(lastElement != NULL && secondToLastElement != NULL);
			assert(lastElement->getElementType()
			       == mapStringtoElementsT(elementName));

			// Assign the child the parent element
			secondToLastElement->setChild(lastElement);

			// Delete che assigned child from the buffer of the XML tree
			elementsLIFO.pop_back();
		}
	}
}

// Set the element type from string label.
// Considering that all the elements classes have a very small memory footprint,
// this function calls mapStringtoPointer() and uses a temporary
// elementContent<mvnxElements_t> to map the elementLabel string to its
// mvnxElements_t enum.
mvnxElements_t mvnxStreamReader::mapStringtoElementsT(string elementLabel)
{
	unordered_map<string, string> dummy_map;

	// Generate a temporary elementContent<mvnxElements_t> object from
	// using the elementLabel string
	mvnxIContent* tmp_element = mapStringtoPointer(elementLabel, dummy_map);

	// Get the element type and return it as mvnxElements_t
	if (tmp_element != NULL) {
		mvnxElements_t tmp_element_type = tmp_element->getElementType();
		delete tmp_element;
		return tmp_element_type;
	}
	else
		return NONE;
}

// This functions processes the name (string) of an element and returns a
// polymorphic pointer to its child class.
mvnxIContent*
mvnxStreamReader::mapStringtoPointer(string elementLabel,
                                     unordered_map<string, string> attributes)
{
	// ELEMENT CONTENT
	if (elementLabel == "mvnx")
		return dynamic_cast<mvnxIContent*>(new mvnx(MVNX, attributes));
	else if (elementLabel == "subject")
		return dynamic_cast<mvnxIContent*>(
		          new mvnxSubject(SUBJECT, attributes));
	else if (elementLabel == "mvn")
		return dynamic_cast<mvnxIContent*>(new mvn(MVN, attributes));
	else if (elementLabel == "segment")
		return dynamic_cast<mvnxIContent*>(
		          new mvnxSegment(SEGMENT, attributes));
	else if (elementLabel == "segments")
		return dynamic_cast<mvnxIContent*>(
		          new mvnxSegments(SEGMENTS, attributes));
	else if (elementLabel == "point")
		return dynamic_cast<mvnxIContent*>(new mvnxPoint(POINT, attributes));
	else if (elementLabel == "points")
		return dynamic_cast<mvnxIContent*>(new mvnxPoints(POINTS, attributes));
	else if (elementLabel == "sensor")
		return dynamic_cast<mvnxIContent*>(new mvnxSensor(SENSOR, attributes));
	else if (elementLabel == "sensors")
		return dynamic_cast<mvnxIContent*>(
		          new mvnxSensors(SENSORS, attributes));
	else if (elementLabel == "joint")
		return dynamic_cast<mvnxIContent*>(new mvnxJoint(JOINT, attributes));
	else if (elementLabel == "joints")
		return dynamic_cast<mvnxIContent*>(new mvnxJoints(JOINTS, attributes));
	else if (elementLabel == "frame")
		return dynamic_cast<mvnxIContent*>(new mvnxFrame(FRAME, attributes));
	else if (elementLabel == "frames")
		return dynamic_cast<mvnxIContent*>(new mvnxFrames(FRAMES, attributes));
	else if (elementLabel == "securityCode")
		return dynamic_cast<mvnxIContent*>(
		          new mvnxSecurityCode(SECURITY_CODE, attributes));
	// TEXT CONTENT
	else if (elementLabel == "comment")
		return dynamic_cast<mvnxIContent*>(new mvnxComment(COMMENT));
	else if (elementLabel == "pos_s")
		return dynamic_cast<mvnxIContent*>(new mvnxPosS(POS_S));
	else if (elementLabel == "connector1")
		return dynamic_cast<mvnxIContent*>(new mvnxConnector(CONNECTOR1));
	else if (elementLabel == "connector2")
		return dynamic_cast<mvnxIContent*>(new mvnxConnector(CONNECTOR2));
	else if (elementLabel == "position")
		return dynamic_cast<mvnxIContent*>(new mvnxPosition(POSITION));
	else if (elementLabel == "orientation")
		return dynamic_cast<mvnxIContent*>(new mvnxOrientation(ORIENTATION));
	else if (elementLabel == "acceleration")
		return dynamic_cast<mvnxIContent*>(new mvnxAcceleration(ACCELERATION));
	else if (elementLabel == "velocity")
		return dynamic_cast<mvnxIContent*>(new mvnxVelocity(VELOCITY));
	else if (elementLabel == "angularVelocity")
		return dynamic_cast<mvnxIContent*>(
		          new mvnxAngularVelocity(ANGULAR_VELOCITY));
	else if (elementLabel == "angularAcceleration")
		return dynamic_cast<mvnxIContent*>(
		          new mvnxAcceleration(ANGULAR_ACCELERATION));
	else if (elementLabel == "sensorAcceleration")
		return dynamic_cast<mvnxIContent*>(
		          new mvnxSensorAcceleration(SENSOR_ACCELERATION));
	else if (elementLabel == "sensorAngularVelocity")
		return dynamic_cast<mvnxIContent*>(
		          new mvnxSensorAngularVelocity(SENSOR_ANGULAR_VELOCITY));
	else if (elementLabel == "sensorMagneticField")
		return dynamic_cast<mvnxIContent*>(
		          new mvnxSensorMagneticField(SENSOR_MAGNETIC_FIELD));
	else if (elementLabel == "sensorOrientation")
		return dynamic_cast<mvnxIContent*>(
		          new mvnxSensorOrientation(SENSOR_ORIENTATION));
	else if (elementLabel == "jointAngle")
		return dynamic_cast<mvnxIContent*>(new mvnxJointAngle(JOINT_ANGLE));
	else if (elementLabel == "jointAngleXZY")
		return dynamic_cast<mvnxIContent*>(
		          new mvnxJointAngleXZY(JOINT_ANGLE_XZY));
	else if (elementLabel == "centerOfMass")
		return dynamic_cast<mvnxIContent*>(
		          new mvnxCenterOfMass(CENTER_OF_MASS));
	else
		return NULL;
}
