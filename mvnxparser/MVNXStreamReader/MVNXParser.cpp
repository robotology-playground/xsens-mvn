/*
 * Copyright (C) 2018 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * GNU Lesser General Public License v2.1 or any later version.
 */

#include "MVNXStreamReader.h"
#include "XMLDataContainers.h"

#include <QCommandLineParser>
#include <iostream>

using namespace xmlstream::mvnx;

int main(int argc, char* argv[])
{
    QCoreApplication mvnxParser(argc, argv);
    QCoreApplication::setApplicationName("MVNXParser");
    QCoreApplication::setApplicationVersion("1.0");

    // Define available arguments and options for the current program
    // ==============================================================

    QCommandLineParser optionsParser;
    optionsParser.setApplicationDescription("MVNX Parser");
    optionsParser.addHelpOption();
    optionsParser.addVersionOption();

    optionsParser.addPositionalArgument("inputFile",
                                        QCoreApplication::translate("main", "MVNX file to parse"));

    // Boolean Option to enable only data for runtime parsing
    QCommandLineOption runtimeDataOnlyOption(
        QStringList() << "rdo"
                      << "runtimeDataOnly",
        QCoreApplication::translate("main", "Save only runtime data for MAP computation"));
    optionsParser.addOption(runtimeDataOnlyOption);

    // Boolean option to enable only data required for model creation
    QCommandLineOption modelCreationDataOnlyOption(
        QStringList() << "mcdo"
                      << "modelCreationDataOnly",
        QCoreApplication::translate("main", "Save only HDE/MAP model creation data"));
    optionsParser.addOption(modelCreationDataOnlyOption);

    // Option to enable mvnx validation based on user-provided XSD file
    QCommandLineOption validateSchemaOption(
        "validate",
        QCoreApplication::translate("main",
                                    "Validate MVNX file using the specified <xsd-file-path>."),
        QCoreApplication::translate("main", "xsd-file-path"));
    optionsParser.addOption(validateSchemaOption);

    // Option to enable mvnx validation based on user-provided XSD file
    QCommandLineOption targetDirectoryOption(
        "outputFolder",
        QCoreApplication::translate("main", "Save parser outputs in <directory>."),
        QCoreApplication::translate("main", "directory"));
    optionsParser.addOption(targetDirectoryOption);

    // process the command line arguments and options used
    optionsParser.process(mvnxParser);

    // create MVNXStreamReader object
    MVNXStreamReader mvnx;

    // verify presence of MVNX file to parse and open it in read-only mode
    QFileInfo inputFileInfo(optionsParser.positionalArguments().first());
    inputFileInfo.makeAbsolute();

    if (!inputFileInfo.exists()
        || !mvnx.setDocument(inputFileInfo.absoluteFilePath().toStdString())) {
        std::cerr << "MVNX file not found or unable to open it in read-only mode" << std::endl;
        return EXIT_FAILURE;
    }

    // if requested, validate the MVNX file using the user-specified XSD schema
    if (optionsParser.isSet(validateSchemaOption)) {
        QFileInfo inputXSDFileInfo(optionsParser.value(validateSchemaOption));
        inputXSDFileInfo.makeAbsolute();
        if (!inputXSDFileInfo.exists() || !inputXSDFileInfo.isReadable()) {
            std::cerr << "Schema file doesn't exist!" << std::endl;
            return EXIT_FAILURE;
        }
        mvnx.setSchema(inputXSDFileInfo.absoluteFilePath().toStdString().c_str());

        // if the XSD validation fails, notify the user and try to proceed anyway
        if (!mvnx.validate()) {
            std::cerr << "Failed to validate the MVNX file using the specified XSD schema"
                      << std::endl
                      << "Attempting to parse anyway" << std::endl;
        }
    }

    // handle output folder creation. If option is enabled use the specified one, otherwise create a
    // outputData folder inside the current working directory
    QDir outputFolder;
    if (!optionsParser.isSet(targetDirectoryOption)) {
        outputFolder.setPath(QDir::currentPath());
        if (!outputFolder.exists("outputData") && !outputFolder.mkdir("outputData")) {
            std::cerr << "Failed to create default output data folder" << std ::endl;
            return EXIT_FAILURE;
        }
        outputFolder.setPath(QDir::currentPath() + QDir::separator() + "outputData");
        outputFolder.makeAbsolute();
    }
    else {
        outputFolder.setPath(optionsParser.value(targetDirectoryOption));
        outputFolder.makeAbsolute();
        if (!outputFolder.exists(outputFolder.absolutePath())
            && !QDir().mkpath(outputFolder.absolutePath())) {
            std::cerr << "Failed to create requested output data folder" << std ::endl;
            return EXIT_FAILURE;
        }
    }

    // parse the MVNX file
    if (!mvnx.parse()) {
        std::cerr << "Failed to parse the MVNX file" << std ::endl;
        return EXIT_FAILURE;
    }

    // create output files
    // ===================

    // if runtimeDataOnly == false print the calibration data
    if (!optionsParser.isSet(runtimeDataOnlyOption)) {
        // print .log file containing model metadata
        std::string metadataLogFile =
            (outputFolder.absolutePath() + QDir::separator()).toStdString()
            + inputFileInfo.baseName().toStdString() + ".log";
        mvnx.printCalibrationFile_LOG(metadataLogFile, ',');

        // print .xml file containing model metadata
        std::string metadataXmlFile =
            (outputFolder.absolutePath() + QDir::separator()).toStdString()
            + inputFileInfo.baseName().toStdString() + ".xml";
        mvnx.printCalibrationFile_XML(metadataXmlFile);

        // print .csv file containing data required to create the MAPEST/HDE model
        std::string modelCreationDataFile =
            (outputFolder.absolutePath() + QDir::separator()).toStdString()
            + inputFileInfo.baseName().toStdString() + ".csv";
        mvnx.printDataFile(modelCreationDataFile,
                           std::vector<MVNXStreamReader::OutputDataType>{
                               MVNXStreamReader::OutputDataType::LINK_ACCELERATION,
                               MVNXStreamReader::OutputDataType::LINK_ORIENTATION,
                               MVNXStreamReader::OutputDataType::LINK_ANGULAR_VELOCITY,
                               MVNXStreamReader::OutputDataType::LINK_ANGULAR_ACCELERATION,
                               MVNXStreamReader::OutputDataType::SENSOR_ORIENTATION,
                               MVNXStreamReader::OutputDataType::SENSOR_FREE_BODY_ACCELERATION,
                           },
                           ',');
    }

    // if modelCreationDataOnly == false print the runtime data
    if (!optionsParser.isSet(modelCreationDataOnlyOption)) {
        // print lightweight CSV file containing only a subset of data for runtime computation
        std::string runtimeDataFile =
            (outputFolder.absolutePath() + QDir::separator()).toStdString()
            + inputFileInfo.baseName().toStdString() + "_runtime.csv";
        mvnx.printDataFile(runtimeDataFile,
                           std::vector<MVNXStreamReader::OutputDataType>{
                               MVNXStreamReader::OutputDataType::SENSOR_FREE_BODY_ACCELERATION,
                           },
                           ',');
    }

    return EXIT_SUCCESS;
}
