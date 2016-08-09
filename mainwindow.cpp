#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "OpenVol.h"

#include <QVBoxLayout>

#include <vtkMetaImageReader.h>
#include <vtkVolumeRayCastMapper.h>
#include <vtkVolumeRayCastCompositeFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkCamera.h>
#include <vtkProperty.h>
#include <vtkImageFlip.h>

#include <vtkTransformPolyDataFilter.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkImageStencilData.h>
#include <vtkImageStencilToImage.h>
#include <vtkImageMathematics.h>
#include <vtkImageBlend.h>

#include <vtkMetaImageWriter.h>

#include <vnl/vnl_matrix.h>
#include <vnl/vnl_vector.h>

const double MainWindow::axialElements[16] = {    
1, 0, 0, 0,
0, 1, 0, 0,
0, 0, 1, 0,
0, 0, 0, 1       
};

const double MainWindow::sagitalElements[16] = {  
0, 0, 1, 0,
1, 0, 0, 0,
0, 1, 0, 0,
0, 0, 0, 1               
};

const double MainWindow::coronalElements[16] = { 
1, 0, 0, 0,
0, 0, 1, 0,
0, 1, 0, 0,
0, 0, 0, 1        
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

	//initializing volume display widget
	volWidget = new QVTKWidget;
	QVBoxLayout *volLayout = new QVBoxLayout;
	volLayout->setContentsMargins(0, 0, 0, 0);
	volLayout->setSpacing(0);
	volLayout->QLayout::addWidget(volWidget);
	ui->volView->setLayout(volLayout);

	//initializing sagital display widget
	sagitalWidget = new QVTKWidget;
	QVBoxLayout *sagitalLayout = new QVBoxLayout;
	sagitalLayout->setContentsMargins(0, 0, 0, 0);
	sagitalLayout->setSpacing(0);
	sagitalLayout->QLayout::addWidget(sagitalWidget);
	ui->sagitalView->setLayout(sagitalLayout);

	//initializing axial display widget
	axialWidget = new QVTKWidget;
	QVBoxLayout *axialLayout = new QVBoxLayout;
	axialLayout->setContentsMargins(0, 0, 0, 0);
	axialLayout->setSpacing(0);
	axialLayout->QLayout::addWidget(axialWidget);
	ui->axialView->setLayout(axialLayout);

	//initializing coronal display widget
	coronalWidget = new QVTKWidget;
	QVBoxLayout *coronalLayout = new QVBoxLayout;
	coronalLayout->setContentsMargins(0, 0, 0, 0);
	coronalLayout->setSpacing(0);
	coronalLayout->QLayout::addWidget(coronalWidget);
	ui->coronalView->setLayout(coronalLayout);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setNewRotationCenter(double x, double y)
{
	if(x==0&&y==0){

		//click outside the image
		std::cout<<"Please select a point inside the image"<<std::endl;
		return;

	}

	if(ui->sagitalViewBtn->isChecked()){

		//change rotation center in selected view
		rotCenterSagital[0] = x;
		rotCenterSagital[1] = y;

		//change rotation center reference in selected view
		sagitalCenterRef->SetCenter(rotCenterSagital[0],rotCenterSagital[1],0);
		sagitalCenterRef->Update();
		sagitalCenterRefMapper->Update();

		std::cout<<"New rotation center: "<<rotCenterSagital[0]<<","<<rotCenterSagital[1]<<std::endl;

		//displaying new rotation center reference in selected view
		displaySagital();

	}else if(ui->axialViewBtn->isChecked()){

		//change rotation center in selected view
		rotCenterAxial[0] = x;
		rotCenterAxial[1] = y;

		//change rotation center reference in selected view
		axialCenterRef->SetCenter(rotCenterAxial[0],rotCenterAxial[1],0);
		axialCenterRef->Update();
		axialCenterRefMapper->Update();

		std::cout<<"New rotation center: "<<rotCenterAxial[0]<<","<<rotCenterAxial[1]<<std::endl;

		//displaying new rotation center reference in selected view
		displayAxial();

	}else if(ui->coronalViewBtn->isChecked()){

		//change rotation center in selected view
		rotCenterCoronal[0] = x;
		rotCenterCoronal[1] = y;

		//change rotation center reference in selected view
		coronalCenterRef->SetCenter(rotCenterCoronal[0],rotCenterCoronal[1],0);
		coronalCenterRef->Update();
		coronalCenterRefMapper->Update();

		std::cout<<"New rotation center: "<<rotCenterCoronal[0]<<","<<rotCenterCoronal[1]<<std::endl;

		//displaying new rotation center reference in selected view
		displayCoronal();

	}

}

void MainWindow::setSegmentedPath(vtkSmartPointer<vtkPolyData> anotation)
{
	if(!segmented){
		segmented = true;
		ui->saveSegBtn->setEnabled(true);

		//initializing segmented image
		segmentedImage = vtkSmartPointer<vtkImageData>::New();
		segmentedImage->SetExtent(volumeData->GetExtent());
		segmentedImage->SetSpacing(volumeData->GetSpacing());
		segmentedImage->SetOrigin(volumeData->GetOrigin());
		segmentedImage->SetScalarTypeToUnsignedChar();
		segmentedImage->SetNumberOfScalarComponents(1);
		segmentedImage->AllocateScalars();

		unsigned char * segmentedImageVoxel;

		for(int i=0; i<dimensions[0]; i++){
			for(int j=0; j<dimensions[1]; j++){
				for(int k=0; k<dimensions[2]; k++){
                    // get pointer to the current volume voxel   
                    segmentedImageVoxel = static_cast<unsigned char *> (segmentedImage->GetScalarPointer(i,j,k));                                     
                    segmentedImageVoxel[0] = 0;                                    
                }
			}
        }
  	}

	vtkSmartPointer<vtkImageData> segmentedSlice;
	vtkSmartPointer<vtkTransform> transform;
	vtkSmartPointer<vtkPolyDataToImageStencil> dataToStencil = vtkSmartPointer<vtkPolyDataToImageStencil>::New();
	vtkSmartPointer<vtkImageStencilToImage> stencilToImageFilter = vtkSmartPointer<vtkImageStencilToImage>::New();

	if(ui->sagitalViewBtn->isChecked()){	
		segmentedSlice = reslicerSagital->GetOutput();
		transform = transformSagital;
	}else if(ui->axialViewBtn->isChecked()){
		segmentedSlice = reslicerAxial->GetOutput();
		transform = transformAxial;
	}else if(ui->coronalViewBtn->isChecked()){
		segmentedSlice = reslicerCoronal->GetOutput();
		transform = transformCoronal;
	}

	dataToStencil->SetInput(anotation);
	dataToStencil->SetOutputSpacing(segmentedSlice->GetSpacing());
	dataToStencil->SetOutputOrigin(volumeData->GetOrigin());
	dataToStencil->SetOutputWholeExtent(segmentedSlice->GetWholeExtent());
	dataToStencil->Update();

	stencilToImageFilter->SetInput(dataToStencil->GetOutput());
	stencilToImageFilter->SetOutsideValue(0);
	stencilToImageFilter->SetInsideValue(255);
	stencilToImageFilter->SetOutputScalarTypeToUnsignedChar();
	stencilToImageFilter->Update();

	vtkSmartPointer<vtkImageData> stencilImage = stencilToImageFilter->GetOutput();

	int *imageSize = stencilImage->GetDimensions();
	double *spacingImage = stencilImage->GetSpacing();

	for(int x = 0; x<imageSize[0]; x++){
            for(int y = 0; y<imageSize[1]; y++){

				unsigned char * imagePixel = static_cast<unsigned char *> (stencilImage->GetScalarPointer(x,y,0));

				if(imagePixel[0]==255){
					
					float tempPoint[3];

					if(ui->sagitalViewBtn->isChecked()){	
						tempPoint[0] = 0;
						tempPoint[1] = spacingImage[1]*x;
						tempPoint[2] = spacingImage[0]*y; 
					}else if(ui->axialViewBtn->isChecked()){
						tempPoint[0] = spacingImage[0]*x;
						tempPoint[1] = spacingImage[1]*y;
						tempPoint[2] = 0;
					}else if(ui->coronalViewBtn->isChecked()){
						tempPoint[0] = spacingImage[0]*x;
						tempPoint[1] = 0;
						tempPoint[2] = spacingImage[1]*y;
					}

					const float point[3] = {tempPoint[0],tempPoint[1],tempPoint[2]};

					float transformedPoint[3];
					transform->TransformPoint(point,transformedPoint);
					
					int voxel[3];
					voxel[0] = vtkMath::Floor((transformedPoint[0]/spacing[0]) + 0.5);
					voxel[1] = vtkMath::Floor((transformedPoint[1]/spacing[1]) + 0.5);
					voxel[2] = vtkMath::Floor((transformedPoint[2]/spacing[2]) + 0.5);

					if( (-1<voxel[0]&&voxel[0]<dimensions[0])&&(-1<voxel[1]&&voxel[1]<dimensions[1])&&(-1<voxel[2]&&voxel[2]<dimensions[2]) ){
						unsigned char * volumeVoxel = static_cast<unsigned char *> (segmentedImage->GetScalarPointer(voxel[0],voxel[1],voxel[2]));
						volumeVoxel[0] = 255;
					}
				}
			}
	}

	vtkSmartPointer<vtkMetaImageWriter> writer = vtkSmartPointer<vtkMetaImageWriter>::New();
	writer->SetFileName("C:/Users/Fabian/Desktop/slice.mhd");
	writer->SetRAWFileName("C:/Users/Fabian/Desktop/slice.raw");
	writer->SetInput(segmentedImage);
	writer->Write();
	
}

void MainWindow::displayVol()
{
	volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();

	//setting volume rendering methods and properties
    vtkSmartPointer<vtkVolumeRayCastCompositeFunction> rayCastFunction = vtkSmartPointer<vtkVolumeRayCastCompositeFunction>::New();

    volumeScalarOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
    volumeScalarOpacity->AddPoint(0,1.00);
    volumeScalarOpacity->AddPoint(255,1.00);
    volumeScalarOpacity->Update();
    volumeProperty->SetScalarOpacity(volumeScalarOpacity);

    vtkSmartPointer<vtkColorTransferFunction> volumeColor = vtkSmartPointer<vtkColorTransferFunction>::New();
    volumeColor->AddRGBPoint(0,0.0,0.0,0.0);
    volumeColor->AddRGBPoint(64,0.25,0.25,0.25);
    volumeColor->AddRGBPoint(128,0.5,0.5,0.5);
    volumeColor->AddRGBPoint(192,0.75,0.75,0.75);
    volumeColor->AddRGBPoint(255,1.0,1.0,1.0);
    volumeProperty->SetColor(volumeColor);

    vtkSmartPointer<vtkVolumeRayCastCompositeFunction> compositeFunction = vtkSmartPointer<vtkVolumeRayCastCompositeFunction>::New();

    vtkSmartPointer<vtkVolumeRayCastMapper> volumeMapper = vtkSmartPointer<vtkVolumeRayCastMapper>::New();
    volumeMapper->SetVolumeRayCastFunction(compositeFunction);
    volumeMapper->CroppingOff();
    volumeMapper->SetInput(volumeData);

	//setting volume rendering data
    volume = vtkSmartPointer<vtkVolume>::New();
    volume->SetMapper(volumeMapper);
    volume->SetOrigin(0,0,0);
    volume->SetProperty(volumeProperty);
    volume->Update();

	//setting 3D scene properties
	renderer = vtkSmartPointer<vtkRenderer>::New();
    renderer->SetBackground(183.0/255.0,197.0/255.0,253.0/255.0);
    renderer->AddVolume(volume);
    renderer->GetActiveCamera()->SetFocalPoint(volume->GetCenter());
    renderer->GetActiveCamera()->Roll(90);
    
    double cameraPos[3];
    cameraPos[0] = volume->GetCenter()[0] + volume->GetMaxXBound() + 30;
    cameraPos[1] = volume->GetCenter()[1];
    cameraPos[2] = volume->GetCenter()[2];
    
    renderer->GetActiveCamera()->SetPosition(cameraPos);

	//displaying volume
    renwin = vtkSmartPointer<vtkRenderWindow>::New();
    renwin->AddRenderer(renderer);
    
    volWidget->SetRenderWindow(renwin);
    std::cout<<std::endl;
    std::cout<<"Displaying volume"<<std::endl<<std::endl;
    renwin->Render();
}

void MainWindow::setRenderingData()
{
	//initializing sagital 2D view
	imageStyleSagital = vtkSmartPointer<vtkInteractorStyleImage>::New();
	viewerSagital = vtkSmartPointer<vtkImageViewer2>::New();
	imageActorSagital = vtkSmartPointer<vtkImageActor>::New();
    sagitalWidget->SetRenderWindow(viewerSagital->GetRenderWindow());
	imageActorSagital = viewerSagital->GetImageActor();
	imageActorSagital->InterpolateOff();
	viewerSagital->GetRenderWindow()->GetInteractor()->SetInteractorStyle(imageStyleSagital);

	//initializing axial 2D view
	imageStyleAxial = vtkSmartPointer<vtkInteractorStyleImage>::New();
	viewerAxial = vtkSmartPointer<vtkImageViewer2>::New();
	imageActorAxial = vtkSmartPointer<vtkImageActor>::New();
    axialWidget->SetRenderWindow(viewerAxial->GetRenderWindow());
	imageActorAxial = viewerAxial->GetImageActor();
	imageActorAxial->InterpolateOff();
	viewerAxial->GetRenderWindow()->GetInteractor()->SetInteractorStyle(imageStyleAxial);

	//initializing coronal 2D view
	imageStyleCoronal = vtkSmartPointer<vtkInteractorStyleImage>::New();
	viewerCoronal = vtkSmartPointer<vtkImageViewer2>::New();
	imageActorCoronal = vtkSmartPointer<vtkImageActor>::New();
    coronalWidget->SetRenderWindow(viewerCoronal->GetRenderWindow());
	imageActorCoronal = viewerCoronal->GetImageActor();
	imageActorCoronal->InterpolateOff();
	viewerCoronal->GetRenderWindow()->GetInteractor()->SetInteractorStyle(imageStyleCoronal);

	segmented = false;
}

void MainWindow::setSlicesData()
{
	std::cout<<"Displaying Slices"<<std::endl;

	//getting volume properties
	volumeData->GetSpacing(spacing);
	volumeData->GetOrigin(origin);
    volumeData->GetDimensions(dimensions);

	//setting center of volume 
	centerSlice[0] = floor(dimensions[0]*0.5)-1;
	centerSlice[1] = floor(dimensions[1]*0.5)-1;
	centerSlice[2] = floor(dimensions[2]*0.5)-1;

	positionCenter[0] = origin[0] + spacing[0] * centerSlice[0];
	positionCenter[1] = origin[1] + spacing[1] * centerSlice[1]; 
    positionCenter[2] = origin[2] + spacing[2] * centerSlice[2];

	//configureation of 2D views
	configSagitalView();
	configAxialView();
	configCoronalView();

	//setting initial values for sliders
	ui->rotXSld->setRange(-90,90);
	ui->rotXSld->setTickInterval(1);
	ui->rotXSld->setValue(0);

	ui->rotYSld->setRange(-90,90);
	ui->rotYSld->setTickInterval(1);
	ui->rotYSld->setValue(0);

	ui->rotZSld->setRange(-90,90);
	ui->rotZSld->setTickInterval(1);
	ui->rotZSld->setValue(0);

	ui->sliceSld->setRange(0,dimensions[2]-1);
	ui->sliceSld->setTickInterval(1);
	ui->sliceSld->setValue(centerSlice[2]);

	addActorsAxialView();

	//displaying 2D views
	displaySagital();
	displayCoronal();
}

void MainWindow::configSagitalView()
{
	//setting initial variable values
	sliceSagital = centerSlice[0];
	angleXSagital = 0;
	angleYSagital = 0;
	angleZSagital = 0; 
	
	//initializing rotation center
	rotCenterSagital[0] = positionCenter[1]-origin[1];
	rotCenterSagital[1] = positionCenter[2]-origin[2];

	//initializing slicer data
	resliceAxesSagital = vtkSmartPointer<vtkMatrix4x4>::New();
	reslicerSagital = vtkSmartPointer<vtkImageReslice>::New();
	transformSagital = vtkSmartPointer<vtkTransform>::New();

	transformSagital->PostMultiply();
	transformSagital->Translate(positionCenter[0],origin[1],origin[2]);

	resliceAxesSagital->DeepCopy(sagitalElements);

	reslicerSagital->SetInput(volumeData);
	reslicerSagital->SetOutputDimensionality(2);
	reslicerSagital->SetResliceAxes(resliceAxesSagital);
	reslicerSagital->SetResliceTransform(transformSagital);
	reslicerSagital->SetInterpolationModeToLinear();
	
	//setting visual references for axial and coronal slices
	sagitalRefInAxialView = vtkSmartPointer<vtkLineSource>::New();
	sagitalRefInAxialViewMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	sagitalRefInAxialViewActor = vtkSmartPointer<vtkActor>::New();

	double p0Axial[3] = {sliceSagital*spacing[0],dimensions[1]*spacing[1],0.0};
	double p1Axial[3] = {sliceSagital*spacing[0],0.0,0.0};
	
	sagitalRefInAxialView->SetPoint1(p0Axial);
	sagitalRefInAxialView->SetPoint2(p1Axial);
	sagitalRefInAxialView->Update();
	sagitalRefInAxialViewMapper->SetInputConnection(sagitalRefInAxialView->GetOutputPort());
	sagitalRefInAxialViewActor->SetMapper(sagitalRefInAxialViewMapper);
	sagitalRefInAxialViewActor->GetProperty()->SetLineWidth(1);
	sagitalRefInAxialViewActor->GetProperty()->SetColor(0,162,232);

	sagitalRefInCoronalView = vtkSmartPointer<vtkLineSource>::New();
	sagitalRefInCoronalViewMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	sagitalRefInCoronalViewActor = vtkSmartPointer<vtkActor>::New();

	double p0Coronal[3] = {sliceSagital*spacing[0],dimensions[2]*spacing[2],0.0};
	double p1Coronal[3] = {sliceSagital*spacing[0],0.0,0.0};
	
	sagitalRefInCoronalView->SetPoint1(p0Coronal);
	sagitalRefInCoronalView->SetPoint2(p1Coronal);
	sagitalRefInCoronalView->Update();
	sagitalRefInCoronalViewMapper->SetInputConnection(sagitalRefInCoronalView->GetOutputPort());
	sagitalRefInCoronalViewActor->SetMapper(sagitalRefInCoronalViewMapper);
	sagitalRefInCoronalViewActor->GetProperty()->SetLineWidth(1);
	sagitalRefInCoronalViewActor->GetProperty()->SetColor(0,162,232);

	//seting visual reference for rotation center
	sagitalCenterRef = vtkSmartPointer<vtkRegularPolygonSource>::New();
	sagitalCenterRefMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	sagitalCenterRefActor = vtkSmartPointer<vtkActor>::New();
 
	sagitalCenterRef->SetNumberOfSides(50);
	sagitalCenterRef->SetRadius(0.5);
	sagitalCenterRef->SetCenter(rotCenterSagital[0],rotCenterSagital[1],0);

	sagitalCenterRefMapper->SetInputConnection(sagitalCenterRef->GetOutputPort());;
	sagitalCenterRefActor->SetMapper(sagitalCenterRefMapper);
	sagitalCenterRefActor->GetProperty()->SetColor(1.0,0.59,0.08);
}

void MainWindow::configAxialView()
{
	//setting initial variable values
	sliceAxial = centerSlice[2];
	angleXAxial = 0;
	angleYAxial = 0;
	angleZAxial = 0; 

	//initializing rotation center
	rotCenterAxial[0] = positionCenter[0]-origin[0];
	rotCenterAxial[1] = positionCenter[1]-origin[1];

	//initializing slicer data
	resliceAxesAxial = vtkSmartPointer<vtkMatrix4x4>::New();
	reslicerAxial = vtkSmartPointer<vtkImageReslice>::New();
	transformAxial = vtkSmartPointer<vtkTransform>::New();

	transformAxial->PostMultiply();
	transformAxial->Translate(origin[0],origin[1],positionCenter[2]);

	//transformAxial->Translate(-(rotCenterAxial[0]),-(rotCenterAxial[1]),-(sliceAxial*spacing[2]));
	//transformAxial->RotateZ(180);
	//transformAxial->Translate((rotCenterAxial[0]),(rotCenterAxial[1]),(sliceAxial*spacing[2]));

	resliceAxesAxial->DeepCopy(axialElements);

	reslicerAxial->SetInput(volumeData);
	reslicerAxial->SetOutputDimensionality(2);
	reslicerAxial->SetResliceAxes(resliceAxesAxial);
	reslicerAxial->SetResliceTransform(transformAxial);
	reslicerAxial->SetInterpolationModeToLinear();

	//setting visual references for sagital and coronal slices
	axialRefInSagitalView = vtkSmartPointer<vtkLineSource>::New();
	axialRefInSagitalViewMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	axialRefInSagitalViewActor = vtkSmartPointer<vtkActor>::New();

	double p0Sagital[3] = {dimensions[1]*spacing[1],sliceAxial*spacing[2],0.0};
	double p1Sagital[3] = {0.0,sliceAxial*spacing[2],0.0};
	
	axialRefInSagitalView->SetPoint1(p0Sagital);
	axialRefInSagitalView->SetPoint2(p1Sagital);
	axialRefInSagitalView->Update();
	axialRefInSagitalViewMapper->SetInputConnection(axialRefInSagitalView->GetOutputPort());
	axialRefInSagitalViewActor->SetMapper(axialRefInSagitalViewMapper);
	axialRefInSagitalViewActor->GetProperty()->SetLineWidth(1);
	axialRefInSagitalViewActor->GetProperty()->SetColor(237,0,0);

	axialRefInCoronalView = vtkSmartPointer<vtkLineSource>::New();
	axialRefInCoronalViewMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	axialRefInCoronalViewActor = vtkSmartPointer<vtkActor>::New();

	double p0Coronal[3] = {dimensions[0]*spacing[0],sliceAxial*spacing[2],0.0};
	double p1Coronal[3] = {0.0,sliceAxial*spacing[2],0.0};
	
	axialRefInCoronalView->SetPoint1(p0Coronal);
	axialRefInCoronalView->SetPoint2(p1Coronal);
	axialRefInCoronalView->Update();
	axialRefInCoronalViewMapper->SetInputConnection(axialRefInCoronalView->GetOutputPort());
	axialRefInCoronalViewActor->SetMapper(axialRefInCoronalViewMapper);
	axialRefInCoronalViewActor->GetProperty()->SetLineWidth(1);
	axialRefInCoronalViewActor->GetProperty()->SetColor(237,0,0);

	//seting visual reference for rotation center
	axialCenterRef = vtkSmartPointer<vtkRegularPolygonSource>::New();
	axialCenterRefMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	axialCenterRefActor = vtkSmartPointer<vtkActor>::New();
 
	axialCenterRef->SetNumberOfSides(50);
	axialCenterRef->SetRadius(0.5);
	axialCenterRef->SetCenter(rotCenterAxial[0],rotCenterAxial[1],0);

	axialCenterRefMapper->SetInputConnection(axialCenterRef->GetOutputPort());;
	axialCenterRefActor->SetMapper(axialCenterRefMapper);
	axialCenterRefActor->GetProperty()->SetColor(1.0,0.59,0.08);
}

void MainWindow::configCoronalView()
{
	//setting initial variable values
	sliceCoronal = centerSlice[1];
	angleXCoronal = 0;
	angleYCoronal = 0;
	angleZCoronal = 0; 

	//initializing rotation center
	rotCenterCoronal[0] = positionCenter[0]-origin[0];
	rotCenterCoronal[1] = positionCenter[2]-origin[2];

	//initializing slicer data
	resliceAxesCoronal = vtkSmartPointer<vtkMatrix4x4>::New();
	reslicerCoronal = vtkSmartPointer<vtkImageReslice>::New();
	transformCoronal = vtkSmartPointer<vtkTransform>::New();

	transformCoronal->PostMultiply();
	transformCoronal->Translate(origin[0],positionCenter[1],origin[2]);

	resliceAxesCoronal->DeepCopy(coronalElements);

	reslicerCoronal->SetInput(volumeData);
	reslicerCoronal->SetOutputDimensionality(2);
	reslicerCoronal->SetResliceAxes(resliceAxesCoronal);
	reslicerCoronal->SetResliceTransform(transformCoronal);
	reslicerCoronal->SetInterpolationModeToLinear();

	//setting visual references for sagital and axial slices
	coronalRefInSagitalView = vtkSmartPointer<vtkLineSource>::New();
	coronalRefInSagitalViewMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	coronalRefInSagitalViewActor = vtkSmartPointer<vtkActor>::New();

	double p0Sagital[3] = {sliceCoronal*spacing[1],dimensions[2]*spacing[2],0.0};
	double p1Sagital[3] = {sliceCoronal*spacing[1],0.0,0.0};
	
	coronalRefInSagitalView->SetPoint1(p0Sagital);
	coronalRefInSagitalView->SetPoint2(p1Sagital);
	coronalRefInSagitalView->Update();
	coronalRefInSagitalViewMapper->SetInputConnection(coronalRefInSagitalView->GetOutputPort());
	coronalRefInSagitalViewActor->SetMapper(coronalRefInSagitalViewMapper);
	coronalRefInSagitalViewActor->GetProperty()->SetLineWidth(1);
	coronalRefInSagitalViewActor->GetProperty()->SetColor(237,128,0);

	coronalRefInAxialView = vtkSmartPointer<vtkLineSource>::New();
	coronalRefInAxialViewMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	coronalRefInAxialViewActor = vtkSmartPointer<vtkActor>::New();

	double p0Axial[3] = {dimensions[0]*spacing[0],sliceCoronal*spacing[1],0.0};
	double p1Axial[3] = {0.0,sliceCoronal*spacing[1],0.0};
	
	coronalRefInAxialView->SetPoint1(p0Axial);
	coronalRefInAxialView->SetPoint2(p1Axial);
	coronalRefInAxialView->Update();
	coronalRefInAxialViewMapper->SetInputConnection(coronalRefInAxialView->GetOutputPort());
	coronalRefInAxialViewActor->SetMapper(coronalRefInAxialViewMapper);
	coronalRefInAxialViewActor->GetProperty()->SetLineWidth(1);
	coronalRefInAxialViewActor->GetProperty()->SetColor(237,128,0);

	//seting visual reference for rotation center
	coronalCenterRef = vtkSmartPointer<vtkRegularPolygonSource>::New();
	coronalCenterRefMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	coronalCenterRefActor = vtkSmartPointer<vtkActor>::New();
 
	coronalCenterRef->SetNumberOfSides(50);
	coronalCenterRef->SetRadius(0.5);
	coronalCenterRef->SetCenter(rotCenterCoronal[0],rotCenterCoronal[1],0);

	coronalCenterRefMapper->SetInputConnection(coronalCenterRef->GetOutputPort());;
	coronalCenterRefActor->SetMapper(coronalCenterRefMapper);
	coronalCenterRefActor->GetProperty()->SetColor(1.0,0.59,0.08);
}

void MainWindow::addActorsSagitalView()
{
	//add center reference actor
	viewerSagital->GetRenderer()->AddActor(sagitalCenterRefActor);

	//remove reference actors from other slices
	viewerSagital->GetRenderer()->RemoveActor(axialRefInSagitalViewActor);
	viewerSagital->GetRenderer()->RemoveActor(coronalRefInSagitalViewActor);

	//add reference to other slices
	if(ui->axialViewBtn->isChecked()){
		viewerAxial->GetRenderer()->RemoveActor(axialCenterRefActor);
		viewerAxial->GetRenderer()->AddActor(sagitalRefInAxialViewActor);
		viewerAxial->GetRenderer()->AddActor(coronalRefInAxialViewActor);
	}else if(ui->coronalViewBtn->isChecked()){
		viewerCoronal->GetRenderer()->RemoveActor(coronalCenterRefActor);
		viewerCoronal->GetRenderer()->AddActor(sagitalRefInCoronalViewActor);
		viewerCoronal->GetRenderer()->AddActor(axialRefInCoronalViewActor);
	}else{
		viewerAxial->GetRenderer()->RemoveActor(axialCenterRefActor);
		viewerAxial->GetRenderer()->AddActor(sagitalRefInAxialViewActor);
		viewerAxial->GetRenderer()->AddActor(coronalRefInAxialViewActor);
		viewerCoronal->GetRenderer()->RemoveActor(coronalCenterRefActor);
		viewerCoronal->GetRenderer()->AddActor(sagitalRefInCoronalViewActor);
		viewerCoronal->GetRenderer()->AddActor(axialRefInCoronalViewActor);
	}
}

void MainWindow::addActorsAxialView()
{
	//add center reference actor
	viewerAxial->GetRenderer()->AddActor(axialCenterRefActor);

	//remove reference actors from other slices
	viewerAxial->GetRenderer()->RemoveActor(sagitalRefInAxialViewActor);
	viewerAxial->GetRenderer()->RemoveActor(coronalRefInAxialViewActor);

	//add reference to other slices
	if(ui->sagitalViewBtn->isChecked()){
		viewerSagital->GetRenderer()->RemoveActor(sagitalCenterRefActor);
		viewerSagital->GetRenderer()->AddActor(axialRefInSagitalViewActor);
		viewerSagital->GetRenderer()->AddActor(coronalRefInSagitalViewActor);
	}else if(ui->coronalViewBtn->isChecked()){
		viewerCoronal->GetRenderer()->RemoveActor(coronalCenterRefActor);
		viewerCoronal->GetRenderer()->AddActor(sagitalRefInCoronalViewActor);
		viewerCoronal->GetRenderer()->AddActor(axialRefInCoronalViewActor);
	}else{
		viewerSagital->GetRenderer()->RemoveActor(sagitalCenterRefActor);
		viewerSagital->GetRenderer()->AddActor(axialRefInSagitalViewActor);
		viewerSagital->GetRenderer()->AddActor(coronalRefInSagitalViewActor);
		viewerCoronal->GetRenderer()->RemoveActor(coronalCenterRefActor);
		viewerCoronal->GetRenderer()->AddActor(axialRefInCoronalViewActor);
		viewerCoronal->GetRenderer()->AddActor(sagitalRefInCoronalViewActor);
	}
}

void MainWindow::addActorsCoronalView()
{
	//add center reference actor
	viewerCoronal->GetRenderer()->AddActor(coronalCenterRefActor);

	//remove reference actors from other slices
	viewerCoronal->GetRenderer()->RemoveActor(sagitalRefInCoronalViewActor);
	viewerCoronal->GetRenderer()->RemoveActor(axialRefInCoronalViewActor);

	//add reference to other slices
	if(ui->axialViewBtn->isChecked()){
		viewerAxial->GetRenderer()->RemoveActor(axialCenterRefActor);
		viewerAxial->GetRenderer()->AddActor(sagitalRefInAxialViewActor);
		viewerAxial->GetRenderer()->AddActor(coronalRefInAxialViewActor);
	}else if(ui->sagitalViewBtn->isChecked()){
		viewerSagital->GetRenderer()->RemoveActor(sagitalCenterRefActor);
		viewerSagital->GetRenderer()->AddActor(axialRefInSagitalViewActor);
		viewerSagital->GetRenderer()->AddActor(coronalRefInSagitalViewActor);
	}else{
		viewerAxial->GetRenderer()->RemoveActor(axialCenterRefActor);
		viewerAxial->GetRenderer()->AddActor(sagitalRefInAxialViewActor);
		viewerAxial->GetRenderer()->AddActor(coronalRefInAxialViewActor);
		viewerSagital->GetRenderer()->RemoveActor(sagitalCenterRefActor);
		viewerSagital->GetRenderer()->AddActor(axialRefInSagitalViewActor);
		viewerSagital->GetRenderer()->AddActor(coronalRefInSagitalViewActor);
	}
}

void MainWindow::removeActorsSagitalView()
{
	//remove actors from all views
	viewerSagital->GetRenderer()->RemoveActor(sagitalCenterRefActor);
	viewerAxial->GetRenderer()->RemoveActor(sagitalRefInAxialViewActor);
	viewerCoronal->GetRenderer()->RemoveActor(sagitalRefInCoronalViewActor);
}

void MainWindow::removeActorsAxialView()
{
	//remove actors from all views
	viewerAxial->GetRenderer()->RemoveActor(axialCenterRefActor);
	viewerSagital->GetRenderer()->RemoveActor(axialRefInSagitalViewActor);
	viewerCoronal->GetRenderer()->RemoveActor(axialRefInCoronalViewActor);
}

void MainWindow::removeActorsCoronalView()
{
	//remove actors from all views
	viewerCoronal->GetRenderer()->RemoveActor(coronalCenterRefActor);
	viewerSagital->GetRenderer()->RemoveActor(coronalRefInSagitalViewActor);
	viewerAxial->GetRenderer()->RemoveActor(coronalRefInAxialViewActor);
}

void MainWindow::displaySagital()
{
	//updating slice image
	reslicerSagital->Update();
	sliceImageSagital = reslicerSagital->GetOutput();
	sliceImageSagital->Update();
	viewerSagital->SetInput(sliceImageSagital);
    viewerSagital->Render();

	//updating visual references
	viewerAxial->Render();
	viewerCoronal->Render();
}

void MainWindow::displayAxial()
{
	//updating slice image
	reslicerAxial->Update();
	sliceImageAxial = reslicerAxial->GetOutput();
	sliceImageAxial->Update();
	viewerAxial->SetInput(sliceImageAxial);
    viewerAxial->Render();

	//updating visual references
	viewerSagital->Render();
	viewerCoronal->Render();
}

void MainWindow::displayCoronal()
{
	//updating slice image
	reslicerCoronal->Update();
	sliceImageCoronal = reslicerCoronal->GetOutput();
	sliceImageCoronal->Update();
	viewerCoronal->SetInput(sliceImageCoronal);
    viewerCoronal->Render();

	//updating visual references
	viewerSagital->Render();
	viewerAxial->Render();
}

void MainWindow::openMHD()
{
	std::cout<<"Loading Volume"<<std::endl;

	//initializing reader
    this->volumeFilename = QFileDialog::getOpenFileName(this, tr("Open Volume .mhd"),
        QDir::currentPath(), tr("Volume Files (*.mhd)"));
    
	vtkSmartPointer<vtkMetaImageReader> reader = vtkSmartPointer<vtkMetaImageReader>::New();
    reader->SetFileName(volumeFilename.toAscii().data());
    reader->Update();

    volumeData = reader->GetOutput();

	//displaying volume
	displayVol();
	setRenderingData();
	setSlicesData();
}

void MainWindow::openVol()
{
	std::cout<<"Loading Volume"<<std::endl;

	//initializing reader
    this->volumeFilename = QFileDialog::getOpenFileName(this, tr("Open Volume .vol"),
        QDir::currentPath(), tr("Volume Files (*.vol)"));
    
	OpenVol * reader = OpenVol::New();
    reader->SetFilename(volumeFilename);
	reader->Update();

	vtkSmartPointer<vtkImageFlip> volFlipY = vtkSmartPointer<vtkImageFlip>::New();
	volFlipY->SetInput(reader->GetOutput());
	volFlipY->SetFilteredAxis(1);
	volFlipY->Update();

	volumeData = volFlipY->GetOutput();

	//displaying volume
	displayVol();
	setRenderingData();
	setSlicesData();
}

void MainWindow::reslice(int slice)
{
	if(ui->sagitalViewBtn->isChecked()){
		
		//change slice
		double translate = slice - sliceSagital;
		transformSagital->Translate(translate*spacing[0],0,0);
		sliceSagital = slice;

		//change visual reference
		double p0Axial[3] = {sliceSagital*spacing[0],dimensions[1]*spacing[1],0.0};
		double p1Axial[3] = {sliceSagital*spacing[0],0.0,0.0};
	
		sagitalRefInAxialView->SetPoint1(p0Axial);
		sagitalRefInAxialView->SetPoint2(p1Axial);
		sagitalRefInAxialView->Update();
		sagitalRefInAxialViewMapper->Update();

		double p0Coronal[3] = {sliceSagital*spacing[0],dimensions[2]*spacing[2],0.0};
		double p1Coronal[3] = {sliceSagital*spacing[0],0.0,0.0};
	
		sagitalRefInCoronalView->SetPoint1(p0Coronal);
		sagitalRefInCoronalView->SetPoint2(p1Coronal);
		sagitalRefInCoronalView->Update();
		sagitalRefInCoronalViewMapper->Update();

		//displaying new slice
		displaySagital();

	}else if(ui->axialViewBtn->isChecked()){

		//change slice
		double translate = slice - sliceAxial;
		transformAxial->Translate(0,0,translate*spacing[2]);
		sliceAxial = slice;

		//change visual reference
		double p0Sagital[3] = {dimensions[1]*spacing[1],sliceAxial*spacing[2],0.0};
		double p1Sagital[3] = {0.0,sliceAxial*spacing[2],0.0};
	
		axialRefInSagitalView->SetPoint1(p0Sagital);
		axialRefInSagitalView->SetPoint2(p1Sagital);
		axialRefInSagitalView->Update();
		axialRefInSagitalViewMapper->Update();

		double p0Coronal[3] = {dimensions[0]*spacing[0],sliceAxial*spacing[2],0.0};
		double p1Coronal[3] = {0.0,sliceAxial*spacing[2],0.0};
	
		axialRefInCoronalView->SetPoint1(p0Coronal);
		axialRefInCoronalView->SetPoint2(p1Coronal);
		axialRefInCoronalView->Update();
		axialRefInCoronalViewMapper->Update();

		//displaying new slice
		displayAxial();

	}else if(ui->coronalViewBtn->isChecked()){

		//change slice
		double translate = slice - sliceCoronal;
		transformCoronal->Translate(0,translate*spacing[1],0);
		sliceCoronal = slice;

		//change visual reference
		double p0Sagital[3] = {sliceCoronal*spacing[1],dimensions[2]*spacing[2],0.0};
		double p1Sagital[3] = {sliceCoronal*spacing[1],0.0,0.0};
	
		coronalRefInSagitalView->SetPoint1(p0Sagital);
		coronalRefInSagitalView->SetPoint2(p1Sagital);
		coronalRefInSagitalView->Update();
		coronalRefInSagitalViewMapper->Update();

		double p0Axial[3] = {dimensions[0]*spacing[0],sliceCoronal*spacing[1],0.0};
		double p1Axial[3] = {0.0,sliceCoronal*spacing[1],0.0};
	
		coronalRefInAxialView->SetPoint1(p0Axial);
		coronalRefInAxialView->SetPoint2(p1Axial);
		coronalRefInAxialView->Update();
		coronalRefInAxialViewMapper->Update();

		//display new slice
		displayCoronal();

	}
}

void MainWindow::rotateX(int angle)
{
	if(ui->sagitalViewBtn->isChecked()){

		//rotate slice
		double rotate = angle - angleXSagital;
		transformSagital->Translate(-(sliceSagital*spacing[0]),-(rotCenterSagital[0]),-(rotCenterSagital[1]));
		transformSagital->RotateX(rotate);
		transformSagital->Translate((sliceSagital*spacing[0]),(rotCenterSagital[0]),(rotCenterSagital[1]));
		angleXSagital = angle;

		//display new slice
		displaySagital();

	}else if(ui->axialViewBtn->isChecked()){

		//rotate slice
		double rotate = angle - angleXAxial;
		transformAxial->Translate(-(rotCenterAxial[0]),-(rotCenterAxial[1]),-(sliceAxial*spacing[2]));
		transformAxial->RotateX(rotate);
		transformAxial->Translate((rotCenterAxial[0]),(rotCenterAxial[1]),(sliceAxial*spacing[2]));
		angleXAxial = angle;

		//display new slice
		displayAxial();

	}else if(ui->coronalViewBtn->isChecked()){

		double rotate = angle - angleXCoronal;
		transformCoronal->Translate(-(rotCenterCoronal[0]),-(sliceCoronal*spacing[1]),-(rotCenterCoronal[1]));
		transformCoronal->RotateX(rotate);
		transformCoronal->Translate((rotCenterCoronal[0]),(sliceCoronal*spacing[1]),(rotCenterCoronal[1]));
		angleXCoronal = angle;

		//display new slice
		displayCoronal();

	}

}

void MainWindow::rotateY(int angle)
{
	if(ui->sagitalViewBtn->isChecked()){

		//rotate slice
		double rotate = angle - angleYSagital;
		transformSagital->Translate(-(sliceSagital*spacing[0]),-(rotCenterSagital[0]),-(rotCenterSagital[1]));
		transformSagital->RotateY(rotate);
		transformSagital->Translate((sliceSagital*spacing[0]),(rotCenterSagital[0]),(rotCenterSagital[1]));
		angleYSagital = angle;

		//display new slice
		displaySagital();

	}else if(ui->axialViewBtn->isChecked()){

		//rotate slice
		double rotate = angle - angleYAxial;
		transformAxial->Translate(-(rotCenterAxial[0]),-(rotCenterAxial[1]),-(sliceAxial*spacing[2]));
		transformAxial->RotateY(rotate);
		transformAxial->Translate((rotCenterAxial[0]),(rotCenterAxial[1]),(sliceAxial*spacing[2]));
		angleYAxial = angle;

		//display new slice
		displayAxial();

	}else if(ui->coronalViewBtn->isChecked()){

		//rotate slice
		double rotate = angle - angleYCoronal;
		transformCoronal->Translate(-(rotCenterCoronal[0]),-(sliceCoronal*spacing[1]),-(rotCenterCoronal[1]));
		transformCoronal->RotateY(rotate);
		transformCoronal->Translate((rotCenterCoronal[0]),(sliceCoronal*spacing[1]),(rotCenterCoronal[1]));
		angleYCoronal = angle;

		//display new slice
		displayCoronal();

	}

}

void MainWindow::rotateZ(int angle)
{
	if(ui->sagitalViewBtn->isChecked()){

		//rotate slice
		double rotate = angle - angleZSagital;
		transformSagital->Translate(-(sliceSagital*spacing[0]),-(rotCenterSagital[0]),-(rotCenterSagital[1]));
		transformSagital->RotateZ(rotate);
		transformSagital->Translate((sliceSagital*spacing[0]),(rotCenterSagital[0]),(rotCenterSagital[1]));
		angleZSagital = angle;

		//display new slice
		displaySagital();

	}else if(ui->axialViewBtn->isChecked()){

		//rotate slice
		double rotate = angle - angleZAxial;
		transformAxial->Translate(-(rotCenterAxial[0]),-(rotCenterAxial[1]),-(sliceAxial*spacing[2]));
		transformAxial->RotateZ(rotate);
		transformAxial->Translate((rotCenterAxial[0]),(rotCenterAxial[1]),(sliceAxial*spacing[2]));
		angleZAxial = angle;

		//display new slice
		displayAxial();

	}else if(ui->coronalViewBtn->isChecked()){

		//rotate slice
		double rotate = angle - angleZCoronal;
		transformCoronal->Translate(-(rotCenterCoronal[0]),-(sliceCoronal*spacing[1]),-(rotCenterCoronal[1]));
		transformCoronal->RotateZ(rotate);
		transformCoronal->Translate((rotCenterCoronal[0]),(sliceCoronal*spacing[1]),(rotCenterCoronal[1]));
		angleZCoronal = angle;

		//display new slice
		displayCoronal();

	}
}

void MainWindow::sagitalBtnClicked(bool value)
{		
	//check button
	ui->sagitalViewBtn->setChecked(true);

	addActorsSagitalView();

	if(value){

		//change the view when only one view is active 
		if(ui->displayOneBtn->isChecked()){
			sagitalWidget->show();
			axialWidget->hide();
			coronalWidget->hide();
			volWidget->hide();
		}
		
		if(ui->axialViewBtn->isChecked()){

			//uncheck button
			ui->axialViewBtn->setChecked(false);

			//change reslice slider to current position
			if((dimensions[2]-1)>=(dimensions[0]-1)){
				ui->sliceSld->setValue(sliceSagital);
				ui->sliceSld->setRange(0,dimensions[0]-1);
			}else{
				ui->sliceSld->setRange(0,dimensions[0]-1);
				ui->sliceSld->setValue(sliceSagital);
			}

			//uncheck buttons and return view to normal
			if(ui->rotCenterBtn->isChecked()){
				ui->rotCenterBtn->setChecked(false);
				imageStyleAxial->RemoveAllObservers();
			}

			if(ui->segmentBtn->isChecked()){
				ui->segmentBtn->setChecked(false);
				segmentationStyleAxial->clearTracer();
				viewerAxial->GetRenderWindow()->GetInteractor()->SetInteractorStyle(imageStyleAxial);
				viewerAxial->Render();
			}

		}else if(ui->coronalViewBtn->isChecked()){

			//uncheck button
			ui->coronalViewBtn->setChecked(false);

			//change reslice slider to current position
			if((dimensions[1]-1)>=(dimensions[0]-1)){
				ui->sliceSld->setValue(sliceSagital);
				ui->sliceSld->setRange(0,dimensions[0]-1);
			}else{
				ui->sliceSld->setRange(0,dimensions[0]-1);
				ui->sliceSld->setValue(sliceSagital);
			}

			//uncheck buttons and return view to normal
			if(ui->rotCenterBtn->isChecked()){
				ui->rotCenterBtn->setChecked(false);
				imageStyleCoronal->RemoveAllObservers();
			}

			if(ui->segmentBtn->isChecked()){
				ui->segmentBtn->setChecked(false);
				segmentationStyleCoronal->clearTracer();
				viewerCoronal->GetRenderWindow()->GetInteractor()->SetInteractorStyle(imageStyleCoronal);
				viewerCoronal->Render();
			}

		}else if(ui->volViewBtn->isChecked()){
			
			//uncheck button
			ui->volViewBtn->setChecked(false);
		
		}

		//change rotation sliders to current position
		ui->rotXSld->setValue(angleXSagital);
		ui->rotYSld->setValue(angleYSagital);
		ui->rotZSld->setValue(angleZSagital);
			
	}
}

void MainWindow::axialBtnClicked(bool value)
{
	//check button
	ui->axialViewBtn->setChecked(true);
	
	addActorsAxialView();

	if(value){

		//change the view when only one view is active 
		if(ui->displayOneBtn->isChecked()){
			sagitalWidget->hide();
			axialWidget->show();
			coronalWidget->hide();
			volWidget->hide();
		}

		//remove observer
		imageStyleAxial->RemoveAllObservers();

		if(ui->sagitalViewBtn->isChecked()){

			//uncheck button
			ui->sagitalViewBtn->setChecked(false);

			//change reslice slider to current position
			if((dimensions[0]-1)>=(dimensions[2]-1)){
				ui->sliceSld->setValue(sliceAxial);
				ui->sliceSld->setRange(0,dimensions[2]-1);
			}else{
				ui->sliceSld->setRange(0,dimensions[2]-1);
				ui->sliceSld->setValue(sliceAxial);
			}

			//uncheck buttons and return view to normal
			if(ui->rotCenterBtn->isChecked()){
				ui->rotCenterBtn->setChecked(false);
				imageStyleSagital->RemoveAllObservers();
			}

			if(ui->segmentBtn->isChecked()){
				ui->segmentBtn->setChecked(false);
				segmentationStyleSagital->clearTracer();
				viewerSagital->GetRenderWindow()->GetInteractor()->SetInteractorStyle(imageStyleSagital);
				viewerSagital->Render();
			}

		}else if(ui->coronalViewBtn->isChecked()){

			//uncheck button
			ui->coronalViewBtn->setChecked(false);

			//change reslice slider to current position
			if((dimensions[1]-1)>=(dimensions[2]-1)){
				ui->sliceSld->setValue(sliceAxial);
				ui->sliceSld->setRange(0,dimensions[2]-1);
			}else{
				ui->sliceSld->setRange(0,dimensions[2]-1);
				ui->sliceSld->setValue(sliceAxial);
			}

			//uncheck buttons and return view to normal
			if(ui->rotCenterBtn->isChecked()){
				ui->rotCenterBtn->setChecked(false);
				imageStyleCoronal->RemoveAllObservers();
			}

			if(ui->segmentBtn->isChecked()){
				ui->segmentBtn->setChecked(false);
				segmentationStyleCoronal->clearTracer();
				viewerCoronal->GetRenderWindow()->GetInteractor()->SetInteractorStyle(imageStyleCoronal);
				viewerCoronal->Render();
			}

		}else if(ui->volViewBtn->isChecked()){
			
			//uncheck button
			ui->volViewBtn->setChecked(false);
		
		}

		//change rotation sliders to current position
		ui->rotXSld->setValue(angleXAxial);
		ui->rotYSld->setValue(angleYAxial);
		ui->rotZSld->setValue(angleZAxial);
			
	}
}

void MainWindow::coronalBtnClicked(bool value)
{
	//check button
	ui->coronalViewBtn->setChecked(true);

	addActorsCoronalView();

	if(value){

		//change the view when only one view is active 
		if(ui->displayOneBtn->isChecked()){
			sagitalWidget->hide();
			axialWidget->hide();
			coronalWidget->show();
			volWidget->hide();
		}

		//remove observer
		imageStyleCoronal->RemoveAllObservers();

		if(ui->sagitalViewBtn->isChecked()){

			//uncheck button
			ui->sagitalViewBtn->setChecked(false);

			//change reslice slider to current position
			if((dimensions[0]-1)>=(dimensions[1]-1)){
				ui->sliceSld->setValue(sliceCoronal);
				ui->sliceSld->setRange(0,dimensions[1]-1);
			}else{
				ui->sliceSld->setRange(0,dimensions[1]-1);
				ui->sliceSld->setValue(sliceCoronal);
			}

			//uncheck buttons and return view to normal
			if(ui->rotCenterBtn->isChecked()){
				ui->rotCenterBtn->setChecked(false);
				imageStyleSagital->RemoveAllObservers();
			}

			if(ui->segmentBtn->isChecked()){
				ui->segmentBtn->setChecked(false);
				segmentationStyleSagital->clearTracer();
				viewerSagital->GetRenderWindow()->GetInteractor()->SetInteractorStyle(imageStyleSagital);
				viewerSagital->Render();
			}

		}else if(ui->axialViewBtn->isChecked()){

			//uncheck button
			ui->axialViewBtn->setChecked(false);

			//change reslice slider to current position
			if((dimensions[2]-1)>=(dimensions[0]-1)){
				ui->sliceSld->setValue(sliceCoronal);
				ui->sliceSld->setRange(0,dimensions[1]-1);
			}else{
				ui->sliceSld->setRange(0,dimensions[1]-1);
				ui->sliceSld->setValue(sliceCoronal);
			}

			//uncheck buttons and return view to normal
			if(ui->rotCenterBtn->isChecked()){
				ui->rotCenterBtn->setChecked(false);
				imageStyleAxial->RemoveAllObservers();
			}

			if(ui->segmentBtn->isChecked()){
				ui->segmentBtn->setChecked(false);
				segmentationStyleAxial->clearTracer();
				viewerAxial->GetRenderWindow()->GetInteractor()->SetInteractorStyle(imageStyleAxial);
				viewerAxial->Render();
			}

		}else if(ui->volViewBtn->isChecked()){
			
			//uncheck button
			ui->volViewBtn->setChecked(false);
		
		}

		//change rotation sliders to current position
		ui->rotXSld->setValue(angleXCoronal);
		ui->rotYSld->setValue(angleYCoronal);
		ui->rotZSld->setValue(angleZCoronal);
			
	}
}

void MainWindow::volBtnClicked(bool value)
{
	//check button
	ui->volViewBtn->setChecked(true);
	
	if(value){

		//uncheck buttons
		ui->rotCenterBtn->setChecked(false);

		//change the view when only one view is active 
		if(ui->displayOneBtn->isChecked()){
			sagitalWidget->hide();
			axialWidget->hide();
			coronalWidget->hide();
			volWidget->show();
		}

		//uncheck buttons
		ui->sagitalViewBtn->setChecked(false);
		ui->axialViewBtn->setChecked(false);
		ui->coronalViewBtn->setChecked(false);
	}
}

void MainWindow::allBtnClicked(bool value)
{
	//check and uncheck view buttons
	ui->displayAllBtn->setChecked(true);
	ui->displayOneBtn->setChecked(false);

	//show all views
	if(value){
		sagitalWidget->show();
		axialWidget->show();
		coronalWidget->show();
		volWidget->show();
	}
}

void MainWindow::oneBtnClicked(bool value)
{
	//check and uncheck view buttons
	ui->displayOneBtn->setChecked(true);
	ui->displayAllBtn->setChecked(false);

	if(value){

		if(ui->sagitalViewBtn->isChecked()){

			//showing selected view
			sagitalWidget->show();
			axialWidget->hide();
			coronalWidget->hide();
			volWidget->hide();

		}else if(ui->axialViewBtn->isChecked()){

			//showing selected view
			sagitalWidget->hide();
			axialWidget->show();
			coronalWidget->hide();
			volWidget->hide();

		}else if(ui->coronalViewBtn->isChecked()){

			//showing selected view
			sagitalWidget->hide();
			axialWidget->hide();
			coronalWidget->show();
			volWidget->hide();

		}else if(ui->volViewBtn->isChecked()){

			//showing selected view
			sagitalWidget->hide();
			axialWidget->hide();
			coronalWidget->hide();
			volWidget->show();

		}
	}
}

void MainWindow::rotCenterBtnClicked(bool value)
{
	if(value){
		
		//initialize picker objects
		vtkSmartPointer<vtkPropPicker> propPicker = vtkSmartPointer<vtkPropPicker>::New();
		vtkSmartPointer<QVTKImageWidgetCommand<QVTKWidget> > callback = vtkSmartPointer<QVTKImageWidgetCommand<QVTKWidget> >::New();

		callback->SetMainWindow(this);
		propPicker->PickFromListOn();
		callback->SetPicker(propPicker);
		
		if(ui->sagitalViewBtn->isChecked()){

			std::cout<<std::endl<<"Change rotation center in sagital view"<<std::endl;

			//add observer to selecter view
			propPicker->AddPickList(imageActorSagital);
			callback->SetImageViewer(viewerSagital);
			imageStyleSagital->AddObserver(vtkCommand::LeftButtonPressEvent, callback);

		}else if(ui->axialViewBtn->isChecked()){

			std::cout<<std::endl<<"Change rotation center in axial view"<<std::endl;

			//add observer to selecter view
			propPicker->AddPickList(imageActorAxial);
			callback->SetImageViewer(viewerAxial);
			imageStyleAxial->AddObserver(vtkCommand::LeftButtonPressEvent, callback);

		}else if(ui->coronalViewBtn->isChecked()){

			std::cout<<std::endl<<"Change rotation center in coronal view"<<std::endl;

			//add observer to selecter view
			propPicker->AddPickList(imageActorCoronal);
			callback->SetImageViewer(viewerCoronal);
			imageStyleCoronal->AddObserver(vtkCommand::LeftButtonPressEvent, callback);

		}
	}else{
	
		//remove observers from all views
		imageStyleSagital->RemoveAllObservers();
		imageStyleAxial->RemoveAllObservers();
		imageStyleCoronal->RemoveAllObservers();

	}
}

void MainWindow::segmentBtnClicked(bool value)
{	
	if(value){
	
		if(ui->sagitalViewBtn->isChecked()){
			
			//initialize segmentation interactor style in selected view
			segmentationStyleSagital = vtkSmartPointer< vtkTracerInteractorStyle<QVTKWidget> >::New();
			segmentationStyleSagital->setCallerWidget(sagitalWidget);
			segmentationStyleSagital->setMainWindow(this);
			viewerSagital->GetRenderWindow()->GetInteractor()->SetInteractorStyle(segmentationStyleSagital);
			segmentationStyleSagital->initTracer(imageActorSagital);
			segmentationStyleSagital->setAutoCloseOn();

			viewerSagital->GetRenderer()->RemoveActor(sagitalCenterRefActor);

		}else if(ui->axialViewBtn->isChecked()){
			
			//initialize segmentation interactor style in selected view
			segmentationStyleAxial = vtkSmartPointer< vtkTracerInteractorStyle<QVTKWidget> >::New();
			segmentationStyleAxial->setCallerWidget(axialWidget);
			segmentationStyleAxial->setMainWindow(this);
			viewerAxial->GetRenderWindow()->GetInteractor()->SetInteractorStyle(segmentationStyleAxial);
			segmentationStyleAxial->initTracer(imageActorAxial);
			segmentationStyleAxial->setAutoCloseOn();

			viewerAxial->GetRenderer()->RemoveActor(axialCenterRefActor);

		}else if(ui->coronalViewBtn->isChecked()){
			
			//initialize segmentation interactor style in selected view
			segmentationStyleCoronal = vtkSmartPointer< vtkTracerInteractorStyle<QVTKWidget> >::New();
			segmentationStyleCoronal->setCallerWidget(coronalWidget);
			segmentationStyleCoronal->setMainWindow(this);
			viewerCoronal->GetRenderWindow()->GetInteractor()->SetInteractorStyle(segmentationStyleCoronal);
			segmentationStyleCoronal->initTracer(imageActorCoronal);
			segmentationStyleCoronal->setAutoCloseOn();

			viewerCoronal->GetRenderer()->RemoveActor(coronalCenterRefActor);

		}

	}else{

		if(ui->sagitalViewBtn->isChecked()){

			//removing segmentation interactor style from selected view 
			segmentationStyleSagital->clearTracer();
			viewerSagital->GetRenderer()->AddActor(sagitalCenterRefActor);
			viewerSagital->GetRenderWindow()->GetInteractor()->SetInteractorStyle(imageStyleSagital);

		}else if(ui->axialViewBtn->isChecked()){

			//removing segmentation interactor style from selected view 
			segmentationStyleAxial->clearTracer();
			viewerAxial->GetRenderer()->AddActor(axialCenterRefActor);
			viewerAxial->GetRenderWindow()->GetInteractor()->SetInteractorStyle(imageStyleAxial);

		}else if(ui->coronalViewBtn->isChecked()){

			//removing segmentation interactor style from selected view 
			segmentationStyleCoronal->clearTracer();
			viewerCoronal->GetRenderer()->AddActor(coronalCenterRefActor);
			viewerCoronal->GetRenderWindow()->GetInteractor()->SetInteractorStyle(imageStyleCoronal);

		}

	}
}

void MainWindow::resetSlice()
{
	if(ui->sagitalViewBtn->isChecked()){
		
		std::cout<<"Reset Sagital View to center slice"<<std::endl;

		//update data and actors of current view
		removeActorsSagitalView();
		configSagitalView();
		addActorsSagitalView();

		//setting initial values for sliders
		ui->rotXSld->setValue(0);;
		ui->rotYSld->setValue(0);
		ui->rotZSld->setValue(0);
		ui->sliceSld->setValue(centerSlice[0]);

	}else if(ui->axialViewBtn->isChecked()){
			
		std::cout<<"Reset Axial View to center slice"<<std::endl;

		//update data and actors of current view
		removeActorsAxialView();
		configAxialView();
		addActorsAxialView();

		//setting initial values for sliders
		ui->rotXSld->setValue(0);
		ui->rotYSld->setValue(0);
		ui->rotZSld->setValue(0);
		ui->sliceSld->setValue(centerSlice[2]);

	}else if(ui->coronalViewBtn->isChecked()){

		std::cout<<"Reset Coronal View to center slice"<<std::endl;

		//update data and actors of current view
		removeActorsCoronalView();
		configCoronalView();
		addActorsCoronalView();

		//setting initial values for sliders
		ui->rotXSld->setValue(0);
		ui->rotYSld->setValue(0);
		ui->rotZSld->setValue(0);
		ui->sliceSld->setValue(centerSlice[1]);

	}
}

void MainWindow::resetAll()
{
	std::cout<<"Reset All Views to center slice"<<std::endl;

	//update data from all views
	removeActorsSagitalView();
	configSagitalView();
	
	removeActorsAxialView();
	configAxialView();

	removeActorsCoronalView();
	configCoronalView();

	if(ui->sagitalViewBtn->isChecked()){
				
		//add actors to current view
		addActorsSagitalView();

		//setting initial values for sliders
		ui->rotXSld->setValue(0);;
		ui->rotYSld->setValue(0);
		ui->rotZSld->setValue(0);
		ui->sliceSld->setValue(centerSlice[0]);

		displayAxial();
		displayCoronal();

	}else if(ui->axialViewBtn->isChecked()){
		
		//add actors to current view
		addActorsAxialView();

		//setting initial values for sliders
		ui->rotXSld->setValue(0);
		ui->rotYSld->setValue(0);
		ui->rotZSld->setValue(0);
		ui->sliceSld->setValue(centerSlice[2]);

		displaySagital();
		displayCoronal();

	}else if(ui->coronalViewBtn->isChecked()){

		//add actors to current view
		addActorsCoronalView();

		//setting initial values for sliders
		ui->rotXSld->setValue(0);
		ui->rotYSld->setValue(0);
		ui->rotZSld->setValue(0);
		ui->sliceSld->setValue(centerSlice[1]);

		displayAxial();
		displaySagital();

	}
}

void MainWindow::saveSeg()
{
	segmented = true;
	ui->saveSegBtn->setEnabled(true);

	vtkSmartPointer<vtkImageBlend> blend = vtkSmartPointer<vtkImageBlend>::New();
	blend->SetInput(0,volumeData);
	blend->SetInput(1,segmentedImage);
	blend->SetOpacity(1,0.5);
	blend->Update();

	QString saveDirectory = QFileDialog::getSaveFileName(
                this, tr("Choose File to Save Mask Volume"), QDir::currentPath());

	QString saveMhdDirectory = saveDirectory;
	QString saveRawDirectory = saveDirectory;

    QString mhdFilename = ".mhd";
	QString rawFilename = ".raw";

    QString qtSaveMhdFile = saveMhdDirectory.append(mhdFilename);
	QString qtSaveRawFile = saveRawDirectory.append(rawFilename);

	std::string str1 = std::string(qtSaveMhdFile.toAscii().data());
	const char * saveMhdFile = str1.c_str();

	std::string str2 = std::string(qtSaveRawFile.toAscii().data());
	const char * saveRawFile = str2.c_str();

	std::cout<<"Saving Mask Volume in files:"<<std::endl<<std::endl;
	std::cout<<saveMhdFile<<std::endl<<std::endl;
	std::cout<<saveRawFile<<std::endl;

	vtkSmartPointer<vtkMetaImageWriter> writer = vtkSmartPointer<vtkMetaImageWriter>::New();
	writer->SetFileName(saveMhdFile);
	writer->SetRAWFileName(saveRawFile);
	writer->SetInput(blend->GetOutput());

	try{
	writer->Write();
	}catch(std::exception& e){
		std::cout<<e.what()<<std::endl;
	}
}
