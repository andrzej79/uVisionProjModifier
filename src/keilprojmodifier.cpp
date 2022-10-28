#include "keilprojmodifier.h"
#include <QDebug>

/**
 * @brief KeilProjModifier::KeilProjModifier
 */
KeilProjModifier::KeilProjModifier( QObject *parent )
	: QObject( parent )
{
}

/**
 * @brief KeilProjModifier::~KeilProjModifier
 */
KeilProjModifier::~KeilProjModifier()
{
	if( _file != nullptr ) {
		_file->deleteLater();
		_file = nullptr;
	}
	if( _doc != nullptr ) {
		delete _doc;
		_doc = nullptr;
	}
	qDebug() << Q_FUNC_INFO << "destructor finished";
}

/**
 * @brief KeilProjModifier::openProjFile
 * @param fileName
 * @return
 */
bool KeilProjModifier::openProjFile(QString fileName)
{
	if( _file != nullptr ) {
		_file->deleteLater();
		_file = nullptr;
	}
	if( _doc != nullptr ) {
		delete _doc;
		_doc = nullptr;
	}
	_docModified = false;
	_fileName = fileName;
	if( _fileName.isEmpty() ) {
		emit errorMessage_SIG( "Project File Name Is Empty!" );
		return false;
	}

	_file = new QFile( fileName );
	_doc = new QDomDocument();
	if( !_file->open( QIODevice::ReadOnly ) || !_doc->setContent( _file ) ) {
		emit errorMessage_SIG( "Project File Open Error!" );
		return false;
	}
	_file->close();
	_file->deleteLater();
	_file = nullptr;

	return true;
}

/**
 * @brief KeilProjModifier::saveProjFile
 * @return
 */
bool KeilProjModifier::saveProjFile()
{
	return saveProjFileAs( _fileName );
}

/**
 * @brief KeilProjModifier::saveProjFileAs
 * @param fileName
 * @return
 */
bool KeilProjModifier::saveProjFileAs(QString fileName)
{
	if( _doc == nullptr ) {
		emit errorMessage_SIG( "No Data To Save, Document is Empty!" );
		return false;
	}
	if( _file != nullptr ) {
		_file->deleteLater();
		_file = nullptr;
	}

	_file = new QFile( fileName );
	if( !_file->open( QIODevice::ReadWrite ) ) {
		emit errorMessage_SIG( "Can't Open File: '" + fileName + "' For Writing!" );
		return false;
	}
	_file->resize( 0 );
	QTextStream ts( _file );
	_doc->save( ts, 1 );
	_file->close();
	_file->deleteLater();
	_file = nullptr;

	emit infoMessage_SIG( QString("File saved to: '%1'").arg( fileName ) );

	return true;
}

/**
 * @brief KeilProjModifier::getOpenFileName
 * @return
 */
QString KeilProjModifier::getOpenFileName()
{
	return _fileName;
}

/**
 * @brief KeilProjModifier::getSrcGroupsNames
 * @return
 */
QStringList KeilProjModifier::getSrcGroupsNames()
{
	QStringList uniqueGrNames;

	QDomNodeList groups = _doc->elementsByTagName( "Group" );

	for( int i = 0; i < groups.size(); i++ ) {
		QDomNode node = groups.at(i);
		QDomElement g = node.firstChildElement( "GroupName" );
		if( g.isNull() ) {
			continue;
		}
		if( g.text().startsWith("::") ) {
			// skip keil rte components
			continue;
		}
		if( uniqueGrNames.contains( g.text() ) == false ) {
			uniqueGrNames << g.text();
		}
	}

	return QStringList( uniqueGrNames );
}

/**
 * @brief KeilProjModifier::getSrcFileNames
 * @param groupName
 * @return
 */
QStringList KeilProjModifier::getSrcFileNames(QString groupName)
{
	QStringList uniqueFileNames;

	QDomNodeList files = _doc->elementsByTagName( "File" );

	for( int i = 0; i < files.size(); i++ ) {
		QDomNode node = files.at(i);
		QDomElement f = node.firstChildElement( "FileName" );

		if( f.isNull() ) {
			continue;
		}

		QString fileGrName = node.parentNode().parentNode().firstChildElement( "GroupName" ).text();
		if( groupName.isEmpty() == false && groupName != fileGrName ) {
			continue;
		}

		if( uniqueFileNames.contains( f.text() ) == false ) {
			uniqueFileNames << f.text();
		}
	}

	return QStringList( uniqueFileNames );
}

/**
 * @brief KeilProjModifier::setCppOptFlags
 * @param fileNames
 * @param cpp11FlagState
 * @param removeAllOpts
 */
void KeilProjModifier::setCppOptFlags(QStringList fileNames, bool cpp11FlagState, bool removeAllOpts)
{
	if( _doc == nullptr || _doc->isNull() ) {
		qCritical() << "no document!";
		emit errorMessage_SIG( "No Document!" );
		return;
	}

	QDomNodeList files = _doc->elementsByTagName( "File" );

	for( int i = 0; i < files.size(); i++ ) {
		QDomNode node = files.at( i );
		QDomElement fName = node.firstChildElement( "FileName" );
		if( fName.isNull() ) {
			// no filename tag
			continue;
		}
		if( fName.text().endsWith(".cpp") == false ) {
			// take only cpp files
			qDebug() << Q_FUNC_INFO << "skiping not cpp file:" << fName.text();
			continue;
		}
		if( fileNames.contains( fName.text() ) == false ) {
			// file not in list
			qDebug() << Q_FUNC_INFO << "skiping file not in list:" << fName.text();
			continue;
		}
		QDomElement fileOption = node.firstChildElement("FileOption");
		QDomElement commonProp = fileOption.firstChildElement("CommonProperty");
		QDomElement fileArmAds = fileOption.firstChildElement("FileArmAds");
		QDomElement cAds = fileArmAds.firstChildElement("Cads");
		QDomElement variousControls = cAds.firstChildElement("VariousControls");
		QDomElement miscControls = variousControls.firstChildElement("MiscControls");

    if(removeAllOpts == true) {
      if(fileOption.isNull() == false) {
        node.removeChild(fileOption);
      }
    } else if( miscControls.isNull() == false ) {
			qDebug() << "misc controls ;" << miscControls.text() << "; found in file: " << fName.text();
      if( miscControls.text().contains("--cpp11") == false && cpp11FlagState == true ) {
				// add --cpp11 flag
				if( miscControls.text().length() > 0 ) {
					miscControls.firstChild().setNodeValue( miscControls.text() + " " + "--cpp11" );
				} else {
					miscControls.firstChild().setNodeValue( "--cpp11" );
				}
			}
      if( miscControls.text().contains("--cpp11") == true && cpp11FlagState == false ) {
				// remove --cpp11 flag
				miscControls.firstChild().setNodeValue( miscControls.text().replace( "--cpp11", "" ) );
				miscControls.firstChild().setNodeValue( miscControls.text().replace( "  ", " " ) );
			}
    } else if( cpp11FlagState == true ){
			fileOption = _doc->createElement("FileOption");
			fileArmAds = _doc->createElement("FileArmAds");
			cAds = _doc->createElement("Cads");
			variousControls = _doc->createElement("VariousControls");
			miscControls = _doc->createElement("MiscControls");
			QDomText mcval = _doc->createTextNode("--cpp11");
			node.appendChild(fileOption).appendChild(fileArmAds).appendChild(cAds).appendChild(variousControls).appendChild(miscControls).appendChild(mcval);

			if( commonProp.isNull() ) {
				// no common props, set to default identical as uVision do so
				commonProp = _doc->createElement("CommonProperty");
				QDomNode commonPropNode = fileOption.appendChild( commonProp );
				commonPropNode.appendChild( _doc->createElement( "UseCPPCompiler" ) ).appendChild( _doc->createTextNode( "2" ) );
				commonPropNode.appendChild( _doc->createElement( "RVCTCodeConst" ) ).appendChild( _doc->createTextNode( "0" ) );
				commonPropNode.appendChild( _doc->createElement( "RVCTZI" ) ).appendChild( _doc->createTextNode( "0" ) );
				commonPropNode.appendChild( _doc->createElement( "RVCTOtherData" ) ).appendChild( _doc->createTextNode( "0" ) );
				commonPropNode.appendChild( _doc->createElement( "ModuleSelection" ) ).appendChild( _doc->createTextNode( "0" ) );
				commonPropNode.appendChild( _doc->createElement( "IncludeInBuild" ) ).appendChild( _doc->createTextNode( "2" ) );
				commonPropNode.appendChild( _doc->createElement( "AlwaysBuild" ) ).appendChild( _doc->createTextNode( "2" ) );
				commonPropNode.appendChild( _doc->createElement( "GenerateAssemblyFile" ) ).appendChild( _doc->createTextNode( "2" ) );
				commonPropNode.appendChild( _doc->createElement( "AssembleAssemblyFile" ) ).appendChild( _doc->createTextNode( "2" ) );
				commonPropNode.appendChild( _doc->createElement( "PublicsOnly" ) ).appendChild( _doc->createTextNode( "2" ) );
				commonPropNode.appendChild( _doc->createElement( "StopOnExitCode" ) ).appendChild( _doc->createTextNode( "2" ) );
				commonPropNode.appendChild( _doc->createElement( "CustomArgument" ) ).appendChild( _doc->createTextNode( "" ) );
				commonPropNode.appendChild( _doc->createElement( "IncludeLibraryModules" ) ).appendChild( _doc->createTextNode( "" ) );
				commonPropNode.appendChild( _doc->createElement( "ComprImg" ) ).appendChild( _doc->createTextNode( "1" ) );
			}
		}
	}
  if(removeAllOpts == false) {
    emit infoMessage_SIG( QString("C++11 flag %1.").arg( (cpp11FlagState ? "added" : "removed") ) );
  } else {
    emit infoMessage_SIG( QString("All options removed from cpp files") );
  }
}

/**
 * @brief KeilProjModifier::sortSrcFilesInGroups
 * @param groupNames
 */
void KeilProjModifier::sortSrcFilesInGroups(QStringList groupNames)
{
	if( _doc == nullptr || _doc->isNull() ) {
		qCritical() << "no document!";
		emit errorMessage_SIG( "No Document!" );
		return;
	}

	QDomNodeList groups = _doc->elementsByTagName( "Group" );

	for( int i = 0; i < groups.size(); i++ ) {
		QDomNode node = groups.at( i );
		QDomElement gName = node.firstChildElement( "GroupName" );
		if( gName.text().isEmpty() ) {
			// no groupname tag
			qCritical() << Q_FUNC_INFO << "GroupName is null!";
			continue;
		}
		if( groupNames.contains( gName.text() ) == false ) {
			// group not in list
			//qDebug() << Q_FUNC_INFO << "skipping not selected group:" << gName.text();
			continue;
		}

		QList< QDomNode > gFiles;
		QDomElement file = node.firstChildElement( "Files" ).firstChildElement( "File" );
		QDomElement fName = file.firstChildElement( "FileName" );
		while( fName.isNull() == false ) {
			//qDebug() << Q_FUNC_INFO << "Group:" << gName.text() << "File:" << fName.text();
			gFiles << file;
			file = file.nextSiblingElement( "File" );
			fName = file.firstChildElement( "FileName" );
		}

		// sort nodes by filename
		std::sort( gFiles.begin(), gFiles.end(), []( const QDomNode &v1, const QDomNode &v2 ){
			return v1.firstChildElement( "FileName" ).text().toLower() < v2.firstChildElement( "FileName" ).text().toLower();
		} );
		// remove files childs
		for( QDomNode n: gFiles ) {
			QDomElement tmp = node.firstChildElement( "Files" );
			tmp.removeChild( n );
		}
		// and add them in sorted order back to group
		for( QDomNode n: gFiles ) {
			QDomElement tmp = node.firstChildElement( "Files" );
			tmp.appendChild( n );
		}
	}

	qDebug() << Q_FUNC_INFO << "sort finished";

	emit infoMessage_SIG( "Sorting finished." );
}

