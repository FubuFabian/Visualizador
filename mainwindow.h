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

	void displayVol(vtkSmartPointer<vtkImageData>);
    
private:
    Ui::MainWindow *ui;

	QVTKWidget *volWidget;

	QVTKWidget *sagitalWidget;

	QVTKWidget *axialWidget;

	QVTKWidget *coronalWidget;

	/** \brief The filename of the selected volume */
	QString volumeFilename;

	/** The displayed volume data*/
    vtkSmartPointer<vtkVolume> volume;

	/** The displayed volume data*/
    vtkSmartPointer<vtkImageData> volumeData;

	/** The displayed volume opacity*/
    vtkSmartPointer<vtkPiecewiseFunction> volumeScalarOpacity;

    /** The displayed volume properties*/
    vtkSmartPointer<vtkVolumeProperty> volumeProperty;

	/** The VTK render window*/
    vtkSmartPointer<vtkRenderWindow> renwin;

    /** The VTK renderer */
    vtkSmartPointer<vtkRenderer> renderer;

private slots:

	/**
   * \brief Set a volume filename and open in format MHD
   */
    void openMHD();
};

#endif // MAINWINDOW_H
