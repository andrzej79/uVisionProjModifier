#ifndef KEILPROJMODIFIER_H
#define KEILPROJMODIFIER_H

#include <QObject>
#include <QString>
#include <QDomDocument>
#include <QFile>

/**
 * @brief The KeilProjModifier class
 */
class KeilProjModifier : public QObject
{
	Q_OBJECT

public:
	KeilProjModifier( QObject *parent = nullptr );
	~KeilProjModifier( );

	// file ops
	bool openProjFile ( QString fileName );
	bool saveProjFile	( );
	bool saveProjFileAs ( QString fileName );

	// getters
	QString getOpenFileName ( );
	QStringList getSrcGroupsNames ( );
	QStringList getSrcFileNames ( QString groupName = "" );

	// modifiers
	void setCpp11Flag ( QStringList fileNames, bool flagState );
	void sortSrcFilesInGroups (QStringList groupNames );

private:
	bool						_docModified = false;
	QString					_fileName;
	QFile						*_file = nullptr;
	QDomDocument		*_doc = nullptr;

signals:
	void errorMessage_SIG	( QString msg );
	void infoMessage_SIG	( QString msg );
};

#endif // KEILPROJMODIFIER_H
