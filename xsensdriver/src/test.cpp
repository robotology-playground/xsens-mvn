#include "XsensMVN.h"
#include <yarp/os/ResourceFinder.h>
#include <yarp/os/Value.h>
#include <unordered_map>
#include <xme.h>
#include <windows.h>
#include <yarp/os/ConstString.h>
#include <yarp/dev/PolyDriver.h>
#include <iostream>
#include <conio.h>

int main(int argc, char **argv) {

    yarp::os::ResourceFinder rf = yarp::os::ResourceFinder::getResourceFinderSingleton();

    rf.configure(argc, argv);

  
    std::string configuration = rf.check("config", yarp::os::Value("prova"), "checking MVN configuration").toString();
    std::cout << configuration;

    rf.setDefault("config","Human/FullBody");
    
  
    yarp::dev::XsensMVN xsens;
    if (!xsens.open(rf)) {
        std::cerr << "Failed to open Xsens device\n";
        return -1;
    }


    std::unordered_map<std::string, yarp::os::Value> dimensions;
    dimensions.insert(std::unordered_map<std::string, yarp::os::Value>::value_type("bodyHeight", yarp::os::Value(1.7)));
    dimensions.insert(std::unordered_map<std::string, yarp::os::Value>::value_type("footSize", yarp::os::Value(0.4)));
    bool result = xsens.setBodyDimensions(dimensions);
    result = result && xsens.calibrate("Tpose");
    std::cerr << "Result of calibration " << result << "\n";

    char a; std::cin >> a;
    return 0;
}
