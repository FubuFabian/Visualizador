#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QVBoxLayout>

#include <vtkMetaImageReader.h>
#include <vtkVolumeRayCastMapper.h>
#include <vtkVolumeRayCastCompositeFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkCamera.h>
#include <vtkProperty.h>

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

	volWidget = new QVTKWidget;

	QVBoxLayout *volLayout = new QVBoxLayout;
	volLayout->setContentsMargins(0, 0, 0, 0);
	volLayout->setSpacing(0);
	volLayout->QLayout::addWidget(volWidget);
	ui->volView->setLayout(volLayout);

	sagitalWidget = new QVTKWidget;

	QVBoxLayout *sagitalLayout = new QVBoxLayout;
	sagitalLayout->setContentsMargins(0, 0, 0, 0);
	sagitalLayout->setSpacing(0);
	sagitalLayout->QLayout::addWidget(sagitalWidget);
	ui->sagitalView->setLayout(sagitalLayout);

	axialWidget = new QVTKWidget;

	QVBoxLayout *axialLayout = new QVBoxLayout;
	axialLayout->setContentsMargins(0, 0, 0, 0);
	axialLayout->setSpacing(0);
	axialLayout->QLayout::addWidget(axialWidget);
	ui->axialView->setLayout(axialLayout);

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

void MainWindow::displayVol()
{
	volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();

    vtkSmartPointer<vtkVolumeRayCastCompositeFunction> rayCastFunction =
                vtkSmartPointer<vtkVolumeRayCastCompositeFunction>::New();

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

    vtkSmartPointer<vtkVolumeRayCastCompositeFunction> compositeFunction =
            vtkSmartPointer<vtkVolumeRayCastCompositeFunction>::New();

    vtkSmartPointer<vtkVolumeRayCastMapper> volumeMapper = vtkSmartPointer<vtkVolumeRayCastMapper>::New();
    volumeMapper->SetVolumeRayCastFunction(compositeFunction);
    volumeMapper->CroppingOff();
    volumeMapper->SetInput(volumeData);

    volume = vtkSmartPointer<vtkVolume>::New();
    volume->SetMapper(volumeMapper);
    volume->SetOrigin(0,0,0);
    volume->SetProperty(volumeProperty);
    volume->Update();

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

    renwin = vtkSmartPointer<vtkRenderWindow>::New();
    renwin->AddRenderer(renderer);
    
    volWidget->SetRenderWindow(renwin);
    std::cout<<std::endl;
    std::cout<<"Displaying volume"<<std::endl<<std::endl;
    renwin->Render();
}

void MainWindow::setRenderingData()
{
	imageStyleSagital = vtkSmartPointer<vtkInteractorStyleImage>::New();
	viewerSagital = vtkSmartPointer<vtkImageViewer2>::New();
	imageActorSagital = vtkSmartPointer<vtkImageActor>::New();
    sagitalWidget->SetRenderWindow(viewerSagital->GetRenderWindow());
	imageActorSagital = viewerSagital->GetImageActor();
	imageActorSagital->InterpolateOff();
	viewerSagital->GetRenderWindow()->GetInteractor()->SetInteractorStyle(imageStyleSagital);

	imageStyleAxial = vtkSmartPointer<vtkInteractorStyleImage>::New();
	viewerAxial = vtkSmartPointer<vtkImageViewer2>::New();
	imageActorAxial = vtkSmartPointer<vtkImageActor>::New();
    axialWidget->SetRenderWindow(viewerAxial->GetRenderWindow());
	imageActorAxial = viewerSagital->GetImageActor();
	imageActorAxial->InterpolateOff();
	viewerAxial->GetRenderWindow()->GetInteractor()->SetInteractorStyle(imageStyleAxial);

	imageStyleCoronal = vtkSmartPointer<vtkInteractorStyleImage>::New();
	viewerCoronal = vtkSmartPointer<vtkImageViewer2>::New();
	imageActorCoronal = vtkSmartPointer<vtkImageActor>::New();
    coronalWidget->SetRenderWindow(viewerCoronal->GetRenderWindow());
	imageActorCoronal = viewerSagital->GetImageActor();
	imageActorCoronal->InterpolateOff();
	viewerCoronal->GetRenderWindow()->GetInteractor()->SetInteractorStyle(imageStyleCoronal);
}

void MainWindow::setSlicesData()
{
	std::cout<<"Displaying Slices"<<std::endl;
	volumeData->GetSpacing(spacing);
	volumeData->GetOrigin(origin);
    volumeData->GetDimensions(dimensions);

	centerSlice[0] = floor(dimensions[0]*0.5)-1;
	centerSlice[1] = floor(dimensions[1]*0.5)-1;
	centerSlice[2] = floor(dimensions[2]*0.5)-1;

	positionCenter[0] = origin[0] + spacing[0] * centerSlice[0];
	positionCenter[1] = origin[1] + spacing[1] * centerSlice[1]; 
    positionCenter[2] = origin[2] + spacing[2] * centerSlice[2];

	configSagitalView();
	configAxialView();
	configCoronalView();

	ui->rotXSld->setRange(-90,90);
	ui->rotXSld->setTickInterval(1);
	ui->rotXSld->setValue(0);

	ui->rotYSld->setRange(-90,90);
	ui->rotYSld->setTickInterval(1);
	ui->rotYSld->setValue(0);

	ui->rotZSld->setRange(-90,90);
	ui->rotZSld->setTickInterval(1);
	ui->rotZSld->setValue(0);

	ui->sliceSld->setRange(0,dimensions[0]-1);
	ui->sliceSld->setTickInterval(1);
	ui->sliceSld->setValue(centerSlice[0]);

	displayAxial();
	displayCoronal();
}

void MainWindow::configSagitalView()
{
	resliceAxesSagital = vtkSmartPointer<vtkMatrix4x4>::New();
	reslicerSagital = vtkSmartPointer<vtkImageReslice>::New();
	transformSagital = vtkSmartPointer<vtkTransform>::New();

	sliceSagital = centerSlice[0];
	angleXSagital = 0;
	angleYSagital = 0;
	angleZSagital = 0; 

	transformSagital->PostMultiply();
	transformSagital->Translate(positionCenter[0],origin[1],origin[2]);

	resliceAxesSagital->DeepCopy(sagitalElements);

	reslicerSagital->SetInput(volumeData);
	reslicerSagital->SetOutputDimensionality(2);
	reslicerSagital->SetResliceAxes(resliceAxesSagital);
	reslicerSagital->SetResliceTransform(transformSagital);
	reslicerSagital->SetInterpolationModeToLinear();
	
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

	viewerAxial->GetRenderer()->AddActor(sagitalRefInAxialViewActor);
	viewerCoronal->GetRenderer()->AddActor(sagitalRefInCoronalViewActor);

	sagitalCenterRef = vtkSmartPointer<vtkRegularPolygonSource>::New();
	sagitalCenterRefMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	sagitalCenterRefActor = vtkSmartPointer<vtkActor>::New();
 
	sagitalCenterRef->SetNumberOfSides(50);
	sagitalCenterRef->SetRadius(1);
	sagitalCenterRef->SetCenter(positionCenter[1],positionCenter[2],0);

	sagitalCenterRefMapper->SetInputConnection(sagitalCenterRef->GetOutputPort());;
	sagitalCenterRefActor->SetMapper(sagitalCenterRefMapper);
	sagitalCenterRefActor->GetProperty()->SetColor(1.0,0.59,0.08);

	viewerSagital->GetRenderer()->AddActor(sagitalCenterRefActor);
}

void MainWindow::configAxialView()
{
	resliceAxesAxial = vtkSmartPointer<vtkMatrix4x4>::New();
	reslicerAxial = vtkSmartPointer<vtkImageReslice>::New();
	transformAxial = vtkSmartPointer<vtkTransform>::New();

	sliceAxial = centerSlice[2];
	angleXAxial = 0;
	angleYAxial = 0;
	angleZAxial = 0; 

	transformAxial->PostMultiply();
	transformAxial->Translate(origin[0],origin[1],positionCenter[2]);

	resliceAxesAxial->DeepCopy(axialElements);

	reslicerAxial->SetInput(volumeData);
	reslicerAxial->SetOutputDimensionality(2);
	reslicerAxial->SetResliceAxes(resliceAxesAxial);
	reslicerAxial->SetResliceTransform(transformAxial);
	reslicerAxial->SetInterpolationModeToLinear();

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

	viewerCoronal->GetRenderer()->AddActor(axialRefInCoronalViewActor);
}

void MainWindow::configCoronalView()
{
	resliceAxesCoronal = vtkSmartPointer<vtkMatrix4x4>::New();
	reslicerCoronal = vtkSmartPointer<vtkImageReslice>::New();
	transformCoronal = vtkSmartPointer<vtkTransform>::New();

	sliceCoronal = centerSlice[1];
	angleXCoronal = 0;
	angleYCoronal = 0;
	angleZCoronal = 0; 

	transformCoronal->PostMultiply();
	transformCoronal->Translate(origin[0],positionCenter[1],origin[2]);

	resliceAxesCoronal->DeepCopy(coronalElements);

	reslicerCoronal->SetInput(volumeData);
	reslicerCoronal->SetOutputDimensionality(2);
	reslicerCoronal->SetResliceAxes(resliceAxesCoronal);
	reslicerCoronal->SetResliceTransform(transformCoronal);
	reslicerCoronal->SetInterpolationModeToLinear();

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

	viewerAxial->GetRenderer()->AddActor(coronalRefInAxialViewActor);
}

void MainWindow::displaySagital()
{
	reslicerSagital->Update();
	sliceImageSagital = reslicerSagital->GetOutput();
	sliceImageSagital->Update();

	
	viewerAxial->Render();
	viewerCoronal->Render();
	
	viewerSagital->SetInput(sliceImageSagital);
    viewerSagital->Render();
}

void MainWindow::displayAxial()
{
	reslicerAxial->Update();
	sliceImageAxial = reslicerAxial->GetOutput();
	sliceImageAxial->Update();

	viewerSagital->Render();
	viewerCoronal->Render();

	viewerAxial->SetInput(sliceImageAxial);
    viewerAxial->Render();
}

void MainWindow::displayCoronal()
{
	reslicerCoronal->Update();
	sliceImageCoronal = reslicerCoronal->GetOutput();
	sliceImageCoronal->Update();

	viewerSagital->Render();
	viewerAxial->Render();

	viewerCoronal->SetInput(sliceImageCoronal);
    viewerCoronal->Render();
}

void MainWindow::openMHD()
{
	std::cout<<"Loading Volume"<<std::endl;

    this->volumeFilename = QFileDialog::getOpenFileName(this, tr("Open Volume .mhd"),
        QDir::currentPath(), tr("Volume Files (*.mhd)"));
    
	vtkSmartPointer<vtkMetaImageReader> reader = vtkSmartPointer<vtkMetaImageReader>::New();
    reader->SetFileName(volumeFilename.toAscii().data());
    reader->Update();

    volumeData = reader->GetOutput();

	displayVol();
	setRenderingData();
	setSlicesData();
}

void MainWindow::reslice(int slice)
{
	if(ui->sagitalViewBtn->isChecked()){
		
		double translate = slice - sliceSagital;
		transformSagital->Translate(translate*spacing[0],0,0);
		sliceSagital = slice;

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

		displaySagital();

	}else if(ui->axialViewBtn->isChecked()){

		double translate = slice - sliceAxial;
		transformAxial->Translate(0,0,translate*spacing[2]);
		sliceAxial = slice;
		displayAxial();

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

	}else if(ui->coronalViewBtn->isChecked()){

		double translate = slice - sliceCoronal;
		transformCoronal->Translate(0,translate*spacing[1],0);
		sliceCoronal = slice;

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

		displayCoronal();
	}
}

void MainWindow::rotateX(int angle)
{
	if(ui->sagitalViewBtn->isChecked()){
		double rotate = angle - angleXSagital;
		transformSagital->Translate(-(sliceSagital*spacing[0]),-(positionCenter[1]-origin[1]),-(positionCenter[2]-origin[2]));
		transformSagital->RotateX(rotate);
		transformSagital->Translate((sliceSagital*spacing[0]),(positionCenter[1]-origin[1]),(positionCenter[2]-origin[2]));
		angleXSagital = angle;
		displaySagital();
	}else if(ui->axialViewBtn->isChecked()){
		double rotate = angle - angleXAxial;
		transformAxial->Translate(-(positionCenter[0]-origin[0]),-(positionCenter[1]-origin[1]),-(sliceAxial*spacing[2]));
		transformAxial->RotateX(rotate);
		transformAxial->Translate((positionCenter[0]-origin[0]),(positionCenter[1]-origin[1]),(sliceAxial*spacing[2]));
		angleXAxial = angle;
		displayAxial();
	}else if(ui->coronalViewBtn->isChecked()){
		double rotate = angle - angleXCoronal;
		transformCoronal->Translate(-(positionCenter[0]-origin[0]),-(sliceCoronal*spacing[1]),-(positionCenter[2]-origin[2]));
		transformCoronal->RotateX(rotate);
		transformCoronal->Translate((positionCenter[0]-origin[0]),(sliceCoronal*spacing[1]),(positionCenter[2]-origin[2]));
		angleXCoronal = angle;
		displayCoronal();
	}

}

void MainWindow::rotateY(int angle)
{
	if(ui->sagitalViewBtn->isChecked()){
		double rotate = angle - angleYSagital;
		transformSagital->Translate(-(sliceSagital*spacing[0]),-(positionCenter[1]-origin[1]),-(positionCenter[2]-origin[2]));
		transformSagital->RotateY(rotate);
		transformSagital->Translate((sliceSagital*spacing[0]),(positionCenter[1]-origin[1]),(positionCenter[2]-origin[2]));
		angleYSagital = angle;
		displaySagital();
	}else if(ui->axialViewBtn->isChecked()){
		double rotate = angle - angleYAxial;
		transformAxial->Translate(-(positionCenter[0]-origin[0]),-(positionCenter[1]-origin[1]),-(sliceAxial*spacing[2]));
		transformAxial->RotateY(rotate);
		transformAxial->Translate((positionCenter[0]-origin[0]),(positionCenter[1]-origin[1]),(sliceAxial*spacing[2]));
		angleYAxial = angle;
		displayAxial();
	}else if(ui->coronalViewBtn->isChecked()){
		double rotate = angle - angleXCoronal;
		transformCoronal->Translate(-(positionCenter[0]-origin[0]),-(sliceCoronal*spacing[1]),-(positionCenter[2]-origin[2]));
		transformCoronal->RotateY(rotate);
		transformCoronal->Translate((positionCenter[0]-origin[0]),(sliceCoronal*spacing[1]),(positionCenter[2]-origin[2]));
		angleYCoronal = angle;
		displayCoronal();
	}

}

void MainWindow::rotateZ(int angle)
{
	if(ui->sagitalViewBtn->isChecked()){
		double rotate = angle - angleZSagital;
		transformSagital->Translate(-(sliceSagital*spacing[0]),-(positionCenter[1]-origin[1]),-(positionCenter[2]-origin[2]));
		transformSagital->RotateZ(rotate);
		transformSagital->Translate((sliceSagital*spacing[0]),(positionCenter[1]-origin[1]),(positionCenter[2]-origin[2]));
		angleZSagital = angle;
		displaySagital();
	}else if(ui->axialViewBtn->isChecked()){
		double rotate = angle - angleZAxial;
		transformAxial->Translate(-(positionCenter[0]-origin[0]),-(positionCenter[1]-origin[1]),-(sliceAxial*spacing[2]));
		transformAxial->RotateZ(rotate);
		transformAxial->Translate((positionCenter[0]-origin[0]),(positionCenter[1]-origin[1]),(sliceAxial*spacing[2]));
		angleZAxial = angle;
		displayAxial();
	}else if(ui->coronalViewBtn->isChecked()){
		double rotate = angle - angleXCoronal;
		transformCoronal->Translate(-(positionCenter[0]-origin[0]),-(sliceCoronal*spacing[1]),-(positionCenter[2]-origin[2]));
		transformCoronal->RotateZ(rotate);
		transformCoronal->Translate((positionCenter[0]-origin[0]),(sliceCoronal*spacing[1]),(positionCenter[2]-origin[2]));
		angleZCoronal = angle;
		displayCoronal();
	}
}

void MainWindow::sagitalBtnClicked(bool value)
{		
	ui->sagitalViewBtn->setChecked(true);
	ui->volViewBtn->setChecked(false);

	if(ui->displayOneBtn->isChecked()){
		sagitalWidget->show();
		axialWidget->hide();
		coronalWidget->hide();
		volWidget->hide();
	}

	if(value){

		viewerSagital->GetRenderer()->RemoveActor(axialRefInSagitalViewActor);
		viewerSagital->GetRenderer()->RemoveActor(coronalRefInSagitalViewActor);

		viewerAxial->GetRenderer()->AddActor(sagitalRefInAxialViewActor);
		viewerCoronal->GetRenderer()->AddActor(sagitalRefInCoronalViewActor);
		
		if(ui->axialViewBtn->isChecked()){

			ui->axialViewBtn->setChecked(false);

			viewerAxial->GetRenderer()->AddActor(coronalRefInAxialViewActor);

			if((dimensions[2]-1)>=(dimensions[0]-1)){
				ui->sliceSld->setValue(sliceSagital);
				ui->sliceSld->setRange(0,dimensions[0]-1);
			}else{
				ui->sliceSld->setRange(0,dimensions[0]-1);
				ui->sliceSld->setValue(sliceSagital);
			}

		}else if(ui->coronalViewBtn->isChecked()){

			ui->coronalViewBtn->setChecked(false);

			viewerCoronal->GetRenderer()->AddActor(axialRefInCoronalViewActor);

			if((dimensions[1]-1)>=(dimensions[0]-1)){
				ui->sliceSld->setValue(sliceSagital);
				ui->sliceSld->setRange(0,dimensions[0]-1);
			}else{
				ui->sliceSld->setRange(0,dimensions[0]-1);
				ui->sliceSld->setValue(sliceSagital);
			}

		}

		ui->rotXSld->setValue(angleXSagital);
		ui->rotYSld->setValue(angleYSagital);
		ui->rotZSld->setValue(angleZSagital);
			
	}
}

void MainWindow::axialBtnClicked(bool value)
{
	ui->axialViewBtn->setChecked(true);
	ui->volViewBtn->setChecked(false);

	if(ui->displayOneBtn->isChecked()){
		sagitalWidget->hide();
		axialWidget->show();
		coronalWidget->hide();
		volWidget->hide();
	}

	if(value){

		viewerAxial->GetRenderer()->RemoveActor(sagitalRefInAxialViewActor);
		viewerAxial->GetRenderer()->RemoveActor(coronalRefInAxialViewActor);

		viewerSagital->GetRenderer()->AddActor(axialRefInSagitalViewActor);
		viewerCoronal->GetRenderer()->AddActor(axialRefInCoronalViewActor);

		if(ui->sagitalViewBtn->isChecked()){

			ui->sagitalViewBtn->setChecked(false);

			viewerSagital->GetRenderer()->AddActor(coronalRefInSagitalViewActor);

			if((dimensions[0]-1)>=(dimensions[2]-1)){
				ui->sliceSld->setValue(sliceAxial);
				ui->sliceSld->setRange(0,dimensions[2]-1);
			}else{
				ui->sliceSld->setRange(0,dimensions[2]-1);
				ui->sliceSld->setValue(sliceAxial);
			}

		}else if(ui->coronalViewBtn->isChecked()){

			ui->coronalViewBtn->setChecked(false);

			viewerCoronal->GetRenderer()->AddActor(sagitalRefInCoronalViewActor);

			if((dimensions[1]-1)>=(dimensions[2]-1)){
				ui->sliceSld->setValue(sliceAxial);
				ui->sliceSld->setRange(0,dimensions[2]-1);
			}else{
				ui->sliceSld->setRange(0,dimensions[2]-1);
				ui->sliceSld->setValue(sliceAxial);
			}

		}

		ui->rotXSld->setValue(angleXAxial);
		ui->rotYSld->setValue(angleYAxial);
		ui->rotZSld->setValue(angleZAxial);
			
	}
}

void MainWindow::coronalBtnClicked(bool value)
{
	ui->coronalViewBtn->setChecked(true);
	ui->volViewBtn->setChecked(false);

	if(ui->displayOneBtn->isChecked()){
		sagitalWidget->hide();
		axialWidget->hide();
		coronalWidget->show();
		volWidget->hide();
	}

	if(value){
		
		viewerCoronal->GetRenderer()->RemoveActor(sagitalRefInCoronalViewActor);
		viewerCoronal->GetRenderer()->RemoveActor(axialRefInCoronalViewActor);
		
		viewerSagital->GetRenderer()->AddActor(coronalRefInSagitalViewActor);
		viewerAxial->GetRenderer()->AddActor(coronalRefInAxialViewActor);

		if(ui->sagitalViewBtn->isChecked()){

			ui->sagitalViewBtn->setChecked(false);

			viewerSagital->GetRenderer()->AddActor(axialRefInSagitalViewActor);

			if((dimensions[0]-1)>=(dimensions[1]-1)){
				ui->sliceSld->setValue(sliceCoronal);
				ui->sliceSld->setRange(0,dimensions[1]-1);
			}else{
				ui->sliceSld->setRange(0,dimensions[1]-1);
				ui->sliceSld->setValue(sliceCoronal);
			}

		}else if(ui->axialViewBtn->isChecked()){

			ui->axialViewBtn->setChecked(false);

			viewerAxial->GetRenderer()->AddActor(sagitalRefInAxialViewActor);

			if((dimensions[2]-1)>=(dimensions[0]-1)){
				ui->sliceSld->setValue(sliceCoronal);
				ui->sliceSld->setRange(0,dimensions[1]-1);
			}else{
				ui->sliceSld->setRange(0,dimensions[1]-1);
				ui->sliceSld->setValue(sliceCoronal);
			}

		}

		ui->rotXSld->setValue(angleXCoronal);
		ui->rotYSld->setValue(angleYCoronal);
		ui->rotZSld->setValue(angleZCoronal);
			
	}
}

void MainWindow::volBtnClicked(bool value)
{
	ui->volViewBtn->setChecked(true);

	if(ui->displayOneBtn->isChecked()){
		sagitalWidget->hide();
		axialWidget->hide();
		coronalWidget->hide();
		volWidget->show();
	}
	
	if(value){
		ui->sagitalViewBtn->setChecked(false);
		ui->axialViewBtn->setChecked(false);
		ui->coronalViewBtn->setChecked(false);
	}
}

void MainWindow::allBtnClicked(bool value)
{
	ui->displayAllBtn->setChecked(true);
	ui->displayOneBtn->setChecked(false);

	if(value){
		sagitalWidget->show();
		axialWidget->show();
		coronalWidget->show();
		volWidget->show();
	}
}

void MainWindow::oneBtnClicked(bool value)
{
	ui->displayOneBtn->setChecked(true);
	ui->displayAllBtn->setChecked(false);

	if(value){
		if(ui->sagitalViewBtn->isChecked()){
			sagitalWidget->show();
			axialWidget->hide();
			coronalWidget->hide();
			volWidget->hide();
		}else if(ui->axialViewBtn->isChecked()){
			sagitalWidget->hide();
			axialWidget->show();
			coronalWidget->hide();
			volWidget->hide();
		}else if(ui->coronalViewBtn->isChecked()){
			sagitalWidget->hide();
			axialWidget->hide();
			coronalWidget->show();
			volWidget->hide();
		}else if(ui->volViewBtn->isChecked()){
			sagitalWidget->hide();
			axialWidget->hide();
			coronalWidget->hide();
			volWidget->show();
		}
	}
}