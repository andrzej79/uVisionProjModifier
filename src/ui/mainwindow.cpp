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

	//
	QPixmap pixLogoKeil;
	pixLogoKeil.load("://gfx/1200px-Keil_logo.svg.png");
	ui->lbKeilLogo->setPixmap(pixLogoKeil.scaled(200, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	QPixmap pixLogoCs;
	pixLogoCs.load("://gfx/cs-lab-logo_web2.png");
	ui->lbCsLogo->setPixmap(pixLogoCs.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));

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
  connect( ui->btnFileOpen, &QAbstractButton::clicked, this, &MainWindow::btnFileOpenClicked_SLOT );
  connect( ui->btnQuit, &QAbstractButton::clicked, this, &MainWindow::btnQuitClicked_SLOT );
  connect( ui->btnFileSaveAs, &QAbstractButton::clicked, this, &MainWindow::btnFileSaveAsClicked_SLOT );
  connect( ui->cbFileShowFilter, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::cbFilterIndexChanged_SLOT );

  connect( ui->btnAddCpp11Flag, &QPushButton::clicked, this, [this](){
    _keilProjMod->setCppOptFlags( getSelectedFileNames(), true );
	} );

  connect( ui->btnClrCpp11Flag, &QPushButton::clicked, this, [this](){
    _keilProjMod->setCppOptFlags( getSelectedFileNames(), false );
	} );

  connect( ui->btnRemoveOpts, &QPushButton::clicked, this, [this]() {
    _keilProjMod->setCppOptFlags( getSelectedFileNames(), false, true);
  });

  connect( ui->btnFileSave, &QPushButton::clicked, this, [this](){
		_keilProjMod->saveProjFile();
	} );

  connect( ui->btnSortFilesInGroups, &QPushButton::clicked, this, [this](){
		_keilProjMod->sortSrcFilesInGroups( getSelectedGroups() );
	} );

	// priv objs connections
  connect( _keilProjMod, &KeilProjModifier::errorMessage_SIG, this, &MainWindow::kpmErrorMessage_SLOT);
  connect( _keilProjMod, &KeilProjModifier::infoMessage_SIG, this, &MainWindow::kpmInfoMessage_SLOT);
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
  for( auto &item : selItems ) {
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
  for( auto &item: selItems ) {
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
		twi->setFlags( twi->flags() & ~( Qt::ItemIsEditable ) );
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

