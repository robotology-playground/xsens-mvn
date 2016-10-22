namespace yarp xsens

service XsensDriverService {

	bool calibrate();
	bool calibrateWithType(1: string calibrationType);
	oneway void startAcquisition();
	oneway void stopAcquisition();
	
	list<string> segments();
	map<string, double> bodyDimensions();
	bool setBodyDimension(1:string dimensionKey, 2:double dimensionValue);
	bool setBodyDimensions(map<string, double> dimensions);

}
