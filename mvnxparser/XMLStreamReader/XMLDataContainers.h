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
    class IContent;
    class XMLContent;
    enum class Content;

} // namespace xmlstream

// A XML entry could be a parent of other elements (ELEMENT) with optional
// attributes, or an entry that contains text (TEXT) with no attributes.
enum class xmlstream::Content
{
    TEXT = 0,
    ELEMENT = 1
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

namespace xmlstream {
    // Aliases for pointers to XML elements
    // Useful expecially for the client side
    typedef std::shared_ptr<xmlstream::IContent> IContentPtrS;
    typedef std::shared_ptr<xmlstream::XMLContent> XMLContentPtrS;
    template <typename T>
    using vector_ptr = std::shared_ptr<std::vector<T>>;
    typedef vector_ptr<IContentPtrS> IContentVecPtrS;
    typedef vector_ptr<XMLContentPtrS> XMLContentVecPtrS;

    // Some useful typedefs for readability
    // Used internally
    typedef xmlstream::IContent child_t;
    typedef std::shared_ptr<child_t> child_ptr;
    typedef std::shared_ptr<child_t> parent_ptr;
    typedef std::weak_ptr<child_t> parent_wptr;
    //
    typedef std::string ElementName;
    typedef std::string ElementText;
    typedef std::string AttributeName;
    typedef std::string AttributeValue;
    //
    typedef std::unordered_map<ElementName, vector_ptr<child_ptr>> children_t;
    typedef std::shared_ptr<children_t> children_ptr;
    typedef std::unordered_map<AttributeName, AttributeValue> Attributes;
} // namespace xmlstream

// This is an abstract class that offers an interface for polymorphic usage
class xmlstream::IContent
{
protected:
    xmlstream::ElementText m_text;
    xmlstream::parent_wptr m_parent;
    xmlstream::children_ptr m_children;
    xmlstream::ElementName m_element;
    xmlstream::Content m_content_type;
    xmlstream::Attributes m_attributes;

public:
    // Constructor
    IContent(xmlstream::ElementName element = {},
             xmlstream::Attributes attributes = {},
             xmlstream::parent_ptr parent = {})
        : m_parent(parent)
        , m_element(element)
        , m_attributes(attributes)
    {}

    // Destructor
    virtual ~IContent() = default;

    // Get method(s)
    virtual std::string getText() const = 0;
    virtual xmlstream::Content getContentType() const = 0;
    virtual ElementName getElementName() const = 0;
    virtual xmlstream::Attributes getAttributes() const = 0;
    virtual parent_ptr getParent() const = 0;
    virtual AttributeValue getAttribute(AttributeName attribute) const = 0;
    virtual children_ptr getChildElements() const = 0;
    virtual vector_ptr<child_ptr> getChildElement(ElementName element) const = 0;

    // Set method(s)
    virtual void setText(std::string text) = 0;
    virtual void setChild(child_ptr childContent) = 0;
    virtual void setParent(parent_ptr parent) = 0;
    virtual void setAttributes(xmlstream::Attributes attributes) = 0;

    // Other methods
    virtual std::vector<child_ptr> findChildElements(ElementName element) = 0;
};

// This class handles both ELEMENT and TEXT xml content.
class xmlstream::XMLContent : public xmlstream::IContent
{
public:
    // Constructor
    XMLContent(ElementName element, xmlstream::Attributes attributes, parent_ptr parent)
        : IContent(element, attributes, parent)
    {}

    // Destructor
    virtual ~XMLContent() = default;

    // Get method(s)
    std::string getText() const override { return m_text; }
    xmlstream::Content getContentType() const override { return m_content_type; }
    xmlstream::ElementName getElementName() const override { return m_element; }
    xmlstream::Attributes getAttributes() const override { return m_attributes; }
    xmlstream::parent_ptr getParent() const override { return m_parent.lock(); }
    xmlstream::AttributeValue
    getAttribute(xmlstream::AttributeName attribute) const override
    {
        if (m_attributes.find(attribute) == m_attributes.end()) {
            return {};
        }
        return m_attributes.at(attribute);
    }
    xmlstream::children_ptr getChildElements() const override { return m_children; }
    vector_ptr<xmlstream::child_ptr>
    getChildElement(xmlstream::ElementName element) const override
    {
        if (m_children->find(element) == m_children->end()) {
            return nullptr;
        }
        return m_children->at(element);
    }

    // Set method(s)
    void setText(std::string text) override
    {
        // If setText() is called, it means that this element has a TEXT content
        m_content_type = xmlstream::Content::TEXT;

        // Set the text
        m_text = text;
    }
    void setChild(xmlstream::child_ptr childContent) override
    {
        // Not a nullptr
        assert(childContent);

        // If setChild() is called, it means that this element has a ELEMENT
        // content
        m_content_type = xmlstream::Content::ELEMENT;

        // If this is the first initialization, create the object and its
        // shared_pointer
        if (!m_children) { // not a nullptr
            m_children = std::make_shared<xmlstream::children_t>();
        }

        // Set the child
        // If there are no children of the same type, create a new entry
        if (m_children->find(childContent->getElementName()) == m_children->end()) {
            m_children->insert(
                std::make_pair(childContent->getElementName(),
                               std::make_shared<std::vector<xmlstream::child_ptr>>()));
        }

        // Add the new child to the vector of the children of the same type
        m_children->at(childContent->getElementName())->push_back(childContent);
    }
    void setParent(xmlstream::parent_ptr parent) override { m_parent = parent; }
    void setAttributes(xmlstream::Attributes attributes) override
    {
        m_attributes = attributes;
    }

    // Other methods
    std::vector<xmlstream::child_ptr>
    findChildElements(xmlstream::ElementName _element) override
    {
        std::vector<xmlstream::child_ptr> foundElements;
        std::vector<xmlstream::child_ptr> foundElementsInChild;

        // For every branch
        if (m_children) {
            for (const auto& child_element_map : *m_children) {
                // Reach its last leaf recursively
                for (const auto& child_element : *(child_element_map.second)) {
                    // Get the matches from the children
                    foundElementsInChild = child_element->findChildElements(_element);
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

#endif // XML_DATA_CONTAINERS_H
