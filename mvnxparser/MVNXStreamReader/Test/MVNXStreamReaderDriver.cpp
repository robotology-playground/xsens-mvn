/*
 * Copyright (C) 2018 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * GNU Lesser General Public License v2.1 or any later version.
 */

#include "MVNXStreamReader.h"
#include "XMLDataContainers.h"
#include <fstream>
#include <iostream>
#include <vector>

using namespace xmlstream;
using namespace xmlstream::mvnx;

bool file_exist(const char* fileName)
{
    std::ifstream infile(fileName);
    return infile.good();
}

int main(int argc, char* argv[])
{
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " full/path/to/file.mvnx"
                  << "full/path/to/schema.xsd" << std::endl
                  << std::endl;
        return EXIT_FAILURE;
    }

    if (!file_exist(argv[1])) {
        std::cerr << "Document file doesn't exist!" << std::endl;
        return EXIT_FAILURE;
    }

    if (!file_exist(argv[2])) {
        std::cerr << "Schema file doesn't exist!" << std::endl;
        return EXIT_FAILURE;
    }

    // Load the document and set the validation schema
    MVNXStreamReader mvnx;
    if (mvnx.setDocument(argv[1])) {
        mvnx.setSchema(argv[2]);
    }
    else {
        std::cerr << "Failed to load the document!" << std::endl;
        return EXIT_FAILURE;
    }

    // Note:
    //
    // - The API of XMLContent return variables based on the IContent type
    // - The API of MVNXStreamReader return variables based on the XMLContent
    //   type
    //
    // Since IContent is an abstract class and objects can't be instantiated
    // from it, you can always cast IContent objects to XMLContent ones if you
    // need to access methods that only XMLContent contains.

    // Validate the document with the provided schema and, if successful,
    // parse it.
    if (!mvnx.validate()) {
        std::cerr << "Validation failed. Proceeeding anyway." << std::endl;
        // return EXIT_FAILURE;
    }

    // For debugging purpose, this function prints all the elements
    // implemented in the parser, showing all the ElementNames and
    // attributes.
    // mvnx.printParsedDocument();

    // Enable only some elements. If the configuration is not set, all
    // elements are enabled by default.
    // Be aware that specifying a configuration alters the final tree
    // structure of the parsed object!
    MVNXConfiguration conf;
    conf["mvnx"] = true; // Root element
    conf["subject"] = true;
    conf["segments"] = true;
    conf["segment"] = true;
    conf["comment"] = true;
    mvnx.setConf(conf);

    // Parse the MVNX
    mvnx.parse();

    // Get the pointer to the tree's root element
    XMLContentPtrS xmlRoot = mvnx.getXmlTreeRoot();

    // Extract a field, e.g. print all the names of segments
    IContentPtrS segments =
        xmlRoot->getChildElement("subject")->front()->getChildElement("segments")->front();
    IContentVecPtrS segmentVector = segments->getChildElement("segment");
    //
    std::cout << "Get segments sweeping the XML tree:" << std::endl;
    for (const auto& segment : *segmentVector) {
        std::cout << segment->getAttribute("label") << std::endl;
    }
    std::cout << std::endl;

    // The class MVNXStreamReader has an utility method findElement() that
    // allows finding iteratively all the elements with the same name.
    // It calls the findChildElements() method from the XMLContent class,
    // and the only difference is that MVNXStreamReader cast the obtained
    // matches to XMLContent.

    // - Passing through the MVNXStreamReader object (preferred)
    std::vector<XMLContentPtrS> segment1 = mvnx.findElement("segment");
    //
    std::cout << "Get segments using the MVNXStreamReader object:" << std::endl;
    for (const auto& segment : segment1) {
        std::cout << segment->getAttribute("label") << std::endl;
    }
    std::cout << std::endl;

    // - Passing through the XMLContent object
    std::vector<IContentPtrS> segment2 = xmlRoot->findChildElements("segment");
    //
    std::cout << "Get segments using the XMLContent object:" << std::endl;
    for (const auto& segment : segment2) {
        std::cout << segment->getAttribute("label") << std::endl;
    }
    std::cout << std::endl;

    // Example of gathering information from a xml element with TEXT content
    std::vector<XMLContentPtrS> comments = mvnx.findElement("comment");
    //
    std::cout << "Get comments as an example for TEXT content elements:" << std::endl;
    for (const auto& comment : comments) {
        std::cout << comment->getText() << std::endl;
    }

    return EXIT_SUCCESS;
}
