#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include <QVTKWidget.h>

#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkPiecewiseFunction.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkMatrix4x4.h>
#include <vtkImageReslice.h>
#include <vtkImageViewer2.h>
#include <vtkImageActor.h>
#include <vtkInteractorStyleImage.h>
#include <vtkTransform.h>


namespace Ui {
class MainWindow;
}

//!Main window for user interaction
/*!
  This class has the main interaction function to load and visualize volume images.
*/

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    Ui::MainWindow *ui;

	/**
	Widget that display the vol
	**/
	QVTKWidget *volWidget;

	/**
	Widget that display the sagital slice
	**/
	QVTKWidget *sagitalWidget;

	/**
	Widget that display the axial slice
	**/
	QVTKWidget *axialWidget;

	/**
	Widget that display the coronal slice
	**/
	QVTKWidget *coronalWidget;

	/**  
	The filename of the selected volume
	**/
	QString volumeFilename;

	/** 
	The displayed volume data
	**/
    vtkSmartPointer<vtkVolume> volume;

	/** 
	The displayed volume data
	**/
    vtkSmartPointer<vtkImageData> volumeData;

	/** 
	The displayed volume opacity
	**/
    vtkSmartPointer<vtkPiecewiseFunction> volumeScalarOpacity;

    /** 
	The displayed volume properties
	**/
    vtkSmartPointer<vtkVolumeProperty> volumeProperty;

	/** 
	The VTK render window
	**/
    vtkSmartPointer<vtkRenderWindow> renwin;

    /** 
	The VTK renderer 
	**/
    vtkSmartPointer<vtkRenderer> renderer;

	/**
	Contains the voxel size of the volumeData
	**/
	double spacing[3];

	/**
	Contains the origin of the volumeData
	**/
	double origin[3];
        
    /**
	Contains the dimensions of the volumeData
	**/
    int dimensions[3];

	/**
	The center slices in each view 
	**/ 
	double centerSlice[3];

	/**
	Position of center
	**/
	double positionCenter[3];

	/**
	Data for the coronal plane
	**/
	static const double coronalElements[16];

	/**
	Data for the sagittal plane
	**/
	static const double sagitalElements[16];

	/**
	Matrix containing the  translation and rotation of the slice sagital plane
	**/
	vtkSmartPointer<vtkMatrix4x4> resliceAxesSagital;

	/**
	Matrix containing the  translation and rotation of the slice sagital plane
	**/
	vtkSmartPointer<vtkTransform> transformSagital;

	/**
	Reslicer that slice the volume and gives the sliced sagital image as an output
	**/
	vtkSmartPointer<vtkImageReslice> reslicerSagital;

	/**
	Image Viewer that render the sliced sagital image
	**/
	vtkSmartPointer<vtkImageViewer2> viewerSagital;

	/**
	 number of sagital slice
	**/
	int sliceSagital;

	/**
	 Angle X of sagital slice
	**/
	int angleXSagital;

	/**
	 Angle Y of sagital slice
	**/
	int angleYSagital;

	/**
	 Angle Z of sagital slice
	**/
	int angleZSagital;
	
	/**
	 sliced sagital image
	**/
	vtkSmartPointer<vtkImageData> sliceImageSagital;
	
	/**
	ImageActor of the sliced sagital image
	**/
	vtkSmartPointer<vtkImageActor> imageActorSagital;

	/**
	Data for the axial plane
	**/
	static const double axialElements[16];

	/**
	Matrix containing the  translation and rotation of the slice axial plane
	**/
	vtkSmartPointer<vtkMatrix4x4> resliceAxesAxial;

	/**
	Matrix containing the  translation and rotation of the slice axial plane
	**/
	vtkSmartPointer<vtkTransform> transformAxial;

	/**
	Reslicer that slice the volume and gives the sliced axial image as an output
	**/
	vtkSmartPointer<vtkImageReslice> reslicerAxial;

	/**
	Image Viewer that render the sliced axial image
	**/
	vtkSmartPointer<vtkImageViewer2> viewerAxial;

	/**
	number of axial slice
	**/
	int sliceAxial;

	/**
	Angle X of axial slice
	**/
	int angleXAxial;

	/**
	Angle Y of axial slice
	**/
	int angleYAxial;

	/**
	Angle Z of axial slice
	**/
	int angleZAxial;
	
	/**
	sliced axial image
	**/
	vtkSmartPointer<vtkImageData> sliceImageAxial;
	
	/**
	ImageActor of the sliced axial image
	**/
	vtkSmartPointer<vtkImageActor> imageActorAxial;

	/**
	Interactor style of the viewer, it has the basic interaction with a 2D image
	**/
	vtkSmartPointer<vtkInteractorStyleImage> imageStyle;

	/**
	Display the volume in volWidget
	**/
	void displayVol();

	/**
	Set the data for slicers
	**/
	void setSlicesData();
	
	/**
	Set the data for volume rendering
	**/
	void setRenderingData();

	/**
	Configuration of sagital view
	**/
	void configSagitalView();

	/**
	Display the slice sagital image
	**/
	void displaySagital();

	/**
	Configuration of axial view
	**/
	void configAxialView();

	/**
	Display the slice axial image
	**/
	void displayAxial();


private slots:

    /**
    Set a volume filename and open in format MHD
    **/
    void openMHD();

	/**
    Select sagital View
    **/
	void sagitalBtnClicked(bool);

	/**
    Select axial view
    **/
	void axialBtnClicked(bool);

	/**
    Set slice in selected view
    **/
	void reslice(int);

	/**
    Rotate slice through x in selected view 
    **/
	void rotateX(int);

	/**
    Rotate slice through y in selected view 
    **/
	void rotateY(int);

	/**
    Rotate slice through z in selected view 
    **/
	void rotateZ(int);
};

#endif // MAINWINDOW_H
