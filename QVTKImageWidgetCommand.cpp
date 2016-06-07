 //
//  OnMouseMovePtr.cpp
//  US_Probe_Calibration
//
//  Created by Zian Fanti on 07/12/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//
#ifndef QVTKIMAGEWIDGETCOMMAND_CPP
#define	QVTKIMAGEWIDGETCOMMAND_CPP

#include "QVTKImageWidgetCommand.h"
#include "mainwindow.h"

#include <vtkImageData.h>
#include <vtkInteractorStyleImage.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

#include <vtkVariant.h>
#include <vtkMath.h>
#include <vtkCommand.h>

#include<sstream>

template<class Widget>
QVTKImageWidgetCommand<Widget>::QVTKImageWidgetCommand() 
{ 

}

template<class Widget>
QVTKImageWidgetCommand<Widget>::~QVTKImageWidgetCommand()
{
  this->Picker = NULL;
}

template<class Widget>
void QVTKImageWidgetCommand<Widget>::SetPicker(vtkSmartPointer<vtkPropPicker> picker)
{
  this->Picker = picker;
}

template<class Widget>
void QVTKImageWidgetCommand<Widget>::Execute(vtkObject* caller, unsigned long event, void *vtkNotUsed(callData))
{
  vtkSmartPointer<vtkRenderWindowInteractor> interactor = Viewer->GetRenderWindow()->GetInteractor();
  
  // Get a shortcut to the pixel data.
  vtkSmartPointer<vtkImageData> imageData = Viewer->GetInput();
      
  // if the mouse is moving inside the viewer
  if (event == vtkCommand::MouseMoveEvent)
    {
            
      int* extent = imageData->GetExtent();
      int* dimension = imageData->GetDimensions();   
      double* spacing = imageData->GetSpacing();
                          
      int* windowPosition = interactor->GetEventPosition();

      int xWindow = windowPosition[0];
      int yWindow = windowPosition[1];
      int zWindow = windowPosition[2];

      this->Picker->Pick(xWindow, yWindow, zWindow, Viewer->GetRenderer());

      double* imPos = this->Picker->GetPickPosition();

	  int xImagePosition = vtkMath::Round(imPos[0]/spacing[0]);
      int yImagePosition = vtkMath::Round(imPos[1]/spacing[1]);
      int zImagePosition = 0;
      
      //int xImagePosition;
      //int yImagePosition;
      //int zImagePosition;
          
      //xImagePosition = vtkMath::Round(imPos[0]);
      //yImagePosition = vtkMath::Round(imPos[1]);
      //zImagePosition = Viewer->GetSlice();
           
      //int center[2];
      //center[0] = floor((float)dimension[0]/2);
      //center[1] = floor((float)dimension[1]/2);
      
      //int xClipPosition = (xImagePosition - extent[0]);
      //int yClipPosition = (dimension[1]-1) - (yImagePosition - extent[2]);
      //int zClipPosition = zImagePosition - extent[4];

	  Mainwindow->setNewRotationCenter(xImagePosition,yImagePosition);

      vtkInteractorStyle *style = vtkInteractorStyle::SafeDownCast(interactor->GetInteractorStyle());
      style->OnMouseMove();
    }

}

template<class Widget>
void QVTKImageWidgetCommand<Widget>::SetMainWindow(MainWindow *mainwindow)
{
	this->Mainwindow = mainwindow;
}

template<class Widget>
void QVTKImageWidgetCommand<Widget>::SetImageViewer(vtkSmartPointer<vtkImageViewer2> viewer)
{
	this->Viewer = viewer;
}

#endif
