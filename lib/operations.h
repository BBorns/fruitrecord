#ifndef OPERATIONS_H
#define OPERATIONS_H
using namespace std;
#include <vector>

int string_to_int(string);
int string_to_int_minusHundred(string, int starting_Index = 0, int number_of_Characters = 9);
int string_to_int_plusHundred(string);
string string_formater(vector<vector<string> > vecStr2D, int numCols);
std::string int_to_string(int);
string substract_string(string a, string b);
bool isStrNumber(string);
int find_lowest_number(vector<int>);
void set_main_directory(char cwd[256]);
string get_main_directory();
vector<int> revers_sort(vector<int> nr_sort);
std::vector<std::vector<string> > getAttributeStandortChoice(bool *staOrt_Auswahl, string *attributeEquals, int sizeArrays);
std::vector<std::vector<string> > getAttributeBaumChoice(bool *baum_Auswahl, string *attributeEquals, int sizeArrays);
std::string dropdownChecker(bool *staOrt_Auswahl, int startIndex, int endIndex);
string modStandortUF(string);
string modStandortPF(string);
string zahl_to_status(string num);
void saveDataToFile(vector<vector<string> > vec, string filenameWiExt);

string * getArrayBaumData();
string * getArrayStaOrtData();
string* getArraySortenVerwData();
string* getArrayWjahrData();
string* getArrayBaumProfil();
string* getAktFeld();
string* getWhereFeld();
void setInsertStatus(string str);
string getInsertStatus();
void setTransfer(string str);
string getTransfer();
void setWjahr(int wjahr);
int getWjahr();

void entrybox_nrBaum_changed(GtkWidget *widget, gpointer data);
void comboBox_artBaum_changed(GtkWidget *widget, gpointer data); void comboBox_sorteBaum_selected(GtkWidget *widget, gpointer data); void comboBox_standortBaum_changed(GtkWidget *widget, gpointer data); void comboBox_wuchsformBaum_changed(GtkWidget *widget, gpointer data);
void entryBox_herkunft_changed(GtkWidget *widget, gpointer data); void entryBox_lieferdatumBaum_changed(GtkWidget *widget, gpointer data); void entryBox_pflanzjahrBaum_changed(GtkWidget *widget, gpointer data); void comboBox_veredeltBaum_changed(GtkWidget *widget, gpointer data);
void comboBox_befruchtungBaum_changed(GtkWidget *widget, gpointer data); void entryBox_anmerkungenBaum_changed(GtkWidget *widget, gpointer data); void entryBox_pflanzbodenzusatzBaum_changed(GtkWidget *widget, gpointer data);
void entryBox_unterlageBaum_changed(GtkWidget *widget, gpointer data); void entryBox_rfidBaum_changed(GtkWidget *widget, gpointer data); void comboBox_statusBaum_changed(GtkWidget *widget, gpointer data); void comboBox_anlieferungBaum_changed(GtkWidget *widget, gpointer data);
void entryBox_fruchtanfangBaum_changed(GtkWidget *widget, gpointer data); void entryBox_fruchtendeBaum_changed(GtkWidget *widget, gpointer data); void entryBox_blueteanfangBaum_changed(GtkWidget *widget, gpointer data); void entryBox_blueteendeBaum_changed(GtkWidget *widget, gpointer data);

void entrybox_feldStaort_changed(GtkWidget *widget, gpointer window); void entrybox_reiheStaort_changed(GtkWidget *widget, gpointer window); void entrybox_platzStaort_changed(GtkWidget *widget, gpointer window); void combobox_statusStaort_changed(GtkWidget *widget, gpointer window); 
void combobox_lichtStaort_changed(GtkWidget *widget, gpointer window); void combobox_gelaendeStaort_changed(GtkWidget *widget, gpointer window); void combobox_bodenStaort_changed(GtkWidget *widget, gpointer window); 
void combobox_bodenfeuchtigkeitStaort_changed(GtkWidget *widget, gpointer window); void entrybox_gpsStaort_changed(GtkWidget *widget, gpointer window);

void comboBox_sorteSorten_changed(GtkWidget *widget, gpointer data); void comboBox_lateinSorten_changed(GtkWidget *widget, gpointer data); void comboBox_befruchtungSorten_changed(GtkWidget *widget, gpointer data); void entrybox_befruchtersorteSorten_changed(GtkWidget *widget, gpointer data);

void entrybox_wjahrWjahr_changed(GtkWidget *widget, gpointer data);

void entrybox_baumIDProfil_changed(GtkWidget *widget, gpointer window); void entrybox_ertragProfil_changed(GtkWidget *widget, gpointer window); void entrybox_hoeheProfil_changed(GtkWidget *widget, gpointer window);
string get_latest_photo();
string get_photoJahr();

bool plausiRange_ok(string userInp, int minWert, int Maxwert);
void runDialog(gpointer window, string show_message);

std::vector<std::string> file_name_list( const std::string& path_to_dir );

void print_txt(GtkWidget *widget, gpointer data);                 //print created txt.file
void screenshot(GtkWidget *widget, gpointer data);                //take screenshot
void show_screenshot_error(GtkWidget *widget, gpointer window);   //screenshot error
void open_manual(GtkWidget *widget, gpointer data);               //open program manual
void backup(string backup_folder, string db_location);            //backup management
void delete_backup(vector<int> backup_dates, string folder);      //pseudo GFS backup management

#endif