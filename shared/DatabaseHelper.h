// DatabaseHelper.h
// Class definition

/** \file
  * \todo Add Doxygen information
  */

#ifndef DATABASE_H
#define DATABASE_H

// Qt includes
#include <QObject>
#include <QSqlQuery>
#include <QString>

// Class definition
class DatabaseHelper
    : public QObject
{
    Q_OBJECT



    // ============================================================== Lifecycle
private:
    // Constructor (never instanciated)
    DatabaseHelper();

public:
    // Destructor
    virtual ~DatabaseHelper();



    // =================================================================== Misc
public:
    // Check if query caused an error
    static bool HasSQLError(QSqlQuery & mrQuery,
        const QString mcFileName, const int mcLineNumber);
};

#endif
