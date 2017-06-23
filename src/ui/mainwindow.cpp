#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFileDialog>
#include <QApplication>
#include <QDesktopServices>
#include <QXmlSimpleReader>
#include <QDomDocument>
#include <QStringList>

/**
 * @brief MainWindow::MainWindow
 * @param parent
 */
MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	createConnections();
}

/**
 * @brief MainWindow::~MainWindow
 */
MainWindow::~MainWindow()
{
	delete ui;
}

/**
 * @brief MainWindow::createConnections
 */
void MainWindow::createConnections()
{

	// ui connections
	connect( ui->btnStart, SIGNAL( clicked(bool) ), this, SLOT( btnStartClicked_SLOT(bool) ) );
}

/**
 * @brief MainWindow::btnStartClicked_SLOT
 * @param checked
 */
void MainWindow::btnStartClicked_SLOT(bool checked)
{
	Q_UNUSED( checked );
	qDebug() << Q_FUNC_INFO << "loading XML file";

	auto fileName = QFileDialog::getOpenFileName( this, "select uVision project", QStandardPaths::writableLocation( QStandardPaths::DocumentsLocation ), "*.uvprojx" );
	qDebug() << Q_FUNC_INFO << "selected file:" << fileName;

	if( fileName == "" ) {
		qDebug() << Q_FUNC_INFO << "no file selected, exit";
		return;
	}

	QDomDocument doc;
	QFile file( fileName );
	if( !file.open( QIODevice::ReadOnly ) || !doc.setContent( &file ) ) {
		qDebug() << Q_FUNC_INFO << "File read error!";
		return;
	}
	file.close();

	// get 'File' tags
	QStringList uniqueFileNames;
	QDomNodeList files = doc.elementsByTagName( "File" );
	qDebug() << Q_FUNC_INFO << "QDomNodeList files.size() = " << files.size();
	for( int i = 0; i < files.size(); i++ ) {
		QDomNode node = files.at(i);
		QDomElement fName = node.firstChildElement("FileName");
		if( fName.isNull() ) {
			// wtf? no file name child?
			qDebug() << "null!";
			continue;
		}
		if(!fName.text().endsWith(".cpp")) {
			// take only cpp files
			continue;
		}

		QDomElement fileOption = node.firstChildElement("FileOption");
		QDomElement commonProp = fileOption.firstChildElement("CommonProperty");
		QDomElement fileArmAds = fileOption.firstChildElement("FileArmAds");
		QDomElement cAds = fileArmAds.firstChildElement("Cads");
		QDomElement variousControls = cAds.firstChildElement("VariousControls");
		QDomElement miscControls = variousControls.firstChildElement("MiscControls");
		if( miscControls.isNull() == false ) {
			qDebug() << "misc controls ;" << miscControls.text() << "; found in file: " << fName.text();
			if( miscControls.text().contains("--cpp11") == false ) {
				miscControls.firstChild().setNodeValue( miscControls.text() + " " + "--cpp11" );
			}
		} else {
			fileOption = doc.createElement("FileOption");
			fileArmAds = doc.createElement("FileArmAds");
			cAds = doc.createElement("Cads");
			variousControls = doc.createElement("VariousControls");
			miscControls = doc.createElement("MiscControls");
			QDomText mcval = doc.createTextNode("--cpp11");
			node.appendChild(fileOption).appendChild(fileArmAds).appendChild(cAds).appendChild(variousControls).appendChild(miscControls).appendChild(mcval);

			if( commonProp.isNull() ) {
				// no common props, set to default identical as uVision do so
				commonProp = doc.createElement("CommonProperty");
				QDomNode commonPropNode = fileOption.appendChild( commonProp );
				commonPropNode.appendChild( doc.createElement( "UseCPPCompiler" ) ).appendChild( doc.createTextNode( "2" ) );
				commonPropNode.appendChild( doc.createElement( "RVCTCodeConst" ) ).appendChild( doc.createTextNode( "0" ) );
				commonPropNode.appendChild( doc.createElement( "RVCTZI" ) ).appendChild( doc.createTextNode( "0" ) );
				commonPropNode.appendChild( doc.createElement( "RVCTOtherData" ) ).appendChild( doc.createTextNode( "0" ) );
				commonPropNode.appendChild( doc.createElement( "ModuleSelection" ) ).appendChild( doc.createTextNode( "0" ) );
				commonPropNode.appendChild( doc.createElement( "IncludeInBuild" ) ).appendChild( doc.createTextNode( "2" ) );
				commonPropNode.appendChild( doc.createElement( "AlwaysBuild" ) ).appendChild( doc.createTextNode( "2" ) );
				commonPropNode.appendChild( doc.createElement( "GenerateAssemblyFile" ) ).appendChild( doc.createTextNode( "2" ) );
				commonPropNode.appendChild( doc.createElement( "AssembleAssemblyFile" ) ).appendChild( doc.createTextNode( "2" ) );
				commonPropNode.appendChild( doc.createElement( "PublicsOnly" ) ).appendChild( doc.createTextNode( "2" ) );
				commonPropNode.appendChild( doc.createElement( "StopOnExitCode" ) ).appendChild( doc.createTextNode( "2" ) );
				commonPropNode.appendChild( doc.createElement( "CustomArgument" ) ).appendChild( doc.createTextNode( "" ) );
				commonPropNode.appendChild( doc.createElement( "IncludeLibraryModules" ) ).appendChild( doc.createTextNode( "" ) );
				commonPropNode.appendChild( doc.createElement( "ComprImg" ) ).appendChild( doc.createTextNode( "1" ) );
			}
		}

		if( uniqueFileNames.contains( fName.text() ) == false ) {
			uniqueFileNames << fName.text();
		}

	}
	// sort file names
	uniqueFileNames.sort( );


	auto fileNameSave = QFileDialog::getSaveFileName( this, "Save To...", QStandardPaths::writableLocation( QStandardPaths::DocumentsLocation ), "*.uvprojx" );
	QFile sf( fileNameSave );
	sf.open( QIODevice::ReadWrite );
	sf.resize(0);
	QTextStream ts( &sf );
	doc.save( ts, 1 );
	sf.close();

}

