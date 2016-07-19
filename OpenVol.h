/*
 * File:   OpenVol
 * Author: Fabian Torres
 * 
 * Created on 06/13/2016
 */

#include <iostream>
#include <string>
#include <QString>

#include <vtkSmartPointer.h>
#include <vtkImageData.h> 

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
	void SetFilename(QString);

	/*
	read the volume data
	*/
	void Update();

	/*
	reaturn the vtk volume
	*/
	vtkSmartPointer<vtkImageData> GetOutput();

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

	/*
	X size
	*/
	int x;

	/*
	Y size
	*/
	int y;

	/*
	Z size
	*/
	int z;

	/*
	Voxel size
	*/
	double voxSize;

	/*
	Volume size
	*/
	int volSize;

	/*
	The output volume
	*/
	vtkSmartPointer<vtkImageData> volume;

};