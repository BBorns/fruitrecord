#ifndef SQL_H
#define SQL_H
using namespace std;

#include <vector>

void saveSqlQuerry(string querry, int numberOfColumns = 1);
void sqlBuilderSelect(vector<string>, vector<string>, vector<string>, vector<string>, vector<vector<string> >, string order = "");
int sqlBuilderInsertBaum(string* arrBaumData);
int sqlBuilderInsertStaOrt(string* arrStaOrtData);
void sqlBuilderUpdateStandort();
void sqlBuilderUpdateBaum();
void sqlBuilderUpdateSorten();
void sqlBuilderNeuesWjahr();
void setDbFile(string db_location);
int openDB();
int insertHeader(void *, int, char **, char **);
int insertAtrributes(void *NotUsed, int argc, char **argv, char **azColName);
int insertTableDataInVector(void *NotUsed, int argc, char **argv, char **azColName);
int insertTableDataInString(void *NotUsed, int argc, char **argv, char **azColName);
vector<string> getHeader(const char* table);
std::vector<std::vector<string> > executeSqlStatement();
string executeSqlStatement_singleHit();
void deleteVectors();
void deleteColumns();

GtkWidget * createTableWithHeader();
GtkTreeModel * insertRecordsinTreeview ();
int getNumberOfRows();
const char* translatorHeader(std::string);

#endif