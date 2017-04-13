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

#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace xmlstream {

// A XML entry could be a parent of other elements (ELEMENT) with optional
// attributes, or an entry that contains text (TEXT) with no attributes.
enum content_t
{
    TEXT = 0,
    ELEMENT
};

//
// Pure abstract class from which `xmlContent` classes will derive.
// `xmlContent` could have a TEXT or ELEMENT content type.
//
// TEXT objects have a string content and xml attributes.
// ELEMENT objects have children (vector<IContent*>) and xml attributes.
//
// IContent will provide the base polymorphic type to handle both classes.
//
class IContent;

// Some useful typedefs for readability
typedef IContent child_t;
typedef std::string elementName;
typedef std::string attributeName;
typedef std::string attributeValue;
typedef std::unordered_map<elementName, std::vector<child_t*>> children_t;
typedef std::unordered_map<attributeName, attributeValue> attributes_t;

class IContent {
protected:
    std::string text;
    children_t children;
    elementName element;
    content_t content_type;
    attributes_t attributes;

public:
    // Constructor(s)
    IContent(elementName _element = "noname") : element(_element){};
    IContent(elementName _element, attributes_t _attributes)
        : element(_element), attributes(_attributes){};

    // Destructor (Enable polymorphism)
    virtual ~IContent() = default;

    // Get method(s)
    virtual std::string getText() const                                  = 0;
    virtual content_t getContentType() const                             = 0;
    virtual elementName getElementName() const                           = 0;
    virtual attributes_t getAttributes() const                           = 0;
    virtual attributeValue getAttribute(attributeName _attribute)        = 0;
    virtual children_t* getChildElements()                               = 0;
    virtual std::vector<child_t*>* getChildElement(elementName _element) = 0;

    // Set method(s)
    virtual void setText(std::string _text)              = 0;
    virtual void setChild(child_t*& _childContent)       = 0;
    virtual void setAttributes(attributes_t _attributes) = 0;

    // Other methods
    virtual std::vector<child_t*> findChildElements(elementName _element) = 0;
};

// This class implements IContent in order to be used for TEXT XML entries.
// It adds text storage support.
class xmlContent : public IContent {
public:
    // Constructor(s)
    xmlContent(elementName _element) : IContent(_element){};
    xmlContent(elementName _element, attributes_t _attributes)
        : IContent(_element, _attributes){};

    // Destructor
    virtual ~xmlContent() = default;

    // Get method(s)
    virtual std::string getText() const { return text; };
    virtual content_t getContentType() const { return content_type; };
    virtual elementName getElementName() const { return element; };
    virtual attributes_t getAttributes() const { return attributes; }
    virtual attributeValue getAttribute(attributeName _attribute)
    {
        if (attributes.find(_attribute) != attributes.end()) {
            return attributes[_attribute];
        }
        else {
            return std::string();
        }
    };
    virtual children_t* getChildElements() { return &children; }
    virtual std::vector<child_t*>* getChildElement(elementName _element)
    {
        if (children.find(_element) != children.end()) {
            return &children[_element];
        }
        else {
            assert(0); // If Debug mode, notify that the accessed element
                       // doesn't exist. When asserts are not enabled, the
                       // program will segfault.
            return nullptr;
        }
    };

    // Set method(s)
    virtual void setText(std::string _text)
    {
        // If setText() is called, it means that this element has a TEXT content
        content_type = TEXT;

        // Set the text
        text = _text;
    };
    virtual void setChild(child_t*& _childContent)
    {
        // If setChild() is called, it means that this element has a ELEMENT
        // content
        content_type = ELEMENT;

        // Set the child
        // If there are no children of the same type, create a new entry
        if (children.find(_childContent->getElementName()) == children.end()) {
            children[_childContent->getElementName()] = std::vector<child_t*>();
        }
        // Add the new children to the vector of the children of the same type
        children[_childContent->getElementName()].push_back(_childContent);
    };
    virtual void setAttributes(attributes_t _attributes)
    {
        attributes = _attributes;
    };

    // Other methods
    virtual std::vector<child_t*> findChildElements(elementName _element)
    {
        std::vector<child_t*> foundElements;
        std::vector<child_t*> foundElementsInChild;

        // For every branch
        for (auto child_vector : children) {
            // Reach its last leaf recursively
            for (auto child : child_vector.second) {
                foundElementsInChild = child->findChildElements(_element);
                // Save all the matches in this branch of the tree
                foundElements.insert(foundElements.end(),
                                     foundElementsInChild.begin(),
                                     foundElementsInChild.end());
            }
        }

        // Insert the current object pointer in the tree vector
        if (getElementName() == _element) {
            foundElements.push_back(this);
        }
        return foundElements;
    };
};

} // namespace xmlstream

#endif // XML_DATA_CONTAINERS_H
