/*
 * File:   OpenVol
 * Author: Fabian Torres
 * 
 * Created on 06/13/2016
 */

#include <iostream>
#include <string>
#include <QString>

using namespace std;


//!Opens *.vol ultrasound images
/*!
  Opens *.Vol ultrasound images that came from GE ultrasound systems
*/
class OpenVol
{

public:
	 
	///Constructor of the class
	static OpenVol *New()
	{
		return new OpenVol;
	} 

	/*
	set volume filename
	*/
	void setFilename(QString);

	/*
	read the volume
	*/
	void update();

private:

	/*
	volumefilename
	*/
	QString filename;

	/*
	format of the volume
	*/
	std::string format;

	/*
	machine mnufacturer
	*/
	std::string manufacturer;

	/*
	patient ID
	*/
	std::string patientID;

	/*
	patient name
	*/
	std::string patientName;

	/*
	institute name
	*/
	std::string instituteName;

	/*
	General electric ms
	*/
	std::string gems;

};