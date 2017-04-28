/*
 * Copyright: (C) 2017 iCub Facility
 * Author: Diego Ferigo
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 *
 */

/**
 * @file MVNXStreamReaderDriver.cpp
 * @brief Driver for the MVNXStreamReader class
 * @author Diego Ferigo
 * @date 18/04/2017
 */

#include "MVNXStreamReader.h"
#include "XMLDataContainers.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;
using namespace xmlstream;
using namespace xmlstream::mvnx;

bool file_exist(const char* fileName)
{
    ifstream infile(fileName);
    return infile.good();
}

int main(int argc, char* argv[])
{
    if (argc != 3) {
        cerr << "Usage: " << argv[0]
             << " /path/to/file.mvnx /path/to/schema.xsd" << endl
             << endl;
        return 1;
    }

    if (not file_exist(argv[1])) {
        cerr << "Document file doesn't exist!" << endl;
        return 1;
    }
    if (not file_exist(argv[2])) {
        cerr << "Schema file doesn't exist!" << endl;
        return 1;
    }

    // Load the document and set the validation schema
    MVNXStreamReader mvnx;
    if (mvnx.setDocument(argv[1])) {
        mvnx.setSchema(argv[2]);
    }
    else {
        cerr << "Failed to load the document!" << endl;
        return 1;
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
    if (mvnx.validate()) {
        // For debugging purpose, this function prints all the elements
        // implemented in the parser, showing all the ElementNames and
        // attributes.
        // mvnx.printParsedDocument();

        // Enable only some elements. If the configuration is not set, all
        // elements are enabled by default.
        // Be aware that specifying a configuration alters the final tree
        // structure of the parsed object!
        MVNXConfiguration conf;
        conf["mvnx"]     = true; // Root element
        conf["subject"]  = true;
        conf["segments"] = true;
        conf["segment"]  = true;
        conf["comment"]  = true;
        mvnx.setConf(conf);

        // Parse the MVNX
        mvnx.parse();

        // Get the pointer to the tree's root element
        XMLContentPtrS xmlRoot = mvnx.getXmlTreeRoot();

        // Extract a field, e.g. print all the names of segments
        IContentPtrS segments = xmlRoot->getChildElement("subject")
                                          ->front()
                                          ->getChildElement("segments")
                                          ->front();
        IContentVecPtrS segmentVector = segments->getChildElement("segment");
        //
        cout << "Get segments sweeping the XML tree:" << endl;
        for (const auto& segment : *segmentVector) {
            cout << segment->getAttribute("label") << endl;
        }
        cout << endl;

        // The class MVNXStreamReader has an utility method findElement() that
        // allows finding iteratively all the elements with the same name.
        // It calls the findChildElements() method from the XMLContent class,
        // and the only difference is that MVNXStreamReader cast the obtained
        // matches to XMLContent.

        // - Passing through the MVNXStreamReader object (preferred)
        vector<XMLContentPtrS> segment1 = mvnx.findElement("segment");
        //
        cout << "Get segments using the MVNXStreamReader object:" << endl;
        for (const auto& segment : segment1) {
            cout << segment->getAttribute("label") << endl;
        }
        cout << endl;

        // - Passing through the XMLContent object
        vector<IContentPtrS> segment2 = xmlRoot->findChildElements("segment");
        //
        cout << "Get segments using the XMLContent object:" << endl;
        for (const auto& segment : segment2) {
            cout << segment->getAttribute("label") << endl;
        }
        cout << endl;

        // Example of gathering information from a xml element with TEXT content
        vector<XMLContentPtrS> comments = mvnx.findElement("comment");
        //
        cout << "Get comments as an example for TEXT content elements:" << endl;
        for (const auto& comment : comments) {
            cout << comment->getText() << endl;
        }
    }
    else {
        return 1;
    }
    return 0;
}
