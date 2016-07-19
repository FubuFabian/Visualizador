/*
 * File:   OpenVol
 * Author: Fabian Torres
 * 
 * Created on 06/13/2016
 */

#include "OpenVol.h"

#include <iostream>
#include <istream>
#include <fstream>
#include <vector>
#include <algorithm>

#include <vtkImageImport.h>

void OpenVol::SetFilename(QString filename)
{
	this->filename = filename;
}

void OpenVol::Update()
{
	std::basic_ifstream<unsigned char> file(filename.toAscii().data(), std::ios_base::in | std::ios_base::binary) ;

	if(file.good()){
		
		std::vector<unsigned char> mDataBuffer;

		//read all data
		unsigned char ch;
		while(file.get(ch))
			mDataBuffer.push_back(ch);

		//First Tag is in first 14 bits
		format = std::string(&mDataBuffer[0],&mDataBuffer[13]);
		std::cout<<"Format: "<<format<<std::endl;

		//search for ffff04000000, where important info begin
		std::vector<unsigned char>::iterator it;
		it = mDataBuffer.begin();

		std::vector<std::vector<unsigned char>::iterator> positions;
		
		it = std::find(it,mDataBuffer.end(),(unsigned char)255);

		while(it!=mDataBuffer.end()){
	
			positions.push_back(it);
			it = std::find(++it,mDataBuffer.end(),(unsigned char)255);

		}

		//vector with position of ffff04000000 tag
		std::vector<int> vector_posi;

		for(unsigned int i=0;i<positions.size();i++){

			int dist = std::distance(mDataBuffer.begin(), positions[i]);
			if((mDataBuffer[dist+1]==(unsigned char)255)&&(mDataBuffer[dist+2]==(unsigned char)4)&&
				(mDataBuffer[dist+3]==(unsigned char)0)&&(mDataBuffer[dist+4]==(unsigned char)0)&&
				(mDataBuffer[dist+5]==(unsigned char)0)){

					vector_posi.push_back(dist);

			}
		}

		//get manufacturer info
		manufacturer = std::string(&mDataBuffer[vector_posi[2]+42],&mDataBuffer[vector_posi[2]+56]);
		std::cout<<"Manufacturer: "<<manufacturer<<std::endl;
		
		std::vector<int> vector_study;
		int index = vector_posi[3]+6;
		int cont = 0;

		while(cont<4){

			if(((mDataBuffer[index]==(unsigned char)0)&&(mDataBuffer[index+1]==(unsigned char)0)&&
				(mDataBuffer[index+2]==(unsigned char)0))||((mDataBuffer[index]==(unsigned char)0)&&
				(mDataBuffer[index+1]==(unsigned char)16)&&(mDataBuffer[index+2]==(unsigned char)1))){
					
					vector_study.push_back(index);
					cont++;

			}

			index++;
		
		}

		std::vector<int> vector_inst;
		index = vector_posi[4]+6;
		cont = 0;

		while(cont<2){

			if(((mDataBuffer[index]==(unsigned char)0)&&(mDataBuffer[index+1]==(unsigned char)0)&&
				(mDataBuffer[index+2]==(unsigned char)0))||((mDataBuffer[index]==(unsigned char)0)&&
				(mDataBuffer[index+1]==(unsigned char)32)&&(mDataBuffer[index+2]==(unsigned char)1))){
					
					vector_inst.push_back(index);
					cont++;

			}

			index++;
		
		}

		//get patient and insitute data
		patientID = std::string(&mDataBuffer[vector_study[0]+3],&mDataBuffer[vector_study[1]]);
		patientName = std::string(&mDataBuffer[vector_study[2]+3],&mDataBuffer[vector_study[3]]);
		instituteName = std::string(&mDataBuffer[vector_inst[0]+3],&mDataBuffer[vector_inst[1]]);
		gems = std::string(&mDataBuffer[vector_posi[6]+18],&mDataBuffer[vector_posi[6]+35]);

		std::cout<<"Patient ID: "<<patientID<<std::endl;
		std::cout<<"Patient Name: "<<patientName<<std::endl;
		std::cout<<"Institute Name: "<<instituteName<<std::endl;
		std::cout<<"GEMS: "<<gems<<std::endl;

		index = vector_posi[10]+6;
		cont = 0;

		//init codes x,y,z
		while(cont<5){

			//x init code 0 192 1 0 2 0 0 0
			if((mDataBuffer[index]==(unsigned char)0)&&(mDataBuffer[index+1]==(unsigned char)192)&&(mDataBuffer[index+2]==(unsigned char)1)&&
				(mDataBuffer[index+3]==(unsigned char)0)&&(mDataBuffer[index+4]==(unsigned char)2)&&(mDataBuffer[index+5]==(unsigned char)0)&&
				(mDataBuffer[index+6]==(unsigned char)0)&&(mDataBuffer[index+7]==(unsigned char)0)&&(cont==0)){

					unsigned char datos[4];
					datos[0] = mDataBuffer[index+8];
					datos[1] = mDataBuffer[index+9];
					datos[2] = 0;
					datos[3] = 0;

					memcpy(&x,datos,sizeof(int));

					std::cout<<"Size in x: "<<x<<std::endl;

					cont++;

			}

			//y init code 0 192 2 0 2 0 0 0
			if((mDataBuffer[index]==(unsigned char)0)&&(mDataBuffer[index+1]==(unsigned char)192)&&(mDataBuffer[index+2]==(unsigned char)2)&&
				(mDataBuffer[index+3]==(unsigned char)0)&&(mDataBuffer[index+4]==(unsigned char)2)&&(mDataBuffer[index+5]==(unsigned char)0)&&
				(mDataBuffer[index+6]==(unsigned char)0)&&(mDataBuffer[index+7]==(unsigned char)0)&&(cont==1)){

					unsigned char datos[4];
					datos[0] = mDataBuffer[index+8];
					datos[1] = mDataBuffer[index+9];
					datos[2] = 0;
					datos[3] = 0;

					memcpy(&y,datos,sizeof(int));

					std::cout<<"Size in y: "<<y<<std::endl;

					cont++;

			}

			//z init code 0 192 3 0 2 0 0 0 
			if((mDataBuffer[index]==(unsigned char)0)&&(mDataBuffer[index+1]==(unsigned char)192)&&(mDataBuffer[index+2]==(unsigned char)3)&&
				(mDataBuffer[index+3]==(unsigned char)0)&&(mDataBuffer[index+4]==(unsigned char)2)&&(mDataBuffer[index+5]==(unsigned char)0)&&
				(mDataBuffer[index+6]==(unsigned char)0)&&(mDataBuffer[index+7]==(unsigned char)0)&&(cont==2)){

					unsigned char datos[4];
					datos[0] = mDataBuffer[index+8];
					datos[1] = mDataBuffer[index+9];
					datos[2] = 0;
					datos[3] = 0;

					memcpy(&z,datos,sizeof(int));

					std::cout<<"Size in z: "<<z<<std::endl;

					cont++;

			}

			//voxel size init code 0 193 1 0 8 0 0 0
			if((mDataBuffer[index]==(unsigned char)0)&&(mDataBuffer[index+1]==(unsigned char)193)&&(mDataBuffer[index+2]==(unsigned char)1)&&
				(mDataBuffer[index+3]==(unsigned char)0)&&(mDataBuffer[index+4]==(unsigned char)8)&&(mDataBuffer[index+5]==(unsigned char)0)&&
				(mDataBuffer[index+6]==(unsigned char)0)&&(mDataBuffer[index+7]==(unsigned char)0)&&(cont==3)){

					unsigned char datos[8];
					datos[0] = mDataBuffer[index+8];
					datos[1] = mDataBuffer[index+9];
					datos[2] = mDataBuffer[index+10];
					datos[3] = mDataBuffer[index+11];
					datos[4] = mDataBuffer[index+12];
					datos[5] = mDataBuffer[index+13];
					datos[6] = mDataBuffer[index+14];
					datos[7] = mDataBuffer[index+15];

					memcpy(&voxSize,datos,sizeof(double));
					voxSize *= 1000;

					std::cout<<"Voxel size: "<<voxSize<<std::endl;

					cont++;

			}

			//volume size init code 0 208 1 0
			if((mDataBuffer[index]==(unsigned char)0)&&(mDataBuffer[index+1]==(unsigned char)208)&&(mDataBuffer[index+2]==(unsigned char)1)&&
				(mDataBuffer[index+3]==(unsigned char)0)&&(cont==4)){

					unsigned char datos[4];
					datos[0] = mDataBuffer[index+4];
					datos[1] = mDataBuffer[index+5];
					datos[2] = mDataBuffer[index+6];
					datos[3] = mDataBuffer[index+7];

					memcpy(&volSize,datos,sizeof(int));

					std::cout<<"Volume size: "<<volSize<<std::endl<<std::endl;

					cont++;

			}

			index++;
		}

		//extracting volume data
		std::vector<unsigned char>::const_iterator first = mDataBuffer.begin() + index + 7;
		std::vector<unsigned char>::const_iterator last = mDataBuffer.begin() + volSize + index + 7;
		std::vector<unsigned char> vol(first,last);

		//creating the vtk volume
		
		volume = vtkSmartPointer<vtkImageData>::New();
		volume->SetExtent(0,x,0,y,0,z);
		volume->SetSpacing(voxSize,voxSize,voxSize);
		volume->SetOrigin(0,0,0);
		volume->SetScalarTypeToUnsignedChar();
		volume->SetNumberOfScalarComponents(1);
		volume->AllocateScalars();

 
		int pix = 0;
		for(int k=0;k<z;k++){
			for(int j=0;j<y;j++){
				for(int i=0;i<x;i++){

					unsigned char *ptr = static_cast<unsigned char *>(volume->GetScalarPointer(i,j,k));
					*ptr = vol[pix];
					pix++;

				}
			}
		}

	}else
		std::cout<<"Incorrect filename"<<std::endl;
}

vtkSmartPointer<vtkImageData> OpenVol::GetOutput()
{
	return this->volume;
}
