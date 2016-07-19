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

#include <vnl/vnl_matrix.h>

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

void MainWindow::setSegmentedPath(vtkSmartPointer<vtkPolyData>)
{
	std::cout<<"segmento"<<std::endl;
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

	viewerCoronal->GetRenderer()->AddActor(sagitalRefInCoronalViewActor);

	//seting visual reference for rotation center
	sagitalCenterRef = vtkSmartPointer<vtkRegularPolygonSource>::New();
	sagitalCenterRefMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	sagitalCenterRefActor = vtkSmartPointer<vtkActor>::New();
 
	sagitalCenterRef->SetNumberOfSides(50);
	sagitalCenterRef->SetRadius(1);
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

	viewerSagital->GetRenderer()->AddActor(axialRefInSagitalViewActor);
	viewerCoronal->GetRenderer()->AddActor(axialRefInCoronalViewActor);

	//seting visual reference for rotation center
	axialCenterRef = vtkSmartPointer<vtkRegularPolygonSource>::New();
	axialCenterRefMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	axialCenterRefActor = vtkSmartPointer<vtkActor>::New();
 
	axialCenterRef->SetNumberOfSides(50);
	axialCenterRef->SetRadius(1);
	axialCenterRef->SetCenter(rotCenterAxial[0],rotCenterAxial[1],0);

	axialCenterRefMapper->SetInputConnection(axialCenterRef->GetOutputPort());;
	axialCenterRefActor->SetMapper(axialCenterRefMapper);
	axialCenterRefActor->GetProperty()->SetColor(1.0,0.59,0.08);

	viewerAxial->GetRenderer()->AddActor(axialCenterRefActor);
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

	viewerSagital->GetRenderer()->AddActor(coronalRefInSagitalViewActor);

	//seting visual reference for rotation center
	coronalCenterRef = vtkSmartPointer<vtkRegularPolygonSource>::New();
	coronalCenterRefMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	coronalCenterRefActor = vtkSmartPointer<vtkActor>::New();
 
	coronalCenterRef->SetNumberOfSides(50);
	coronalCenterRef->SetRadius(1);
	coronalCenterRef->SetCenter(rotCenterCoronal[0],rotCenterCoronal[1],0);

	coronalCenterRefMapper->SetInputConnection(coronalCenterRef->GetOutputPort());;
	coronalCenterRefActor->SetMapper(coronalCenterRefMapper);
	coronalCenterRefActor->GetProperty()->SetColor(1.0,0.59,0.08);
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

	//vtkSmartPointer<vtkImageFlip> volFlipX = vtkSmartPointer<vtkImageFlip>::New();
	//volFlipX->SetInput(volFlipY->GetOutput());
	//volFlipX->SetFilteredAxis(2);
	//volFlipX->Update();

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

	if(value){

		//change the view when only one view is active 
		if(ui->displayOneBtn->isChecked()){
			sagitalWidget->show();
			axialWidget->hide();
			coronalWidget->hide();
			volWidget->hide();
		}

		//remove axial and coronal references, and add rotation center ref
		viewerSagital->GetRenderer()->RemoveActor(axialRefInSagitalViewActor);
		viewerSagital->GetRenderer()->RemoveActor(coronalRefInSagitalViewActor);
		viewerSagital->GetRenderer()->AddActor(sagitalCenterRefActor);

		//adding slice reference in other views
		viewerAxial->GetRenderer()->AddActor(sagitalRefInAxialViewActor);
		viewerCoronal->GetRenderer()->AddActor(sagitalRefInCoronalViewActor);

		
		if(ui->axialViewBtn->isChecked()){

			//uncheck button
			ui->axialViewBtn->setChecked(false);

			//updating refrences
			viewerAxial->GetRenderer()->AddActor(coronalRefInAxialViewActor);
			viewerAxial->GetRenderer()->RemoveActor(axialCenterRefActor);

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

			//updating refrences
			viewerCoronal->GetRenderer()->AddActor(axialRefInCoronalViewActor);
			viewerCoronal->GetRenderer()->RemoveActor(coronalCenterRefActor);

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

	if(value){

		//change the view when only one view is active 
		if(ui->displayOneBtn->isChecked()){
			sagitalWidget->hide();
			axialWidget->show();
			coronalWidget->hide();
			volWidget->hide();
		}

		//remove sagital and coronal references, and add rotation center ref
		viewerAxial->GetRenderer()->RemoveActor(sagitalRefInAxialViewActor);
		viewerAxial->GetRenderer()->RemoveActor(coronalRefInAxialViewActor);
		viewerAxial->GetRenderer()->AddActor(axialCenterRefActor);

		//adding slice reference in other views
		viewerSagital->GetRenderer()->AddActor(axialRefInSagitalViewActor);
		viewerCoronal->GetRenderer()->AddActor(axialRefInCoronalViewActor);

		//remove observer
		imageStyleAxial->RemoveAllObservers();

		if(ui->sagitalViewBtn->isChecked()){

			//uncheck button
			ui->sagitalViewBtn->setChecked(false);

			//updating refrences
			viewerSagital->GetRenderer()->AddActor(coronalRefInSagitalViewActor);
			viewerSagital->GetRenderer()->RemoveActor(sagitalCenterRefActor);

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

			//updating refrences
			viewerCoronal->GetRenderer()->AddActor(sagitalRefInCoronalViewActor);
			viewerCoronal->GetRenderer()->RemoveActor(coronalCenterRefActor);

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

	if(value){

		//change the view when only one view is active 
		if(ui->displayOneBtn->isChecked()){
			sagitalWidget->hide();
			axialWidget->hide();
			coronalWidget->show();
			volWidget->hide();
		}

		//remove sagital and axial references, and add rotation center ref
		viewerCoronal->GetRenderer()->RemoveActor(sagitalRefInCoronalViewActor);
		viewerCoronal->GetRenderer()->RemoveActor(axialRefInCoronalViewActor);
		viewerCoronal->GetRenderer()->AddActor(coronalCenterRefActor);
		
		//adding slice reference in other views
		viewerSagital->GetRenderer()->AddActor(coronalRefInSagitalViewActor);
		viewerAxial->GetRenderer()->AddActor(coronalRefInAxialViewActor);

		//remove observer
		imageStyleCoronal->RemoveAllObservers();

		if(ui->sagitalViewBtn->isChecked()){

			//uncheck button
			ui->sagitalViewBtn->setChecked(false);

			//updating refrences
			viewerSagital->GetRenderer()->AddActor(axialRefInSagitalViewActor);
			viewerSagital->GetRenderer()->RemoveActor(sagitalCenterRefActor);

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

			//updating refrences
			viewerAxial->GetRenderer()->AddActor(sagitalRefInAxialViewActor);
			viewerAxial->GetRenderer()->RemoveActor(axialCenterRefActor);

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
			viewerSagital->GetRenderWindow()->GetInteractor()->SetInteractorStyle(segmentationStyleSagital);
			segmentationStyleSagital->initTracer(imageActorSagital);
			segmentationStyleSagital->setAutoCloseOn();

			//set style in selected view
			viewerSagital->GetRenderer()->RemoveActor(sagitalCenterRefActor);
			viewerSagital->Render();

		}else if(ui->axialViewBtn->isChecked()){
			
			//initialize segmentation interactor style in selected view
			segmentationStyleAxial = vtkSmartPointer< vtkTracerInteractorStyle<QVTKWidget> >::New();
			segmentationStyleAxial->setCallerWidget(axialWidget);
			viewerAxial->GetRenderWindow()->GetInteractor()->SetInteractorStyle(segmentationStyleAxial);
			segmentationStyleAxial->initTracer(imageActorAxial);
			segmentationStyleAxial->setAutoCloseOn();

			//set style in selected view
			viewerAxial->GetRenderer()->RemoveActor(axialCenterRefActor);
			viewerAxial->Render();

		}else if(ui->coronalViewBtn->isChecked()){
			
			//initialize segmentation interactor style in selected view
			segmentationStyleCoronal = vtkSmartPointer< vtkTracerInteractorStyle<QVTKWidget> >::New();
			segmentationStyleCoronal->setCallerWidget(coronalWidget);
			viewerCoronal->GetRenderWindow()->GetInteractor()->SetInteractorStyle(segmentationStyleCoronal);
			segmentationStyleCoronal->initTracer(imageActorCoronal);
			segmentationStyleCoronal->setAutoCloseOn();

			//set style in selected view
			viewerCoronal->GetRenderer()->RemoveActor(coronalCenterRefActor);
			viewerCoronal->Render();

		}

	}else{

		if(ui->sagitalViewBtn->isChecked()){

			//removing segmentation interactor style from selected view 
			segmentationStyleSagital->clearTracer();
			viewerSagital->GetRenderer()->AddActor(sagitalCenterRefActor);
			viewerSagital->GetRenderWindow()->GetInteractor()->SetInteractorStyle(imageStyleSagital);
			viewerSagital->Render();

		}else if(ui->axialViewBtn->isChecked()){

			//removing segmentation interactor style from selected view 
			segmentationStyleAxial->clearTracer();
			viewerAxial->GetRenderer()->AddActor(axialCenterRefActor);
			viewerAxial->GetRenderWindow()->GetInteractor()->SetInteractorStyle(imageStyleAxial);
			viewerAxial->Render();

		}else if(ui->coronalViewBtn->isChecked()){

			//removing segmentation interactor style from selected view 
			segmentationStyleCoronal->clearTracer();
			viewerCoronal->GetRenderer()->AddActor(coronalCenterRefActor);
			viewerCoronal->GetRenderWindow()->GetInteractor()->SetInteractorStyle(imageStyleCoronal);
			viewerCoronal->Render();

		}

	}
}

void MainWindow::resetSlice()
{
	if(ui->sagitalViewBtn->isChecked()){
		
		std::cout<<"Reset Sagital View to center slice"<<std::endl;

		viewerAxial->GetRenderer()->RemoveActor(sagitalRefInAxialViewActor);
		viewerCoronal->GetRenderer()->RemoveActor(sagitalRefInCoronalViewActor);
		viewerSagital->GetRenderer()->RemoveActor(sagitalCenterRefActor);

		configSagitalView();

		viewerAxial->GetRenderer()->AddActor(sagitalRefInAxialViewActor);
		viewerCoronal->GetRenderer()->AddActor(sagitalRefInCoronalViewActor);
		viewerSagital->GetRenderer()->AddActor(sagitalCenterRefActor);

		//setting initial values for sliders
		ui->rotXSld->setValue(0);;
		ui->rotYSld->setValue(0);
		ui->rotZSld->setValue(0);
		ui->sliceSld->setValue(centerSlice[0]);

	}else if(ui->axialViewBtn->isChecked()){
			
		std::cout<<"Reset Axial View to center slice"<<std::endl;

		viewerSagital->GetRenderer()->RemoveActor(axialRefInSagitalViewActor);
		viewerCoronal->GetRenderer()->RemoveActor(axialRefInCoronalViewActor);
		viewerAxial->GetRenderer()->RemoveActor(axialCenterRefActor);

		configAxialView();

		//setting initial values for sliders
		ui->rotXSld->setValue(0);
		ui->rotYSld->setValue(0);
		ui->rotZSld->setValue(0);
		ui->sliceSld->setValue(centerSlice[2]);

	}else if(ui->coronalViewBtn->isChecked()){

		std::cout<<"Reset Coronal View to center slice"<<std::endl;

		viewerSagital->GetRenderer()->RemoveActor(coronalRefInSagitalViewActor);
		viewerAxial->GetRenderer()->RemoveActor(coronalRefInAxialViewActor);
		viewerCoronal->GetRenderer()->RemoveActor(coronalCenterRefActor);

		configCoronalView();

		viewerSagital->GetRenderer()->AddActor(coronalRefInSagitalViewActor);
		viewerAxial->GetRenderer()->AddActor(coronalRefInAxialViewActor);
		viewerCoronal->GetRenderer()->AddActor(coronalCenterRefActor);

		//setting initial values for sliders
		ui->rotXSld->setValue(0);
		ui->rotYSld->setValue(0);
		ui->rotZSld->setValue(0);
		ui->sliceSld->setValue(centerSlice[1]);

	}
}

void MainWindow::resetAll()
{

}
