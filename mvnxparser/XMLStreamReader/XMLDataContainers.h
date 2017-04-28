/*
 * Copyright: (C) 2017 iCub Facility
 * Author: Diego Ferigo
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 *
 */

/**
 * @file XMLDataContainers.cpp
 * @brief Base classes for handling TEXT and CONTENT xml elements
 * @author Diego Ferigo
 * @date 18/04/2017
 */

#ifndef XML_DATA_CONTAINERS_H
#define XML_DATA_CONTAINERS_H

#include <cassert>
#include <iostream>
#include <memory>
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
// Pure abstract class from which `XMLContent` classes will derive.
// `XMLContent` could have a TEXT or ELEMENT content type.
//
// TEXT objects have a string content and xml attributes.
// ELEMENT objects have children (vector<IContent*>) and xml attributes.
//
// IContent will provide the base polymorphic type to handle both classes.
//
class IContent;
class XMLContent;

// Aliases for pointers to XML elements
// Useful expecially for the client side
typedef std::shared_ptr<IContent> IContentPtrS;
typedef std::shared_ptr<XMLContent> XMLContentPtrS;
template <typename T>
using vector_ptr = std::shared_ptr<std::vector<T>>;
typedef vector_ptr<IContentPtrS> IContentVecPtrS;
typedef vector_ptr<XMLContentPtrS> XMLContentVecPtrS;

// Some useful typedefs for readability
// Used internally
typedef IContent child_t;
typedef std::shared_ptr<child_t> child_ptr;
typedef std::shared_ptr<child_t> parent_ptr;
typedef std::weak_ptr<child_t> parent_wptr;
//
typedef std::string ElementName;
typedef std::string AttributeName;
typedef std::string AttributeValue;
//
typedef std::unordered_map<ElementName, vector_ptr<child_ptr>> children_t;
typedef std::shared_ptr<children_t> children_ptr;
typedef std::unordered_map<AttributeName, AttributeValue> attributes_t;

// This is an abstract class that offers an interface for polymorphic usage
class IContent {
protected:
    std::string text;
    parent_wptr parent;
    children_ptr children;
    ElementName element;
    content_t content_type;
    attributes_t attributes;

public:
    // Constructor(s)
    IContent(ElementName _element = "noname") : element(_element) {}
    IContent(ElementName _element, attributes_t _attributes, parent_ptr _parent)
        : parent(_parent), element(_element), attributes(_attributes)
    {
    }

    // Destructor
    virtual ~IContent() = default;

    // Get method(s)
    virtual std::string getText() const                                 = 0;
    virtual content_t getContentType() const                            = 0;
    virtual ElementName getElementName() const                          = 0;
    virtual attributes_t getAttributes() const                          = 0;
    virtual parent_ptr getParent() const                                = 0;
    virtual AttributeValue getAttribute(AttributeName _attribute)       = 0;
    virtual children_ptr getChildElements()                             = 0;
    virtual vector_ptr<child_ptr> getChildElement(ElementName _element) = 0;

    // Set method(s)
    virtual void setText(std::string _text)              = 0;
    virtual void setChild(child_ptr _childContent)       = 0;
    virtual void setParent(parent_ptr _parent)           = 0;
    virtual void setAttributes(attributes_t _attributes) = 0;

    // Other methods
    virtual std::vector<child_ptr> findChildElements(ElementName _element) = 0;
};

// This class handles both ELEMENT and TEXT xml content.
class XMLContent : public IContent {
public:
    // Constructor(s)
    XMLContent() = default;
    XMLContent(ElementName _element) : IContent(_element) {}
    XMLContent(ElementName _element,
               attributes_t _attributes,
               parent_ptr _parent)
        : IContent(_element, _attributes, _parent){};

    // Destructor
    virtual ~XMLContent() = default;

    // Get method(s)
    virtual std::string getText() const { return text; }
    virtual content_t getContentType() const { return content_type; }
    virtual ElementName getElementName() const { return element; }
    virtual attributes_t getAttributes() const { return attributes; }
    virtual parent_ptr getParent() const { return parent.lock(); }
    virtual AttributeValue getAttribute(AttributeName _attribute)
    {
        if (attributes.find(_attribute) != attributes.end()) {
            return attributes[_attribute];
        }
        else {
            return std::string();
        }
    }
    virtual children_ptr getChildElements() { return children; }
    virtual vector_ptr<child_ptr> getChildElement(ElementName _element)
    {
        if (children->find(_element) != children->end()) {
            return children->at(_element);
        }
        else {
            assert(0); // If Debug mode, notify that the accessed element
                       // doesn't exist. When asserts are not enabled, the
                       // program will segfault.
            return nullptr;
        }
    }

    // Set method(s)
    virtual void setText(std::string _text)
    {
        // If setText() is called, it means that this element has a TEXT content
        content_type = TEXT;

        // Set the text
        text = _text;
    }
    virtual void setChild(child_ptr _childContent)
    {
        // Not a nullptr
        assert(_childContent);

        // If setChild() is called, it means that this element has a ELEMENT
        // content
        content_type = ELEMENT;

        // If this is the first initialization, create the object and its
        // shared_pointer
        if (not children) { // not a nullptr
            children = std::make_shared<children_t>();
        }

        // Set the child
        // If there are no children of the same type, create a new entry
        if (children->find(_childContent->getElementName())
            == children->end()) {
            children->insert(std::make_pair(
                      _childContent->getElementName(),
                      std::make_shared<std::vector<child_ptr>>()));
        }

        // Add the new children to the vector of the children of the same type
        children->at(_childContent->getElementName())->push_back(_childContent);
    }
    virtual void setParent(parent_ptr _parent) { parent = _parent; }
    virtual void setAttributes(attributes_t _attributes)
    {
        attributes = _attributes;
    }

    // Other methods
    virtual std::vector<child_ptr> findChildElements(ElementName _element)
    {
        std::vector<child_ptr> foundElements;
        std::vector<child_ptr> foundElementsInChild;

        // For every branch
        if (children) { // Not a nullptr
            for (const auto& child_element_map : *children) {
                // Reach its last leaf recursively
                for (const auto& child_element : *(child_element_map.second)) {
                    // Get the matches from the children
                    foundElementsInChild
                              = child_element->findChildElements(_element);
                    // Save all the matches of this branch of the tree
                    foundElements.insert(foundElements.end(),
                                         foundElementsInChild.begin(),
                                         foundElementsInChild.end());
                    // Add the children itself if it matches the wanted
                    // element
                    if (child_element->getElementName() == _element) {
                        foundElements.push_back(child_element);
                    }
                }
            }
        }
        return foundElements;
    }
};

} // namespace xmlstream

#endif // XML_DATA_CONTAINERS_H
