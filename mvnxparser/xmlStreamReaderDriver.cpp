/*
 * Copyright: (C) 2017 iCub Facility
 * Author: Diego Ferigo
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 *
 */

/**
 * @file xmlStreamReaderDriver.cpp
 * @brief Driver for the xmlStreamReader class
 * @author Diego Ferigo
 * @date 06/04/2017
 */

#include "xmlStreamReader.h"
#include <cassert>
#include <iostream>
#include <vector>

using namespace std;

int main(int argc, char* argv[])
{
    xmlStreamReader parser;
    if (parser.setDocument("/home/dferigo/git/xml-demo-qt/Meri-020.mvnx")) {
        cout << "Document loaded" << endl;
    }

    else {
        cerr << "Error loading document!" << endl;
        return 1;
    }

    if (parser.setSchema("/home/dferigo/git/xml-demo-qt/schema.xsd"))
        cout << "Schema loaded and valid" << endl;

    else {
        cerr << "Error loading the schema!" << endl;
        return 1;
    }

    if (parser.validate())
        cout << "Document has been validated successfully" << endl;
    else {
        cerr << "Document is not valid!" << endl;
    }
    return 0;
}
