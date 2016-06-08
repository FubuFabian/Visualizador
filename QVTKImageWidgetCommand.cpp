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
      
  // if the mouse is moving inside the viewer
  if (event == vtkCommand::LeftButtonPressEvent)
    {
                          
      int* windowPosition = interactor->GetEventPosition();

      int xWindow = windowPosition[0];
      int yWindow = windowPosition[1];
      int zWindow = windowPosition[2];

      this->Picker->Pick(xWindow, yWindow, zWindow, Viewer->GetRenderer());

      double* imPos = this->Picker->GetPickPosition();

	  Mainwindow->setNewRotationCenter(imPos[0],imPos[1]);

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
