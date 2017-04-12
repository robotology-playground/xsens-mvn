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
 * @date 12/04/2017
 */

#include "mvnxDataContainers.h"
#include "mvnxStreamReader.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;
using namespace mvnx_ns;

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
	assert(file_exist(argv[1]));
	assert(file_exist(argv[2]));

	mvnxStreamReader mvnx;
	assert(mvnx.setDocument(argv[1]));
	mvnx.setSchema(argv[2]);

	// Validate the document with the provided schema and, if successful,
	// parse it.
	if (mvnx.validate()) {
		// For debugging purpose, this function prints all the elements
		// implemented in the parser, showing all the elementNames and
		// attributes.
		// mvnx.printParsedDocument();

		// Parse the MVNX
		mvnx.parse();

		// Extract a field, e.g. the print all the segments and their origin
		vector<mvnxSegment*> segments = mvnx.getSubjects()
		                                          .front()
		                                          ->getSegments()
		                                          ->getSegmentVector();

		// Print extracted information, e.g. the segments names
		cout << endl << "Segments labels of the first subject are:" << endl;
		for (auto segment : segments) {
			cout << "\t" << segment->getAttributes()["label"] << endl;
		}
	}
	else
		return 1;
}
