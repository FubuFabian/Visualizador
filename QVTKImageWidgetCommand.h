//
//  OnMouseMovePtr.h
//  US_Probe_Calibration
//
//  Created by Zian Fanti on 07/12/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
#ifndef US_Probe_Calibration_OnMouseMovePtr_h
#define US_Probe_Calibration_OnMouseMovePtr_h

#include <vtkCommand.h>
#include <vtkPropPicker.h>
#include <vtkCornerAnnotation.h>
#include <vtkSmartPointer.h>
#include <vtkImageViewer2.h>

class MainWindow;

//!Interaction with mouse
/*!
  The mouse motion callback, to pick the image and recover pixel values
*/
template<class Widget>
class QVTKImageWidgetCommand : public vtkCommand
{
public:

  typedef Widget WidgetType;

  static QVTKImageWidgetCommand *New()
  {
        return new QVTKImageWidgetCommand;
  }
  
  vtkTypeMacro(QVTKImageWidgetCommand, vtkCommand);

  QVTKImageWidgetCommand();

  ~QVTKImageWidgetCommand();

  void SetPicker(vtkSmartPointer<vtkPropPicker> picker);

  void SetMainWindow(MainWindow* mainwidow);

  void SetImageViewer(vtkSmartPointer<vtkImageViewer2>);

  /**
   *
   */
  void Execute(vtkObject *, unsigned long vtkNotUsed(event), void *);
  

private:

  /** Pointer to the picker */
  vtkSmartPointer<vtkPropPicker> Picker;

  MainWindow* Mainwindow;

  vtkSmartPointer<vtkImageViewer2> Viewer;
  
};

#ifndef VTK_MANUAL_INSTANTIATION
#include "QVTKImageWidgetCommand.cpp"
#endif

#endif