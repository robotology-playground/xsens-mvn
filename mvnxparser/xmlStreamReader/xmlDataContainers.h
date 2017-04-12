/*
 * Copyright: (C) 2017 iCub Facility
 * Author: Diego Ferigo
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 *
 */

/**
 * @file xmlDataContainers.cpp
 * @brief Base classes for handling TEXT and CONTENT xml elements
 * @author Diego Ferigo
 * @date 12/04/2017
 */
#ifndef XML_DATA_CONTAINERS_H
#define XML_DATA_CONTAINERS_H

#include <string>
#include <unordered_map>

using namespace std;

// A XML entry could be a parent of other entries (ELEMENT) with optional
// attributes, or an entry that contains text (TEXT) with no attributes.
enum content_t { TEXT = 0, ELEMENT };

// Pure abstract class from which TEXT and ELEMENT classes will derive.
// This will provide the base polymorphic type to handle both classes.
template <typename T>
class IContent {
protected:
	T element_type;
	content_t content_type;

public:
	// Constructor(s)
	IContent(T _element_type) : element_type(_element_type){};

	// Destructor (Enable polymorphism)
	virtual ~IContent() = default;

	// Set method(s)
	virtual void setText(string _text)                 = 0;
	virtual void setChild(IContent<T>*& _childContent) = 0;

	// Get method(s)
	virtual string getText() const   = 0;
	virtual T getElementType() const = 0;
};

// This class implements IContent in order to be used for TEXT XML entries.
// It adds text storage support.
template <typename T>
class textContent : public IContent<T> {
protected:
	string text;

public:
	// Constructor(s)
	textContent(T _element_type) : IContent<T>(_element_type){};
	textContent(T _element_type, string _text)
	    : IContent<T>(_element_type), text(_text){};

	// Destructor (Enable polymorphism)
	virtual ~textContent() = default; // TODO

	// Set method(s)
	virtual void setText(string _text) { text = _text; };
	virtual void setChild(IContent<T>*& _childElement){};

	// Get method(s)
	virtual string getText() const { return text; };
	virtual T getElementType() const
	{
		// This is required because the parent's class is a template
		return this->element_type;
	}; // TODO; check
};

// This class implements IContent in order to be used for ELEMENT XML entries.
// It adds attributes and defines child(s) inclusion.
template <typename T>
class elementContent : public IContent<T> {
protected:
	unordered_map<string, string> attributes;

public:
	// Constructor(s)
	elementContent(T _element_type, unordered_map<string, string> _attributes)
	    : IContent<T>(_element_type), attributes(_attributes)
	{
		// This is required because the parent's class is a template
		this->content_type = ELEMENT;
	};

	// Destructor (Enable polymorphism)
	virtual ~elementContent() = default;

	// Get method(s)
	virtual string getText() const { return string(); }; // TODO: hide
	virtual T getElementType() const { return this->element_type; }; // TODO
	unordered_map<string, string> getAttributes() const { return attributes; };

	// Set child elements
	virtual void setText(string _text){};
	virtual void setChild(IContent<T>*& _childElement){};
};

#endif // XML_DATA_CONTAINERS_H
