/*
 * Copyright: (C) 2017 iCub Facility
 * Author: Diego Ferigo
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 *
 */

/**
 * @file mvnxDataContainers.cpp
 * @brief Classes that will contain parsed data from the mvnx file
 * @author Diego Ferigo
 * @date 12/04/2017
 */
#ifndef MVNX_DATA_CONTAINERS_H
#define MVNX_DATA_CONTAINERS_H

#include "xmlDataContainers.h"
#include <algorithm>
#include <array>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

// Definition of the mvnx elements.
// This structure will set the type of the elements of templated general class
// IContent<T>.
// If some MVNX elements are not implemented, they are ignored by the parser.
enum mvnxElements_t {
	NONE = 0, // NONE is mandatory: this is the default enum value
	MVNX,
	MVN,
	SUBJECT,
	METADATA,
	COMMENT,
	SEGMENT,
	SEGMENTS,
	POINTS,
	POINT,
	POS_S,
	SENSOR,
	SENSORS,
	JOINTS,
	JOINT,
	CONNECTOR1,
	CONNECTOR2,
	FRAMES,
	FRAME,
	POSITION,
	ORIENTATION,
	VELOCITY,
	ACCELERATION,
	ANGULAR_VELOCITY,
	ANGULAR_ACCELERATION,
	SENSOR_ACCELERATION,
	SENSOR_ANGULAR_VELOCITY,
	SENSOR_MAGNETIC_FIELD,
	SENSOR_ORIENTATION,
	JOINT_ANGLE,
	JOINT_ANGLE_XZY,
	CENTER_OF_MASS,
	SECURITY_CODE
};

typedef IContent<mvnxElements_t> mvnxIContent;
typedef textContent<mvnxElements_t> mvnxTextContent;
typedef elementContent<mvnxElements_t> mvnxElementContent;

// These are elements that contain only text, and they have no child elements.
// For the time being, no parsing requirement has been implemented. The parsed
// text is a string, and its mapping to a more complex data type (e.g. a vector)
// is demanded to the user implementation.
typedef mvnxTextContent mvnxPosS;
typedef mvnxTextContent mvnxComment;
typedef mvnxTextContent mvnxPosition;
typedef mvnxTextContent mvnxVelocity;
typedef mvnxTextContent mvnxConnector;
typedef mvnxTextContent mvnxJointAngle;
typedef mvnxTextContent mvnxOrientation;
typedef mvnxTextContent mvnxCenterOfMass;
typedef mvnxTextContent mvnxAcceleration;
typedef mvnxTextContent mvnxJointAngleXZY;
typedef mvnxTextContent mvnxAngularVelocity;
typedef mvnxTextContent mvnxSensorOrientation;
typedef mvnxTextContent mvnxSensorAcceleration;
typedef mvnxTextContent mvnxAngularAcceleration;
typedef mvnxTextContent mvnxSensorMagneticField;
typedef mvnxTextContent mvnxSensorAngularVelocity;

//
// These classes only have attributes:
//

class mvn : public mvnxElementContent {
public:
	mvn(mvnxElements_t _element_type, unordered_map<string, string> _attributes)
	    : mvnxElementContent(_element_type, _attributes){};
};

class mvnxSensor : public mvnxElementContent {
public:
	mvnxSensor(mvnxElements_t _element_type,
	           unordered_map<string, string> _attributes)
	    : mvnxElementContent(_element_type, _attributes){};
};

class mvnxSecurityCode : public mvnxElementContent {
public:
	mvnxSecurityCode(mvnxElements_t _element_type,
	                 unordered_map<string, string> _attributes)
	    : mvnxElementContent(_element_type, _attributes){};
};

//
// These classes have child elements:
//

class mvnxPoint : public mvnxElementContent {
private:
	vector<mvnxIContent*> pos_s;

public:
	mvnxPoint(mvnxElements_t _element_type,
	          unordered_map<string, string> _attributes)
	    : mvnxElementContent(_element_type, _attributes){};
	virtual void setChild(mvnxIContent*& _childElement)
	{
		switch (_childElement->getElementType()) {
			case POS_S:
				pos_s.push_back(_childElement);
				break;
			default:
				break;
		}
	}
};

class mvnxPoints : public mvnxElementContent {
private:
	vector<mvnxIContent*> point;

public:
	mvnxPoints(mvnxElements_t _element_type,
	           unordered_map<string, string> _attributes)
	    : mvnxElementContent(_element_type, _attributes){};
	virtual void setChild(mvnxIContent*& _childElement)
	{
		switch (_childElement->getElementType()) {
			case POINT:
				point.push_back(_childElement);
				break;
			default:
				break;
		}
	}
};

class mvnxSegment : public mvnxElementContent {
private:
	mvnxIContent* points;

public:
	mvnxSegment(mvnxElements_t _element_type,
	            unordered_map<string, string> _attributes)
	    : mvnxElementContent(_element_type, _attributes), points(NULL){};
	void setChild(mvnxIContent*& _childElement)
	{
		switch (_childElement->getElementType()) {
			case POINTS:
				points = _childElement;
				break;
			default:
				break;
		}
	}
};

class mvnxSegments : public mvnxElementContent {
private:
	vector<mvnxIContent*> segments;

public:
	mvnxSegments(mvnxElements_t _element_type,
	             unordered_map<string, string> _attributes)
	    : mvnxElementContent(_element_type, _attributes){};
	void setChild(mvnxIContent*& _childElement)
	{
		switch (_childElement->getElementType()) {
			case SEGMENT:
				segments.push_back(_childElement);
				break;
			default:
				break;
		}
	}

	// Get child element(s)
	vector<mvnxSegment*> getSegmentVector() const
	{
		vector<mvnxSegment*> segmentsCasted(segments.size());
		transform(segments.begin(), segments.end(), segmentsCasted.begin(),
		          [](mvnxIContent* e) {
			          return dynamic_cast<mvnxSegment*>(e);
		          });
		return segmentsCasted;
	};
};

class mvnxJoint : public mvnxElementContent {
private:
	pair<mvnxIContent*, mvnxIContent*> connectors;

public:
	mvnxJoint(mvnxElements_t _element_type,
	          unordered_map<string, string> _attributes)
	    : mvnxElementContent(_element_type, _attributes){};
	void setChild(mvnxIContent*& _childElement)
	{
		switch (_childElement->getElementType()) {
			case CONNECTOR1:
				connectors.first = _childElement;
				break;
			case CONNECTOR2:
				connectors.second = _childElement;
				break;
			default:
				break;
		}
	}
};

class mvnxJoints : public mvnxElementContent {
private:
	vector<mvnxIContent*> joints;

public:
	mvnxJoints(mvnxElements_t _element_type,
	           unordered_map<string, string> _attributes)
	    : mvnxElementContent(_element_type, _attributes){};
	void setChild(mvnxIContent*& _childElement)
	{
		switch (_childElement->getElementType()) {
			case JOINT:
				joints.push_back(_childElement);
				break;
			default:
				break;
		}
	}
};

class mvnxFrame : public mvnxElementContent {
private:
	pair<mvnxIContent*, mvnxIContent*> connectors;

public:
	mvnxFrame(mvnxElements_t _element_type,
	          unordered_map<string, string> _attributes)
	    : mvnxElementContent(_element_type, _attributes){};
	void setChild(mvnxIContent*& _childElement)
	{
		switch (_childElement->getElementType()) {
			case ORIENTATION:
				connectors.first = _childElement;
				break;
			case POSITION:
				connectors.second = _childElement;
				break;
			default:
				break;
		}
	}
};

class mvnxFrames : public mvnxElementContent {
private:
	vector<mvnxIContent*> frames;

public:
	mvnxFrames(mvnxElements_t _element_type,
	           unordered_map<string, string> _attributes)
	    : mvnxElementContent(_element_type, _attributes){};
	void setChild(mvnxIContent*& _childElement)
	{
		switch (_childElement->getElementType()) {
			case FRAME:
				frames.push_back(_childElement);
				break;
			default:
				break;
		}
	}

	// Get child element(s)
	vector<mvnxFrame*> getFrames() const
	{
		vector<mvnxFrame*> framesCasted(frames.size());
		transform(frames.begin(), frames.end(), framesCasted.begin(),
		          [](mvnxIContent* e) { return dynamic_cast<mvnxFrame*>(e); });
		return framesCasted;
	};
};

class mvnxSensors : public mvnxElementContent {
private:
	vector<mvnxIContent*> sensors;

public:
	mvnxSensors(mvnxElements_t _element_type,
	            unordered_map<string, string> _attributes)
	    : mvnxElementContent(_element_type, _attributes){};
	void setChild(mvnxIContent*& _childElement)
	{
		switch (_childElement->getElementType()) {
			case SENSOR:
				sensors.push_back(_childElement);
				break;
			default:
				break;
		}
	}
};

class mvnxSubject : public mvnxElementContent {
private:
	vector<mvnxIContent*> comments;
	mvnxIContent* segments;
	mvnxIContent* sensors;
	mvnxIContent* joints;
	mvnxIContent* frames;

public:
	mvnxSubject(mvnxElements_t _element_type,
	            unordered_map<string, string> _attributes)
	    : mvnxElementContent(_element_type, _attributes)
	    , segments(NULL)
	    , sensors(NULL)
	    , joints(NULL)
	    , frames(NULL){};

	void setChild(mvnxIContent*& _childElement)
	{
		switch (_childElement->getElementType()) {
			case COMMENT:
				comments.push_back(_childElement);
				break;
			case SEGMENTS:
				segments = _childElement;
				break;
			case SENSORS:
				sensors = _childElement;
				break;
			case JOINTS:
				joints = _childElement;
				break;
			case FRAMES:
				frames = _childElement;
				break;
			default:
				break;
		}
	}

	// Get child elements
	mvnxSegments* getSegments() const
	{
		return dynamic_cast<mvnxSegments*>(segments);
	};

	mvnxSensors* getSensors() const
	{
		return dynamic_cast<mvnxSensors*>(sensors);
	};

	mvnxJoints* getJoints() const { return dynamic_cast<mvnxJoints*>(joints); };
	mvnxFrames* getFrames() const { return dynamic_cast<mvnxFrames*>(frames); };
};

class mvnx : public mvnxElementContent {
private:
	mvnxIContent* mvn;
	vector<mvnxIContent*> comments;
	vector<mvnxIContent*> subjects;

public:
	mvnx(mvnxElements_t _element_type,
	     unordered_map<string, string> _attributes)
	    : mvnxElementContent(_element_type, _attributes), mvn(NULL){};

	virtual void setChild(mvnxIContent*& _childElement)
	{
		switch (_childElement->getElementType()) {
			case MVN:
				mvn = _childElement;
				break;
			case COMMENT:
				comments.push_back(_childElement);
				break;
			case SUBJECT:
				subjects.push_back(_childElement);
				break;
			default:
				break;
		}
	}

	// Get child elements
	vector<mvnxSubject*> getSubjects() const
	{
		// Cast to mvnxSubject the vector's item using a lambda expression
		vector<mvnxSubject*> subjectsCasted(subjects.size());
		transform(subjects.begin(), subjects.end(), subjectsCasted.begin(),
		          [](mvnxIContent* e) {
			          return dynamic_cast<mvnxSubject*>(e);
		          });
		return subjectsCasted;
	};
};

#endif // MVNX_DATA_CONTAINERS_H
