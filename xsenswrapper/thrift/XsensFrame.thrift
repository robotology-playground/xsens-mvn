namespace yarp xsens

struct Vector3 {
	1: double c1;
	2: double c2;
	3: double c3;
}

struct Vector4 {
	1: double c1;
	2: double c2;
	3: double c3;
	4: double c4;
}

struct XsensSegmentData {
	//Linear quantities
	1:  Vector3 position;
	2:  Vector3 velocity;
	3:  Vector3 acceleration;
	
	//Angular quantities
	4:  Vector4 orientation;
	5:  Vector3 angularVelocity;
	6:  Vector3 angularAcceleration;
}

struct XsensSensorData {
	1:  Vector3 acceleration;
	2:  Vector3 angularVelocity;
	3:  Vector3 magnetometer;
	4:  Vector4 orientation;
}

struct XsensFrame {
	1: double time;
	2: list<XsensSegmentData> segmentsData;
	3: list<XsensSensorData> sensorsData
}