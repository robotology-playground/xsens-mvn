/*
 * Copyright: (C) 2017 iCub Facility
 * Author: Diego Ferigo
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 *
 */

/**
 * @file mvnxStreamReaderDriver.cpp
 * @brief Driver for the xmlStreamReader class
 * @author Diego Ferigo
 * @date 18/04/2017
 */

#include "mvnxStreamReader.h"
#include "xmlDataContainers.h"
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
    mvnxStreamReader mvnx;
    if (mvnx.setDocument(argv[1])) {
        mvnx.setSchema(argv[2]);
    }
    else {
        cerr << "Failed to load the document!" << endl;
        return 1;
    }

    // Note:
    //
    // - The API of xmlContent return variables based on the IContent type
    // - The API of mvnxStreamReader return variables based on the xmlContent
    //   type
    //
    // Since IContent is an abstract class and objects can't be instantiated
    // from it, you can always cast IContent objects to xmlContent ones if you
    // need to access methods that only xmlContent contains.

    // Validate the document with the provided schema and, if successful,
    // parse it.
    if (mvnx.validate()) {
        // For debugging purpose, this function prints all the elements
        // implemented in the parser, showing all the elementNames and
        // attributes.
        // mvnx.printParsedDocument();

        // Enable only some elements. If the configuration is not set, all
        // elements are enabled by default.
        // Be aware that specifying a configuration alters the final tree
        // structure of the parsed object!
        mvnxConf conf;
        conf["mvnx"]     = true; // Root element
        conf["subject"]  = true;
        conf["segments"] = true;
        conf["segment"]  = true;
        mvnx.setConf(conf);

        // Parse the MVNX
        mvnx.parse();

        // Get the pointer to the tree's root element
        xmlContentPtrS xmlRoot = mvnx.getXmlTreeRoot();

        // Extract a field, e.g. the print all the segments and their origin
        IContentPtrS segments = xmlRoot->getChildElement("subject")
                                          ->front()
                                          ->getChildElement("segments")
                                          ->front();
        IContentVecPtrS segmentVector = segments->getChildElement("segment");
        //
        cout << "Get segments sweeping the XML tree:" << endl;
        for (auto segment : *segmentVector) {
            cout << segment->getAttribute("label") << endl;
        }
        cout << endl;

        // The class mvnxStreeamReader has an utility method findElement() that
        // allows finding iteratively all the elements with the same name.
        // It calls the findChildElements() method from the xmlContent class,
        // and the only difference is that mvnxStreamReader cast the obtained
        // matches to xmlContent.

        // - Passing through the mvnxStreamReader object (preferred)
        vector<xmlContentPtrS> segment1 = mvnx.findElement("segment");
        //
        cout << "Get segments using the mvnxStreamReader object:" << endl;
        for (auto segment : segment1) {
            cout << segment->getAttribute("label") << endl;
        }
        cout << endl;

        // - Passing through the xmlContent object
        vector<IContentPtrS> segment2 = xmlRoot->findChildElements("segment");
        //
        cout << "Get segments using the xmlContent object:" << endl;
        for (auto segment : segment2) {
            cout << segment->getAttribute("label") << endl;
        }
    }
    else {
        return 1;
    }
    return 0;
}
