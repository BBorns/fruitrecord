#include <gtk/gtk.h>
#include <iostream>
#include <string>
#include <string.h>
#include <sstream>
#include <iomanip>
#include <vector>
#include <fstream>
#include <ctime>
#include <filesystem>
#include <algorithm>
#if _WIN32
  #include <windows.h>
  #include <shellapi.h>
  #include <filesystem>
#elif __APPLE__
  #include <stdlib.h>
#elif __linux__
  #include <stdlib.h>
#endif
#include "lib/operations.h"
#include "lib/sql.h"
using namespace std;

std::vector<std::vector<std::string> > vecUserStandortAuswahl;
std::vector<std::vector<std::string> > vecUserBaumAuswahl;

string arrBaumData[] = {"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""};
string arrStaOrtData[] = {"1", "", "", "", "", "", "", "", ""};
string arrSortenVerwData[] = {"", "", "", "", ""};
string arrWjahrData[] = {"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""};
string arrBaumProfil[] = {"", "", ""};
string whereFeld = "StandortID BETWEEN 101000000 AND 102000000";
string* ptrWhereFeld = &whereFeld;
string insertStatus = "";
string transfer = "";
string program_directory = "";
string photoJahr = "";
int wjahr = 0;

int string_to_int(string str) {
    
    int number;
    stringstream ss;
    ss << str;
    ss >> number;
    
    return number;
}
int string_to_int_minusHundred(string str, int position, int size) {
    
    std::string subString = str.substr(position, size);
    int number;
    stringstream ss;
    ss << subString;
    ss >> number;
    number -= 100;
    
    return number;
}
int string_to_int_plusHundred(string str) {

    int number;
    stringstream ss;
    ss << str;
    ss >> number;
    number += 100;
    
    return number;
}

//takes 2D vector, number of columns and returns formated strings
string string_formater(vector<vector<string> > vecStr2D, int numCols) {

    string str;
    // stringstream a_stream;
    for (int i = 0; i < vecStr2D.size(); i++) {
        for (int j = 0; j < vecStr2D[i].size(); j++) {
            if (i%numCols == 0) { str += "\n"; }
            stringstream a_stream;
            a_stream << std::setiosflags ( std::ios_base::right ) << std::setw ( 5 ) << vecStr2D[i].at(j);
            str += a_stream.str() + ", ";
            // felder += (i%numCols != 0) ? "\t" + vecStr2D[i].at(j) : "\n" + vecStr2D[i].at(j);
        }
    }
    // int pos = str.find("\n");
    // str = str.substr(pos + 1);

    int length = str.size();
    if (length > 0) { str = str.erase(length-2, 2); }
    
    return str;
}

string int_to_string(int numberInt) {
    
    std::string numberString;
    stringstream ss;
    ss << numberInt;
    ss >> numberString;

    return numberString;
}

// erases the length of string b from the beginning of string a
string substract_string(string a, string b){
    a.erase (0,b.length());
    return a;
}

int find_lowest_number(vector<int> vecNumber) {

    int lowestNumber = 0;
    for (int i = 0; i < vecNumber.size(); i++) {
        lowestNumber = (vecNumber.at(i) < lowestNumber) ? vecNumber.at(i) : lowestNumber;
    }
    return lowestNumber;
}

void set_main_directory(char cwd[256]){
    program_directory = cwd;
}
string get_main_directory(){
    return program_directory;
}

// sort a vector of int in decending order
vector<int> revers_sort(vector<int> nr_sort){
    std::sort(nr_sort.rbegin(), nr_sort.rend());
    return nr_sort;
}

bool isStrNumber(string str) {
    
    if (str.length() == 0) {
        return FALSE;
    }

    int i = str.length() -1;
    // cout << str << " : " << int(str[i]) << endl;
    if ((int(str[i]) == 48 || int(str[i]) == 49 || int(str[i]) == 50 || int(str[i]) == 51 || int(str[i]) == 52 || int(str[i]) == 53 || int(str[i]) == 54 || int(str[i]) == 55 || int(str[i]) == 56 || int(str[i]) == 57  || int(str[i]) == 45) && !(int(str[i]) == 45 && str.length() > 1)) {
        return TRUE;
    }
        

    return FALSE;
}

vector<vector<string> > getAttributeStandortChoice(bool *staOrt_Auswahl, string *attributeEquals, int sizeArrays) {
    
    vecUserStandortAuswahl.clear();
    vecUserStandortAuswahl.resize(0);
    for (int h = 0; h < 5; h++) { 
        vecUserStandortAuswahl.push_back(std::vector<std::string>()); 
        }
    for (int i = 0; i <= 3; i++) {
        if (staOrt_Auswahl[i] == TRUE) {
            vecUserStandortAuswahl[0].push_back(attributeEquals[i]);
            }
    }
    for (int i = 4; i <= 7; i++) {
        if (staOrt_Auswahl[i] == TRUE) {
            vecUserStandortAuswahl[1].push_back(attributeEquals[i]);
            }
    }
    for (int i = 8; i <= 15; i++) {
        if (staOrt_Auswahl[i] == TRUE) {
            vecUserStandortAuswahl[2].push_back(attributeEquals[i]);
            } 
    }

    for (int i = 16; i <= 19; i++) {
        if (staOrt_Auswahl[i] == TRUE) {
            vecUserStandortAuswahl[3].push_back(attributeEquals[i]);
            }
    }

    for (int i = 20; i <= 25; i++) {
        if (staOrt_Auswahl[i] == TRUE) {
            vecUserStandortAuswahl[4].push_back(attributeEquals[i]);
            }
    }

    return vecUserStandortAuswahl;

    // for (int i = 0; i < vecUserStandortAuswahl[4].size(); i++) {
    //     std::cout << vecUserStandortAuswahl[4].at(i) << std::endl;
    // }
    //     std::cout << "\n";
    
}

vector<vector<string> > getAttributeBaumChoice(bool *baum_Auswahl, string *attributeEquals, int sizeArrays) {

    vecUserBaumAuswahl.clear();
    vecUserBaumAuswahl.resize(0);
    for (int h = 0; h < 6; h++) { 
        vecUserBaumAuswahl.push_back(std::vector<std::string>()); 
        }
    for (int i = 0; i <= 24; i++) {
        if (baum_Auswahl[i] == TRUE) {
            vecUserBaumAuswahl[0].push_back(attributeEquals[i]);
            }
    }

    for (int i = 25; i <= 30; i++) {
        if (baum_Auswahl[i] == TRUE) {
            vecUserBaumAuswahl[1].push_back(attributeEquals[i]);
            }
    }

    for (int i = 31; i <= 33; i++) {
        if (baum_Auswahl[i] == TRUE) {
            vecUserBaumAuswahl[2].push_back(attributeEquals[i]);
            }
    }
    
    for (int i = 34; i <= 37; i++) {
        if (baum_Auswahl[i] == TRUE) {
            vecUserBaumAuswahl[3].push_back(attributeEquals[i]);
            }
    }

    for (int i = 38; i <= 44; i++) {
        if (baum_Auswahl[i] == TRUE) {
            vecUserBaumAuswahl[4].push_back(attributeEquals[i]);
            }
    }

    for (int i = 45; i <= 47; i++) {
        if (baum_Auswahl[i] == TRUE) {
            vecUserBaumAuswahl[5].push_back(attributeEquals[i]);
            }
    }

    return vecUserBaumAuswahl;
}

// returns number of unchecked attributes
string dropdownChecker(bool *staOrt_Auswahl, int startIndex, int endIndex) {

    int numberFalseInt = 0;
    for (int i = startIndex; i <= endIndex ; i++)
    {
        if (staOrt_Auswahl[i] == FALSE) {
            numberFalseInt++;
        }
    }

    if (numberFalseInt == 0) {
        
        return "";
    }else {
        string numberFalseStr = "(" + int_to_string(numberFalseInt) + ")";

        return numberFalseStr;
    }
}

string modStandortUF(string staOrt) {
    
    if (staOrt != "") {
        std::string sto = "";
        string frp[3] = {"F", "R", "P"};
        for (int k = 0; k < 3; k++) {
            int numberInt = string_to_int_minusHundred(staOrt,k*3, 3);
            std::string numberString = int_to_string(numberInt);
            int size = numberString.length();
            if (size == 1)
            {
                numberString = "  " + numberString;
            }else if (numberInt < 0) {
                numberString = " " + numberString;
            }
            
            sto += frp[k] + numberString + "   ";
        }
        return sto;
    }

    return "";
    
}

string modStandortPF(string staOrt) {
    int currentIndex = 0;

    string strNumber_tmp = "";
    string strNumber = "";
    int intNumber = 0;

    if (staOrt == "Kein Standort") { return "Kein Standort"; }

    for (int i = 0; i < staOrt.length(); i++){
        // If i is 0-9 or '-'
        if (int(staOrt[i]) == 48 || int(staOrt[i]) == 49 || int(staOrt[i]) == 50 || int(staOrt[i]) == 51 || int(staOrt[i]) == 52 || int(staOrt[i]) == 53 || int(staOrt[i]) == 54 || int(staOrt[i]) == 55 || int(staOrt[i]) == 56 || int(staOrt[i]) == 57 || int(staOrt[i]) == 45)
        {
            strNumber_tmp += staOrt[i];
        }
        if (int(staOrt[i]) == 80 || int(staOrt[i]) == 82) {
            intNumber = string_to_int(strNumber_tmp);
            intNumber += 100;
            string rNum = int_to_string(intNumber);
            if (rNum.length() == 1) {rNum = "00" + rNum;}
            else if (rNum.length() == 2) {rNum = "0" + rNum;}
            strNumber += rNum;
            strNumber_tmp = "";
            intNumber = 0;
        }  
    }
    intNumber = string_to_int(strNumber_tmp);
    intNumber += 100;
    string pNum = int_to_string(intNumber);
    if (pNum.length() == 1) {pNum = "00" + pNum;}
    else if (pNum.length() == 2) {pNum = "0" + pNum;}
    strNumber += pNum;
    
    strNumber_tmp = "";
    intNumber = 0;
    
    return strNumber;
}

string zahl_to_status(string num) {

    string status;
    if (num == "0") { status = "nicht zu verwenden"; }
    else if (num == "1") { status = "aktiv und besetzt"; }
    else if (num == "2") { status = "Unterlage vorhanden"; }
    else if (num == "3") { status = "zu inaktivieren"; }
    else if (num == "4") { status = "aktiv und frei"; }

    return status;
}

void saveDataToFile(vector<vector<string> > vec, string filenameWiExt) {

    string filename("./print/" + filenameWiExt);
    fstream file_out;
    
    file_out.open(filename, std::ios_base::out);
    for (int i = 0; i < vec.size(); i++)
    {
        for (int j = 0; j < vec[0].size(); j++)
        {
            file_out << vec[i].at(j) << "     ";
        }
        file_out << "\n";
        
    }
}

string * getArrayBaumData() {
    return arrBaumData;
}
string * getArrayStaOrtData() {
    return arrStaOrtData;
}
string* getArraySortenVerwData() {

    return arrSortenVerwData;
}
string* getArrayWjahrData() {
    return arrWjahrData;
}
string* getWhereFeld() {
    return ptrWhereFeld;
}
string* getArrayBaumProfil() {
    return arrBaumProfil;
}

void setInsertStatus(string str) {
    insertStatus = str;
}
string getInsertStatus() {
    string inStat_cp = insertStatus;
    insertStatus = "";
    
    return inStat_cp;
}
void setTransfer(string str) {
    transfer = str;
}
string getTransfer() {
    string transfer_cp = transfer;
    transfer = "";
    
    return transfer_cp;
}

//Set Wirtschaftsjahr
void setWjahr(int _wjahr) {
    wjahr = _wjahr;
}
int getWjahr() {
    return wjahr;
}

//User Input 'Baum'
void entrybox_nrBaum_changed(GtkWidget *widget, gpointer data) {
    const gchar *attWert_tmp = gtk_entry_get_text(GTK_ENTRY(widget));
    string attWert = attWert_tmp;
    arrBaumData[0] = attWert;

}
void comboBox_artBaum_changed(GtkWidget *widget, gpointer data){
  
    gchar *attWert_tmp = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));
    string attWert = attWert_tmp;
    arrBaumData[1] = attWert;
    g_free(attWert_tmp);  

}
void comboBox_sorteBaum_selected(GtkWidget *widget, gpointer data) {

  gchar *attWert_tmp = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));
  string attWert = attWert_tmp;
  arrBaumData[2] = attWert;
  g_free(attWert_tmp);
}
void comboBox_standortBaum_changed(GtkWidget *widget, gpointer data) {

  gchar *attWert_tmp = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));
  string attWert = attWert_tmp;
  arrBaumData[3] = modStandortPF(attWert);
  g_free(attWert_tmp);
}
void comboBox_wuchsformBaum_changed(GtkWidget *widget, gpointer data) {

  gchar *attWert_tmp = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));
  string attWert = attWert_tmp;
  arrBaumData[4] = attWert;
  g_free(attWert_tmp);
}
void entryBox_herkunft_changed(GtkWidget *widget, gpointer data) {
    
  const gchar *attWert_tmp = gtk_entry_get_text(GTK_ENTRY(widget));
  string attWert = attWert_tmp;
  arrBaumData[5] = attWert;
}
void entryBox_lieferdatumBaum_changed(GtkWidget *widget, gpointer window) {

    int i = 6;
    const gchar *attWert_tmp = gtk_entry_get_text(GTK_ENTRY(widget));
    string attWert = attWert_tmp;
    
    if (attWert != "") {
        if (isStrNumber(attWert)) {
            arrBaumData[i] = attWert;
        }else {
            runDialog(window, "Wertebereich: 0-9");
            const char* oldEntry = arrBaumData[i].c_str();
            gtk_entry_set_text(GTK_ENTRY(widget), oldEntry);
        }
    }else {
        arrBaumData[i] = "";
    }
} 
void entryBox_pflanzjahrBaum_changed(GtkWidget *widget, gpointer window) {

    int i = 7;
    const gchar *attWert_tmp = gtk_entry_get_text(GTK_ENTRY(widget));
    string attWert = attWert_tmp;
    
    if (attWert != "") {
        if (isStrNumber(attWert)) {
            arrBaumData[i] = attWert;
        }else {
            runDialog(window, "Wertebereich: 0-9");
            const char* oldEntry = arrBaumData[i].c_str();
            gtk_entry_set_text(GTK_ENTRY(widget), oldEntry);
        }
    }else {
        arrBaumData[i] = "";
    }

} 
void comboBox_veredeltBaum_changed(GtkWidget *widget, gpointer data) {

    gchar *attWert_tmp = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));
    string attWert = attWert_tmp;
    arrBaumData[8] = attWert;
    g_free(attWert_tmp);
}
void comboBox_befruchtungBaum_changed(GtkWidget *widget, gpointer data) {

    gchar *attWert_tmp = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));
    string attWert = attWert_tmp;
    arrBaumData[9] = attWert;
    g_free(attWert_tmp);    
} 
void entryBox_anmerkungenBaum_changed(GtkWidget *widget, gpointer data) {

    const gchar *attWert_tmp = gtk_entry_get_text(GTK_ENTRY(widget));
    string attWert = attWert_tmp;
    arrBaumData[10] = attWert;
} 
void entryBox_pflanzbodenzusatzBaum_changed(GtkWidget *widget, gpointer data) {

    const gchar *attWert_tmp = gtk_entry_get_text(GTK_ENTRY(widget));
    string attWert = attWert_tmp;
    arrBaumData[11] = attWert;
}
void entryBox_unterlageBaum_changed(GtkWidget *widget, gpointer window) {

    const gchar *attWert_tmp = gtk_entry_get_text(GTK_ENTRY(widget));
    string attWert = attWert_tmp;
    // Ignore entry if 'Veredelt' is "Nein"
    if (arrBaumData[8] == "Nein" && attWert != "") {
        runDialog(window, "Bitte das Feld 'Veredelt' auf 'Nein' setzen");
        gtk_entry_set_text(GTK_ENTRY(widget), "");
    } else {
        arrBaumData[12] = attWert;
    }
} 
void entryBox_rfidBaum_changed(GtkWidget *widget, gpointer data) {

    const gchar *attWert_tmp = gtk_entry_get_text(GTK_ENTRY(widget));
    string attWert = attWert_tmp;
    arrBaumData[13] = attWert;
} 
void comboBox_statusBaum_changed(GtkWidget *widget, gpointer data) {

    gchar *attWert_tmp = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));
    string attWert = attWert_tmp;
    arrBaumData[14] = attWert;
    g_free(attWert_tmp);  
} 
void comboBox_anlieferungBaum_changed(GtkWidget *widget, gpointer data) {

    gchar *attWert_tmp = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));
    string attWert = attWert_tmp;
    arrBaumData[15] = attWert;
    g_free(attWert_tmp);  
}
void entryBox_fruchtanfangBaum_changed(GtkWidget *widget, gpointer window) {

    int i = 16;
    const gchar *attWert_tmp = gtk_entry_get_text(GTK_ENTRY(widget));
    string attWert = attWert_tmp;
    
    if (attWert != "") {
        if (isStrNumber(attWert)) {
            if (plausiRange_ok(attWert, 1, 53)) {
                arrBaumData[i] = attWert;
            }else {
                runDialog(window, "Wertebereich: 1-53");
                const char* oldEntry = arrBaumData[i].c_str();
                gtk_entry_set_text(GTK_ENTRY(widget), oldEntry);
            }
        }else {
            runDialog(window, "Wertebereich: 1-53");
            const char* oldEntry = arrBaumData[i].c_str();
            gtk_entry_set_text(GTK_ENTRY(widget), oldEntry);
        }
    }else {
        arrBaumData[i] = "";
    }
} 
void entryBox_fruchtendeBaum_changed(GtkWidget *widget, gpointer window) {

    int i = 17;
    const gchar *attWert_tmp = gtk_entry_get_text(GTK_ENTRY(widget));
    string attWert = attWert_tmp;
    
    if (attWert != "") {
        if (isStrNumber(attWert)) {
            if (plausiRange_ok(attWert, 1, 53)) {
                arrBaumData[i] = attWert;
            }else {
                runDialog(window, "Wertebereich: 1-53");
                gtk_entry_set_text(GTK_ENTRY(widget), "");
            }
        }else {
            runDialog(window, "Wertebereich: 1-53");
            const char* oldEntry = arrBaumData[i].c_str();
            gtk_entry_set_text(GTK_ENTRY(widget), oldEntry);
        }
    }else {
        arrBaumData[i] = "";
    }
} 
void entryBox_blueteanfangBaum_changed(GtkWidget *widget, gpointer window) {

    int i = 18;
    const gchar *attWert_tmp = gtk_entry_get_text(GTK_ENTRY(widget));
    string attWert = attWert_tmp;
    
    if (attWert != "") {
        if (isStrNumber(attWert)) {
            if (plausiRange_ok(attWert, 1, 53)) {
                arrBaumData[i] = attWert;
            }else {
                runDialog(window, "Wertebereich: 1-53");
                const char* oldEntry = arrBaumData[i].c_str();
                gtk_entry_set_text(GTK_ENTRY(widget), oldEntry);
            }
        }else {
            runDialog(window, "Wertebereich: 1-53");
            const char* oldEntry = arrBaumData[i].c_str();
            gtk_entry_set_text(GTK_ENTRY(widget), oldEntry);
        }
    }else {
        arrBaumData[i] = "";
    }
} 
void entryBox_blueteendeBaum_changed(GtkWidget *widget, gpointer window) {

    int i = 19;
    const gchar *attWert_tmp = gtk_entry_get_text(GTK_ENTRY(widget));
    string attWert = attWert_tmp;
    
    if (attWert != "") {
        if (isStrNumber(attWert)) {
            if (plausiRange_ok(attWert, 1, 53)) {
                arrBaumData[i] = attWert;
            }else {
                runDialog(window, "Wertebereich: 1-53");
                gtk_entry_set_text(GTK_ENTRY(widget), "");
            }
        }else {
            runDialog(window, "Wertebereich: 1-53");
            const char* oldEntry = arrBaumData[i].c_str();
            gtk_entry_set_text(GTK_ENTRY(widget), oldEntry);
        }
    }else {
        arrBaumData[i] = "";
    }
}

//User Input 'Standort'
void entrybox_feldStaort_changed(GtkWidget *widget, gpointer window) {

    int i = 0;
    const gchar *attWert_tmp = gtk_entry_get_text(GTK_ENTRY(widget));
    string attWert = attWert_tmp;
    
    if (attWert != "") {
        if (isStrNumber(attWert)) {
            if (plausiRange_ok(attWert, 1, 899)) {
                arrStaOrtData[i] = attWert;
                int numberIntLow = string_to_int_plusHundred(attWert);
                numberIntLow *= 1000000;
                int numberIntHigh = numberIntLow + 1000000;
                string numberStrLow = int_to_string(numberIntLow);
                string numberStrHigh = int_to_string(numberIntHigh);
                whereFeld = "StandortID BETWEEN " + numberStrLow + " AND " + numberStrHigh;
            }else {
                runDialog(window, "Wertebereich: 1-899");
                const char* oldEntry = arrStaOrtData[i].c_str();
                gtk_entry_set_text(GTK_ENTRY(widget), oldEntry);
            }
        }else {
            runDialog(window, "Wertebereich: 1-899");
            const char* oldEntry = arrStaOrtData[i].c_str();
            gtk_entry_set_text(GTK_ENTRY(widget), oldEntry);
        }
    }else {
        arrStaOrtData[i] = "";
    }

    // for (int i = 0; i < 9; i++)
    // {
    //     cout << arrStaOrtData[i] << endl;
    // }
} 
void entrybox_reiheStaort_changed(GtkWidget *widget, gpointer window) {

    int i = 1;
    const gchar *attWert_tmp = gtk_entry_get_text(GTK_ENTRY(widget));
    string attWert = attWert_tmp;
    
    if (attWert != "") {
        if (isStrNumber(attWert)) {
            if (plausiRange_ok(attWert, -100, 899)) {
                arrStaOrtData[i] = attWert;
            }else {
                runDialog(window, "Wertebereich: -100-899");
                const char* oldEntry = arrStaOrtData[i].c_str();
                gtk_entry_set_text(GTK_ENTRY(widget), oldEntry);
            }
        }else {
            runDialog(window, "Wertebereich: -100-899");
            const char* oldEntry = arrStaOrtData[i].c_str();
            gtk_entry_set_text(GTK_ENTRY(widget), oldEntry);
        }
    }else {
        arrStaOrtData[i] = "";
    }
} 
void entrybox_platzStaort_changed(GtkWidget *widget, gpointer window) {
    
    int i = 2;
    const gchar *attWert_tmp = gtk_entry_get_text(GTK_ENTRY(widget));
    string attWert = attWert_tmp;
    
    if (attWert != "") {
        if (isStrNumber(attWert)) {
            if (plausiRange_ok(attWert, -100, 899)) {
                arrStaOrtData[i] = attWert;
            }else {
                runDialog(window, "Wertebereich: -100-899");
                const char* oldEntry = arrStaOrtData[i].c_str();
                gtk_entry_set_text(GTK_ENTRY(widget), oldEntry);
            }
        }else {
            runDialog(window, "Wertebereich: -100-899");
            const char* oldEntry = arrStaOrtData[i].c_str();
            gtk_entry_set_text(GTK_ENTRY(widget), oldEntry);
        }
    }else {
        arrStaOrtData[i] = "";
    }
} 
void combobox_statusStaort_changed(GtkWidget *widget, gpointer window) {
    gchar *attWert_tmp = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));
    string attWert = attWert_tmp;

    if (attWert == "nicht zu verwenden") { arrStaOrtData[3] = "0"; }
    else if (attWert == "aktiv & besetzt") { arrStaOrtData[3] = "1"; }
    else if (attWert == "Unterlage vorhanden") { arrStaOrtData[3] = "2"; }
    else if (attWert == "zu inaktivieren") { arrStaOrtData[3] = "3"; }
    else { arrStaOrtData[3] = "4"; }

    g_free(attWert_tmp);    
} 
void combobox_lichtStaort_changed(GtkWidget *widget, gpointer window) {
    gchar *attWert_tmp = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));
    string attWert = attWert_tmp;
    arrStaOrtData[4] = attWert;
    g_free(attWert_tmp);
} 
void combobox_gelaendeStaort_changed(GtkWidget *widget, gpointer window) {
    gchar *attWert_tmp = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));
    string attWert = attWert_tmp;
    arrStaOrtData[5] = attWert;
    g_free(attWert_tmp);
} 
void combobox_bodenStaort_changed(GtkWidget *widget, gpointer window) {
    gchar *attWert_tmp = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));
    string attWert = attWert_tmp;
    arrStaOrtData[6] = attWert;
    g_free(attWert_tmp);
} 
void combobox_bodenfeuchtigkeitStaort_changed(GtkWidget *widget, gpointer window) {
    gchar *attWert_tmp = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));
    string attWert = attWert_tmp;
    arrStaOrtData[7] = attWert;
    g_free(attWert_tmp);
} 
void entrybox_gpsStaort_changed(GtkWidget *widget, gpointer window) {
    const gchar *attWert_tmp = gtk_entry_get_text(GTK_ENTRY(widget));
    string attWert = attWert_tmp;
    arrStaOrtData[8] = attWert;

    // for (int i = 0; i < 9; i++)
    // {
    //     cout << arrStaOrtData[i] << endl;
    // }
}

//User Input 'Sorte Verwalten'
void comboBox_sorteSorten_changed(GtkWidget *widget, gpointer data) {
    gchar *attWert_tmp = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));
    string attWert = attWert_tmp;
    attWert[0] = toupper(attWert[0]);
    arrSortenVerwData[1] = attWert;
    g_free(attWert_tmp);
}
void comboBox_lateinSorten_changed(GtkWidget *widget, gpointer data) {

    gchar *attWert_tmp = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));
    string attWert = attWert_tmp;
    arrSortenVerwData[2] = attWert;
    g_free(attWert_tmp);
} 
void comboBox_befruchtungSorten_changed(GtkWidget *widget, gpointer data) {

    gchar *attWert_tmp = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));
    string attWert = attWert_tmp;
    arrSortenVerwData[3] = attWert;
    g_free(attWert_tmp);
} 
void entrybox_befruchtersorteSorten_changed(GtkWidget *widget, gpointer data) {

    const gchar *attWert_tmp = gtk_entry_get_text(GTK_ENTRY(widget));
    string attWert = attWert_tmp;
    arrSortenVerwData[4] = attWert;
}

//User input 'Wirtschaftsjahr'
void entrybox_wjahrWjahr_changed(GtkWidget *widget, gpointer window) {

    int i = 0;
    const gchar *attWert_tmp = gtk_entry_get_text(GTK_ENTRY(widget));
    string attWert = attWert_tmp;

    if (attWert != "") {
        if (isStrNumber(attWert)) {
            arrWjahrData[i] = attWert;
        }else {
            runDialog(window, "Wertebereich: 0-9");
            const char* oldEntry = arrWjahrData[i].c_str();
            gtk_entry_set_text(GTK_ENTRY(widget), oldEntry);
        }
    }else {
        arrWjahrData[i] = "";
    }
}

// User input 'Baum Profil'
void entrybox_baumIDProfil_changed(GtkWidget *widget, gpointer window) {

    int i = 0;
    const gchar *attWert_tmp = gtk_entry_get_text(GTK_ENTRY(widget));
    string attWert = attWert_tmp;

    if (attWert != "") {
        if (isStrNumber(attWert)) {
            if (plausiRange_ok(attWert, 1, 99999)) {
                arrBaumProfil[i] = attWert;
                int numberIntLow = string_to_int_plusHundred(attWert);
                numberIntLow *= 1000000;
                int numberIntHigh = numberIntLow + 1000000;
                string numberStrLow = int_to_string(numberIntLow);
                string numberStrHigh = int_to_string(numberIntHigh);
                whereFeld = "StandortID BETWEEN " + numberStrLow + " AND " + numberStrHigh;
            }else {
                runDialog(window, "Wertebereich: 1-99999");
                const char* oldEntry = arrBaumProfil[i].c_str();
                gtk_entry_set_text(GTK_ENTRY(widget), oldEntry);
            }
        }else {
            runDialog(window, "Wertebereich: 1-99999");
            const char* oldEntry = arrBaumProfil[i].c_str();
            gtk_entry_set_text(GTK_ENTRY(widget), oldEntry);
        }
    }else {
        arrBaumProfil[i] = "";
    }
} 
void entrybox_ertragProfil_changed(GtkWidget *widget, gpointer window) {
    
    int i = 1;
    const gchar *attWert_tmp = gtk_entry_get_text(GTK_ENTRY(widget));
    string attWert = attWert_tmp;

    if (attWert != "") {
        if (isStrNumber(attWert)) {
            arrBaumProfil[i] = attWert;
        }else {
            runDialog(window, "Wertebereich: 0-9");
            const char* oldEntry = arrBaumProfil[i].c_str();
            gtk_entry_set_text(GTK_ENTRY(widget), oldEntry);
        }
    }else {
        arrBaumProfil[i] = "";
    }
} 
void entrybox_hoeheProfil_changed(GtkWidget *widget, gpointer window) {

    int i = 2;
    const gchar *attWert_tmp = gtk_entry_get_text(GTK_ENTRY(widget));
    string attWert = attWert_tmp;

    if (attWert != "") {
        if (isStrNumber(attWert)) {
            arrBaumProfil[i] = attWert;
        }else {
            runDialog(window, "Wertebereich: 0-9");
            const char* oldEntry = arrBaumProfil[i].c_str();
            gtk_entry_set_text(GTK_ENTRY(widget), oldEntry);
        }
    }else {
        arrBaumProfil[i] = "";
    }
}
string get_latest_photo(){

    string* arrBaumProfil = getArrayBaumProfil();
    string baumID = arrBaumProfil[0];
    string photo = "images/jpg.jpg";

    if(!baumID.empty()){
        photo = "images/Platzhalter_mittel.png";
        photoJahr = "";

        string directory = get_main_directory();
        string slash = "/";
        string subdirectory = "/images/baeume/";
        string photo_folder = directory + subdirectory + baumID + slash;

        vector<string> photo_list = file_name_list(photo_folder);

        if(!photo_list.empty()){
        vector<int> photo_dates;

        // create a int vector from the content of the photo folder
        for(int i = 0; i < photo_list.size(); i++){
            photo_dates.push_back(string_to_int(substract_string(photo_list[i], (photo_folder))));
        }
        vector<int> photo_dates_new = revers_sort(photo_dates);
        string latest_file = int_to_string(photo_dates_new[0]);
        photo = photo_folder + latest_file;
        photoJahr = latest_file;
        }
        photo_list.clear();
    }
    return photo;
}
string get_photoJahr(){
    return photoJahr;
}

// Converts string to number and checks given boundries
bool plausiRange_ok(string userInput, int minWert, int maxWert) {

   int userInp = string_to_int(userInput);
   if (userInp < minWert || userInp > maxWert) {
       return FALSE;
   }
   return TRUE;
}

// Pops up dialog for window and shows message
void runDialog(gpointer window, string show_message) {

    GtkWidget *dialog;
    dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "\n\n%s", show_message.c_str());
    gtk_window_set_title(GTK_WINDOW(dialog), "Fruitrecord");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// return a list of names of regular files in the directory 
std::vector<std::string> file_name_list( const std::string& path_to_dir ){
    namespace fs = std::filesystem ;

    if( fs::is_directory(path_to_dir) )
    {
        std::vector<std::string> file_names ;
        
        for( const auto& entry : fs::directory_iterator(path_to_dir) )
            if( entry.is_regular_file() ) {
                file_names.push_back( fs::absolute( entry.path() ).string() ) ;
            }

        return file_names ;
    }

    else return {} ;  // not a directory; return empty vector
}

//Sonstiges (verschieben in Operations?)
/* render app in cairo... -> for future use



void draw_page(GtkPrintOperation *print, GtkPrintContext *context, int page_nr) {

  cairo_t *cr = gtk_print_context_get_cairo_context (context);

  cairo_surface_t *image;
  image = cairo_image_surface_create_from_png("screenshot.png");

  cairo_paint (cr);

  return;
}
*/

void openfile(const char* fileName)
{
  std::string file = fileName;
  #if _WIN32
    ShellExecuteA(NULL,"open",fileName,NULL,NULL,SW_SHOWNORMAL);
  #endif

  #if __APPLE__
    char command[100];
    snprintf(command, sizeof command, "%s %s", "open ", fileName);
    system(command);
  #endif

  #if __linux__
    char command[100];
    snprintf(command, sizeof command, "%s %s", "xdg-open ", fileName);
    system(command);
  #endif

}
void screenshot(GtkWidget *widget, gpointer data){

GtkPrintOperation *print;
GtkPrintOperationResult res;
GtkPrintSettings *settings;
GError *error = NULL;
//cairo_surface_t *image;

// Take Screenshot
GdkWindow *main_window;
GdkPixbuf *screenshot;
main_window = gtk_widget_get_parent_window(widget);
screenshot = gdk_pixbuf_get_from_window (main_window, 0, 0, gdk_window_get_width(main_window),
                                        gdk_window_get_height(main_window));
if(!screenshot){
  cout << "Error in Pixbuf while creating the screenshot";
}
gdk_pixbuf_save (screenshot, "./print/screenshot.png", "png", NULL, NULL);

#if _WIN32
  char file_location[200];
  GetCurrentDirectory(200, file_location);
  char file[30] ="/print/screenshot.png";
  strcat(file_location, file);
  openfile(file_location);
#else
  openfile("./print/screenshot.png");
#endif

/* start print operation... -> for further use
//image = cairo_image_surface_create_from_png("screenshot.png");

// printer settings
GtkPrintSettings *print_settings;
GtkPageSetup *page_setup;

print_settings = gtk_print_settings_new();
gtk_print_settings_set_orientation(print_settings,GTK_PAGE_ORIENTATION_LANDSCAPE);
gtk_print_settings_set_paper_size(print_settings,gtk_paper_size_new(GTK_PAPER_NAME_A4));
page_setup = gtk_page_setup_new();
gtk_page_setup_set_orientation(page_setup,GTK_PAGE_ORIENTATION_LANDSCAPE);
gtk_page_setup_set_paper_size_and_default_margins(page_setup,gtk_paper_size_new(GTK_PAPER_NAME_A4));

// initialize new Print Operation
print = gtk_print_operation_new ();
gtk_print_operation_set_print_settings(print,print_settings);
gtk_print_operation_set_default_page_setup(print,page_setup);
gtk_print_operation_set_show_progress(print,TRUE);
gtk_print_operation_set_track_print_status(print, TRUE);
g_signal_connect (print, "draw_page", G_CALLBACK (draw_page), NULL);
res = gtk_print_operation_run (print, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
                                 NULL, NULL);
*/
}
void print_txt(GtkWidget *widget, gpointer data){

#if _WIN32
  char file_location[200];
  GetCurrentDirectory(200, file_location);
  char file[30] ="/print/Baeume_Anzeigen.txt";
  strcat(file_location, file);
  openfile(file_location);
#else
  openfile("./print/Baeume_Anzeigen.txt");
#endif
}
void open_manual(GtkWidget *widget, gpointer data){

#if _WIN32
  char file_location[200];
  GetCurrentDirectory(200, file_location);
  char file[30] ="/Handbuch.pdf";
  strcat(file_location, file);
  openfile(file_location);
#else
  openfile("./Handbuch.pdf");
#endif
}

// Backup routine
void backup(string backup_folder, string db_location){

    time_t now = time(0);
    tm *ltm = localtime(&now);

    int year = (1900 + ltm->tm_year);
    int month = (1 + ltm->tm_mon);
    int day = (ltm->tm_mday);

    string leading_zero_month = "";
    if(month<10){
        leading_zero_month = "0";
    }
    string leading_zero_day = "";
    if(day<10){
        leading_zero_day = "0";
    }
    string s_year = int_to_string(year);
    string s_month = leading_zero_month + int_to_string(month);
    string s_day = leading_zero_day + int_to_string(day);
    string slash = "/";

    string fileName = s_year + s_month + s_day;
    string backup_file = backup_folder + slash + fileName;

    try{
        std::filesystem::copy_file(db_location, backup_file);
    }
    catch (...){

    }

    vector<string> file_list = file_name_list(backup_folder);
    vector<int> backup_dates;

    // create a int vector from the content of the backup folder
    for(int i = 0; i < file_list.size(); i++){
    backup_dates.push_back(string_to_int(substract_string(file_list[i], (backup_folder + slash))));
    }
    vector<int> backup_dates_new = revers_sort(backup_dates);

    delete_backup(backup_dates_new, (backup_folder + slash));
}
void delete_backup(vector<int> backup_dates, string folder){

    //delete all files exeeding filecount of 16    
    while(backup_dates.size()>16){
        std::filesystem::remove(folder + int_to_string(backup_dates[(backup_dates.size()-1)]));
        backup_dates.pop_back();
    }

    // if a new file has been added and there are at least 15 files
    if(backup_dates.size()>15){

        // delete file 6 if the diference to file 7 is less the 7 days
        if((backup_dates[5]-backup_dates[6])<7){
            std::filesystem::remove(folder + int_to_string(backup_dates[5]));
        }
        
        // delete file 11 if the diference to file 12 is less the 30 days
        if((backup_dates[10]-backup_dates[11])<30){
            std::filesystem::remove(folder + int_to_string(backup_dates[10]));
        }
    }
}


//only for testing purpose