#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QSettings>
#include <QStringList>

namespace Ui {
	class MainWindow;
}

class KeilProjModifier;

/**
 * @brief The MainWindow class
 */
class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private:
	Ui::MainWindow		*ui;
	const QString			LAST_DIR_KEY = "last_open_dir";
	QSettings					_mySettings;
	KeilProjModifier	*_keilProjMod;

	void createConnections ( );
	QStringList getSelectedFileNames ( );
	QStringList getSelectedGroups ( );

private slots:
	// ui
	void btnFileOpenClicked_SLOT			( );
	void btnFileSaveAsClicked_SLOT		( );
	void btnQuitClicked_SLOT					( );
	void cbFilterIndexChanged_SLOT		( int index );

	// priv objects
	void kpmErrorMessage_SLOT					( QString msg );
	void kpmInfoMessage_SLOT					( QString msg );

};

#endif // MAINWINDOW_H
