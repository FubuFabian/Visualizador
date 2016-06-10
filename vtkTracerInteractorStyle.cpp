#ifndef VTKTRACERINTERACTORSTYLE_CPP
#define	VTKTRACERINTERACTORSTYLE_CPP


#include "vtkTracerInteractorStyle.h"
#include "mainwindow.h"

#include <vtkObjectFactory.h>
#include <vtkProperty.h>
#include <vtkCommand.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSelectEnclosedPoints.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkImageStencilToImage.h>
#include <vtkBMPWriter.h>


template<class Widget> 
vtkTracerInteractorStyle<Widget>::vtkTracerInteractorStyle()
{
    tracer = vtkSmartPointer<vtkImageTracerWidget>::New();
    tracer->GetLineProperty()->SetLineWidth(1);
    tracer->AddObserver(vtkCommand::EndInteractionEvent, this, &vtkTracerInteractorStyle::catchEvent);
}

template<class Widget> 
void vtkTracerInteractorStyle<Widget>::initTracer(vtkSmartPointer<vtkImageActor> imageActor)
{
    
    image = imageActor->GetInput();
    
	tracer->SetInput(image);
    tracer->SetInteractor(this->Interactor);
    tracer->SetViewProp(imageActor);
    
    tracer->ProjectToPlaneOn();
    tracer->SnapToImageOn();
    
    tracer->On();
    
    
}

template<class Widget> 
void vtkTracerInteractorStyle<Widget>::setAutoCloseOn()
{
    tracer->SetCaptureRadius(100);
    tracer->AutoCloseOn();
}

template<class Widget> 
void vtkTracerInteractorStyle<Widget>::setAutoCloseOff()
{
    tracer->AutoCloseOff();
}

template<class Widget> 
void vtkTracerInteractorStyle<Widget>::catchEvent(vtkObject* caller, long unsigned int eventId, void* callData)
{

  vtkImageTracerWidget* tracerEvent =
    static_cast<vtkImageTracerWidget*>(caller);

  vtkSmartPointer<vtkPolyData> path = vtkSmartPointer<vtkPolyData>::New();
  tracerEvent->GetPath(path);

  this->mainwindow->setSegmentedPath(path);
}

template<class Widget> 
void vtkTracerInteractorStyle<Widget>::clearTracer()
{
  vtkSmartPointer<vtkPoints> emptyPoints = vtkSmartPointer<vtkPoints>::New();
  emptyPoints->InsertNextPoint(0, 0, 0);
  emptyPoints->InsertNextPoint(0, 0, 0);

  tracer->InitializeHandles(emptyPoints);
  tracer->Modified();
  tracer->Off();
}

template<class Widget> 
void vtkTracerInteractorStyle<Widget>::setCallerWidget(WidgetType* callerWidget)
{
  this->caller = callerWidget;
}

template<class Widget> 
void vtkTracerInteractorStyle<Widget>::setMainWindow(MainWindow* mainwindow)
{
  this->mainwindow = mainwindow;
}


#endif