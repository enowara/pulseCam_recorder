//=============================================================================
// Copyright © 2008 Point Grey Research, Inc. All Rights Reserved.
//
// This software is the confidential and proprietary information of Point
// Grey Research, Inc. ("Confidential Information").  You shall not
// disclose such Confidential Information and shall use it only in
// accordance with the terms of the license agreement you entered into
// with PGR.
//
// PGR MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF THE
// SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE, OR NON-INFRINGEMENT. PGR SHALL NOT BE LIABLE FOR ANY DAMAGES
// SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR DISTRIBUTING
// THIS SOFTWARE OR ITS DERIVATIVES.
//=============================================================================
//=============================================================================
// $Id: CustomImageEx.cpp,v 1.20 2010-02-26 23:24:47 soowei Exp $
//=============================================================================

#include "stdafx.h"
#include "stdio.h"

#include "FlyCapture2.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdlib.h>
#include <fstream>
#include <string>

using namespace FlyCapture2;
using namespace std;
void PrintBuildInfo()
{
	FC2Version fc2Version;
	Utilities::GetLibraryVersion( &fc2Version );

	ostringstream version;
	version << "FlyCapture2 library version: " << fc2Version.major << "." << fc2Version.minor << "." << fc2Version.type << "." << fc2Version.build;
	cout << version.str() << endl;

	ostringstream timeStamp;
	timeStamp << "Application build date: " << __DATE__ << " " << __TIME__;
	cout << timeStamp.str() << endl << endl;
}

void PrintCameraInfo( CameraInfo* pCamInfo )
{
	cout << endl;
	cout << "*** CAMERA INFORMATION ***" << endl;
	cout << "Serial number -" << pCamInfo->serialNumber << endl;
	cout << "Camera model - " << pCamInfo->modelName << endl;
	cout << "Camera vendor - " << pCamInfo->vendorName << endl;
	cout << "Sensor - " << pCamInfo->sensorInfo << endl;
	cout << "Resolution - " << pCamInfo->sensorResolution << endl;
	cout << "Firmware build time - " << pCamInfo->firmwareBuildTime << endl << endl;


}

void PrintFormat7Capabilities( Format7Info fmt7Info )
{
	cout << "Max image pixels: (" << fmt7Info.maxWidth << ", " << fmt7Info.maxHeight << ")" << endl;
	cout << "Image Unit size: (" << fmt7Info.imageHStepSize << ", " << fmt7Info.imageVStepSize << ")" << endl;
	cout << "Offset Unit size: (" << fmt7Info.offsetHStepSize << ", " << fmt7Info.offsetVStepSize << ")" << endl;
	cout << "Pixel format bitfield: 0x" << fmt7Info.pixelFormatBitField << endl;

}

void PrintError( Error error )
{
	error.PrintErrorTrace();
}

int main(int argc, char *argv[])
{
	PrintBuildInfo();

	const Mode k_fmt7Mode = MODE_0;
	const PixelFormat k_fmt7PixFmt = PIXEL_FORMAT_RAW8;

	Error error;

	// Since this application saves images in the current folder
	// we must ensure that we have permission to write to this folder.
	// If we do not have permission, fail right away.
	FILE* tempFile = fopen("test.txt", "w+");
	if (tempFile == NULL)
	{
		cout << "Failed to create file in current folder.  Please check permissions." << endl;
		return -1;
	}
	fclose(tempFile);
	remove("test.txt");

	BusManager busMgr;
	unsigned int numCameras;
	error = busMgr.GetNumOfCameras(&numCameras);
	if (error != PGRERROR_OK)
	{
		PrintError( error );
		return -1;
	}

	cout << "Number of cameras detected: " << numCameras << endl;

	if ( numCameras < 1 )
	{
		fprintf(stderr, "Insufficient number of cameras... exiting");
		return -1;
	}
	// command line argument selecting cameraID
	unsigned int i = atoi(argv[1]);
	std::stringstream id;
	id << i;
	std::string filename = "Config"+ id.str() +".txt";

	PGRGuid guid;
	error = busMgr.GetCameraFromIndex(i, &guid);
	if (error != PGRERROR_OK)
	{
		PrintError( error );
		return -1;
	}

	Camera cam;

	// Connect to a camera
	error = cam.Connect(&guid);
	if (error != PGRERROR_OK)
	{
		PrintError( error );
		return -1;
	}

	// Get the camera information
	CameraInfo camInfo;
	error = cam.GetCameraInfo(&camInfo);
	if (error != PGRERROR_OK)
	{
		PrintError( error );
		return -1;
	}

	PrintCameraInfo(&camInfo);

	// Query for available Format 7 modes
	Format7Info fmt7Info;
	bool supported;
	fmt7Info.mode = k_fmt7Mode;
	error = cam.GetFormat7Info( &fmt7Info, &supported );
	if (error != PGRERROR_OK)
	{
		PrintError( error );
		return -1;
	}


	// Read Configuration from Config.txt, which contains information in the order of Gain, Shutter and Frame rate.
	// User defined configuration
	float user_gain; // in dB
	float user_fps; // in fps
	float user_shutter; // in ms
	float user_img_left; // offset
	float user_img_top; // in pixels
	float user_img_width;
	float user_img_height; // in pixels
	
	std::ifstream infile(filename.c_str());
	infile>> user_gain;
	infile>>user_shutter;
	infile>>user_fps;
	infile>>user_img_left;
	infile>>user_img_top;	
	infile>>user_img_width;
	infile>>user_img_height;

	cout<<"The user defined Gain is: "<<user_gain<<" dB"<<endl;
	cout<<"The user defined Shutter is: "<<user_shutter<<" ms"<<endl;
	cout<<"The user defined Frame rate is: "<<user_fps<<" fps"<<endl;
	cout<<"The user defined image left offset is: "<<user_img_left<<" fps"<<endl;
	cout<<"The user defined image top offset is: "<<user_img_top<<" fps"<<endl;
	cout<<"The user defined image width is: "<<user_img_width<<" fps"<<endl;
	cout<<"The user defined inage heigth is: "<<user_img_height<<" fps"<<endl;




	PrintFormat7Capabilities( fmt7Info );

	if ( (k_fmt7PixFmt & fmt7Info.pixelFormatBitField) == 0 )
	{
		// Pixel format not supported!
		cout << "Pixel format is not supported" << endl;
		return -1;
	}

	Format7ImageSettings fmt7ImageSettings;
	fmt7ImageSettings.mode = k_fmt7Mode;
	fmt7ImageSettings.offsetX = user_img_left;
	fmt7ImageSettings.offsetY = user_img_top;
	fmt7ImageSettings.width = user_img_width;//1280;//fmt7Info.maxWidth/2;
	fmt7ImageSettings.height = user_img_height; //1024;//fmt7Info.maxHeight/2;
	fmt7ImageSettings.pixelFormat = k_fmt7PixFmt;

	bool valid;
	Format7PacketInfo fmt7PacketInfo;

	// Validate the settings to make sure that they are valid
	error = cam.ValidateFormat7Settings(
			&fmt7ImageSettings,
			&valid,
			&fmt7PacketInfo );
	if (error != PGRERROR_OK)
	{
		PrintError( error );
		return -1;
	}

	if ( !valid )
	{
		// Settings are not valid
		cout << "Format7 settings are not valid" << endl;
		return -1;
	}

	// Set the settings to the camera
	error = cam.SetFormat7Configuration(
			&fmt7ImageSettings,
			fmt7PacketInfo.recommendedBytesPerPacket );
	if (error != PGRERROR_OK)
	{
		PrintError( error );
		return -1;
	}



	// Set frame rate property
	Property frmRate;
	frmRate.type = FRAME_RATE;
	frmRate.onePush = false;
	frmRate.absControl = true;
	frmRate.onOff = true;
	frmRate.autoManualMode=false;
	frmRate.absValue = user_fps;
	error = cam.SetProperty( &frmRate);
	if (error != PGRERROR_OK)
	{
		PrintError( error );
		return -1;
	}

	// Retrieve frame rate property
	Property frmRate2;
	frmRate2.type = FRAME_RATE;
	error = cam.GetProperty( &frmRate2 );
	if (error != PGRERROR_OK)
	{
		PrintError( error );
		return -1;
	}

	cout << "Frame rate is " << fixed << setprecision(2) << frmRate2.absValue << " fps" << endl;


	// Set gain property
	Property gn;
	gn.type = GAIN;
	gn.onePush = false;
	gn.absControl = true;
	gn.onOff = true;
	gn.autoManualMode=false;
	gn.absValue = user_gain;
	error = cam.SetProperty( &gn);
	if (error != PGRERROR_OK)
	{
		PrintError( error );
		return -1;
	}

	// Retrieve gain property
	Property gain;
	gain.type = GAIN;
	error = cam.GetProperty( &gain );
	if (error != PGRERROR_OK)
	{
		PrintError( error );
		pthread_exit(NULL);
	}
	cout << "Gain is " << fixed << gain.absValue << endl;

	// Set exposure property
	Property shuttr;
	shuttr.type = SHUTTER;
	shuttr.onePush = false;
	shuttr.absControl = true;
	shuttr.onOff = true;
	shuttr.autoManualMode=false;
	shuttr.absValue = user_shutter;
	error = cam.SetProperty( &shuttr);
	if (error != PGRERROR_OK)
	{
		PrintError( error );
		return -1;
	}

	//Retrieve exposure property
	Property shtr;
	shtr.type = SHUTTER;
	error = cam.GetProperty( &shtr );
	if (error != PGRERROR_OK)
	{
		PrintError( error );
		pthread_exit(NULL);
	}
	cout << "Exposure is " << fixed << shtr.absValue << endl;
	
}
