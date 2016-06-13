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

void OpenVol::setFilename(QString filename)
{
	this->filename = filename;
}

void OpenVol::update()
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
		int i = vector_posi[3]+6;
		int x = 0;

		while(x<4){

			if(((mDataBuffer[i]==(unsigned char)0)&&(mDataBuffer[i+1]==(unsigned char)0)&&
				(mDataBuffer[i+2]==(unsigned char)0))||((mDataBuffer[i]==(unsigned char)0)&&
				(mDataBuffer[i+1]==(unsigned char)16)&&(mDataBuffer[i+2]==(unsigned char)1))){
					
					vector_study.push_back(i);
					x++;

			}

			i++;
		
		}

		std::vector<int> vector_inst;
		int j = vector_posi[4]+6;
		int x1 = 0;

		while(x1<2){

			if(((mDataBuffer[j]==(unsigned char)0)&&(mDataBuffer[j+1]==(unsigned char)0)&&
				(mDataBuffer[j+2]==(unsigned char)0))||((mDataBuffer[j]==(unsigned char)0)&&
				(mDataBuffer[j+1]==(unsigned char)32)&&(mDataBuffer[j+2]==(unsigned char)1))){
					
					vector_inst.push_back(j);
					x1++;

			}

			j++;
		
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







	}else
		std::cout<<"Incorrect filename"<<std::endl;


}