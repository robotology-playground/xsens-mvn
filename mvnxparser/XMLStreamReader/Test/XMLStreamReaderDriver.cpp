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

using namespace xmlstream;

bool file_exist(const char* fileName)
{
    std::ifstream infile(fileName);
    return infile.good();
}

int main(int argc, char* argv[])
{
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " /path/to/file.xml /path/to/schema.xsd"
                  << std::endl
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

    XMLStreamReader parser;

    // Parse the document
    if (!parser.setDocument(argv[1])) {
        std::cerr << "Error loading document!" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Document loaded" << std::endl;

    // Load the document
    if (!parser.setSchema(argv[2])) {
        std::cerr << "Error loading the schema!" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Schema loaded and valid" << std::endl;

    // Validate the document
    if (!parser.validate()) {
        std::cerr << "Document is not valid!" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Document has been validated successfully" << std::endl;

    return EXIT_SUCCESS;
}
