#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QVBoxLayout>

#include <vtkMetaImageReader.h>
#include <vtkVolumeRayCastMapper.h>
#include <vtkVolumeRayCastCompositeFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkCamera.h>
#include <vtkImageChangeInformation.h>


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
	imageStyle = vtkSmartPointer<vtkInteractorStyleImage>::New();

	viewerSagital = vtkSmartPointer<vtkImageViewer2>::New();
	imageActorSagital = vtkSmartPointer<vtkImageActor>::New();
    sagitalWidget->SetRenderWindow(viewerSagital->GetRenderWindow());
	imageActorSagital = viewerSagital->GetImageActor();
	imageActorSagital->InterpolateOff();
	viewerSagital->GetRenderWindow()->GetInteractor()->SetInteractorStyle(imageStyle);

	viewerAxial = vtkSmartPointer<vtkImageViewer2>::New();
	imageActorAxial = vtkSmartPointer<vtkImageActor>::New();
    axialWidget->SetRenderWindow(viewerAxial->GetRenderWindow());
	imageActorAxial = viewerSagital->GetImageActor();
	imageActorAxial->InterpolateOff();
	viewerAxial->GetRenderWindow()->GetInteractor()->SetInteractorStyle(imageStyle);
}

void MainWindow::setSlicesData()
{
	std::cout<<"Displaying Slices"<<std::cout;
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
}

void MainWindow::displaySagital()
{
	reslicerSagital->Update();
	sliceImageSagital = reslicerSagital->GetOutput();
	sliceImageSagital->Update();

	viewerSagital->SetInput(sliceImageSagital);
    viewerSagital->Render();
}

void MainWindow::displayAxial()
{
	reslicerAxial->Update();
	sliceImageAxial = reslicerAxial->GetOutput();
	sliceImageAxial->Update();

	viewerAxial->SetInput(sliceImageAxial);
    viewerAxial->Render();
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
		displaySagital();
	}else if(ui->axialViewBtn->isChecked()){
		double translate = slice - sliceAxial;
		transformAxial->Translate(0,0,translate*spacing[2]);
		sliceAxial = slice;
		displayAxial();
	}
}

void MainWindow::rotateX(int angle)
{
	if(ui->sagitalViewBtn->isChecked()){
		double rotate = angle - angleXSagital;
		transformSagital->Translate(0,-(positionCenter[1]-origin[1]),-(positionCenter[2]-origin[2]));
		transformSagital->RotateX(rotate);
		transformSagital->Translate(0,(positionCenter[1]-origin[1]),(positionCenter[2]-origin[2]));
		angleXSagital = angle;
		displaySagital();
	}else if(ui->axialViewBtn->isChecked()){
		double rotate = angle - angleXAxial;
		transformAxial->Translate(-(positionCenter[0]-origin[0]),-(positionCenter[1]-origin[1]),0);
		transformAxial->RotateX(rotate);
		transformAxial->Translate((positionCenter[0]-origin[0]),(positionCenter[1]-origin[1]),0);
		angleXAxial = angle;
		displayAxial();
	}

}

void MainWindow::rotateY(int angle)
{
	if(ui->sagitalViewBtn->isChecked()){
		double rotate = angle - angleYSagital;
		transformSagital->Translate(0,-(positionCenter[1]-origin[1]),-(positionCenter[2]-origin[2]));
		transformSagital->RotateY(rotate);
		transformSagital->Translate(0,(positionCenter[1]-origin[1]),(positionCenter[2]-origin[2]));
		angleYSagital = angle;
		displaySagital();
	}else if(ui->axialViewBtn->isChecked()){
		double rotate = angle - angleYAxial;
		transformAxial->Translate(-(positionCenter[0]-origin[0]),-(positionCenter[1]-origin[1]),0);
		transformAxial->RotateY(rotate);
		transformAxial->Translate((positionCenter[0]-origin[0]),(positionCenter[1]-origin[1]),0);
		angleYAxial = angle;
		displayAxial();
	}

}

void MainWindow::rotateZ(int angle)
{
	if(ui->sagitalViewBtn->isChecked()){
		double rotate = angle - angleZSagital;
		transformSagital->Translate(0,-(positionCenter[1]-origin[1]),-(positionCenter[2]-origin[2]));
		transformSagital->RotateZ(rotate);
		transformSagital->Translate(0,(positionCenter[1]-origin[1]),(positionCenter[2]-origin[2]));
		angleZSagital = angle;
		displaySagital();
	}else if(ui->axialViewBtn->isChecked()){
		double rotate = angle - angleZAxial;
		transformAxial->Translate(-(positionCenter[0]-origin[0]),-(positionCenter[1]-origin[1]),0);
		transformAxial->RotateZ(rotate);
		transformAxial->Translate((positionCenter[0]-origin[0]),(positionCenter[1]-origin[1]),0);
		angleZAxial = angle;
		displayAxial();
	}
}

void MainWindow::sagitalBtnClicked(bool value)
{
	if(!value){
		ui->sagitalViewBtn->setChecked(true);
	}else{
		ui->sagitalViewBtn->setChecked(true);
		ui->axialViewBtn->setChecked(false);
		ui->coronalViewBtn->setChecked(false);
		ui->volViewBtn->setChecked(false);

		ui->sliceSld->setRange(0,dimensions[0]-1);
		ui->sliceSld->setValue(sliceSagital);

		ui->rotXSld->setValue(angleXSagital);
		ui->rotYSld->setValue(angleYSagital);
		ui->rotZSld->setValue(angleZSagital);
	}
}

void MainWindow::axialBtnClicked(bool value)
{
	if(!value){
		ui->axialViewBtn->setChecked(true);
	}else{
		ui->sagitalViewBtn->setChecked(false);
		ui->axialViewBtn->setChecked(true);
		ui->coronalViewBtn->setChecked(false);
		ui->volViewBtn->setChecked(false);

		ui->sliceSld->setRange(0,dimensions[2]-1);
		ui->sliceSld->setValue(sliceAxial);

		ui->rotXSld->setValue(angleXAxial);
		ui->rotYSld->setValue(angleYAxial);
		ui->rotZSld->setValue(angleZAxial);
	}
}