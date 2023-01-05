#include <gtk/gtk.h>
#include <iostream>
#include <sqlite3.h>
#include <vector>
#include <map>
#include <string.h>
#include <sstream>
#include <iomanip>
#include "lib/sql.h"
#include "lib/operations.h"
using namespace std;

enum headersAll
{
    // Baum
    COL_BAUMID, COL_WUCHSFORM, COL_URSPRUNG, COL_LIEFERDATUM, COL_PFLANZJAHR, COL_VEREDELTFLAG, COL_BEFRUCHTUNG, COL_ANMERKUNGEN, COL_PFLANZBODENZUSATZ, COL_UNTERLAGE, COL_RFIDCODE, COL_EHEMALIGERSTANDORTID, COL_AUSFALLURSACHE, COL_STATUS, COL_FRUCHTREIFEANFANG, COL_FRUCHTREIFEENDE, COL_BLUETEZEITANFANG, COL_BLUETEZEITENDE, COL_FREMDTRANSPORTFLAG,
    // Sorte
    COL_SORTENID, COL_OBERART, COL_UNTERART, COL_LATEINNAME, COL_BEFRUCHTUNG2, COL_BEFRUCHTERSORTE,
    // Standort
    COL_STANDORTID, COL_LICHT, COL_GELAENDE, COL_BODENTYP, COL_BODENFEUCHTIGKEIT, COL_GPSKOORDINATEN, COL_SCHAEDLINGFLAEG, COL_WAESSERUNGFLAG, COL_MAEHRENRUPFENFLAG, COL_SCHNEIDENFLAG, COL_DUENGENFLAG, COL_ASTBRUCHFLAG, COL_VERBISSFLAG, COL_FROSTSCHADENFLAG, COL_AKRIVINAKTIV, COL_KALKANSTRICHFLAG, COL_ERNTENFLAG, COL_LEIMRINGFLAG, COL_BODENAUFRAEUMENFLAG, COL_MUMIENFAULOBSTFLAG, COL_AUSDUENNENFLAG,
    // Wirtschaftsjahr
    COL_Wjahr, COL_ERTRAG, COL_AKTUELLEHOEHE, COL_ARCHIVGEWAESSERT, COL_ARCHIVGEMAEHTGERUPFT, COL_ARCHIVGESCHNITTEN, COL_ARCHIVFROSTSCHADENBEHANDELT, COL_ARCHIVASTBRUCHBEHANDELT, COL_ARCHIVVERBISSBEHANDELT, COL_ARCHIVKALKANGESTRICHEN, COL_ARCHIVLEIMRINGANGBRACHT, COL_AKTUELLERTAMMUMFANG, COL_LETZTGEWAESSERT, COL_LETZTGEMAEHTGERUPFT, COL_LETZTASTBRUCHBEHANDELT, COL_LETZTFROSTSCHADENBEHANDELT, COL_LETZTVERBISSBEHANDELT, COL_LETZTGESCHNITTEN, COL_LETZTKALKANGESTRICHEN, COL_LETZTLEIMANGEBRACHT,
    // Schaedling
    COL_SCHAEDLINGID, COL_DATUM, COL_SCHAEDLINGSART, COL_GEGENAKTION,
    // Duengemittel
    COL_DUENGEMITTELID, COL_DUENGEMITTEL,
    NUM_COLS
} ;

GtkWidget *view;

sqlite3 *db;
int rc, numberColumns, colCounter, rowCounter, numberRows;
char *err_msg = 0;
std::map<std::string,headersAll> mapHeader;
std::map<std::string,headersAll>::iterator it;
std::string sqlStatement;
vector<string> header;
vector<string> *vecSelectSql = new vector<string>();
std::vector<string> vecFromJoin;
std::vector<string> vecOn;
std::vector<string> vecWhereFeld;
string orderBy;
std::vector<std::vector<string> > vecRecords;
string record;
std::vector<std::vector<string> > vecWhereAttributes;
string db_file_location;

void saveSqlQuerry(string querry, int numColumns) {

    deleteVectors();
    numberColumns = numColumns;
    sqlStatement = querry;

    // std::cout << sqlStatement << std::endl;
}

// Start of sql routine
void sqlBuilderSelect(vector<string> select, vector<string> fromJoin, vector<string> on, vector<string> whereFeld, vector<vector<string> > whereAttributes, string order) {

    deleteVectors();
    *vecSelectSql = select;
    vecFromJoin = fromJoin;
    vecOn = on;
    vecWhereFeld = whereFeld;
    vecWhereAttributes = whereAttributes;
    orderBy = order;
    numberColumns = vecSelectSql->size();
    
    // SELECT
    sqlStatement = "SELECT ";
    if (vecSelectSql->empty()) {
        sqlStatement += "*";
    }else {
        sqlStatement += vecSelectSql->at(0);
        for (int i = 1; i < vecSelectSql->size(); i++) {   
            sqlStatement += " ," + vecSelectSql->at(i);
        }
    }

    // FROM
    sqlStatement += " FROM ";
    if (!vecFromJoin.empty()) {
        sqlStatement += vecFromJoin.at(0);
    }else {
        //error
        cout << "No Table Name";
    }

    // JOIN
    if (vecFromJoin.size() > 1) { 
        for (int j = 1; j < vecFromJoin.size(); j++) {
        sqlStatement += " JOIN "; 
        // ON
            sqlStatement += vecFromJoin.at(j) + " ON " + vecFromJoin.at(j-1) + "." + vecOn.at(j-1) + " = " + vecFromJoin.at(1) + "." + vecOn.at(j-1);
        }
    }

    //WHERE
    if (!vecWhereFeld.empty()) { // Checks if vector Feld is not empty
        sqlStatement += " WHERE " + vecWhereFeld[0];
    }

    if (vecWhereFeld.empty() && !vecWhereAttributes.empty()) { // if vector feld is empty && vector Attributes is NOT
        if (whereAttributes[0].empty()) {
                sqlStatement += " AND NOT (" + vecSelectSql->at(0); // checks if inner vector is empty 
        }else {
            sqlStatement += " WHERE (" + vecWhereAttributes[0].at(0);
        }
    }else if (!vecWhereFeld.empty() && !vecWhereAttributes.empty()) { // checks if both are NOT empty
        if (whereAttributes[0].empty()) {
                sqlStatement += " AND NOT (" + vecSelectSql->at(0); // checks if inner vector is empty
        }else {
            sqlStatement += " AND (" + vecWhereAttributes[0].at(0);
        }
    }

    for (int i = 1; i < vecWhereAttributes[0].size(); i++) {// checks if first vector has more than one argument
        sqlStatement += " OR " + vecWhereAttributes[0].at(i);
    }
    sqlStatement += ")";
    
    for (int i = 1; i < vecWhereAttributes.size(); i++){
        if (whereAttributes[i].empty()) {
                sqlStatement += " AND NOT (" + vecSelectSql->at(0) + ")"; // checks if inner vector is empty and contines
                continue;
        }else {
            sqlStatement += " AND (" + vecWhereAttributes[i].at(0);
        }
        
        for (int j = 1; j < vecWhereAttributes[i].size(); j++) {
            
            sqlStatement += " OR " + vecWhereAttributes[i].at(j);
        }
        sqlStatement += ")"; 
    }

    // ORDER BY
    if (orderBy != "") {
        sqlStatement += " ORDER BY " + orderBy;
    }

    // std::cout << sqlStatement << std::endl;
    // std::cout << "\n";
     
}

int sqlBuilderInsertBaum(string* arrBaumData) {

    deleteVectors();
    //SortenID bestimmen aus Ober und Unterart
    sqlStatement = "SELECT SortenID FROM Sorte WHERE OberArt = \"" + arrBaumData[1] +"\" AND UnterArt = \"" + arrBaumData[2] + "\"";
    vector<vector<string> > vecSortenID = executeSqlStatement();
    // Abort if OberArt and UnterArt are no match
    if (vecSortenID.size() == 0) { 
        return 0; 
    }
    
    // SETS 'Fruchtreife Ende' and 'Bluete Ende' if empty
    if (arrBaumData[17] == "") { arrBaumData[17] = arrBaumData[16]; }
    if (arrBaumData[19] == "") { arrBaumData[19] = arrBaumData[18]; }

    // INSERT INTO Baum
    arrBaumData[2] = vecSortenID[0].at(0);
    sqlStatement = "INSERT INTO Baum (SortenID, StandortID, WuchsForm, Ursprung, Lieferdatum, PflanzJahr, VeredeltFlag, Befruchtung, Anmerkungen, PflanzBodenzusatz, Unterlage, RfidCode, Status, FremdTransportFlag, FruchtReifeAnfang, FruchtReifeEnde, BlueteZeitAnfang, BlueteZeitEnde) VALUES (";
    sqlStatement += arrBaumData[2];
    for (int i = 3; i < 20; i++) {   
        if (arrBaumData[i] == "" || arrBaumData[i] == "Kein Standort") {
            sqlStatement += ", NULL";
            continue;
        }
        if (i == 3 || i >= 16 ) {
            sqlStatement += ", " + arrBaumData[i];
        }else {
            sqlStatement += ", '" + arrBaumData[i] + "'";
        }
    }
    sqlStatement += ")";

    // cout << sqlStatement << endl;
    // cout << "\n";
    return 1;
}   

int sqlBuilderInsertStaOrt(string* arrStaOrtData) {

    deleteVectors();

    //INSERT INTO  Standort
    sqlStatement = "INSERT INTO Standort (StandortID, AktivInaktiv, Licht, Gelaende, BodenTyp, BodenFeuchtigkeit, GpsKoordinaten) VALUES (";
    sqlStatement += arrStaOrtData[2] + ", " + arrStaOrtData[3];
    for (int i = 4; i < 9; i++) {
        if (arrStaOrtData[i] == "") {
            sqlStatement += ", NULL";
            continue;
        }else {
            sqlStatement += ", '" + arrStaOrtData[i] + "'";
        }
    }
    sqlStatement += ")";

    // cout << sqlStatement << endl;
    // cout << "\n";
    
    return 1;

}

void sqlBuilderUpdateStandort() {
    string* arrStaOrtData = getArrayStaOrtData();

    string standortUF = "F" + arrStaOrtData[0] + "R" + arrStaOrtData[1] + "P" + arrStaOrtData[2];
    string standortPF = modStandortPF(standortUF);

    saveSqlQuerry("SELECT AktivInaktiv, Licht, Gelaende, BodenTyp, BodenFeuchtigkeit, GpsKoordinaten FROM Standort WHERE StandortID = " + standortPF, 6);
    vector<vector<string> > vecStaOrtData = executeSqlStatement();

    if (arrStaOrtData[3] == "") { arrStaOrtData[3] = vecStaOrtData[0].at(0); }
    for (int i = 4; i < 9; i++) {
        if (arrStaOrtData[i] == "") { 
            if (vecStaOrtData[0].at(i-3) == "") {
                arrStaOrtData[i] = "NULL";
            }else {
                arrStaOrtData[i] = "\"" + vecStaOrtData[0].at(i-3) + "\""; 
            }
        }else {
            arrStaOrtData[i] = "\"" + arrStaOrtData[i] + "\"";
        }
    }
    
    sqlStatement = "UPDATE Standort SET AktivInaktiv=" + arrStaOrtData[3] + ", Licht=" + arrStaOrtData[4] + ", Gelaende=" + arrStaOrtData[5] + ", BodenTyp=" + arrStaOrtData[6] + ", BodenFeuchtigkeit=" + arrStaOrtData[7] + ", GpsKoordinaten=" + arrStaOrtData[8] + " WHERE StandortID=" + standortPF;
    executeSqlStatement();
    // cout << sqlStatement << endl;
}

void sqlBuilderUpdateBaum() {

    string* arrBaumData = getArrayBaumData();

    // Get data from joined Baum and Sorte
    string sql = "SELECT BaumID, OberArt, UnterArt, StandortID, WuchsForm, Ursprung, Lieferdatum, PflanzJahr, VeredeltFlag, Befruchtung, Anmerkungen, PflanzBodenzusatz, Unterlage, RfidCode, Status, FremdTransportFlag, FruchtReifeAnfang, FruchtReifeEnde, BlueteZeitAnfang, BlueteZeitEnde FROM Baum, Sorte WHERE Baum.SortenID = Sorte.SortenID AND BaumID=" + arrBaumData[0];
    saveSqlQuerry(sql, 20);
    vector<vector<string> > vecJoinBaumSorte = executeSqlStatement();

    //Check for user made changes
    for (int i = 1; i < 20; i++) {
        if (arrBaumData[i] == "") {
            if (vecJoinBaumSorte[0].at(i) == "") {
                arrBaumData[i] = "NULL";
            }else {
                arrBaumData[i] = "\"" + vecJoinBaumSorte[0].at(i) + "\"";
            }
        }else {
                arrBaumData[i] = "\"" + arrBaumData[i] + "\"";
        }
    }
    
    // Get SortenID
    sql = "SELECT SortenID FROM Sorte WHERE OberArt=" + arrBaumData[1] + " AND UnterArt=" + arrBaumData[2];
    saveSqlQuerry(sql);
    vector<vector<string> > vecSortenID = executeSqlStatement();
    
    //Update Data
    sqlStatement = "UPDATE Baum SET SortenID=" + vecSortenID[0].at(0) + ", StandortID=" + arrBaumData[3] + ", WuchsForm=" + arrBaumData[4] + ", Ursprung=" + arrBaumData[5] + ", Lieferdatum=" + arrBaumData[6] + ", PflanzJahr=" + arrBaumData[7] + ", VeredeltFlag=" + arrBaumData[8] + ", Befruchtung=" + arrBaumData[9] + ", Anmerkungen=" + arrBaumData[10] + ", PflanzBodenzusatz=" + arrBaumData[11] + ", Unterlage=" + arrBaumData[12] + ", RfidCode=" + arrBaumData[13] + ", Status=" + arrBaumData[14] + ", FremdTransportFlag=" + arrBaumData[15] + ", FruchtReifeAnfang=" + arrBaumData[16] + ", FruchtReifeEnde=" + arrBaumData[17] + ", BlueteZeitAnfang=" + arrBaumData[18] + ", BlueteZeitEnde=" + arrBaumData[19] + " WHERE BaumID=" + arrBaumData[0];
    executeSqlStatement();
}

void sqlBuilderUpdateSorten() {
    string* arrSortenVerwData = getArraySortenVerwData();

    saveSqlQuerry("SELECT LateinName, Befruchtung2, BefruchterSorte FROM Sorte WHERE OberArt='" + arrSortenVerwData[0] + "' AND UnterArt='" + arrSortenVerwData[1] + "'", 3);
    vector<vector<string> > vecSortenData = executeSqlStatement();

    for (int i = 2; i < 5; i++) {
        if (arrSortenVerwData[i] == "") {
            arrSortenVerwData[i] = "\"" + vecSortenData[0].at(i-2) + "\"";
        }else if (arrSortenVerwData[i] == "...löschen...") {
            arrSortenVerwData[i] = "NULL";
        }
        else {
            arrSortenVerwData[i] = "\"" + arrSortenVerwData[i] + "\"";
        }
    }
    sqlStatement = "UPDATE Sorte SET LateinName=" + arrSortenVerwData[2] + ", Befruchtung2=" + arrSortenVerwData[3] + ", BefruchterSorte=" + arrSortenVerwData[4] + " WHERE OberArt='" + arrSortenVerwData[0] + "' AND UnterArt='" + arrSortenVerwData[1] + "'";
    // cout << sqlStatement << endl;
}

void sqlBuilderNeuesWjahr() {

    string* arrWjahrData = getArrayWjahrData();
    string neuesWjahr = arrWjahrData[0];
    saveSqlQuerry("SELECT BaumID, StandortID FROM Baum WHERE Status='aktiv'", 2);
    vector<vector<string> > vecBaumData = executeSqlStatement();

    for (int i = 0; i < vecBaumData.size(); i++) {
        saveSqlQuerry("INSERT INTO WirtschaftsJahr (WJahr, BaumID, StandortID) VALUES (" + neuesWjahr + ", " + vecBaumData[i].at(0) + ", " + vecBaumData[i].at(1) + ")");
        executeSqlStatement();        
    }
    saveSqlQuerry("UPDATE Baum SET Status='archiv' WHERE Status='inaktiv'");
    executeSqlStatement();
}

// Genertes treeview with Metadata
GtkWidget * createTableWithHeader (void)
{
    GtkCellRenderer     *renderer;
    GtkTreeModel        *model;
    
    // const char *table = "Baum";
    // getHeader(table);

    //fill map with attributes
    //Baum
    mapHeader["BaumID"]=COL_BAUMID;
    mapHeader["WuchsForm"]=COL_WUCHSFORM;
    mapHeader["Ursprung"]=COL_URSPRUNG;
    mapHeader["Lieferdatum"]=COL_LIEFERDATUM;
    mapHeader["PflanzJahr"]=COL_PFLANZJAHR;
    mapHeader["VeredeltFlag"]=COL_VEREDELTFLAG;
    mapHeader["Befruchtung"]=COL_BEFRUCHTUNG;
    mapHeader["Anmerkungen"]=COL_ANMERKUNGEN;
    mapHeader["PflanzBodenzusatz"]=COL_PFLANZBODENZUSATZ;
    mapHeader["Unterlage"]=COL_UNTERLAGE;
    mapHeader["RfidCode"]=COL_RFIDCODE;
    mapHeader["EhemaligeStandortID"]=COL_EHEMALIGERSTANDORTID;
    mapHeader["AusfallUrsache"]=COL_AUSFALLURSACHE;
    mapHeader["Status"]=COL_STATUS;
    mapHeader["FruchtReifeAnfang"]=COL_FRUCHTREIFEANFANG;
    mapHeader["FruchtReifeEnde"]=COL_FRUCHTREIFEENDE;
    mapHeader["BlueteZeitAnfang"]=COL_BLUETEZEITANFANG;
    mapHeader["BlueteZeitEnde"]=COL_BLUETEZEITENDE;
    mapHeader["FremdTransportFlag"]=COL_FREMDTRANSPORTFLAG;
    //Sorte
    mapHeader["SortenID"]=COL_SORTENID;
    mapHeader["OberArt"]=COL_OBERART;
    mapHeader["UnterArt"]=COL_UNTERART;
    mapHeader["LateinName"]=COL_LATEINNAME;
    mapHeader["Befruchtung2"]=COL_BEFRUCHTUNG2;
    mapHeader["BefruchterSorte"]=COL_BEFRUCHTERSORTE;
    //Standort
    mapHeader["StandortID"]=COL_STANDORTID;
    mapHeader["Licht"]=COL_LICHT;
    mapHeader["Gelaende"]=COL_GELAENDE;
    mapHeader["BodenTyp"]=COL_BODENTYP;
    mapHeader["BodenFeuchtigkeit"]=COL_BODENFEUCHTIGKEIT;
    mapHeader["GpsKoordinaten"]=COL_GPSKOORDINATEN;
    mapHeader["SchaedlingFlag"]=COL_SCHAEDLINGFLAEG;
    mapHeader["WaesserungFlag"]=COL_WAESSERUNGFLAG;
    mapHeader["MaehrenRupfenFlag"]=COL_MAEHRENRUPFENFLAG;
    mapHeader["SchneidenFlag"]=COL_SCHNEIDENFLAG;
    mapHeader["DuengenFlag"]=COL_DUENGENFLAG;
    mapHeader["AstbruchFlag"]=COL_ASTBRUCHFLAG;
    mapHeader["VerbissFlag"]=COL_VERBISSFLAG;
    mapHeader["FrostschadenFlag"]=COL_FROSTSCHADENFLAG;
    mapHeader["AktivInaktiv"]=COL_AKRIVINAKTIV;
    mapHeader["KalkAnstrichFlag"]=COL_KALKANSTRICHFLAG;
    mapHeader["ErntenFlag"]=COL_ERNTENFLAG;
    mapHeader["LeimringFlag"]=COL_LEIMRINGFLAG;
    mapHeader["BodenaufraeumenFlag"]=COL_BODENAUFRAEUMENFLAG;
    mapHeader["Mumien/FaulobstentfernenFlag"]=COL_MUMIENFAULOBSTFLAG;
    mapHeader["AusduennenFlag"]=COL_AUSDUENNENFLAG;
    //Wjahr
    mapHeader["WJahr"]=COL_Wjahr;
    mapHeader["Ertrag"]=COL_ERTRAG;
    mapHeader["AktuelleHoehe"]=COL_AKTUELLEHOEHE;
    mapHeader["ArchivGewaessert"]=COL_ARCHIVGEWAESSERT;
    mapHeader["ArchivGemaehtGerupft"]=COL_ARCHIVGEMAEHTGERUPFT;
    mapHeader["ArchivGeschnitten"]=COL_ARCHIVGESCHNITTEN;
    mapHeader["ArchivFrostschadenbehandelt"]=COL_ARCHIVFROSTSCHADENBEHANDELT;
    mapHeader["ArchivAstbruchbehandlet"]=COL_ARCHIVASTBRUCHBEHANDELT;
    mapHeader["ArchivVerbissbehandelt"]=COL_ARCHIVVERBISSBEHANDELT;
    mapHeader["ArchivKalkAngestrichen"]=COL_ARCHIVKALKANGESTRICHEN;
    mapHeader["ArchivLeimringangebracht"]=COL_ARCHIVLEIMRINGANGBRACHT;
    mapHeader["AktuellerStammumfang"]=COL_AKTUELLERTAMMUMFANG;
    mapHeader["LetztGewaessert"]=COL_LETZTGEWAESSERT;
    mapHeader["LetztGemaehtGerupft"]=COL_LETZTGEMAEHTGERUPFT;
    mapHeader["LetztAstbruchBehandelt"]=COL_LETZTASTBRUCHBEHANDELT;
    mapHeader["LetztFrostschadenBehandelt"]=COL_LETZTFROSTSCHADENBEHANDELT;
    mapHeader["LetztVerbissBehandelt"]=COL_LETZTVERBISSBEHANDELT;
    mapHeader["LetztGeschnitten"]=COL_LETZTGESCHNITTEN;
    mapHeader["LetztKalkAngestrichen"]=COL_LETZTGESCHNITTEN;
    mapHeader["LetztLeimAngebracht"]=COL_LETZTLEIMANGEBRACHT;
    // Schaedling
    mapHeader["SchaedlingID"]=COL_SCHAEDLINGID;
    mapHeader["Datum"]=COL_DATUM;
    mapHeader["SchaedlingsArt"]=COL_SCHAEDLINGSART;
    mapHeader["GegenAktion"]=COL_GEGENAKTION;
    //Duengemittel
    mapHeader["DuengemittelID"]=COL_DUENGEMITTELID;
    mapHeader["Duengemittel"]=COL_DUENGEMITTEL;

    view = gtk_tree_view_new ();

    // loop to fill in the headers
    for (int i = 0; i < numberColumns; i++) {
        it = mapHeader.find(vecSelectSql->at(i));
        const gchar *colHeader = translatorHeader(vecSelectSql->at(i));
        // cout << colHeader << " = " << typeid(colHeader).name() <<endl;
        renderer = gtk_cell_renderer_text_new ();
        gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                                    -1,      
                                                    colHeader,  
                                                    renderer,
                                                    "text", it->second,
                                                    NULL);
    }

    // Inserts data into table
    model = insertRecordsinTreeview ();

    // deleteColumns ();

    gtk_tree_view_set_model (GTK_TREE_VIEW (view), model);

    /* The tree view has acquired its own reference to the
    *  model, so we can drop ours. That way the model will
    *  be freed automatically when the tree view is destroyed */
    g_object_unref (model);

    return view;
}

//Fills treeview
GtkTreeModel * insertRecordsinTreeview ()
{
    GtkListStore  *store;
    GtkTreeIter    iter;

    executeSqlStatement();
    
    // There must be a better solution for this. Number of G_TYPE_STRING define max number of columns. Insert more for bigger tables
    store = gtk_list_store_new (NUM_COLS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

    // append another row
    for (int i = 0; i < numberRows; i++) {
        gtk_list_store_append (store, &iter);
        // fill in data
        std::string sto;
        for (int j = 0;  j< numberColumns; j++) {
        it = mapHeader.find(vecSelectSql->at(j));
        if (it->first == "StandortID" || it->first == "EhemaligeStandortID") {
            sto = modStandortUF(vecRecords[i].at(j));
        }else {
            sto = vecRecords[i].at(j);
        }
        const char *cell = sto.c_str();
        
        gtk_list_store_set (store, &iter, it->second, cell, -1);
        }
    }
    
    return GTK_TREE_MODEL (store);
}

// executes saved SQL querry and returns 2D vector with TableData for SELECT querries
std::vector<std::vector<string> > executeSqlStatement() {

    deleteVectors();
    openDB();
    rowCounter = 0;
    const char *sql = sqlStatement.c_str();
    rc = sqlite3_exec(db, sql, insertTableDataInVector, 0, &err_msg);
    numberRows = vecRecords.size();

    return vecRecords;
}
// inserts table data into vector
int insertTableDataInVector(void *NotUsed, int argc, char **argv, char **azColName) {    

    /*for (int i = 0; i < argc; i++) {

        if (!argv[i]) {
            cout<< azColName[i]<<" = "<< "NULL" << "\n";
            continue;
        }
        cout<< azColName[i]<<" = "<< argv[i] << "\n";
        // cout<< azColName[i]<<" = "<< argv[i] ? argv[i] :"NULL" <<"\n";
    }
    
    cout<<"\n";*/
    
    vecRecords.push_back(std::vector<std::string>());
    for (int i = 0; i < numberColumns; i++)
    {   
        if (!argv[i]) {
            vecRecords[rowCounter].push_back("");
            continue;
        }
        vecRecords[rowCounter].push_back(argv[i]);
    }
    rowCounter++;    
    
    return 0;
}

string executeSqlStatement_singleHit() {

    deleteVectors();
    openDB();
    const char *sql = sqlStatement.c_str();
    rc = sqlite3_exec(db, sql, insertTableDataInString, 0, &err_msg);
    

    return record;
}

int insertTableDataInString(void *NotUsed, int argc, char **argv, char **azColName) {
    
     /*for (int i = 0; i < argc; i++) {

        if (!argv[i]) {
            cout<< azColName[i]<<" = "<< "NULL" << "\n";
            continue;
        }
        cout<< azColName[i]<<" = "<< argv[i] << "\n";
    }
    
    cout<<"\n";*/

    if (!argv[0]) {
        record = "";
        return 0;
    }
    record = argv[0]; 
    
    return 0;
}

//returns number of rows for Zähler anzeige
int getNumberOfRows() {

    return numberRows;
}

const char* translatorHeader(std::string dbHeader) {

    const char *headernew;

    std::map<std::string,const char *> mapHeaderTrans;
    std::map<std::string,const char *>::iterator itTrans;
    
    mapHeaderTrans["BaumID"]="BaumNr.";
    mapHeaderTrans["StandortID"]="Standort";
    mapHeaderTrans["OberArt"]="Art";
    mapHeaderTrans["UnterArt"]="Sorte";
    mapHeaderTrans["WuchsForm"]="Wuchsform";
    mapHeaderTrans["Ursprung"]="Herkunft";
    mapHeaderTrans["Lieferdatum"]="Lieferdatum";
    mapHeaderTrans["PflanzJahr"]="Pflanzjahr";
    mapHeaderTrans["VeredeltFlag"]="Veredelt";
    mapHeaderTrans["Befruchtung"]="Befruchtung";
    mapHeaderTrans["Befruchtung2"]="Befruchtung";
    mapHeaderTrans["Anmerkungen"]="Anmerkungen";
    mapHeaderTrans["PflanzBodenzusatz"]="Pflanzbodenzusatz";
    mapHeaderTrans["Unterlage"]="Unterlage";
    mapHeaderTrans["RfidCode"]="RFIDKennung";
    mapHeaderTrans["EhemaligeStandortID"]="Ehemaliger Standort";
    mapHeaderTrans["AusfallUrsache"]="Ausfallursache";
    mapHeaderTrans["Status"]="Status";
    mapHeaderTrans["FruchtReifeAnfang"]="Fruchtreife Anfang";
    mapHeaderTrans["FruchtReifeEnde"]="Fruchtreife Ende";
    mapHeaderTrans["BlueteZeitAnfang"]="Blüte Anfang";
    mapHeaderTrans["BlueteZeitEnde"]="Blüte Ende";
    mapHeaderTrans["FremdTransportFlag"]="Anlieferung";
     
    itTrans =mapHeaderTrans.find(dbHeader);
    headernew = itTrans->second;
    

    return headernew;


    // const char *header;

    // if (dbHeader == "BaumID") { header = "Bom"; }
    // else if (dbHeader == "StandortID") { header = "Standort"; }
    // else { header = dbHeader.c_str(); }
    
    // return header;
}

// deletes all Data from vectors
void deleteVectors() {
    
    //*vecSelectSql->erase();
    vecFromJoin.clear();
    vecOn.clear();
    vecRecords.clear();
    vecRecords.resize(0);
}

int openDB() {

    char db_file [255] = "";

    if(db_file_location.length() == 0){

        //only for convenience while programming, uncomment error message
        // char test_file [30] = "TestdatenV2.db";
        // strcpy(db_file, test_file);

        // error message
        GtkWidget *dialog_window;
        dialog_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_position(GTK_WINDOW(dialog_window), GTK_WIN_POS_CENTER);
        runDialog(dialog_window, "Es wurde keine Datenbank ausgewählt.\nBitte öffnen Sie erst eine Datenbank im Hauptmenü.");
        
    }
    else{
        strcpy(db_file, db_file_location.c_str());
    }

    rc = sqlite3_open(db_file, &db);

    if (rc != SQLITE_OK) {
        
        cout<<"Cannot open database: "<<sqlite3_errmsg(db);
        sqlite3_close(db);
        
        return 1;
    }
    return 0;
}

void setDbFile(string db_location){
    db_file_location = db_location;
}

//returns vector with name of headers
vector<string> getHeader(const char* table) {
        
    openDB();
    char sql[50];
    snprintf(sql, 50, "PRAGMA table_info(%s)", table);
    rc = sqlite3_exec(db, sql, insertHeader, 0, &err_msg);
    char sql2[50] = "PRAGMA table_info(Sorte)";
    rc = sqlite3_exec(db, sql2, insertHeader, 0, &err_msg);
    
    if (rc != SQLITE_OK ) {
        
        cout<< "Failed to select data\n";
        cout<< "SQL error: "<< err_msg;
        cout<< "evtl. nicht genug Speicher für sql querrie reserviert\n";

        sqlite3_free(err_msg);
        sqlite3_close(db);
    } 
    
    sqlite3_close(db);

    return header;
}

int insertHeader(void *NotUsed, int argc, char **argv, char **azColName) {    

    string nameHeader = argv[1];    
    header.push_back(nameHeader);

    return 0;
}

/*void deleteColumns() {

    gtk_tree_view_remove_column (GTK_TREE_VIEW(view), gtk_tree_view_get_column (GTK_TREE_VIEW(view), 26));
    gtk_tree_view_remove_column (GTK_TREE_VIEW(view), gtk_tree_view_get_column (GTK_TREE_VIEW(view), 24));
    gtk_tree_view_remove_column (GTK_TREE_VIEW(view), gtk_tree_view_get_column (GTK_TREE_VIEW(view), 21));
    gtk_tree_view_remove_column (GTK_TREE_VIEW(view), gtk_tree_view_get_column (GTK_TREE_VIEW(view), 19));
    gtk_tree_view_remove_column (GTK_TREE_VIEW(view), gtk_tree_view_get_column (GTK_TREE_VIEW(view), 18));
    gtk_tree_view_remove_column (GTK_TREE_VIEW(view), gtk_tree_view_get_column (GTK_TREE_VIEW(view), 17));
    gtk_tree_view_remove_column (GTK_TREE_VIEW(view), gtk_tree_view_get_column (GTK_TREE_VIEW(view), 16));
    gtk_tree_view_remove_column (GTK_TREE_VIEW(view), gtk_tree_view_get_column (GTK_TREE_VIEW(view), 14));
    gtk_tree_view_remove_column (GTK_TREE_VIEW(view), gtk_tree_view_get_column (GTK_TREE_VIEW(view), 13));
    gtk_tree_view_remove_column (GTK_TREE_VIEW(view), gtk_tree_view_get_column (GTK_TREE_VIEW(view), 12));
    gtk_tree_view_remove_column (GTK_TREE_VIEW(view), gtk_tree_view_get_column (GTK_TREE_VIEW(view), 11));
    gtk_tree_view_remove_column (GTK_TREE_VIEW(view), gtk_tree_view_get_column (GTK_TREE_VIEW(view), 10));
    gtk_tree_view_remove_column (GTK_TREE_VIEW(view), gtk_tree_view_get_column (GTK_TREE_VIEW(view), 9));
    gtk_tree_view_remove_column (GTK_TREE_VIEW(view), gtk_tree_view_get_column (GTK_TREE_VIEW(view), 1));

}*/