/*
 * Copyright: (C) 2017 iCub Facility
 * Author: Diego Ferigo
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 *
 */

/**
 * @file XMLStreamReaderDriver.cpp
 * @brief Driver for the XMLStreamReader class
 * @author Diego Ferigo
 * @date 06/04/2017
 */

#include "XMLStreamReader.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;
using namespace xmlstream;

bool file_exist(const char* fileName)
{
    ifstream infile(fileName);
    return infile.good();
}

int main(int argc, char* argv[])
{
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " /path/to/file.xml /path/to/schema.xsd"
             << endl
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

    XMLStreamReader parser;
    if (parser.setDocument(argv[1])) {
        cout << "Document loaded" << endl;
    }

    else {
        cerr << "Error loading document!" << endl;
        return 1;
    }

    if (parser.setSchema(argv[2]))
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
