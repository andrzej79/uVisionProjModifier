#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFileDialog>
#include <QDesktopServices>
#include <QStringList>
#include <QSettings>
#include <QMessageBox>
#include "keilprojmodifier.h"

/**
 * @brief MainWindow::MainWindow
 * @param parent
 */
MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	_keilProjMod = new KeilProjModifier( this );
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
	connect( ui->btnFileOpen, SIGNAL( clicked(bool) ), this, SLOT( btnFileOpenClicked_SLOT() ) );
	connect( ui->btnQuit, SIGNAL( clicked(bool) ), this, SLOT( btnQuitClicked_SLOT() ) );
	connect( ui->btnFileSaveAs, SIGNAL( clicked(bool) ), this, SLOT( btnFileSaveAsClicked_SLOT() ) );
	connect( ui->cbFileShowFilter, SIGNAL( currentIndexChanged(int) ), this, SLOT( cbFilterIndexChanged_SLOT(int) ) );

	connect( ui->btnAddCpp11Flag, &QPushButton::clicked, [this](){
		_keilProjMod->setCpp11Flag( getSelectedFileNames(), true );
	} );

	connect( ui->btnClrCpp11Flag, &QPushButton::clicked, [this](){
		_keilProjMod->setCpp11Flag( getSelectedFileNames(), false );
	} );

	connect( ui->btnFileSave, &QPushButton::clicked, [this](){
		_keilProjMod->saveProjFile();
	} );

	connect( ui->btnSortFilesInGroups, &QPushButton::clicked, [this](){
		_keilProjMod->sortSrcFilesInGroups( getSelectedGroups() );
	} );

	// priv objs connections
	connect( _keilProjMod, SIGNAL( errorMessage_SIG(QString) ), this, SLOT( kpmErrorMessage_SLOT(QString) ), Qt::UniqueConnection );
	connect( _keilProjMod, SIGNAL( infoMessage_SIG(QString) ), this, SLOT( kpmInfoMessage_SLOT(QString) ), Qt::UniqueConnection );
}

/**
 * @brief MainWindow::getFileNames
 * @return
 */
QStringList MainWindow::getSelectedFileNames()
{
	// get selected filenames
	auto selItems = ui->tabFiles->selectedItems();
	QStringList fNames;
	for( auto item : selItems ) {
		fNames << item->text();
	}
	return fNames;
}

/**
 * @brief MainWindow::getSelectedGroups
 * @return
 */
QStringList MainWindow::getSelectedGroups()
{
	// get selected group names
	auto selItems = ui->tabGroups->selectedItems();
	QStringList gNames;
	for( auto item: selItems ) {
		gNames << item->text();
	}
	return gNames;
}

/**
 * @brief MainWindow::btnFileOpenClicked_SLOT
 */
void MainWindow::btnFileOpenClicked_SLOT()
{
	qDebug() << "Settings path:" << _mySettings.fileName();
	auto fileName = QFileDialog::getOpenFileName( this, "select uVision project",
																								_mySettings.value( LAST_DIR_KEY,
																																	QStandardPaths::writableLocation( QStandardPaths::DocumentsLocation ) ).toString(),
																								"*.uvprojx" );
	qDebug() << Q_FUNC_INFO << "selected file:" << fileName;
	if( fileName.isEmpty() ) {
		qDebug() << Q_FUNC_INFO << "no file selected, exit";
		return;
	}
	_mySettings.setValue( LAST_DIR_KEY, fileName );
	_keilProjMod->openProjFile( fileName );


	QStringList groups = _keilProjMod->getSrcGroupsNames();
	ui->tabGroups->clearContents();
	ui->tabGroups->setRowCount( groups.length() );
	ui->cbFileShowFilter->clear();
	ui->cbFileShowFilter->addItem( "Show All" );

	QSignalBlocker bl0( ui->cbFileShowFilter );
	QSignalBlocker bl1( ui->tabGroups );
	QSignalBlocker bl2( ui->tabFiles );

	for( int i = 0; i < groups.length(); i++ ) {
		QTableWidgetItem *twi =  new QTableWidgetItem( groups[i] );
		twi->setFlags( twi->flags() & ~(Qt::ItemIsEditable) );
		ui->tabGroups->setItem( i, 0, twi );
		//
		ui->cbFileShowFilter->addItem( groups[i] );
	}


//	QFile file( fileName );
//	QDomDocument doc;
//	qDebug() << Q_FUNC_INFO << "loading XML file";
//	if( !file.open( QIODevice::ReadOnly ) || !doc.setContent( &file ) ) {
//		qDebug() << Q_FUNC_INFO << "File read error!";
//		return;
//	}
//	file.close();

//	// get 'File' tags
//	QStringList uniqueFileNames;
//	QDomNodeList files = doc.elementsByTagName( "File" );
//	qDebug() << Q_FUNC_INFO << "QDomNodeList files.size() = " << files.size();
//	for( int i = 0; i < files.size(); i++ ) {
//		QDomNode node = files.at(i);
//		QDomElement fName = node.firstChildElement("FileName");
//		if( fName.isNull() ) {
//			// wtf? no file name child?
//			qDebug() << "null!";
//			continue;
//		}
//		if(!fName.text().endsWith(".cpp")) {
//			// take only cpp files
//			continue;
//		}

//		QDomElement fileOption = node.firstChildElement("FileOption");
//		QDomElement commonProp = fileOption.firstChildElement("CommonProperty");
//		QDomElement fileArmAds = fileOption.firstChildElement("FileArmAds");
//		QDomElement cAds = fileArmAds.firstChildElement("Cads");
//		QDomElement variousControls = cAds.firstChildElement("VariousControls");
//		QDomElement miscControls = variousControls.firstChildElement("MiscControls");
//		if( miscControls.isNull() == false ) {
//			qDebug() << "misc controls ;" << miscControls.text() << "; found in file: " << fName.text();
//			if( miscControls.text().contains("--cpp11") == false ) {
//				miscControls.firstChild().setNodeValue( miscControls.text() + " " + "--cpp11" );
//			}
//		} else {
//			fileOption = doc.createElement("FileOption");
//			fileArmAds = doc.createElement("FileArmAds");
//			cAds = doc.createElement("Cads");
//			variousControls = doc.createElement("VariousControls");
//			miscControls = doc.createElement("MiscControls");
//			QDomText mcval = doc.createTextNode("--cpp11");
//			node.appendChild(fileOption).appendChild(fileArmAds).appendChild(cAds).appendChild(variousControls).appendChild(miscControls).appendChild(mcval);

//			if( commonProp.isNull() ) {
//				// no common props, set to default identical as uVision do so
//				commonProp = doc.createElement("CommonProperty");
//				QDomNode commonPropNode = fileOption.appendChild( commonProp );
//				commonPropNode.appendChild( doc.createElement( "UseCPPCompiler" ) ).appendChild( doc.createTextNode( "2" ) );
//				commonPropNode.appendChild( doc.createElement( "RVCTCodeConst" ) ).appendChild( doc.createTextNode( "0" ) );
//				commonPropNode.appendChild( doc.createElement( "RVCTZI" ) ).appendChild( doc.createTextNode( "0" ) );
//				commonPropNode.appendChild( doc.createElement( "RVCTOtherData" ) ).appendChild( doc.createTextNode( "0" ) );
//				commonPropNode.appendChild( doc.createElement( "ModuleSelection" ) ).appendChild( doc.createTextNode( "0" ) );
//				commonPropNode.appendChild( doc.createElement( "IncludeInBuild" ) ).appendChild( doc.createTextNode( "2" ) );
//				commonPropNode.appendChild( doc.createElement( "AlwaysBuild" ) ).appendChild( doc.createTextNode( "2" ) );
//				commonPropNode.appendChild( doc.createElement( "GenerateAssemblyFile" ) ).appendChild( doc.createTextNode( "2" ) );
//				commonPropNode.appendChild( doc.createElement( "AssembleAssemblyFile" ) ).appendChild( doc.createTextNode( "2" ) );
//				commonPropNode.appendChild( doc.createElement( "PublicsOnly" ) ).appendChild( doc.createTextNode( "2" ) );
//				commonPropNode.appendChild( doc.createElement( "StopOnExitCode" ) ).appendChild( doc.createTextNode( "2" ) );
//				commonPropNode.appendChild( doc.createElement( "CustomArgument" ) ).appendChild( doc.createTextNode( "" ) );
//				commonPropNode.appendChild( doc.createElement( "IncludeLibraryModules" ) ).appendChild( doc.createTextNode( "" ) );
//				commonPropNode.appendChild( doc.createElement( "ComprImg" ) ).appendChild( doc.createTextNode( "1" ) );
//			}
//		}

//		if( uniqueFileNames.contains( fName.text() ) == false ) {
//			uniqueFileNames << fName.text();
//		}
//	}

//	// sort file names
//	uniqueFileNames.sort( );

//	auto fileNameSave = QFileDialog::getSaveFileName( this, "Save To...", fileName, "*.uvprojx" );
//	QFile sf( fileNameSave );
//	sf.open( QIODevice::ReadWrite );
//	sf.resize(0);
//	QTextStream ts( &sf );
//	doc.save( ts, 1 );
//	sf.close();

}

/**
 * @brief MainWindow::btnFileSaveAsClicked_SLOT
 */
void MainWindow::btnFileSaveAsClicked_SLOT()
{
	auto fileNameSave = QFileDialog::getSaveFileName( this, "Save To...", _keilProjMod->getOpenFileName(), "*.uvprojx" );
	if( fileNameSave.isEmpty() == false ) {
		_keilProjMod->saveProjFileAs( fileNameSave );
	}
}

/**
 * @brief MainWindow::btnQuitClicked_SLOT
 */
void MainWindow::btnQuitClicked_SLOT()
{
	int res =  QMessageBox::question( this, "Quit?", "Are You Sure You Want To Quit Application?", QMessageBox::Yes, QMessageBox::No );
	if( res == QMessageBox::Yes ) {
		QApplication::instance()->quit();
	}
}

/**
 * @brief MainWindow::cbFilterIndexChanged_SLOT
 * @param index
 */
void MainWindow::cbFilterIndexChanged_SLOT(int index)
{
	QString grName;
	if( index > 0 ) {
		grName = ui->cbFileShowFilter->itemText( index );
	}
	QStringList grFiles = _keilProjMod->getSrcFileNames( grName );

	ui->tabFiles->clearContents();
	ui->tabFiles->setRowCount( grFiles.length() );

	for( int i = 0; i < grFiles.length(); i++ ) {
		auto twi = new QTableWidgetItem( grFiles[i] );
		ui->tabFiles->setItem( i, 0, twi );
	}
}

/**
 * @brief MainWindow::kpmErrorMessage_SLOT
 * @param msg
 */
void MainWindow::kpmErrorMessage_SLOT(QString msg)
{
	QMessageBox::critical( this, "Keil Project Modifier Error", msg );
}

/**
 * @brief MainWindow::kpmInfoMessage_SLOT
 * @param msg
 */
void MainWindow::kpmInfoMessage_SLOT(QString msg)
{
	QMessageBox::information( this, "Keil Project Modifier Info", msg );
}

