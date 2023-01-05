#include <gtk/gtk.h>
#include <cairo.h>
#include <iostream>
#include <sqlite3.h>
#include <vector>
#include <map>
#include <filesystem>
#include <unistd.h>
#include <string.h>
#include <ctime>
#if _WIN32
  #include <windows.h>
  #include <shellapi.h>
#elif __APPLE__
  #include <stdlib.h>
#elif __linux__
  #include <stdlib.h>
#endif
#include "lib/sql.h"
#include "lib/operations.h"

using namespace std;
using std::filesystem::current_path;

void hauptmenue(GtkWidget *widget, gpointer data); //Hauptmenü
GdkPixbuf *create_pixbuf(const gchar * filename);  //Pixbuff for application-icon
void db_open_dialog(GtkWidget* button, gpointer window);  //Dialog Fenster zum öffnen einer Datenbank
void db_create_dialog(GtkWidget* button, gpointer window);  //Dialog zum erstellen einer Datenbank
void stammd_standort_activated(GtkWidget *widget, gpointer data); //STAMMDATEN->Standort
void standortAnzeigen_clicked(GtkWidget *widget, gpointer data);  //              -> Standort Anzeigen
int btn_staortBearb_clicked(GtkWidget *widget, gpointer data);
int btn_staOrtBearbSpeichern_clicked(GtkWidget *widget, gpointer data);
void stammd_baeume_activated(GtkWidget *widget, gpointer data);   //STAMMDATEN->  Bäume
void baeumeAnzeigen_clicked(GtkWidget *widget, gpointer data);   //              ->Bäume_anzeigen
int btn_baumBearbeiten_clicked(GtkWidget *widget, gpointer data);
int btn_BaumBearbSpeichern_clicked(GtkWidget *widget, gpointer data);
void btn_staortHinzu_clicked(GtkWidget *widget, gpointer data);   //Quick Buttons -> Standort Hinzufügen
void btn_baumHinzu_clicked(GtkWidget *widget, gpointer data);     //Quick Buttons -> Baum Hinzufügen
void standortAnzeigen_refresh(GtkWidget *widget, gpointer data);  //refresh standort
void baeumeAnzeigen_refresh(GtkWidget *widget, gpointer data);  //refresh baeume_anzeigen
GtkWidget * create_standort_map ();

void btn_baumHinzuSpeichern_clicked(GtkWidget *widget, gpointer window);
void btn_StaOrtHinzuSpeichern_clicked(GtkWidget *widget, gpointer window);

void stammd_sortenVerwLv1_activated(GtkWidget *widget, gpointer data); //STAMMDATEN->  Obtsorten verwalten 
void stammd_sortenVerwLv2_activated(GtkWidget *widget, gpointer data);
void stammd_sortenVerwLv3_activated(GtkWidget *widget, gpointer data);
void sortenVerwLv1_btn_sorte_clicked(GtkWidget *widget, gpointer data);
void sortenVerwLv2_btn_submit_clicked (GtkWidget *widget, gpointer data);
void sortenVerwLv2_btn_hinzufuegen_clicked (GtkWidget *widget, gpointer data);
void sortenVerwLv2_btn_loeschen_clicked (GtkWidget *widget, gpointer data);
void sortenVerwLv3_btn_speichern_clicked (GtkWidget *widget, gpointer data);

void stammd_neuesWjahr_activated(GtkWidget *widget, gpointer data);
void stammd_neuesWjahrSpeichern_activated(GtkWidget *widget, gpointer data);
void stammd_wjahrAktualisieren_activated(GtkWidget *widget, gpointer data);

void pflege_baumProfil_activated(GtkWidget *widget, gpointer data);
void pflege_baumProfilSpeichern_activated(GtkWidget *widget, gpointer data);

void photo_open_dialog(GtkWidget* button, gpointer window);

void hilfeVersion_clicked(GtkWidget *widget, gpointer data);
void hilfeLizenz_clicked(GtkWidget *widget, gpointer data);

void popup_destroy_clicked(GtkWidget *widget, gpointer data);

// define static wigets for standortAnzeigen
static GtkWidget  *licht_schatten, *licht_halbschatten, *licht_sonnig, *licht_na,
                  *gelaende_flach, *gelaende_hang, *gelaende_steilhang, *gelaende_na,
                  *bodentyp_sandig, *bodentyp_lemig, *bodentyp_erdig, *bodentyp_supmf, *bodentyp_verw_feld, *bodentyp_steinig, *bodentyp_na,
                  *bodentyp_gestein, *bodenfeuch_trocken, *bodenfeuch_durchschnitt, *bodenfeuch_staunaesse, *bodenfeuch_na,
                  *aktiv_nichtVerwenden, *aktiv_besetzt, *aktiv_unterlage, *aktiv_inaktivieren, *aktiv_frei, *aktiv_na;

// define static wigets for baeumeAnzeigen
static GtkWidget    *art_kirsche, *art_apfel, *art_birne,*art_quitte, *art_reneklode, *art_pflaume, *art_mirabelle,
                    *art_zwetschge, *art_mandel, *art_pfirsich, *art_nektarine, *art_aprikose, *art_wallnuss,
                    *art_pekanuss, *art_feige, *art_mango, *art_papaya, *art_avocado, *art_olive, *art_orange,
                    *art_clementine, *art_mandarine, *art_grapefruit, *art_maulbeere, *art_tafeltraube,
                    *wuchsform_buschbaum, *wuchsform_viertelstamm, *wuchsform_halbstamm, *wuchsform_hochstamm, *wuchsform_spindel, *wuchsform_na,
                    *veredelt_ja, *veredelt_nein, *veredelt_na,
                    *befruchtung_selbstbefruchter, *befruchtung_teilselbstbefruchter, *befruchtung_fremdbefruchter, *befruchtung_na,
                    *status_bestellt, *status_geliefert, *status_gepflanzt, *status_aktiv, *status_inaktiv, *status_archiv, *status_na,
                    *anlieferung_selbst, *anlieferung_extern, *anlieferung_na;

static GdkPixbuf *app_icon;

bool standort_auswahl[26];        // Boolean array for standort check-menue
bool baeume_auswahl[48];          // Boolean array for Bäume check-menue

//Main window
GtkWidget *window, *window_pu;

int main(int argc, char *argv[]) {

  GtkWidget *vbox, *hbox, *button;

  // initial value set for standortAnzeige check-buttons
  for(int i = 0; i < 26; ++i){              // Set Standort check-menue all true
    standort_auswahl[i] = TRUE;
  }

  // initial value set for baeumeAnzeige check-buttons
  for(int i = 0; i < 48; ++i){              // Set Baeume check-menue all true
    baeume_auswahl[i] = TRUE;
  }

  // create directory print. If it allready exist, nothing will happen
  std::filesystem::create_directory("print");
  std::filesystem::create_directory("database");
  std::filesystem::create_directory("database/backups");
  std::filesystem::create_directory("images/baeume");

  char cwd[256];
  getcwd(cwd, 256);
  
  // define cwd in operations
  set_main_directory(cwd);

  char subfolder[10] ="/print";
  strcat(cwd, subfolder);

  gtk_init(&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
  gtk_window_set_title(GTK_WINDOW(window), "FRUITRECORD");

  app_icon = create_pixbuf("./images/menueiconsmall.png");  
  gtk_window_set_icon(GTK_WINDOW(window), app_icon);

  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

  /*
  //Generiert 'Start'-btn
  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  button = gtk_button_new_with_label("START");

  gtk_container_add(GTK_CONTAINER(window), vbox);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 100);
  gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 100);
  gtk_widget_show_all(window);

  g_signal_connect(G_OBJECT(button), "clicked",
      G_CALLBACK(hauptmenue), vbox);*/

  g_signal_connect(window, "destroy",
    G_CALLBACK(gtk_main_quit), NULL);

  // !!! Own method for faster code checking !!!

  // baeumeAnzeigen_clicked(window, vbox); // Only use for application development
  hauptmenue(window, vbox);

  gtk_main();

  return 0;
}

GdkPixbuf *create_pixbuf(const gchar * filename) {
    
   GdkPixbuf *pixbuf;
   GError *error = NULL;
   pixbuf = gdk_pixbuf_new_from_file(filename, &error);
   
   if (!pixbuf) {
       
      fprintf(stderr, "%s\n", error->message);
      g_error_free(error);
   }

   return pixbuf;
}

void hauptmenue(GtkWidget *widget, gpointer data){

  gtk_widget_destroy(GTK_WIDGET(data));

  GtkWidget *vbox, *hbox_quickBtns, *vbox_quickBtns_clmn1, *vbox_quickBtns_clmn2, *vbox_quickBtns_clmn3,
            *vbox_menu_bar, *vbox_menu_bar_clmn1, *vbox_menu_bar_clmn2, *menubar,
            *mMenu_Menue, *mMenu_Stammd, *mMenu_Pflege, *mMenu_Ausw, *mMenu_Hilfe,
            *menuMi, *stammdMi, *pflegeMi, *auswMi, *hilfeMi, *aktuellesWJahr, *aktuellesWJahr_value,
            *openDbMi, *createDbMi, *quitMi,
            *standortMi, *baeumeMi, *obstsortVerwMi, *wJahrNeuMi, *wJahrAktualisierenMi,
            *baumProfilMi, *eingBedarfMi, *loeschBedarfMi, *wJahrMi, *anzBedarfMi,
            *ertrageMi,
            *handbuchMi, *versMi, *lizenzMi,
            *btn_staortHinzu, *btn_baumHinzu, *btn_anzBedarf, *btn_eingBedarf, *btn_entfBedarf, *btn_hinzuErnte,
            *image;

  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  vbox_menu_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  hbox_quickBtns = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  vbox_quickBtns_clmn1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  vbox_quickBtns_clmn2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  vbox_quickBtns_clmn3 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

  // Main Menu
  menubar = gtk_menu_bar_new();
  mMenu_Menue = gtk_menu_new();
  mMenu_Stammd = gtk_menu_new();
  mMenu_Pflege = gtk_menu_new();
  mMenu_Ausw = gtk_menu_new();
  mMenu_Hilfe = gtk_menu_new();

  //Menuebar display current Wirtschaftsjahr or 'keins'
  aktuellesWJahr = gtk_label_new("Aktuelles Wirtschaftsjahr:");
  saveSqlQuerry("SELECT max(DISTINCT(WJahr)) FROM WirtschaftsJahr");
  string maxWjahr = executeSqlStatement_singleHit();
  if (maxWjahr == "") {
    maxWjahr = "<b>keins</b>";
  }else {
    time_t current_time = time(NULL);
    int yearNow = 1970 + current_time / 31537970; 
    setWjahr(string_to_int(maxWjahr));
    if (yearNow > string_to_int(maxWjahr)) {
      maxWjahr = "<span foreground=\"red\"><b>" + maxWjahr + "</b></span>";
    } else {
      maxWjahr = "<span foreground=\"green\"><b>" + maxWjahr + "</b></span>";
    }
  }

  
  

  aktuellesWJahr_value = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(aktuellesWJahr_value), maxWjahr.c_str());

  // Label
  menuMi = gtk_menu_item_new_with_label("MENÜ");
  stammdMi = gtk_menu_item_new_with_label("STAMMDATEN");
  pflegeMi = gtk_menu_item_new_with_label("LAUF. PFLEGE");
  auswMi = gtk_menu_item_new_with_label("AUSWERTUNG");
  hilfeMi = gtk_menu_item_new_with_label("HILFE");
  openDbMi = gtk_menu_item_new_with_label("DB öffnen");
  createDbMi = gtk_menu_item_new_with_label("DB erstellen");
  quitMi = gtk_menu_item_new_with_label("Beenden");
  standortMi = gtk_menu_item_new_with_label("Standort");
  baeumeMi = gtk_menu_item_new_with_label("Bäume");
  obstsortVerwMi = gtk_menu_item_new_with_label("Obstsorten Verw.");
  wJahrNeuMi = gtk_menu_item_new_with_label("Neues Wirtschaftsjahr");
  wJahrAktualisierenMi = gtk_menu_item_new_with_label("Wirtschaftsjahr aktualisieren");
  baumProfilMi = gtk_menu_item_new_with_label("Baum Profil anzeigen und bearbeiten");
  eingBedarfMi = gtk_menu_item_new_with_label("Bedarf eingeben");
  loeschBedarfMi = gtk_menu_item_new_with_label("Bedarf entfernen");
  wJahrMi = gtk_menu_item_new_with_label("Wirtschaftsjahr");
  anzBedarfMi = gtk_menu_item_new_with_label("Bedarfsanzeige");
  ertrageMi = gtk_menu_item_new_with_label("Erträge");
  handbuchMi = gtk_menu_item_new_with_label("Handbuch");
  versMi = gtk_menu_item_new_with_label("Version");
  lizenzMi = gtk_menu_item_new_with_label("Lizenz");
  btn_staortHinzu = gtk_button_new_with_label(" Standort\nhinzufügen");
  btn_baumHinzu = gtk_button_new_with_label("   Baum\nhinzufügen");
  btn_anzBedarf = gtk_button_new_with_label("Bedarfsanzeige");
  btn_eingBedarf = gtk_button_new_with_label("Bedarf eingeben");
  btn_entfBedarf = gtk_button_new_with_label("Bedarf entfernen");
  btn_hinzuErnte = gtk_button_new_with_label("Erntemenge\n hinzufügen");

  image = gtk_image_new_from_file ("images/fruits2.png");

  // Assign Label to Menu
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuMi), mMenu_Menue);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(stammdMi), mMenu_Stammd);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(pflegeMi), mMenu_Pflege);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(auswMi), mMenu_Ausw);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(hilfeMi), mMenu_Hilfe);

  // Assign Item to Menubar
  gtk_menu_shell_append(GTK_MENU_SHELL(menubar), menuMi);
  gtk_menu_shell_append(GTK_MENU_SHELL(menubar), stammdMi);
  gtk_menu_shell_append(GTK_MENU_SHELL(menubar), pflegeMi);
  gtk_menu_shell_append(GTK_MENU_SHELL(menubar), auswMi);
  gtk_menu_shell_append(GTK_MENU_SHELL(menubar), hilfeMi);

  // Assign Subitem to Menu
  gtk_menu_shell_append(GTK_MENU_SHELL(mMenu_Menue), openDbMi);
  gtk_menu_shell_append(GTK_MENU_SHELL(mMenu_Menue), createDbMi);
  gtk_menu_shell_append(GTK_MENU_SHELL(mMenu_Menue), quitMi);
  gtk_menu_shell_append(GTK_MENU_SHELL(mMenu_Stammd), standortMi);
  gtk_menu_shell_append(GTK_MENU_SHELL(mMenu_Stammd), baeumeMi);
  gtk_menu_shell_append(GTK_MENU_SHELL(mMenu_Stammd), obstsortVerwMi);
  gtk_menu_shell_append(GTK_MENU_SHELL(mMenu_Stammd), wJahrNeuMi);
  gtk_menu_shell_append(GTK_MENU_SHELL(mMenu_Stammd), wJahrAktualisierenMi);
  gtk_menu_shell_append(GTK_MENU_SHELL(mMenu_Pflege), baumProfilMi);
  gtk_menu_shell_append(GTK_MENU_SHELL(mMenu_Pflege), eingBedarfMi);
  gtk_menu_shell_append(GTK_MENU_SHELL(mMenu_Pflege), loeschBedarfMi);
  gtk_menu_shell_append(GTK_MENU_SHELL(mMenu_Pflege), wJahrMi);
  gtk_menu_shell_append(GTK_MENU_SHELL(mMenu_Pflege), anzBedarfMi);
  gtk_menu_shell_append(GTK_MENU_SHELL(mMenu_Ausw), ertrageMi);
  gtk_menu_shell_append(GTK_MENU_SHELL(mMenu_Hilfe), handbuchMi);
  gtk_menu_shell_append(GTK_MENU_SHELL(mMenu_Hilfe), versMi);
  gtk_menu_shell_append(GTK_MENU_SHELL(mMenu_Hilfe), lizenzMi);

  // Assign Buttons to vbox_quickBtns_clmn(x)
  gtk_box_pack_start(GTK_BOX(vbox_quickBtns_clmn1), btn_staortHinzu, TRUE, TRUE, 5);
  gtk_box_pack_start(GTK_BOX(vbox_quickBtns_clmn1), btn_baumHinzu, TRUE, TRUE, 5);
  gtk_box_pack_start(GTK_BOX(vbox_quickBtns_clmn2), btn_anzBedarf, TRUE, TRUE, 5);
  gtk_box_pack_start(GTK_BOX(vbox_quickBtns_clmn2), btn_eingBedarf, TRUE, TRUE, 5);
  gtk_box_pack_start(GTK_BOX(vbox_quickBtns_clmn2), btn_entfBedarf, TRUE, TRUE, 5);
  gtk_box_pack_start(GTK_BOX(vbox_quickBtns_clmn3), btn_hinzuErnte, TRUE, TRUE, 5);

  //Assign all boxes to parent box
  gtk_box_pack_start(GTK_BOX(vbox_menu_bar), menubar, TRUE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(vbox_menu_bar), aktuellesWJahr, FALSE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vbox_menu_bar), aktuellesWJahr_value, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(hbox_quickBtns), vbox_quickBtns_clmn1, TRUE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(hbox_quickBtns), vbox_quickBtns_clmn2, TRUE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(hbox_quickBtns), vbox_quickBtns_clmn3, TRUE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(vbox), vbox_menu_bar, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), image, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), hbox_quickBtns, FALSE, FALSE, 0);

  //Click actions
  g_signal_connect(G_OBJECT(window), "destroy",
        G_CALLBACK(gtk_main_quit), NULL);

  g_signal_connect(G_OBJECT(openDbMi), "activate",
    G_CALLBACK(db_open_dialog), vbox);

  g_signal_connect(G_OBJECT(createDbMi), "activate",
    G_CALLBACK(db_create_dialog), vbox);

  g_signal_connect(G_OBJECT(baeumeMi), "activate",
    G_CALLBACK(stammd_baeume_activated), vbox);

  g_signal_connect(G_OBJECT(standortMi), "activate",
      G_CALLBACK(stammd_standort_activated), vbox);

  g_signal_connect(G_OBJECT(obstsortVerwMi), "activate",
      G_CALLBACK(stammd_sortenVerwLv1_activated), vbox);

  g_signal_connect(G_OBJECT(wJahrNeuMi), "activate",
      G_CALLBACK(stammd_neuesWjahr_activated), vbox);

  g_signal_connect(G_OBJECT(wJahrAktualisierenMi), "activate",
      G_CALLBACK(stammd_wjahrAktualisieren_activated), NULL);

  g_signal_connect(G_OBJECT(baumProfilMi), "activate",
      G_CALLBACK(pflege_baumProfil_activated), vbox);

  g_signal_connect(G_OBJECT(quitMi), "activate",
        G_CALLBACK(gtk_main_quit), NULL);

  g_signal_connect(G_OBJECT(btn_staortHinzu), "clicked",
        G_CALLBACK(btn_staortHinzu_clicked), vbox);

  g_signal_connect(G_OBJECT(btn_baumHinzu), "clicked",
        G_CALLBACK(btn_baumHinzu_clicked), vbox);

  g_signal_connect(G_OBJECT(versMi), "activate",
        G_CALLBACK(hilfeVersion_clicked), NULL);

  g_signal_connect(G_OBJECT(lizenzMi), "activate",
        G_CALLBACK(hilfeLizenz_clicked), NULL);

  g_signal_connect(G_OBJECT(handbuchMi), "activate",
        G_CALLBACK(open_manual), vbox);

  gtk_container_add(GTK_CONTAINER(window), vbox);
  gtk_widget_show_all(window);
}

void db_open_dialog(GtkWidget* button, gpointer window){

  GtkWidget *dialog;
  GtkWidget *dialog_window;
  GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
  gint res;
  string db_complete_path;               // location of the database file
  string db_path;                        // folder of the database

  char cwd[256];
  getcwd(cwd, 256);
  char subfolder[10] ="/database";
  strcat(cwd, subfolder);

  dialog_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  app_icon = create_pixbuf("./images/menueiconsmall.png");  
  gtk_window_set_icon(GTK_WINDOW(dialog_window), app_icon);
  dialog = gtk_file_chooser_dialog_new ("Datenbank öffnen", GTK_WINDOW (dialog_window), action, 
                                        "_Cancel", GTK_RESPONSE_CANCEL, "_Open", 
                                        GTK_RESPONSE_ACCEPT, NULL);
  gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),cwd);
  res = gtk_dialog_run (GTK_DIALOG (dialog));
  if(res == GTK_RESPONSE_ACCEPT){
    db_complete_path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    db_path = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(dialog));

    setDbFile(db_complete_path);

    // Datenbank Prüfung
    char table[8] = "Sorte";
    string plausibility;
    string reference = "SortenIDOberArtUnterArtLateinNameBefruchtung2BefruchterSorteSortenIDOberArtUnterArtLateinNameBefruchtung2BefruchterSorte";
    vector<string> header = getHeader(table);
    for (int i = 0; i < header.size(); i++) {
        plausibility = plausibility + header[i];
    }
    if (plausibility.compare(reference)) {
      runDialog(dialog_window, "Die ausgewählte Datenbank hat ein ungültiges Format.");
      //clear database location
      db_complete_path = "";
      setDbFile(db_complete_path);
    }
    else{
      string db_name = substract_string(db_complete_path, db_path); // returns "/filenme"
      string db_folder = cwd;
      string subfolder = "/backups";
      string backup_folder = db_folder + subfolder + db_name;
      std::filesystem::create_directory(backup_folder);
      backup(backup_folder, db_complete_path);
    }
  }
  else
    g_print("You pressed Cancel\n");
    gtk_widget_destroy(dialog);
}

void db_create_dialog(GtkWidget* button, gpointer window){

  GtkWidget *dialog;
  GtkWidget *dialog_window;
  GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
  gint res;
  string db_location;               // location of the database file

  char cwd[256];
  getcwd(cwd, 256);
  char subfolder[10] ="/database";
  strcat(cwd, subfolder);

  dialog_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  app_icon = create_pixbuf("./images/menueiconsmall.png");  
  gtk_window_set_icon(GTK_WINDOW(dialog_window), app_icon);
  dialog = gtk_file_chooser_dialog_new ("Datenbank erstellen", GTK_WINDOW (dialog_window), action, 
                                        "_Cancel", GTK_RESPONSE_CANCEL, "_Save", 
                                        GTK_RESPONSE_ACCEPT, NULL);
  gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),cwd);
  res = gtk_dialog_run (GTK_DIALOG (dialog));
  if(res == GTK_RESPONSE_ACCEPT){
    db_location = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

    char db_template[20] = "/template.sql";
    getcwd(cwd, 256);
    strcat(cwd, db_template);

    try{
      std::filesystem::copy_file(cwd, db_location);
    }
    catch (...){
      runDialog(dialog_window, "Die ausgewählte Datei existiert bereits.");
    }

    setDbFile(db_location);
  }
  else
    g_print("You pressed Cancel\n");
    gtk_widget_destroy(dialog);
}


//Standort
void stammd_standort_activated(GtkWidget *widget, gpointer data) {

  gtk_widget_destroy(GTK_WIDGET(data));

  GtkWidget *vbox, *hbox, *btn_anzeigen, *btn_anlegen, *btn_zurueck;

  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  btn_anzeigen = gtk_button_new_with_label("Standorte Anzeigen");
  btn_anlegen = gtk_button_new_with_label("Neuen Standort\n      Anlegen");
  btn_zurueck = gtk_button_new_with_label("Zurück");

  //Assign all boxes to parent box
  gtk_container_add(GTK_CONTAINER(window), hbox);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 150);
  gtk_box_pack_start(GTK_BOX(vbox), btn_anzeigen, TRUE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(vbox), btn_anlegen, TRUE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(vbox), btn_zurueck, TRUE, TRUE, 10);

  gtk_widget_show_all(window);

  g_signal_connect(G_OBJECT(btn_anzeigen), "clicked", // Show Locations
      G_CALLBACK(standortAnzeigen_clicked), hbox);

  g_signal_connect(G_OBJECT(btn_anlegen), "clicked", // Show Locations
      G_CALLBACK(btn_staortHinzu_clicked), hbox);

  g_signal_connect(G_OBJECT(btn_zurueck), "clicked",
      G_CALLBACK(hauptmenue), hbox); // Back to Main Menu
}

void standortAnzeigen_reset(GtkWidget *widget, gpointer data){
  // initial value set for standortAnzeige check-buttons
  for(int i = 0; i < 26; ++i){              // Set Standort check-menue all true
    standort_auswahl[i] = TRUE;
  }

  // Reset Feld to '1'
  /*string* arrStaOrtData = getArrayStaOrtData();
  string* whereStr = getWhereFeld();
  arrStaOrtData[0] = "1";
  *whereStr = "StandortID BETWEEN 101000000 AND 102000000";*/

  gtk_widget_destroy(GTK_WIDGET(data));
  GtkWidget *vbox;
  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

  standortAnzeigen_clicked(window, vbox);
}

void standortAnzeigen_clicked(GtkWidget *widget, gpointer data) {

  gtk_widget_destroy(GTK_WIDGET(data));

  string* arrStaOrtData = getArrayStaOrtData();

  GtkWidget *vbox, *hbox;
  string numFalse;

  // SECOND menu Licht
  GtkWidget *stMenue_Licht, *lichtStandort;
  stMenue_Licht = gtk_menu_new(); //defeine widget
  numFalse = "Licht " + dropdownChecker(standort_auswahl, 0, 3);
  const char* cNumFalse1 = numFalse.c_str();
  lichtStandort = gtk_menu_item_new_with_label(cNumFalse1); // menu label

  licht_schatten = gtk_check_menu_item_new_with_label("Schatten"); // define submenue with check-item
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(licht_schatten), standort_auswahl[0]); // set check-menue value
  licht_halbschatten = gtk_check_menu_item_new_with_label("Halbschatten");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(licht_halbschatten), standort_auswahl[1]);
  licht_sonnig = gtk_check_menu_item_new_with_label("Sonne");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(licht_sonnig), standort_auswahl[2]);
  licht_na = gtk_check_menu_item_new_with_label("n.a.");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(licht_na), standort_auswahl[3]);

  // Assign Subitem to Menu
  gtk_menu_shell_append(GTK_MENU_SHELL(stMenue_Licht), licht_schatten);
  gtk_menu_shell_append(GTK_MENU_SHELL(stMenue_Licht), licht_halbschatten);
  gtk_menu_shell_append(GTK_MENU_SHELL(stMenue_Licht), licht_sonnig);
  gtk_menu_shell_append(GTK_MENU_SHELL(stMenue_Licht), licht_na);

  gtk_menu_item_set_submenu(GTK_MENU_ITEM(lichtStandort), stMenue_Licht);

  // THIRD menu Gelände
  GtkWidget *stMenue_Gelaende, *gelaendeStandort;

  stMenue_Gelaende = gtk_menu_new();
  numFalse = "Gelände " + dropdownChecker(standort_auswahl, 4, 7);
  const char* cNumFalse2 = numFalse.c_str();
  gelaendeStandort = gtk_menu_item_new_with_label(cNumFalse2);

  gelaende_flach = gtk_check_menu_item_new_with_label("Flach");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gelaende_flach), standort_auswahl[4]);
  gelaende_hang = gtk_check_menu_item_new_with_label("Hang");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gelaende_hang), standort_auswahl[5]);
  gelaende_steilhang = gtk_check_menu_item_new_with_label("Steilhang");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gelaende_steilhang), standort_auswahl[6]);
  gelaende_na = gtk_check_menu_item_new_with_label("n.a.");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(gelaende_na), standort_auswahl[7]);

  gtk_menu_shell_append(GTK_MENU_SHELL(stMenue_Gelaende), gelaende_flach);
  gtk_menu_shell_append(GTK_MENU_SHELL(stMenue_Gelaende), gelaende_hang);
  gtk_menu_shell_append(GTK_MENU_SHELL(stMenue_Gelaende), gelaende_steilhang);
  gtk_menu_shell_append(GTK_MENU_SHELL(stMenue_Gelaende), gelaende_na);

  gtk_menu_item_set_submenu(GTK_MENU_ITEM(gelaendeStandort), stMenue_Gelaende);


  // FOURTH menu Bodentyp
  GtkWidget *stMenue_Bodentyp,*bodentypStandort;

  stMenue_Bodentyp = gtk_menu_new();
  numFalse = "Bodentyp " + dropdownChecker(standort_auswahl, 8, 15);
  const char* cNumFalse3 = numFalse.c_str();
  bodentypStandort = gtk_menu_item_new_with_label(cNumFalse3);

  // Menue Label
  bodentyp_sandig = gtk_check_menu_item_new_with_label("sandig");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(bodentyp_sandig), standort_auswahl[8]);
  bodentyp_lemig = gtk_check_menu_item_new_with_label("lemig");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(bodentyp_lemig), standort_auswahl[9]);
  bodentyp_erdig = gtk_check_menu_item_new_with_label("erdig");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(bodentyp_erdig), standort_auswahl[10]);
  bodentyp_supmf = gtk_check_menu_item_new_with_label("sumpf");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(bodentyp_supmf), standort_auswahl[11]);
  bodentyp_verw_feld = gtk_check_menu_item_new_with_label("verwitterter Fels");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(bodentyp_verw_feld), standort_auswahl[12]);
  bodentyp_steinig = gtk_check_menu_item_new_with_label("steinig");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(bodentyp_steinig), standort_auswahl[13]);
  bodentyp_gestein = gtk_check_menu_item_new_with_label("Gestein");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(bodentyp_gestein), standort_auswahl[14]);
  bodentyp_na = gtk_check_menu_item_new_with_label("n.a.");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(bodentyp_na), standort_auswahl[15]);

  gtk_menu_shell_append(GTK_MENU_SHELL(stMenue_Bodentyp), bodentyp_sandig);
  gtk_menu_shell_append(GTK_MENU_SHELL(stMenue_Bodentyp), bodentyp_lemig);
  gtk_menu_shell_append(GTK_MENU_SHELL(stMenue_Bodentyp), bodentyp_erdig);
  gtk_menu_shell_append(GTK_MENU_SHELL(stMenue_Bodentyp), bodentyp_supmf);
  gtk_menu_shell_append(GTK_MENU_SHELL(stMenue_Bodentyp), bodentyp_verw_feld);
  gtk_menu_shell_append(GTK_MENU_SHELL(stMenue_Bodentyp), bodentyp_steinig);
  gtk_menu_shell_append(GTK_MENU_SHELL(stMenue_Bodentyp), bodentyp_gestein);
  gtk_menu_shell_append(GTK_MENU_SHELL(stMenue_Bodentyp), bodentyp_na);

  gtk_menu_item_set_submenu(GTK_MENU_ITEM(bodentypStandort), stMenue_Bodentyp);


  // FIFTH menu Bodenfeuchtigkeit
  GtkWidget *stMenue_Bodenfeuchtigkeit, *feuchtigkeitStandort;

  stMenue_Bodenfeuchtigkeit = gtk_menu_new();
  numFalse = "Bodenfeuchtigkeit " + dropdownChecker(standort_auswahl, 16, 19);
  const char* cNumFalse4 = numFalse.c_str();
  feuchtigkeitStandort = gtk_menu_item_new_with_label(cNumFalse4);

  bodenfeuch_trocken = gtk_check_menu_item_new_with_label("Trocken");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(bodenfeuch_trocken), standort_auswahl[16]);
  bodenfeuch_durchschnitt = gtk_check_menu_item_new_with_label("Durchschnitt");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(bodenfeuch_durchschnitt), standort_auswahl[17]);
  bodenfeuch_staunaesse = gtk_check_menu_item_new_with_label("Staunässe");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(bodenfeuch_staunaesse), standort_auswahl[18]);
  bodenfeuch_na = gtk_check_menu_item_new_with_label("n.a.");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(bodenfeuch_na), standort_auswahl[19]);

  gtk_menu_shell_append(GTK_MENU_SHELL(stMenue_Bodenfeuchtigkeit), bodenfeuch_trocken);
  gtk_menu_shell_append(GTK_MENU_SHELL(stMenue_Bodenfeuchtigkeit), bodenfeuch_durchschnitt);
  gtk_menu_shell_append(GTK_MENU_SHELL(stMenue_Bodenfeuchtigkeit), bodenfeuch_staunaesse);
  gtk_menu_shell_append(GTK_MENU_SHELL(stMenue_Bodenfeuchtigkeit), bodenfeuch_na);

  gtk_menu_item_set_submenu(GTK_MENU_ITEM(feuchtigkeitStandort), stMenue_Bodenfeuchtigkeit);


  // Sixth menu Aktiv/Inaktiv
  GtkWidget *stMenue_aktivInaktiv, *aktivInaktivStandort;

  stMenue_aktivInaktiv = gtk_menu_new(); //defeine widget
  numFalse = "Aktiv/Inaktiv " + dropdownChecker(standort_auswahl, 20, 25);
  const char* cNumFalse5 = numFalse.c_str();
  aktivInaktivStandort = gtk_menu_item_new_with_label(cNumFalse5); // menu label

  aktiv_nichtVerwenden = gtk_check_menu_item_new_with_label("nicht zu verwenden"); // define submenue with check-item
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(aktiv_nichtVerwenden), standort_auswahl[20]); // set check-menue value
  aktiv_besetzt = gtk_check_menu_item_new_with_label("aktiv und besetzt");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(aktiv_besetzt), standort_auswahl[21]);
  aktiv_unterlage = gtk_check_menu_item_new_with_label("Unterlage vorhanden");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(aktiv_unterlage), standort_auswahl[22]);
  aktiv_inaktivieren = gtk_check_menu_item_new_with_label("zu inaktivieren");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(aktiv_inaktivieren), standort_auswahl[23]);
  aktiv_frei = gtk_check_menu_item_new_with_label("aktiv und frei");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(aktiv_frei), standort_auswahl[24]);
  aktiv_na = gtk_check_menu_item_new_with_label("n.a.");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(aktiv_na), standort_auswahl[25]);

  // Assign Subitem to Menu
  gtk_menu_shell_append(GTK_MENU_SHELL(stMenue_aktivInaktiv), aktiv_nichtVerwenden);
  gtk_menu_shell_append(GTK_MENU_SHELL(stMenue_aktivInaktiv), aktiv_besetzt);
  gtk_menu_shell_append(GTK_MENU_SHELL(stMenue_aktivInaktiv), aktiv_unterlage);
  gtk_menu_shell_append(GTK_MENU_SHELL(stMenue_aktivInaktiv), aktiv_inaktivieren);
  gtk_menu_shell_append(GTK_MENU_SHELL(stMenue_aktivInaktiv), aktiv_frei);
  gtk_menu_shell_append(GTK_MENU_SHELL(stMenue_aktivInaktiv), aktiv_na);

  gtk_menu_item_set_submenu(GTK_MENU_ITEM(aktivInaktivStandort), stMenue_aktivInaktiv);


  // ADDITIONAL Widgets

  // Reload Button
  GtkWidget *image_reload = gtk_image_new_from_file("images/reload_small.png");
  GtkWidget *btn_aktualisieren = gtk_button_new();
  gtk_button_set_image(GTK_BUTTON(btn_aktualisieren), image_reload);

  //Standort Bearbeiten Widgets
  GtkWidget *label_r, *label_p, *entrybox_reihe, *entrybox_platz, *btn_bearbeiten;
  label_r = gtk_label_new("Reihe:");
  label_p = gtk_label_new("Platz:");
  entrybox_reihe = gtk_entry_new();
  gtk_entry_set_width_chars(GTK_ENTRY(entrybox_reihe), 4);
  gtk_entry_set_max_length(GTK_ENTRY(entrybox_reihe), 4);
  entrybox_platz = gtk_entry_new();
  gtk_entry_set_width_chars(GTK_ENTRY(entrybox_platz), 4);
  gtk_entry_set_max_length(GTK_ENTRY(entrybox_platz), 4);
  btn_bearbeiten = gtk_button_new_with_label("  Standort\nbearbeiten");

  // Reset Button
  GtkWidget *reset_btn = gtk_button_new_with_label("\tAuswahl zurücksetzen\t");

  // Zurück Button
  GtkWidget *z_btn = gtk_button_new_with_label("\t\t\tZurück\t\t\t");

  // Drucken Button
  GtkWidget *drucken_btn = gtk_button_new_with_label("\t\t\tDrucken\t\t\t");

  // create menu-bar
  GtkWidget *stMenue_menubar = gtk_menu_bar_new(); // define menubar widget

  // Assign Item to Standort_Menubar
  gtk_menu_shell_append(GTK_MENU_SHELL(stMenue_menubar), lichtStandort);
  gtk_menu_shell_append(GTK_MENU_SHELL(stMenue_menubar), gelaendeStandort);
  gtk_menu_shell_append(GTK_MENU_SHELL(stMenue_menubar), bodentypStandort);
  gtk_menu_shell_append(GTK_MENU_SHELL(stMenue_menubar), feuchtigkeitStandort);
  gtk_menu_shell_append(GTK_MENU_SHELL(stMenue_menubar), aktivInaktivStandort);


  // define structure elements
  GtkWidget *map, *feld, *entrybox_feld, *bottom_menue;

  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  hbox =gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  feld = gtk_label_new("Feld:");
  entrybox_feld = gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(entrybox_feld), arrStaOrtData[0].c_str());
  gtk_entry_set_width_chars(GTK_ENTRY(entrybox_feld), 3);
  gtk_entry_set_alignment(GTK_ENTRY(entrybox_feld), 1);
  gtk_entry_set_max_length(GTK_ENTRY(entrybox_feld), 3);
  gtk_entry_set_overwrite_mode(GTK_ENTRY(entrybox_feld), TRUE);
  //creates string with 'Feldern' for tooltip
  saveSqlQuerry("SELECT DISTINCT StandortID/1000000-100 FROM Standort");
  vector<vector<string> > vecFelder = executeSqlStatement();
  string felder = "aktive Felder:\n\n" + string_formater(vecFelder, 4);
  gtk_widget_set_tooltip_text(GTK_WIDGET(entrybox_feld), felder.c_str());
  bottom_menue = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

  map = create_standort_map ();

  string result;
  if (getNumberOfRows() > 0) {
    result = "Ihre Anfrage ergab: <span foreground='green'><b>" + int_to_string(getNumberOfRows()) + "</b></span> Standorte";
  }else {
    result = "Ihre Anfrage ergab: <span foreground='red'><b>" + int_to_string(getNumberOfRows()) + "</b></span> Standorte";

  }
  GtkWidget *anzahl;
  anzahl = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(anzahl), result.c_str());

  // scrollbar
  GtkWidget *scrollWindow = gtk_scrolled_window_new(NULL, NULL);

  gtk_container_add(GTK_CONTAINER(scrollWindow), map);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollWindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

  // Legende
  GtkWidget   *legende, *image_AktiveFree, *label_AktiveFree, *image_AktiveUsed, *label_AktiveUsed, *image_Rootstock,
              *label_Rootstock, *image_tobeInactivated, *label_tobeInactivated, *image_doNotUse, *label_doNotUse;

  legende = gtk_fixed_new();
  image_AktiveFree = gtk_image_new_from_file ("images/active_and_free.png");
  label_AktiveFree = gtk_label_new("Aktiv & Frei");
  image_AktiveUsed = gtk_image_new_from_file ("images/active_and_used.png");
  label_AktiveUsed = gtk_label_new("Aktiv & Besetzt");
  image_Rootstock = gtk_image_new_from_file ("images/rootstock_present.png");
  label_Rootstock = gtk_label_new("Unterlage vorhanden");
  image_tobeInactivated = gtk_image_new_from_file ("images/to_be_inactivated.png");
  label_tobeInactivated = gtk_label_new("zu inaktivieren");
  image_doNotUse = gtk_image_new_from_file ("images/do_not_use.png");
  label_doNotUse = gtk_label_new("nicht zu verwenden");
  gtk_fixed_put(GTK_FIXED(legende), image_AktiveFree, 10, 10);
  gtk_fixed_put(GTK_FIXED(legende), label_AktiveFree, 47, 14);
  gtk_fixed_put(GTK_FIXED(legende), image_AktiveUsed, 150, 10);
  gtk_fixed_put(GTK_FIXED(legende), label_AktiveUsed, 187, 14);
  gtk_fixed_put(GTK_FIXED(legende), image_Rootstock, 310, 10);
  gtk_fixed_put(GTK_FIXED(legende), label_Rootstock, 347, 14);
  gtk_fixed_put(GTK_FIXED(legende), image_tobeInactivated, 510, 10);
  gtk_fixed_put(GTK_FIXED(legende), label_tobeInactivated, 547, 14);
  gtk_fixed_put(GTK_FIXED(legende), image_doNotUse, 670, 10);
  gtk_fixed_put(GTK_FIXED(legende), label_doNotUse, 707, 14);

  GtkWidget *horizontal_separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);

  // build window
  gtk_box_pack_start(GTK_BOX(bottom_menue), reset_btn, FALSE, TRUE, 10);
  // gtk_box_pack_start(GTK_BOX(bottom_menue), anzahl, TRUE, TRUE, 10);
  gtk_box_pack_end(GTK_BOX(bottom_menue), z_btn, FALSE, TRUE, 10);
  gtk_box_pack_end(GTK_BOX(bottom_menue), drucken_btn, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(vbox), stMenue_menubar, FALSE, FALSE, 10);
  gtk_box_pack_start(GTK_BOX(hbox), feld, FALSE, FALSE, 10);
  gtk_box_pack_start(GTK_BOX(hbox), entrybox_feld, FALSE, FALSE, 5);
  gtk_box_pack_start(GTK_BOX(hbox), btn_aktualisieren, FALSE, FALSE, 5);
  gtk_box_pack_start(GTK_BOX(hbox), anzahl, TRUE, FALSE, 5);
  gtk_box_pack_end(GTK_BOX(hbox), btn_bearbeiten, FALSE, FALSE, 5);
  gtk_box_pack_end(GTK_BOX(hbox), entrybox_platz, FALSE, FALSE, 5);
  gtk_box_pack_end(GTK_BOX(hbox), label_p, FALSE, FALSE, 5);
  gtk_box_pack_end(GTK_BOX(hbox), entrybox_reihe, FALSE, FALSE, 5);
  gtk_box_pack_end(GTK_BOX(hbox), label_r, FALSE, FALSE, 5);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);
  gtk_box_pack_start(GTK_BOX(vbox), scrollWindow, TRUE, TRUE, 5);
  gtk_box_pack_end(GTK_BOX(vbox), bottom_menue, FALSE, FALSE, 5);
  gtk_box_pack_end(GTK_BOX(vbox), legende, FALSE, FALSE, 5);
  gtk_box_pack_end(GTK_BOX(vbox), horizontal_separator, FALSE, TRUE, 5);
  gtk_container_add(GTK_CONTAINER(window), vbox);
  gtk_widget_show_all(window);

  // define action for check-item
  g_signal_connect(G_OBJECT(licht_schatten), "toggled",
    G_CALLBACK(standortAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(licht_halbschatten), "toggled",
    G_CALLBACK(standortAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(licht_sonnig), "toggled",
    G_CALLBACK(standortAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(licht_na), "toggled",
    G_CALLBACK(standortAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(gelaende_flach), "toggled",
    G_CALLBACK(standortAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(gelaende_hang), "toggled",
    G_CALLBACK(standortAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(gelaende_steilhang), "toggled",
    G_CALLBACK(standortAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(gelaende_na), "toggled",
    G_CALLBACK(standortAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(bodentyp_sandig), "toggled",
    G_CALLBACK(standortAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(bodentyp_lemig), "toggled",
    G_CALLBACK(standortAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(bodentyp_erdig), "toggled",
    G_CALLBACK(standortAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(bodentyp_supmf), "toggled",
    G_CALLBACK(standortAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(bodentyp_verw_feld), "toggled",
    G_CALLBACK(standortAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(bodentyp_steinig), "toggled",
    G_CALLBACK(standortAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(bodentyp_gestein), "toggled",
    G_CALLBACK(standortAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(bodentyp_na), "toggled",
    G_CALLBACK(standortAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(bodenfeuch_trocken), "toggled",
    G_CALLBACK(standortAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(bodenfeuch_durchschnitt), "toggled",
    G_CALLBACK(standortAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(bodenfeuch_staunaesse), "toggled",
    G_CALLBACK(standortAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(bodenfeuch_na), "toggled",
    G_CALLBACK(standortAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(aktiv_nichtVerwenden), "toggled",
    G_CALLBACK(standortAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(aktiv_besetzt), "toggled",
    G_CALLBACK(standortAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(aktiv_unterlage), "toggled",
    G_CALLBACK(standortAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(aktiv_inaktivieren), "toggled",
    G_CALLBACK(standortAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(aktiv_frei), "toggled",
    G_CALLBACK(standortAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(aktiv_na), "toggled",
    G_CALLBACK(standortAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(entrybox_feld), "changed", 
        G_CALLBACK(entrybox_feldStaort_changed), (gpointer) window);
  g_signal_connect(G_OBJECT(entrybox_reihe), "changed", 
        G_CALLBACK(entrybox_reiheStaort_changed), (gpointer) window);
  g_signal_connect(G_OBJECT(entrybox_platz), "changed", 
        G_CALLBACK(entrybox_platzStaort_changed), (gpointer) window);
  g_signal_connect(G_OBJECT(btn_aktualisieren), "clicked",
        G_CALLBACK(standortAnzeigen_clicked), vbox);
  g_signal_connect(G_OBJECT(btn_bearbeiten), "clicked",
        G_CALLBACK(btn_staortBearb_clicked), vbox);

  // Reset
  g_signal_connect(G_OBJECT(reset_btn), "clicked",
    G_CALLBACK(standortAnzeigen_reset), vbox); //

  // Drucken
  g_signal_connect(G_OBJECT(drucken_btn), "clicked",
    G_CALLBACK(screenshot), vbox); //

  // Back to Standort Menu
  g_signal_connect(G_OBJECT(z_btn), "clicked",
    G_CALLBACK(stammd_standort_activated), vbox); // Back to Standort Menu
}

int btn_staortBearb_clicked(GtkWidget *widget, gpointer data) {
  
  string* arrStaOrtData = getArrayStaOrtData();
  // Check if FPR not empty
  if (arrStaOrtData[0] == "" || arrStaOrtData[1] == "" || arrStaOrtData[2] == "") {
    runDialog(window, "\nBitte Feld, Reihe und Platz angeben\n");
    return 0;
  }

  //Check if Standort exists
  string standortUF = "F" + arrStaOrtData[0] + "R" + arrStaOrtData[1] + "P" + arrStaOrtData[2];
  string standortPF = modStandortPF(standortUF);
  string sql = "SELECT * FROM Standort WHERE StandortID = " + standortPF;
  saveSqlQuerry(sql);
  vector<vector<string> > vecStandortExists = executeSqlStatement();
  if (vecStandortExists.size() == 0) {
    runDialog(window, "\nDieser Standort existiert leider nicht\n");
    return 0;
  }

  gtk_widget_set_sensitive(GTK_WIDGET(window), FALSE);
  window_pu = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position(GTK_WINDOW(window_pu), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(window_pu), 750, 500);
  gtk_window_set_title(GTK_WINDOW(window_pu), "FRUITRECORD");
  app_icon = create_pixbuf("./images/menueiconsmall.png");  
  gtk_window_set_icon(GTK_WINDOW(window_pu), app_icon);

  GtkWidget *vbox, *hbox, *hbox_frp, *combobox_licht, *combobox_gelaende, *combobox_boden, *combobox_bodenfeuchtigkeit, *entrybox_gps, *combobox_status, *bottom_menue, *btn_speichern, *btn_zurueck, *label_titel,
            *row1, *row2, *rc11, *rc12, *rc13, *rc21, *rc22, *rc23,
            *label_feld, *label_reihe, *label_platz, *label_licht, *label_gelaende, *label_bodentyp, *label_bodenfeuchtigkeit, *label_gps, *label_status;

  //Label
  string titel = "<span size=\"large\"><b>Standort bearbeiten</b></span>";
  const gchar *str = titel.c_str();
  label_titel = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_titel), str);

  sql = "SELECT Licht, Gelaende, BodenTyp, BodenFeuchtigkeit, GpsKoordinaten, AktivInaktiv FROM Standort WHERE StandortID = " + standortPF;
  saveSqlQuerry(sql, 6);
  vector<vector<string> > vecStandortData = executeSqlStatement();

  // for (int i = 0; i < vecStandortData.size(); i++)
  // {
  //   for (int j = 0; j < vecStandortData[i].size(); j++)
  //   {
  //     cout << vecStandortData[i].at(j) << endl;
  //   }
  // }
  
  string feld = "<b>Feld:</b> " + arrStaOrtData[0];
  label_feld =gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_feld), feld.c_str());

  string reihe = "<b>Reihe:</b> " + arrStaOrtData[1];
  label_reihe =gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_reihe), reihe.c_str());

  string platz = "<b>Platz:</b> " + arrStaOrtData[2];
  label_platz =gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_platz), platz.c_str());

  string licht = "<b>Licht:</b> " + vecStandortData[0].at(0);
  label_licht =gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_licht), licht.c_str());

  string gelaende = "<b>Gelände:</b> " + vecStandortData[0].at(1);
  label_gelaende =gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_gelaende), gelaende.c_str());

  string bodentyp = "<b>Bodentyp:</b> " + vecStandortData[0].at(2);
  label_bodentyp =gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_bodentyp), bodentyp.c_str());

  string bodenfeuchtigkeit = "<b>Bodenfeuchtigkeit:</b> " + vecStandortData[0].at(3);
  label_bodenfeuchtigkeit =gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_bodenfeuchtigkeit), bodenfeuchtigkeit.c_str());

  string gps = "<b>GPS-Koordinaten</b>";
  label_gps =gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_gps), gps.c_str());

  string status = "<b>Status:</b> " + zahl_to_status(vecStandortData[0].at(5));
  label_status =gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_status), status.c_str());

  // ComboBox 'Licht'
  combobox_licht = gtk_combo_box_text_new();  
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_licht), NULL, "Schatten");     
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_licht), NULL, "Halbschatten");     
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_licht), NULL, "Sonne");  

  // ComboBox 'Gelände'
  combobox_gelaende = gtk_combo_box_text_new();  
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_gelaende), NULL, "Flach");     
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_gelaende), NULL, "Hang");     
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_gelaende), NULL, "Steilhang");    

  // ComboBox 'Boden'
  combobox_boden = gtk_combo_box_text_new();  
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_boden), NULL, "Sandig");   
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_boden), NULL, "Lemig");   
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_boden), NULL, "Erdig");   
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_boden), NULL, "Sumpf");   
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_boden), NULL, "Verwitterter Fels");   
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_boden), NULL, "Steinig");   
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_boden), NULL, "Gestein");   

  // ComboBox 'Bodenfeuchtigkeit'
  combobox_bodenfeuchtigkeit = gtk_combo_box_text_new();  
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_bodenfeuchtigkeit), NULL, "Trocken");  
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_bodenfeuchtigkeit), NULL, "Durchschnitt");  
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_bodenfeuchtigkeit), NULL, "Staunässe");  

  // EntryBox 'GPS'
  entrybox_gps = gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(entrybox_gps), 255);
  gtk_entry_set_text(GTK_ENTRY(entrybox_gps), (vecStandortData[0].at(4)).c_str());

  // ComboBox 'Status'
  combobox_status = gtk_combo_box_text_new();  
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_status), NULL, "nicht zu verwenden");    
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_status), NULL, "aktiv & besetzt");    
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_status), NULL, "Unterlage vorhanden");    
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_status), NULL, "zu inaktivieren");    
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_status), NULL, "aktiv & frei"); 

  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
  hbox_frp = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
  row1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  row2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  rc11 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc12 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc13 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc21 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc22 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc23 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  bottom_menue = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  btn_speichern = gtk_button_new_with_label("\t\tSpeichern\t\t");
  btn_zurueck = gtk_button_new_with_label("\t\tZurück\t\t");

  gtk_box_pack_start(GTK_BOX(vbox), label_titel, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(hbox_frp), label_feld, TRUE, TRUE, 60);
  gtk_box_pack_start(GTK_BOX(hbox_frp), label_reihe, TRUE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox_frp), label_platz, TRUE, TRUE, 60);
  gtk_box_pack_start(GTK_BOX(vbox), hbox_frp, FALSE, TRUE, 20);
  gtk_box_pack_start(GTK_BOX(rc11), label_licht, FALSE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc11), combobox_licht, FALSE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(row1), rc11, TRUE, TRUE, 20);
  gtk_box_pack_start(GTK_BOX(rc12), label_gelaende, FALSE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc12), combobox_gelaende, FALSE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(row1), rc12, TRUE, TRUE, 20);
  gtk_box_pack_start(GTK_BOX(rc13), label_bodentyp, FALSE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc13), combobox_boden, FALSE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(row1), rc13, TRUE, TRUE, 20);
  gtk_box_pack_start(GTK_BOX(vbox), row1, FALSE, TRUE, 20);
  gtk_box_pack_start(GTK_BOX(rc21), label_bodenfeuchtigkeit, FALSE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc21), combobox_bodenfeuchtigkeit, FALSE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(row2), rc21, TRUE, TRUE, 20);
  gtk_box_pack_start(GTK_BOX(rc22), label_gps, FALSE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc22), entrybox_gps, FALSE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(row2), rc22, TRUE, TRUE, 20);
  gtk_box_pack_start(GTK_BOX(rc23), label_status, FALSE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc23), combobox_status, FALSE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(row2), rc23, TRUE, TRUE, 20);
  gtk_box_pack_start(GTK_BOX(vbox), row2, FALSE, TRUE, 20);
  gtk_box_pack_start(GTK_BOX(bottom_menue), btn_speichern, FALSE, FALSE, 50);
  gtk_box_pack_end(GTK_BOX(bottom_menue), btn_zurueck, FALSE, FALSE, 50);
  gtk_box_pack_end(GTK_BOX(vbox), bottom_menue, FALSE, FALSE, 10);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 20);
  gtk_container_add(GTK_CONTAINER(window_pu), hbox);
  gtk_widget_show_all(window_pu);

  g_signal_connect(G_OBJECT(combobox_status), "changed", 
        G_CALLBACK(combobox_statusStaort_changed), (gpointer) window);
  
  g_signal_connect(G_OBJECT(combobox_licht), "changed", 
        G_CALLBACK(combobox_lichtStaort_changed), (gpointer) window);

  g_signal_connect(G_OBJECT(combobox_gelaende), "changed", 
        G_CALLBACK(combobox_gelaendeStaort_changed), (gpointer) window);

  g_signal_connect(G_OBJECT(combobox_boden), "changed", 
        G_CALLBACK(combobox_bodenStaort_changed), (gpointer) window);

  g_signal_connect(G_OBJECT(combobox_bodenfeuchtigkeit), "changed", 
        G_CALLBACK(combobox_bodenfeuchtigkeitStaort_changed), (gpointer) window);

  g_signal_connect(G_OBJECT(entrybox_gps), "changed", 
        G_CALLBACK(entrybox_gpsStaort_changed), (gpointer) window);

  g_signal_connect(btn_speichern, "clicked",
        G_CALLBACK(btn_staOrtBearbSpeichern_clicked), data);

  g_signal_connect(btn_zurueck, "clicked",
        G_CALLBACK(popup_destroy_clicked), data);

  g_signal_connect(window_pu, "destroy",
        G_CALLBACK(popup_destroy_clicked), data);

  return 1;
}

int btn_staOrtBearbSpeichern_clicked(GtkWidget *widget, gpointer data) {

  string* arrStaOrtData = getArrayStaOrtData();
  //check for activ tree
  if (arrStaOrtData[3] != "" && arrStaOrtData[3] != "3") {
    string standortUF = "F" + arrStaOrtData[0] + "R" + arrStaOrtData[1] + "P" + arrStaOrtData[2];
    string standortPF = modStandortPF(standortUF);
    string sql = "SELECT BaumID FROM Baum WHERE StandortID=" + standortPF;
    saveSqlQuerry(sql);
    vector<vector<string> > vecBaumOnStandort = executeSqlStatement();
    if (vecBaumOnStandort.size() > 0) {
      runDialog(window, "\nStatus kann an diesem Standort nicht geändert werden,\nda hier noch ein aktiver Baum steht\n");
      return 0;
    }
  }

  sqlBuilderUpdateStandort();
  runDialog(window, "\nDaten erfolgreich geändert\n");
  popup_destroy_clicked(widget, data);

  return 1;
}

void btn_staortHinzu_clicked(GtkWidget *widget, gpointer data) {

  gtk_widget_destroy(GTK_WIDGET(data));

  int maxLengthInp = 255;
  string* arrstaOrtAnlData;
  arrstaOrtAnlData = getArrayStaOrtData();
  for (int i = 0; i < 9; i++) { arrstaOrtAnlData[i] = ""; } // Resets array

  GtkWidget   *hbox_StaortHinzu, *vbox, *row1, *rc11, *rc12, *rc13, *rc14, *row2, *rc21, *rc22, *rc23, *row3, *rc31, *rc32, *bottom_menue, *btn_reset, *btn_speichern, *btn_abbrechen,
              *label_titel, *label_feld, *label_reihe, *label_platz, *label_status, *label_licht, *label_gelaende, *label_boden, *label_bodenfeuchtigkeit, *label_gps,
              *entrybox_feld, *entrybox_reihe, *entrybox_platz, *combobox_status, *combobox_licht, *combobox_gelaende, *combobox_boden, *combobox_bodenfeuchtigkeit, *entrybox_gps;

  // Label
  const gchar *str = "<b>Neuen Standort Anlegen</b>";
  label_titel = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_titel), str);  
  label_feld = gtk_label_new("Feld");
  label_reihe = gtk_label_new("Reihe");
  label_platz = gtk_label_new("Platz");
  label_status = gtk_label_new("Status");
  label_licht = gtk_label_new("Licht");
  label_gelaende = gtk_label_new("Gelände");
  label_boden = gtk_label_new("Boden");
  label_bodenfeuchtigkeit = gtk_label_new("Bodenfeuchtigkeit");
  label_gps = gtk_label_new("GPS Koordinaten");

  //Boxes
  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  hbox_StaortHinzu = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  row1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  row2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  row3 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  rc11 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc12 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc13 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc14 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc21 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc22 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc23 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc31 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc32 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  bottom_menue = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  btn_reset = gtk_button_new_with_label("\t   Formular\t\n\tzurücksetzen\t");
  btn_speichern = gtk_button_new_with_label("\t\tSpeichern\t\t");
  btn_abbrechen = gtk_button_new_with_label("\t\tAbbrechen\t\t");

  // EntryBox 'Feld'
  entrybox_feld = gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(entrybox_feld), 3);
  //creates string with 'Feldern' for tooltip
  saveSqlQuerry("SELECT DISTINCT StandortID/1000000-100 FROM Standort");
  vector<vector<string> > vecFelder = executeSqlStatement();
  string felder = "aktive Felder:\n\n" + string_formater(vecFelder, 4);
  gtk_widget_set_tooltip_text(GTK_WIDGET(entrybox_feld), felder.c_str());
  gtk_entry_set_overwrite_mode(GTK_ENTRY(entrybox_feld), TRUE);

  // EntryBox 'Reihe'
  entrybox_reihe = gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(entrybox_reihe), 4);
  gtk_entry_set_overwrite_mode(GTK_ENTRY(entrybox_reihe), TRUE);

  // EntryBox 'Platz'
  entrybox_platz = gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(entrybox_platz), 4);
  gtk_entry_set_overwrite_mode(GTK_ENTRY(entrybox_platz), TRUE);

  // ComboBox 'Status'
  combobox_status = gtk_combo_box_text_new();  
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_status), NULL, "nicht zu verwenden");    
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_status), NULL, "Unterlage vorhanden");    
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_status), NULL, "aktiv & frei"); 

  // ComboBox 'Licht'
  combobox_licht = gtk_combo_box_text_new();  
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_licht), NULL, "Schatten");     
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_licht), NULL, "Halbschatten");     
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_licht), NULL, "Sonne");  

  // ComboBox 'Gelände'
  combobox_gelaende = gtk_combo_box_text_new();  
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_gelaende), NULL, "Flach");     
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_gelaende), NULL, "Hang");     
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_gelaende), NULL, "Steilhang");    

  // ComboBox 'Boden'
  combobox_boden = gtk_combo_box_text_new();  
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_boden), NULL, "Sandig");   
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_boden), NULL, "Lemig");   
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_boden), NULL, "Erdig");   
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_boden), NULL, "Sumpf");   
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_boden), NULL, "Verwitterter Fels");   
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_boden), NULL, "Steinig");   
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_boden), NULL, "Gestein");   

  // ComboBox 'Bodenfeuchtigkeit'
  combobox_bodenfeuchtigkeit = gtk_combo_box_text_new();  
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_bodenfeuchtigkeit), NULL, "Trocken");  
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_bodenfeuchtigkeit), NULL, "Durchschnitt");  
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_bodenfeuchtigkeit), NULL, "Staunässe");  

  // EntryBox 'GPS'
  entrybox_gps = gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(entrybox_gps), 3);

  //Assign all boxes to parent box
  gtk_box_pack_start(GTK_BOX(rc11), label_feld, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(rc11), entrybox_feld, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc12), label_reihe, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(rc12), entrybox_reihe, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc13), label_platz, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(rc13), entrybox_platz, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc14), label_status, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(rc14), combobox_status, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(row1), rc11, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(row1), rc12, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(row1), rc13, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(row1), rc14, FALSE, TRUE, 10);

  gtk_box_pack_start(GTK_BOX(rc21), label_licht, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(rc21), combobox_licht, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc22), label_gelaende, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(rc22), combobox_gelaende, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc23), label_boden, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(rc23), combobox_boden, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(row2), rc21, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(row2), rc22, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(row2), rc23, FALSE, TRUE, 10);

  gtk_box_pack_start(GTK_BOX(rc31), label_bodenfeuchtigkeit, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(rc31), combobox_bodenfeuchtigkeit, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc32), label_gps, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(rc32), entrybox_gps, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(row3), rc31, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(row3), rc32, FALSE, TRUE, 10);

  gtk_box_pack_start(GTK_BOX(vbox), label_titel, FALSE, FALSE, 20);
  gtk_box_pack_start(GTK_BOX(vbox), row1, FALSE, FALSE, 20);
  gtk_box_pack_start(GTK_BOX(vbox), row2, FALSE, FALSE, 20);
  gtk_box_pack_start(GTK_BOX(vbox), row3, FALSE, FALSE, 20);
  gtk_box_pack_start(GTK_BOX(bottom_menue), btn_reset, FALSE, FALSE, 10);
  gtk_box_pack_end(GTK_BOX(bottom_menue), btn_abbrechen, FALSE, FALSE, 10);
  gtk_box_pack_end(GTK_BOX(bottom_menue), btn_speichern, FALSE, FALSE, 10);
  gtk_box_pack_end(GTK_BOX(vbox), bottom_menue, FALSE, FALSE, 10);
  gtk_box_pack_start(GTK_BOX(hbox_StaortHinzu), vbox, TRUE, TRUE, 10);
  gtk_container_add(GTK_CONTAINER(window), hbox_StaortHinzu);
  gtk_widget_show_all(window);

  g_signal_connect(G_OBJECT(entrybox_feld), "changed", 
        G_CALLBACK(entrybox_feldStaort_changed), (gpointer) window);

  g_signal_connect(G_OBJECT(entrybox_reihe), "changed", 
        G_CALLBACK(entrybox_reiheStaort_changed), (gpointer) window);

  g_signal_connect(G_OBJECT(entrybox_platz), "changed", 
        G_CALLBACK(entrybox_platzStaort_changed), (gpointer) window);

  g_signal_connect(G_OBJECT(combobox_status), "changed", 
        G_CALLBACK(combobox_statusStaort_changed), (gpointer) window);
  
  g_signal_connect(G_OBJECT(combobox_licht), "changed", 
        G_CALLBACK(combobox_lichtStaort_changed), (gpointer) window);

  g_signal_connect(G_OBJECT(combobox_gelaende), "changed", 
        G_CALLBACK(combobox_gelaendeStaort_changed), (gpointer) window);

  g_signal_connect(G_OBJECT(combobox_boden), "changed", 
        G_CALLBACK(combobox_bodenStaort_changed), (gpointer) window);

  g_signal_connect(G_OBJECT(combobox_bodenfeuchtigkeit), "changed", 
        G_CALLBACK(combobox_bodenfeuchtigkeitStaort_changed), (gpointer) window);

  g_signal_connect(G_OBJECT(entrybox_gps), "changed", 
        G_CALLBACK(entrybox_gpsStaort_changed), (gpointer) window);

  g_signal_connect(G_OBJECT(btn_reset), "clicked",
        G_CALLBACK(btn_staortHinzu_clicked), hbox_StaortHinzu);

  g_signal_connect(G_OBJECT(btn_speichern), "clicked",
        G_CALLBACK(btn_StaOrtHinzuSpeichern_clicked), hbox_StaortHinzu);

  g_signal_connect(G_OBJECT(btn_abbrechen), "clicked",
       G_CALLBACK(stammd_standort_activated), hbox_StaortHinzu);

}

void btn_StaOrtHinzuSpeichern_clicked(GtkWidget *widget, gpointer data) {

  string *arrStaOrtData;
  arrStaOrtData = getArrayStaOrtData();

  if (arrStaOrtData[0] == "" || arrStaOrtData[1] == "" || arrStaOrtData[2] == "" || arrStaOrtData[3] == "") { // Check if Feld, Reihe, Platz and Status are choosen
    runDialog(window, "Bitte geben Sie mindestens Feld, Reihe, Platz und Status an");
  }else {
    string standortUF = "F" + arrStaOrtData[0] + "R" + arrStaOrtData[1] + "P" + arrStaOrtData[2];
    string standortPF = modStandortPF(standortUF);
    saveSqlQuerry("SELECT StandortID FROM Standort WHERE StandortID = " + standortPF);
    vector<vector<string> > staOrtID = executeSqlStatement();
    if (staOrtID.size() == 1) {
      runDialog(window, "Standort existiert bereits");
    }
    else {
      arrStaOrtData[2] = standortPF;
      sqlBuilderInsertStaOrt(arrStaOrtData);
      executeSqlStatement();

      runDialog(window, "Eingaben erfolgreich gespeichert");
      
      gtk_widget_destroy(GTK_WIDGET(data));
      GtkWidget *vbox;
      vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

      stammd_standort_activated(widget, vbox);
    }
  }

  // for (int i = 0; i < 9; i++)
  // {
  //   cout << arrStaOrtData[i] << endl;
  // }
  
  
}

void standortAnzeigen_refresh(GtkWidget *widget, gpointer data) {

  standort_auswahl[0] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(licht_schatten));
  standort_auswahl[1] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(licht_halbschatten));
  standort_auswahl[2] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(licht_sonnig));
  standort_auswahl[3] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(licht_na));
  standort_auswahl[4] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(gelaende_flach));
  standort_auswahl[5] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(gelaende_hang));
  standort_auswahl[6] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(gelaende_steilhang));
  standort_auswahl[7] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(gelaende_na));
  standort_auswahl[8] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(bodentyp_sandig));
  standort_auswahl[9] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(bodentyp_lemig));
  standort_auswahl[10] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(bodentyp_erdig));
  standort_auswahl[11] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(bodentyp_supmf));
  standort_auswahl[12] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(bodentyp_verw_feld));
  standort_auswahl[13] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(bodentyp_steinig));
  standort_auswahl[14] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(bodentyp_gestein));
  standort_auswahl[15] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(bodentyp_na));
  standort_auswahl[16] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(bodenfeuch_trocken));
  standort_auswahl[17] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(bodenfeuch_durchschnitt));
  standort_auswahl[18] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(bodenfeuch_staunaesse));
  standort_auswahl[19] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(bodenfeuch_na));
  standort_auswahl[20] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(aktiv_nichtVerwenden));
  standort_auswahl[21] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(aktiv_besetzt));
  standort_auswahl[22] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(aktiv_unterlage));
  standort_auswahl[23] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(aktiv_inaktivieren));
  standort_auswahl[24] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(aktiv_frei));
  standort_auswahl[25] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(aktiv_na));

  gtk_widget_destroy(GTK_WIDGET(data));
  GtkWidget *vbox;
  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

  standortAnzeigen_clicked(window, vbox);
}

GtkWidget * create_standort_map (void){

  GtkWidget *map;
  map = gtk_fixed_new();
  int sizeArray = 26;
  string attributeEquals[26] = {"Licht=\"Schatten\"", "Licht=\"Halbschatten\"", "Licht=\"Sonne\"", "Licht=\"\" OR Licht IS NULL",
                                    "Gelaende=\"Flach\"", "Gelaende=\"Hang\"", "Gelaende=\"Steilhang\"", "Gelaende=\"\" OR Gelaende IS NULL",
                                    "BodenTyp=\"Sandig\"", "BodenTyp=\"Lemig\"", "BodenTyp=\"Erdig\"", "BodenTyp=\"Sumpf\"", "BodenTyp=\"verwitterter Fels\"", "BodenTyp=\"Steinig\"", "BodenTyp=\"Gestein\"", "BodenTyp=\"\" OR BodenTyp IS NULL",
                                    "BodenFeuchtigkeit=\"Trocken\"", "BodenFeuchtigkeit=\"Durchschnitt\"", "BodenFeuchtigkeit=\"Staunässe\"", "BodenFeuchtigkeit=\"\" OR BodenFeuchtigkeit IS NULL",
                                    "AktivInaktiv=0", "AktivInaktiv=1", "AktivInaktiv=2", "AktivInaktiv=3", "AktivInaktiv=4", "AktivInaktiv=\"\" OR AktivInaktiv IS NULL"};
  vector<string> vecSelect;
  vector<string> vecFromJoin;
  vector<string> vecOn;
  vector<string> vecWhereFeld;
  vector<vector<string> > vecWhereAttributes;
  std::vector<int> platznummern;
  std::vector<int> platznummernAll;
  std::vector<int> reihennummern;
  vecSelect.push_back("StandortID");   
  vecSelect.push_back("Licht"); 
  vecSelect.push_back("Gelaende"); 
  vecSelect.push_back("BodenTyp"); 
  vecSelect.push_back("BodenFeuchtigkeit"); 
  vecSelect.push_back("AktivInaktiv"); 
  vecFromJoin.push_back("Standort");
  string* ptrWhereFeld = getWhereFeld();
  string whereStr = *ptrWhereFeld;
  saveSqlQuerry("SELECT StandortID FROM Standort WHERE " + whereStr);
  vector<vector<string> > vecStandortIDs = executeSqlStatement();
  saveSqlQuerry("SELECT max(StandortID/1000%100) FROM Standort WHERE " + whereStr);
  int reiheHigh = string_to_int(executeSqlStatement_singleHit());
  vecWhereFeld.push_back(whereStr);
  vecWhereAttributes = getAttributeStandortChoice(standort_auswahl, attributeEquals, sizeArray); // returns 2D vector with 'attributeEquals' if record for standort_auswahl is TRUE
  sqlBuilderSelect(vecSelect, vecFromJoin, vecOn, vecWhereFeld, vecWhereAttributes, "StandortID");
  std::vector<std::vector<string> > vecRecords = executeSqlStatement();
  
  // Returns empty map 
  if (vecRecords.size() == 0) { return map; }
  
  for (int i = 0; i < vecRecords.size(); i++) {
    reihennummern.push_back(string_to_int_minusHundred(vecRecords[i].at(0), 3, 3));
    platznummern.push_back(string_to_int_minusHundred(vecRecords[i].at(0), 6, 3));
  }

  for (int i = 0; i < vecStandortIDs.size(); i++) {
    platznummernAll.push_back(string_to_int_minusHundred(vecStandortIDs[i].at(0), 6, 3));
  }

  // returns lowest number from vector
  int numberLow = find_lowest_number(platznummernAll);
  int x, y, reihe = 0, platz;
  int anzStandorte = vecRecords.size();
  int reiheOffset = reihennummern[reihennummern.size()-1];

  //fills map
  for (int i = vecRecords.size()-1; i >= 0; i--) {

    platz = platznummern[i];
    x = 10 + ((numberLow*(-1)*75) + (platz*75));
    y = 10 + (((-1*reiheHigh+reiheOffset)*(-1)*75) + (reihe*75));

    char koordinaten[8];
    snprintf(koordinaten, 8, "R%dP%d", reihennummern[i], platz); // combine string and integer into variable koordinaten
    GtkWidget *button[i+1], *image[i+1], *label[i+1];
    button[i+1] = gtk_button_new();
    label[i+1] = gtk_label_new(koordinaten);
    int aktInakt = string_to_int(vecRecords[i].at(5));
    std::string aktInaktLabel;
    switch (aktInakt)
    {
    case 0:
      aktInaktLabel = "images/do_not_use.png";
      break;
    case 1:
      aktInaktLabel = "images/active_and_used.png";
      break;
    case 2:
      aktInaktLabel = "images/rootstock_present.png";
      break;
    case 3:
      aktInaktLabel = "images/to_be_inactivated.png";
      break;
    case 4:
      aktInaktLabel = "images/active_and_free.png";
      break;
    default:
      break;
    }
    image[i+1] = gtk_image_new_from_file (aktInaktLabel.c_str());
    gtk_button_set_image(GTK_BUTTON(button[i+1]), image[i+1]);
    gtk_fixed_put(GTK_FIXED(map), label[i+1], x, y);
    gtk_fixed_put(GTK_FIXED(map), button[i+1], x, (y+20));
    gtk_widget_set_size_request(button[i+1], 30, 30);

    if (i > 0) {
      if (reihennummern[i] != reihennummern[i-1]) {
        reihe += reihennummern[i] - reihennummern[i-1];
      }
    }
  }
  
  return map;
}



//Bäume
void stammd_baeume_activated(GtkWidget *widget, gpointer data) {

  gtk_widget_destroy(GTK_WIDGET(data));

  GtkWidget *vbox, *hbox, *btn_anzeigen, *btn_anlegen, *btn_zurueck;

  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  btn_anzeigen = gtk_button_new_with_label("Bäume Anzeigen");
  btn_anlegen = gtk_button_new_with_label("Neuen Baum\n   Anlegen");
  btn_zurueck = gtk_button_new_with_label("Zurück");

  //Assign all boxes to parent box
  gtk_container_add(GTK_CONTAINER(window), hbox);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 150);
  gtk_box_pack_start(GTK_BOX(vbox), btn_anzeigen, TRUE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(vbox), btn_anlegen, TRUE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(vbox), btn_zurueck, TRUE, TRUE, 10);
  gtk_widget_show_all(window);

  g_signal_connect(G_OBJECT(btn_anzeigen), "clicked",
    G_CALLBACK(baeumeAnzeigen_clicked), hbox); //Show trees

 g_signal_connect(G_OBJECT(btn_anlegen), "clicked",
    G_CALLBACK(btn_baumHinzu_clicked), hbox); //Show trees

  g_signal_connect(G_OBJECT(btn_zurueck), "clicked",
      G_CALLBACK(hauptmenue), hbox); // Back to Main Menu
}

void baeumeAnzeigen_reset(GtkWidget *widget, gpointer data) {

// initial value set for baeumeAnzeige check-buttons
  for(int i = 0; i < 48; ++i){              // Set Baeume check-menue all true
    baeume_auswahl[i] = TRUE;
  }

  gtk_widget_destroy(GTK_WIDGET(data));
  GtkWidget *vbox;
  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

  baeumeAnzeigen_clicked(window, vbox);
}

void baeumeAnzeigen_clicked(GtkWidget *widget, gpointer data) {

  gtk_widget_destroy(GTK_WIDGET(data));

  GtkWidget *vbox, *hbox, *hbox_bearb, *bottom_menue,
            *view, *scrollWindow,
            *entrybox_baumnr,
            *anzahl, *btn_bearbeiten, *btn_drucken, *btn_zurueck, *btn_reset;
  string numFalse;  

  // FIRST MENU Art
  GtkWidget *bmMenue_Art, *baumArt;

  bmMenue_Art = gtk_menu_new(); //define widget
  numFalse = "Art " + dropdownChecker(baeume_auswahl, 0, 24);
  const char* cNumFalse1 = numFalse.c_str();
  baumArt = gtk_menu_item_new_with_label(cNumFalse1); // menu label

  art_kirsche = gtk_check_menu_item_new_with_label("Kirsche"); // define submenue with check-item
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(art_kirsche), baeume_auswahl[0]); // set check-menue value
  art_apfel = gtk_check_menu_item_new_with_label("Apfel");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(art_apfel), baeume_auswahl[1]);
  art_birne = gtk_check_menu_item_new_with_label("Birne");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(art_birne), baeume_auswahl[2]);
  art_quitte = gtk_check_menu_item_new_with_label("Quitte");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(art_quitte), baeume_auswahl[3]);
  art_reneklode = gtk_check_menu_item_new_with_label("Reneklode");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(art_reneklode), baeume_auswahl[4]);
  art_pflaume = gtk_check_menu_item_new_with_label("Pflaume");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(art_pflaume), baeume_auswahl[5]);
  art_mirabelle = gtk_check_menu_item_new_with_label("Mirabelle");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(art_mirabelle), baeume_auswahl[6]);
  art_zwetschge = gtk_check_menu_item_new_with_label("Zwetschge");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(art_zwetschge), baeume_auswahl[7]);
  art_mandel = gtk_check_menu_item_new_with_label("Mandel");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(art_mandel), baeume_auswahl[8]);
  art_pfirsich = gtk_check_menu_item_new_with_label("Pfirsich");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(art_pfirsich), baeume_auswahl[9]);
  art_nektarine = gtk_check_menu_item_new_with_label("Nektarine");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(art_nektarine), baeume_auswahl[10]);
  art_aprikose = gtk_check_menu_item_new_with_label("Aprikose");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(art_aprikose), baeume_auswahl[11]);
  art_wallnuss = gtk_check_menu_item_new_with_label("Wallnuss");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(art_wallnuss), baeume_auswahl[12]);
  art_pekanuss = gtk_check_menu_item_new_with_label("Pekanuss");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(art_pekanuss), baeume_auswahl[13]);
  art_feige = gtk_check_menu_item_new_with_label("Feige");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(art_feige), baeume_auswahl[14]);
  art_mango = gtk_check_menu_item_new_with_label("Mango");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(art_mango), baeume_auswahl[15]);
  art_papaya = gtk_check_menu_item_new_with_label("Papaya");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(art_papaya), baeume_auswahl[16]);
  art_avocado = gtk_check_menu_item_new_with_label("Avocado");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(art_avocado), baeume_auswahl[17]);
  art_olive = gtk_check_menu_item_new_with_label("Olive");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(art_olive), baeume_auswahl[18]);
  art_orange = gtk_check_menu_item_new_with_label("Orange");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(art_orange), baeume_auswahl[19]);
  art_clementine = gtk_check_menu_item_new_with_label("Clementine");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(art_clementine), baeume_auswahl[20]);
  art_mandarine = gtk_check_menu_item_new_with_label("Mandarine");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(art_mandarine), baeume_auswahl[21]);
  art_grapefruit = gtk_check_menu_item_new_with_label("Grapefruit");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(art_grapefruit), baeume_auswahl[22]);
  art_maulbeere = gtk_check_menu_item_new_with_label("Maulbeere");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(art_maulbeere), baeume_auswahl[23]);
  art_tafeltraube = gtk_check_menu_item_new_with_label("Tafeltraube");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(art_tafeltraube), baeume_auswahl[24]);

  // Assign Subitem to Menu
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Art), art_kirsche);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Art), art_apfel);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Art), art_birne);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Art), art_quitte);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Art), art_reneklode);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Art), art_pflaume);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Art), art_mirabelle);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Art), art_zwetschge);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Art), art_mandel);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Art), art_pfirsich);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Art), art_nektarine);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Art), art_aprikose);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Art), art_wallnuss);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Art), art_pekanuss);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Art), art_feige);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Art), art_mango);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Art), art_papaya);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Art), art_avocado);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Art), art_olive);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Art), art_orange);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Art), art_clementine);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Art), art_mandarine);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Art), art_grapefruit);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Art), art_maulbeere);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Art), art_tafeltraube);

  gtk_menu_item_set_submenu(GTK_MENU_ITEM(baumArt), bmMenue_Art);


  // SECOND MENU Wuchsform
  GtkWidget *bmMenue_Wuchsform, *baumWuchsform;

  bmMenue_Wuchsform = gtk_menu_new(); //define widget
  numFalse = "Wuchsform " + dropdownChecker(baeume_auswahl, 25, 30);
  const char* cNumFalse2 = numFalse.c_str();
  baumWuchsform = gtk_menu_item_new_with_label(cNumFalse2); // menu label

  wuchsform_buschbaum = gtk_check_menu_item_new_with_label("Buschbaum");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(wuchsform_buschbaum), baeume_auswahl[25]);
  wuchsform_viertelstamm = gtk_check_menu_item_new_with_label("Viertelstamm");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(wuchsform_viertelstamm), baeume_auswahl[26]);
  wuchsform_halbstamm = gtk_check_menu_item_new_with_label("Halbstamm");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(wuchsform_halbstamm), baeume_auswahl[27]);
  wuchsform_hochstamm = gtk_check_menu_item_new_with_label("Hochstamm");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(wuchsform_hochstamm), baeume_auswahl[28]);
  wuchsform_spindel = gtk_check_menu_item_new_with_label("Spindel");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(wuchsform_spindel), baeume_auswahl[29]);
  wuchsform_na = gtk_check_menu_item_new_with_label("n.a.");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(wuchsform_na), baeume_auswahl[30]);

    // Assign Subitem to Menu
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Wuchsform), wuchsform_buschbaum);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Wuchsform), wuchsform_viertelstamm);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Wuchsform), wuchsform_halbstamm);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Wuchsform), wuchsform_hochstamm);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Wuchsform), wuchsform_spindel);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Wuchsform), wuchsform_na);

  gtk_menu_item_set_submenu(GTK_MENU_ITEM(baumWuchsform), bmMenue_Wuchsform);


  // THIRD MENU Veredelt
  GtkWidget *bmMenue_Veredelt, *baumVeredelt;

  bmMenue_Veredelt = gtk_menu_new(); //define widget
  numFalse = "Veredelt " + dropdownChecker(baeume_auswahl, 31, 33);
  const char* cNumFalse3 = numFalse.c_str();
  baumVeredelt = gtk_menu_item_new_with_label(cNumFalse3); // menu label

  veredelt_ja = gtk_check_menu_item_new_with_label("Ja");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(veredelt_ja), baeume_auswahl[31]);
  veredelt_nein = gtk_check_menu_item_new_with_label("Nein");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(veredelt_nein), baeume_auswahl[32]);
  veredelt_na = gtk_check_menu_item_new_with_label("n.a.");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(veredelt_na), baeume_auswahl[33]);

  // Assign Subitem to Menu
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Veredelt), veredelt_ja);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Veredelt), veredelt_nein);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Veredelt), veredelt_na);

  gtk_menu_item_set_submenu(GTK_MENU_ITEM(baumVeredelt), bmMenue_Veredelt);


  // FOURTH MENU Befruchtung
  GtkWidget *bmMenue_Befruchtung, *baumBefruchtung;

  bmMenue_Befruchtung = gtk_menu_new(); //define widget
  numFalse = "Befruchtung " + dropdownChecker(baeume_auswahl, 34, 37);
  const char* cNumFalse4 = numFalse.c_str();
  baumBefruchtung = gtk_menu_item_new_with_label(cNumFalse4); // menu label

  befruchtung_selbstbefruchter = gtk_check_menu_item_new_with_label("Selbstbefruchter");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(befruchtung_selbstbefruchter), baeume_auswahl[34]);
  befruchtung_teilselbstbefruchter = gtk_check_menu_item_new_with_label("Teilselbstbefruchter");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(befruchtung_teilselbstbefruchter), baeume_auswahl[35]);
  befruchtung_fremdbefruchter = gtk_check_menu_item_new_with_label("Fremdbefruchtung");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(befruchtung_fremdbefruchter), baeume_auswahl[36]);
  befruchtung_na = gtk_check_menu_item_new_with_label("n.a.");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(befruchtung_na), baeume_auswahl[37]);


  // Assign Subitem to Menu
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Befruchtung), befruchtung_selbstbefruchter);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Befruchtung), befruchtung_teilselbstbefruchter);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Befruchtung), befruchtung_fremdbefruchter);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Befruchtung), befruchtung_na);

  gtk_menu_item_set_submenu(GTK_MENU_ITEM(baumBefruchtung), bmMenue_Befruchtung);


  // FIFTH MENU Status
  GtkWidget *bmMenue_Status, *baumStatus;

  bmMenue_Status = gtk_menu_new(); //define widget
  numFalse = "Status " + dropdownChecker(baeume_auswahl, 38, 44);
  const char* cNumFalse5 = numFalse.c_str();
  baumStatus = gtk_menu_item_new_with_label(cNumFalse5); // menu label

  status_bestellt = gtk_check_menu_item_new_with_label("bestellt");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(status_bestellt), baeume_auswahl[38]);
  status_geliefert = gtk_check_menu_item_new_with_label("geliefert");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(status_geliefert), baeume_auswahl[39]);
  status_gepflanzt = gtk_check_menu_item_new_with_label("gepflanzt");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(status_gepflanzt), baeume_auswahl[40]);
  status_aktiv = gtk_check_menu_item_new_with_label("aktiv");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(status_aktiv), baeume_auswahl[41]);
  status_inaktiv = gtk_check_menu_item_new_with_label("inaktiv");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(status_inaktiv), baeume_auswahl[42]);
  status_archiv = gtk_check_menu_item_new_with_label("archiv");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(status_archiv), baeume_auswahl[43]);
  status_na = gtk_check_menu_item_new_with_label("n.a.");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(status_na), baeume_auswahl[44]);

  // Assign Subitem to Menu
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Status), status_bestellt);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Status), status_geliefert);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Status), status_gepflanzt);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Status), status_aktiv);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Status), status_inaktiv);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Status), status_archiv);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Status), status_na);

  gtk_menu_item_set_submenu(GTK_MENU_ITEM(baumStatus), bmMenue_Status);


  // SIXTH MENU Anlieferung
  GtkWidget *bmMenue_Anlieferung, *baumAnlieferung;

  bmMenue_Anlieferung = gtk_menu_new(); //define widget
  numFalse = "Anlieferung " + dropdownChecker(baeume_auswahl, 45, 47);
  const char* cNumFalse6 = numFalse.c_str();
  baumAnlieferung = gtk_menu_item_new_with_label(cNumFalse6); // menu label

  anlieferung_selbst = gtk_check_menu_item_new_with_label("selbst");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(anlieferung_selbst), baeume_auswahl[45]);
  anlieferung_extern = gtk_check_menu_item_new_with_label("extern");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(anlieferung_extern), baeume_auswahl[46]);
  anlieferung_na = gtk_check_menu_item_new_with_label("n.a.");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(anlieferung_na), baeume_auswahl[47]);

  // Assign Subitem to Menu
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Anlieferung), anlieferung_selbst);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Anlieferung), anlieferung_extern);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_Anlieferung), anlieferung_na);

  gtk_menu_item_set_submenu(GTK_MENU_ITEM(baumAnlieferung), bmMenue_Anlieferung);


  // create menu-bar
  GtkWidget *bmMenue_menubar = gtk_menu_bar_new(); // define menubar widget

  // Assign Item to Baeume_Menubar
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_menubar), baumArt);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_menubar), baumWuchsform);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_menubar), baumVeredelt);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_menubar), baumBefruchtung);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_menubar), baumStatus);
  gtk_menu_shell_append(GTK_MENU_SHELL(bmMenue_menubar), baumAnlieferung);

  //Entrybox BaumNr
  entrybox_baumnr = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(entrybox_baumnr), "Baum Nr");
  gtk_entry_set_max_length(GTK_ENTRY(entrybox_baumnr), 3);
  gtk_entry_set_width_chars(GTK_ENTRY(entrybox_baumnr), 8);


  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  hbox_bearb = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  bottom_menue = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

  btn_bearbeiten = gtk_button_new_with_label("Baum bearbeiten");
  btn_drucken = gtk_button_new_with_label("Drucken");
  btn_zurueck = gtk_button_new_with_label("Zurück");
  btn_reset = gtk_button_new_with_label("Auswahl zurücksetzen");

  int sizeAray = 48;
  string attributeEquals[48] = {"OberArt=\"Kirsche\"", "OberArt=\"Apfel\"", "OberArt=\"Birne\"", "OberArt=\"Quitte\"", "OberArt=\"Reneklode\"", "OberArt=\"Pflaume\"", "OberArt=\"Mirabelle\"", "OberArt=\"Zwetschge\"", "OberArt=\"Mandel\"", "OberArt=\"Pfirsich\"", "OberArt=\"Nektarine\"", "OberArt=\"Aprikose\"", "OberArt=\"Wallnuss\"", "OberArt=\"Pekanuss\"", "OberArt=\"Feige\"", "OberArt=\"Mango\"", "OberArt=\"Papaya\"", "OberArt=\"Avocado\"", "OberArt=\"Olive\"", "OberArt=\"Orange\"", "OberArt=\"Clementine\"", "OberArt=\"Mandarine\"", "OberArt=\"Grapefruit\"", "OberArt=\"Maulbeere\"", "OberArt=\"Tafeltraube\"",
                                "WuchsForm=\"Buschbaum\"", "WuchsForm=\"Viertelstamm\"", "WuchsForm=\"Halbstamm\"", "WuchsForm=\"Hochstamm\"", "WuchsForm=\"Spindel\"", "WuchsForm=\"\" OR WuchsForm IS NULL",
                                "VeredeltFlag=\"Ja\"", "VeredeltFlag=\"Nein\"", "VeredeltFlag=\"\" OR VeredeltFlag IS NULL", 
                                "Befruchtung=\"Selbstbefruchter\"", "Befruchtung=\"Teilselbstbefruchter\"", "Befruchtung=\"Fremdbefruchtung\"", "Befruchtung=\"\" OR Befruchtung IS NULL",
                                "Status=\"bestellt\"", "Status=\"geliefert\"", "Status=\"gepflanzt\"", "Status=\"aktiv\"", "Status=\"inaktiv\"", "Status=\"archiv\"", "Status=\"\" OR Status IS NULL", 
                                "FremdTransportFlag=\"selbst\"", "FremdTransportFlag=\"extern\"", "FremdTransportFlag=\"\" OR FremdTransportFlag IS NULL"};

  // Preperation for SQL statement
  vector<string> vecSelect;
  vector<string> vecFromJoin;
  vector<string> vecOn;
  vector<string> vecWhereFeld;
  vector<vector<string> > vecWhereAttributes;
  vecSelect.push_back("BaumID");
  vecSelect.push_back("OberArt");
  vecSelect.push_back("UnterArt");
  vecSelect.push_back("WuchsForm");
  vecSelect.push_back("StandortID");
  vecSelect.push_back("Ursprung");
  vecSelect.push_back("Lieferdatum");
  vecSelect.push_back("PflanzJahr");
  vecSelect.push_back("VeredeltFlag");
  vecSelect.push_back("Befruchtung");
  vecSelect.push_back("Anmerkungen");
  vecSelect.push_back("PflanzBodenzusatz");
  vecSelect.push_back("Unterlage");
  vecSelect.push_back("RfidCode");
  vecSelect.push_back("EhemaligeStandortID");
  vecSelect.push_back("AusfallUrsache");
  vecSelect.push_back("Status");
  vecSelect.push_back("FruchtReifeAnfang");
  vecSelect.push_back("FruchtReifeEnde");
  vecSelect.push_back("BlueteZeitAnfang");
  vecSelect.push_back("BlueteZeitEnde");
  vecSelect.push_back("FremdTransportFlag");
  vecFromJoin.push_back("Baum");  
  vecFromJoin.push_back("Sorte");  
  vecOn.push_back("SortenID");
  vecWhereAttributes = getAttributeBaumChoice(baeume_auswahl, attributeEquals, sizeAray);

  
  

  sqlBuilderSelect(vecSelect, vecFromJoin, vecOn, vecWhereFeld, vecWhereAttributes);
  vector<vector<string> > printData = executeSqlStatement();
  saveDataToFile(printData, "Baeume_Anzeigen.txt");
  view = createTableWithHeader();  

  // generating sum of results in search condition
  int result = getNumberOfRows();
  char text[35];
  snprintf(text, 35, "Zähler: %d                      ", result); // combine string and integer into variable koordinaten
  anzahl = gtk_label_new(text);

  //Scroll Window
  scrollWindow = gtk_scrolled_window_new(NULL, NULL);
  gtk_container_add(GTK_CONTAINER(scrollWindow), view);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollWindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

  gtk_box_pack_start(GTK_BOX(vbox), bmMenue_menubar, FALSE, FALSE, 10);
  gtk_box_pack_end(GTK_BOX(hbox_bearb), btn_bearbeiten, FALSE, TRUE, 10);
  gtk_box_pack_end(GTK_BOX(hbox_bearb), entrybox_baumnr, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(vbox), hbox_bearb, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(hbox), scrollWindow, TRUE, TRUE, 20);
  gtk_box_pack_start(GTK_BOX(bottom_menue), btn_reset, TRUE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(bottom_menue), anzahl, TRUE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(bottom_menue), btn_drucken, TRUE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(bottom_menue), btn_zurueck, TRUE, TRUE, 10);
  gtk_box_pack_end(GTK_BOX(vbox), bottom_menue, FALSE, FALSE, 10);
  gtk_container_add (GTK_CONTAINER (window), vbox);

  gtk_widget_show_all (window);

  // define action for check-item
  g_signal_connect(G_OBJECT(art_kirsche), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(art_apfel), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(art_birne), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(art_quitte), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(art_reneklode), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(art_pflaume), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(art_mirabelle), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(art_zwetschge), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(art_mandel), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(art_pfirsich), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(art_nektarine), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(art_aprikose), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(art_wallnuss), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(art_pekanuss), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(art_feige), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(art_mango), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(art_papaya), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(art_avocado), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(art_olive), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(art_orange), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(art_clementine), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(art_mandarine), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(art_grapefruit), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(art_maulbeere), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(art_tafeltraube), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(wuchsform_buschbaum), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(wuchsform_viertelstamm), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(wuchsform_halbstamm), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(wuchsform_hochstamm), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(wuchsform_spindel), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(wuchsform_na), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(veredelt_ja), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(veredelt_nein), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(veredelt_na), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(befruchtung_selbstbefruchter), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(befruchtung_teilselbstbefruchter), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(befruchtung_fremdbefruchter), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(befruchtung_na), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(status_bestellt), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(status_geliefert), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(status_gepflanzt), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(status_aktiv), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(status_inaktiv), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(status_archiv), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(status_na), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(anlieferung_selbst), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(anlieferung_extern), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);
  g_signal_connect(G_OBJECT(anlieferung_na), "toggled",
    G_CALLBACK(baeumeAnzeigen_refresh), vbox);

  g_signal_connect(G_OBJECT(entrybox_baumnr), "changed", 
        G_CALLBACK(entrybox_nrBaum_changed), NULL);
  g_signal_connect(G_OBJECT(btn_bearbeiten), "clicked",
    G_CALLBACK(btn_baumBearbeiten_clicked), NULL);

  // Drucken
  g_signal_connect(G_OBJECT(btn_drucken), "clicked",
    G_CALLBACK(print_txt), vbox); //

  // action für Reset Button
  g_signal_connect(G_OBJECT(btn_reset), "clicked",
      G_CALLBACK(baeumeAnzeigen_reset), vbox);

  // action for Zurück button
  g_signal_connect(G_OBJECT(btn_zurueck), "clicked",
      G_CALLBACK(stammd_baeume_activated), vbox);
}

int btn_baumBearbeiten_clicked(GtkWidget *widget, gpointer data) {

  if (data != NULL) {
    gtk_widget_destroy(GTK_WIDGET(data));
    gtk_widget_destroy(GTK_WIDGET(window_pu));
  }

  int maxLengthInp = 255;
  string *arrBaumData = getArrayBaumData();
  for (int i = 1; i < 20; i++) { arrBaumData[i] = ""; } // Resets array

  string sql = "SELECT BaumID, OberArt, UnterArt, StandortID, WuchsForm, Ursprung, Lieferdatum, PflanzJahr, VeredeltFlag, Befruchtung, Anmerkungen, PflanzBodenzusatz, Unterlage, RfidCode, Status, FremdTransportFlag, FruchtReifeAnfang, FruchtReifeEnde, BlueteZeitAnfang, BlueteZeitEnde FROM Baum, Sorte WHERE Baum.SortenID = Sorte.SortenID AND BaumID=" + arrBaumData[0];
  saveSqlQuerry(sql, 20);
  vector<vector<string> > vecJoinBaumSorte = executeSqlStatement();
  //Check if Baum exists
  if (vecJoinBaumSorte.size() == 0) {
    runDialog(window, "\nDieser Baum existiert leider nicht\n");
    return 0;
  }

  gtk_widget_set_sensitive(GTK_WIDGET(window), FALSE);
  window_pu = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position(GTK_WINDOW(window_pu), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(window_pu), 750, 500);
  gtk_window_set_title(GTK_WINDOW(window_pu), "FRUITRECORD");
  app_icon = create_pixbuf("./images/menueiconsmall.png");  
  gtk_window_set_icon(GTK_WINDOW(window_pu), app_icon);

  

  

  GtkWidget   *hbox_BaumHinzu, *vbox, *row1, *rc11, *rc12, *rc13, *rc14, *rc15, *row2, *rc21, *rc22, *rc23, *rc24, *row3, *rc31, *rc32, *rc33, *row4, *rc41, *rc42, *rc43, *rc44, *row5, *rc51, *rc52, *rc53, *rc54, *bottom_menue, *btn_aktualisieren, *btn_speichern, *btn_zurueck,
              *label_titel, *label_art, *label_sorte, *label_standort, *label_wuchsform, *label_herkunft, *label_lieferdatum, *label_pflanzjahr, *label_veredelt, *label_befruchtung, *label_anmerkungen, *label_pflanzbodenzusatz, *label_unterlage, *label_rfid, *label_status, *label_anlieferung, *label_fruchtanfang, *label_fruchtende, *label_blueteanfang, *label_blueteende, *label_empty,
              *comboBox_art, *comboBox_sorte, *comboBox_standort, *combobox_wuchsform, *entrybox_herkunft, *entrybox_lieferdatum, *entrybox_pflanzjahr, *combobox_veredelt, *combobox_befruchtung, *entrybox_anmerkungen, *entrybox_pflanzbodenzusatz, *entrybox_unterlage, *entrybox_rfid, *combobox_status, *combobox_anlieferung, *entrybox_fruchtanfang, *entrybox_fruchtende, *entrybox_blueteanfang, *entrybox_blueteende,
              *image_reload;

  // Label
  string str = "<span size=\"large\"><b>Baum Nr." + vecJoinBaumSorte[0].at(0) + " bearbeiten</b></span>";
  label_titel = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_titel), str.c_str());

  str = "<b>Art:</b> " + vecJoinBaumSorte[0].at(1);
  label_art =gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_art), str.c_str());

  str = "<b>Sorte:</b> " + vecJoinBaumSorte[0].at(2);
  label_sorte =gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_sorte), str.c_str());

  string standortUF;
  if (vecJoinBaumSorte[0].at(3) != "") {
    standortUF = modStandortUF(vecJoinBaumSorte[0].at(3));
  }
  str = "<b>Standort:</b> " + standortUF;
  label_standort =gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_standort), str.c_str());

  str = "<b>Wuchsform:</b> " + vecJoinBaumSorte[0].at(4);
  label_wuchsform =gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_wuchsform), str.c_str());

  str = "<b>Herkunft</b> ";
  label_herkunft =gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_herkunft), str.c_str());

  str = "<b>Lieferdatum</b> ";
  label_lieferdatum =gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_lieferdatum), str.c_str());

  str = "<b>Pflanzjahr</b> ";
  label_pflanzjahr =gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_pflanzjahr), str.c_str());

  str = "<b>Veredelt:</b> " + vecJoinBaumSorte[0].at(8);
  label_veredelt =gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_veredelt), str.c_str());

  str = "<b>Befruchtung:</b> " + vecJoinBaumSorte[0].at(9);
  label_befruchtung =gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_befruchtung), str.c_str());

  str = "<b>Anmerkungen</b> ";
  label_anmerkungen =gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_anmerkungen), str.c_str());

  str = "<b>Pflanzbodenzusatz</b> ";
  label_pflanzbodenzusatz =gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_pflanzbodenzusatz), str.c_str());

  str = "<b>Unterlage</b> ";
  label_unterlage =gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_unterlage), str.c_str());

  str = "<b>RFID Kennung</b> ";
  label_rfid =gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_rfid), str.c_str());

  str = "<b>Status:</b> " + vecJoinBaumSorte[0].at(14);
  label_status =gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_status), str.c_str());

  str = "<b>Anlieferung:</b> " + vecJoinBaumSorte[0].at(15);
  label_anlieferung =gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_anlieferung), str.c_str());

  str = "<b>Fruchtreife Anfang</b> ";
  label_fruchtanfang =gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_fruchtanfang), str.c_str());

  str = "<b>Fruchtreife Ende</b> ";
  label_fruchtende =gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_fruchtende), str.c_str());

  str = "<b>Bluete Anfang</b> ";
  label_blueteanfang =gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_blueteanfang), str.c_str());

  str = "<b>Bluete Ende</b> ";
  label_blueteende =gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_blueteende), str.c_str());

  label_empty = gtk_label_new(""); 
  image_reload = gtk_image_new_from_file("images/reload_small.png");

  // ComboBox 'Art'
  comboBox_art = gtk_combo_box_text_new();  
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Apfel");    
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Avokado");    
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Aprikose");    
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Birne");    
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Clementine");    
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Feige");    
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Grapefruit");    
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Kirsche");           
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Mandarine");           
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Mandel");           
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Mango");           
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Maulbeere");           
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Mirabelle");           
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Nektarine");           
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Olive");           
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Orange");           
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Papaya");           
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Pekanuss");           
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Pflaume");           
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Pfirsich");           
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Quitte");           
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Reneklode");           
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Tafeltraube");           
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Wallnuss");           
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Zwetschge");           

  // ComboBox 'Sorte'
  comboBox_sorte = gtk_combo_box_text_new();  
  if (arrBaumData[1] == "") {
    saveSqlQuerry("SELECT DISTINCT UnterArt FROM Sorte ORDER BY UnterArt");
    gtk_widget_set_sensitive(GTK_WIDGET(comboBox_sorte), FALSE);
  }else {
    saveSqlQuerry("SELECT DISTINCT UnterArt FROM Sorte WHERE OberArt = '" + arrBaumData[1] + "' ORDER BY UnterArt");
  }
  vector<vector<string> > vecSorten = executeSqlStatement();
  for (int i = 0; i < vecSorten.size(); i++) {
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_sorte), NULL, (vecSorten[i].at(0)).c_str());
    // cout << vecSorten[i].at(0) << "!" << endl;
  }
  
  // ComboBox 'Standort'
  comboBox_standort = gtk_combo_box_text_new();  
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_standort), NULL, "Kein Standort"); 
  saveSqlQuerry("SELECT StandortID FROM Standort WHERE AktivInaktiv = 2 OR AktivInaktiv = 4");
  vector<vector<string> > vecStandorte = executeSqlStatement();
  for (int i = 0; i < vecStandorte.size(); i++) {
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_standort), NULL, (modStandortUF(vecStandorte[i].at(0))).c_str());
  }

  // ComboBox 'Wuchsform'
  combobox_wuchsform = gtk_combo_box_text_new();  
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_wuchsform), NULL, "Buschbaum"); 
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_wuchsform), NULL, "Viertelstamm"); 
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_wuchsform), NULL, "Halbstamm"); 
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_wuchsform), NULL, "Hochstamm"); 
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_wuchsform), NULL, "Spindel"); 

  // EntryBox 'Herkunft'
  entrybox_herkunft = gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(entrybox_herkunft), maxLengthInp);
  gtk_entry_set_text(GTK_ENTRY(entrybox_herkunft), vecJoinBaumSorte[0].at(5).c_str());

  // EntryBox 'Lieferdatum'
  entrybox_lieferdatum = gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(entrybox_lieferdatum), 4);
  gtk_entry_set_overwrite_mode(GTK_ENTRY(entrybox_lieferdatum), TRUE);
  gtk_entry_set_text(GTK_ENTRY(entrybox_lieferdatum), vecJoinBaumSorte[0].at(6).c_str());

  // EntryBox 'Pflanzjahr'
  entrybox_pflanzjahr = gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(entrybox_pflanzjahr), 4);
  gtk_entry_set_overwrite_mode(GTK_ENTRY(entrybox_pflanzjahr), TRUE);
  gtk_entry_set_text(GTK_ENTRY(entrybox_pflanzjahr), vecJoinBaumSorte[0].at(7).c_str());

  // ComboBox 'Veredelt'
  combobox_veredelt = gtk_combo_box_text_new();  
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_veredelt), NULL, "Ja");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_veredelt), NULL, "Nein");

  // ComboBox 'Befruchtung'
  combobox_befruchtung = gtk_combo_box_text_new();  
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_befruchtung), NULL, "Selbstbefruchter");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_befruchtung), NULL, "Teilselbstbefruchter");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_befruchtung), NULL, "Fremdbefruchtung");

  // EntryBox 'Anmerkungen'
  entrybox_anmerkungen = gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(entrybox_anmerkungen), maxLengthInp);
  gtk_entry_set_text(GTK_ENTRY(entrybox_anmerkungen), vecJoinBaumSorte[0].at(10).c_str());

  // EntryBox 'Pflanzbodenzusatz'
  entrybox_pflanzbodenzusatz = gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(entrybox_pflanzbodenzusatz), maxLengthInp);
  gtk_entry_set_text(GTK_ENTRY(entrybox_pflanzbodenzusatz), vecJoinBaumSorte[0].at(11).c_str());

  // EntryBox 'Unterlage'
  entrybox_unterlage = gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(entrybox_unterlage), maxLengthInp);
  gtk_entry_set_text(GTK_ENTRY(entrybox_unterlage), vecJoinBaumSorte[0].at(12).c_str());

  // EntryBox 'RFID'
  entrybox_rfid = gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(entrybox_rfid), maxLengthInp);
  gtk_entry_set_text(GTK_ENTRY(entrybox_rfid), vecJoinBaumSorte[0].at(13).c_str());

  // ComboBox 'Status'
  combobox_status = gtk_combo_box_text_new();  
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_status), NULL, "bestellt");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_status), NULL, "geliefert");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_status), NULL, "gepflanzt");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_status), NULL, "aktiv");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_status), NULL, "inaktiv");

  // ComboBox 'Anlieferung'
  combobox_anlieferung = gtk_combo_box_text_new();  
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_anlieferung), NULL, "selbst");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_anlieferung), NULL, "extern");

  // EntryBox 'Fruchtreife Anfang'
  entrybox_fruchtanfang = gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(entrybox_fruchtanfang), 2);
  gtk_entry_set_text(GTK_ENTRY(entrybox_fruchtanfang), vecJoinBaumSorte[0].at(16).c_str());

  // EntryBox 'Fruchtreife Ende'
  entrybox_fruchtende = gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(entrybox_fruchtende), 2);
  gtk_widget_set_tooltip_text(GTK_WIDGET(entrybox_fruchtende), "Wenn Feld leer gelassen wird, dann wird die KW von Fruchtreife Anfang übernommen");
  gtk_entry_set_text(GTK_ENTRY(entrybox_fruchtende), vecJoinBaumSorte[0].at(17).c_str());

  // EntryBox 'Bluete Anfang'
  entrybox_blueteanfang = gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(entrybox_blueteanfang), 2);
  gtk_entry_set_text(GTK_ENTRY(entrybox_blueteanfang), vecJoinBaumSorte[0].at(18).c_str());

  // EntryBox 'Bluete Ende'
  entrybox_blueteende = gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(entrybox_blueteende), 2);
  gtk_widget_set_tooltip_text(GTK_WIDGET(entrybox_blueteende), "Wenn Feld leer gelassen wird, dann wird die KW von Bluete Anfang übernommen");
  gtk_entry_set_text(GTK_ENTRY(entrybox_blueteende), vecJoinBaumSorte[0].at(19).c_str());

  //Boxes
  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  hbox_BaumHinzu = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  row1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  row2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  row3 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  row4 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  row5 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  rc11 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc12 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc13 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc14 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc15 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc21 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc22 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc23 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc24 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc31 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc32 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc33 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc41 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc42 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc43 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc44 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc51 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc52 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc53 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc54 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  bottom_menue = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  btn_aktualisieren = gtk_button_new();
  gtk_button_set_image(GTK_BUTTON(btn_aktualisieren), image_reload);
  btn_speichern = gtk_button_new_with_label("\t\tSpeichern\t\t");
  btn_zurueck = gtk_button_new_with_label("\t\tZurück\t\t");

  //Assign all boxes to parent box
  gtk_box_pack_start(GTK_BOX(rc11), label_art, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(rc11), comboBox_art, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc12), label_empty, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(rc12), btn_aktualisieren, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc13), label_sorte, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(rc13), comboBox_sorte, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc14), label_standort, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(rc14), comboBox_standort, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc15), label_wuchsform, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(rc15), combobox_wuchsform, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(row1), rc11, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(row1), rc12, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(row1), rc13, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(row1), rc14, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(row1), rc15, FALSE, TRUE, 10);

  gtk_box_pack_start(GTK_BOX(rc21), label_herkunft, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc21), entrybox_herkunft, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc22), label_lieferdatum, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc22), entrybox_lieferdatum, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc23), label_pflanzjahr, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc23), entrybox_pflanzjahr, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc24), label_veredelt, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc24), combobox_veredelt, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(row2), rc21, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(row2), rc22, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(row2), rc23, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(row2), rc24, FALSE, TRUE, 10);
  
  gtk_box_pack_start(GTK_BOX(rc31), label_befruchtung, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc31), combobox_befruchtung, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc32), label_anmerkungen, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc32), entrybox_anmerkungen, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc33), label_pflanzbodenzusatz, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc33), entrybox_pflanzbodenzusatz, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(row3), rc31, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(row3), rc32, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(row3), rc33, FALSE, TRUE, 10);

  gtk_box_pack_start(GTK_BOX(rc41), label_unterlage, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc41), entrybox_unterlage, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc42), label_rfid, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc42), entrybox_rfid, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc43), label_status, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc43), combobox_status, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc44), label_anlieferung, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc44), combobox_anlieferung, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(row4), rc41, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(row4), rc42, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(row4), rc43, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(row4), rc44, FALSE, TRUE, 10);

  gtk_box_pack_start(GTK_BOX(rc51), label_fruchtanfang, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc51), entrybox_fruchtanfang, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc52), label_fruchtende, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc52), entrybox_fruchtende, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc53), label_blueteanfang, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc53), entrybox_blueteanfang, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc54), label_blueteende, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc54), entrybox_blueteende, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(row5), rc51, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(row5), rc52, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(row5), rc53, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(row5), rc54, FALSE, TRUE, 10);


  gtk_box_pack_start(GTK_BOX(vbox), label_titel, FALSE, FALSE, 20);
  gtk_box_pack_start(GTK_BOX(vbox), row1, FALSE, FALSE, 20);
  gtk_box_pack_start(GTK_BOX(vbox), row2, FALSE, FALSE, 20);
  gtk_box_pack_start(GTK_BOX(vbox), row3, FALSE, FALSE, 20);
  gtk_box_pack_start(GTK_BOX(vbox), row4, FALSE, FALSE, 20);
  gtk_box_pack_start(GTK_BOX(vbox), row5, FALSE, FALSE, 20);
  gtk_box_pack_end(GTK_BOX(bottom_menue), btn_zurueck, FALSE, FALSE, 50);
  gtk_box_pack_start(GTK_BOX(bottom_menue), btn_speichern, FALSE, FALSE, 50);
  gtk_box_pack_end(GTK_BOX(vbox), bottom_menue, FALSE, FALSE, 10);
  gtk_box_pack_start(GTK_BOX(hbox_BaumHinzu), vbox, TRUE, TRUE, 10);
  gtk_container_add(GTK_CONTAINER(window_pu), hbox_BaumHinzu);
  gtk_widget_show_all(window_pu);

  g_signal_connect(G_OBJECT(comboBox_art), "changed", 
        G_CALLBACK(comboBox_artBaum_changed), NULL);

  g_signal_connect(G_OBJECT(comboBox_sorte), "changed", 
        G_CALLBACK(comboBox_sorteBaum_selected), NULL);

  g_signal_connect(G_OBJECT(comboBox_standort), "changed", 
        G_CALLBACK(comboBox_standortBaum_changed), NULL);

  g_signal_connect(G_OBJECT(combobox_wuchsform), "changed", 
        G_CALLBACK(comboBox_wuchsformBaum_changed), NULL);

  g_signal_connect(G_OBJECT(entrybox_herkunft), "changed", 
        G_CALLBACK(entryBox_herkunft_changed), NULL);

  g_signal_connect(G_OBJECT(entrybox_lieferdatum), "changed", 
        G_CALLBACK(entryBox_lieferdatumBaum_changed), (gpointer) window);

  g_signal_connect(G_OBJECT(entrybox_pflanzjahr), "changed", 
        G_CALLBACK(entryBox_pflanzjahrBaum_changed), (gpointer) window);

  g_signal_connect(G_OBJECT(combobox_veredelt), "changed", 
        G_CALLBACK(comboBox_veredeltBaum_changed), NULL);

  g_signal_connect(G_OBJECT(combobox_befruchtung), "changed", 
        G_CALLBACK(comboBox_befruchtungBaum_changed), NULL);

  g_signal_connect(G_OBJECT(entrybox_anmerkungen), "changed", 
        G_CALLBACK(entryBox_anmerkungenBaum_changed), NULL);

  g_signal_connect(G_OBJECT(entrybox_pflanzbodenzusatz), "changed", 
        G_CALLBACK(entryBox_pflanzbodenzusatzBaum_changed), NULL);

  g_signal_connect(G_OBJECT(entrybox_unterlage), "changed", 
        G_CALLBACK(entryBox_unterlageBaum_changed), (gpointer) window);

  g_signal_connect(G_OBJECT(entrybox_rfid), "changed", 
        G_CALLBACK(entryBox_rfidBaum_changed), NULL);

  g_signal_connect(G_OBJECT(combobox_status), "changed", 
        G_CALLBACK(comboBox_statusBaum_changed), NULL);

  g_signal_connect(G_OBJECT(combobox_anlieferung), "changed", 
        G_CALLBACK(comboBox_anlieferungBaum_changed), NULL);

  g_signal_connect(G_OBJECT(entrybox_fruchtanfang), "changed", 
        G_CALLBACK(entryBox_fruchtanfangBaum_changed), (gpointer) window);

  g_signal_connect(G_OBJECT(entrybox_fruchtende), "changed", 
        G_CALLBACK(entryBox_fruchtendeBaum_changed), (gpointer) window);

  g_signal_connect(G_OBJECT(entrybox_blueteanfang), "changed", 
        G_CALLBACK(entryBox_blueteanfangBaum_changed), (gpointer) window);

  g_signal_connect(G_OBJECT(entrybox_blueteende), "changed", 
        G_CALLBACK(entryBox_blueteendeBaum_changed), (gpointer) window);

  g_signal_connect(G_OBJECT(btn_aktualisieren), "clicked",
        G_CALLBACK(btn_baumBearbeiten_clicked), hbox_BaumHinzu);

  g_signal_connect(G_OBJECT(btn_speichern), "clicked",
        G_CALLBACK(btn_BaumBearbSpeichern_clicked), NULL);

  g_signal_connect(G_OBJECT(btn_zurueck), "clicked",
       G_CALLBACK(popup_destroy_clicked), NULL);

  g_signal_connect(window_pu, "destroy",
        G_CALLBACK(popup_destroy_clicked), NULL);

  return 1;
}

int btn_BaumBearbSpeichern_clicked(GtkWidget *widget, gpointer data) {

  string *arrBaumData = getArrayBaumData();
  saveSqlQuerry("SELECT StandortID FROM Baum WHERE BaumID=" + arrBaumData[0]);
  string standortID = executeSqlStatement_singleHit();
  // Set ehemaliger Standort
  if (arrBaumData[14] == "inaktiv") {
    saveSqlQuerry("UPDATE Baum SET EhemaligeStandortID=" + standortID + ", StandortID=NULL WHERE BaumID=" + arrBaumData[0]);
    executeSqlStatement();
    saveSqlQuerry("UPDATE Standort SET Aktivinaktiv=4 WHERE StandortID=" + standortID + " AND NOT Aktivinaktiv=3");
    executeSqlStatement();
  }
  // Check if Standort has been changed
  if (standortID != arrBaumData[3]) {
    saveSqlQuerry("UPDATE Standort SET Aktivinaktiv=4 WHERE StandortID=" + standortID + " AND NOT Aktivinaktiv=3");
    executeSqlStatement();
    saveSqlQuerry("UPDATE Standort SET Aktivinaktiv=1 WHERE StandortID=" + arrBaumData[3]);
    executeSqlStatement();
  }
  // Check if Standort Status needs to be set from 'zu inaktivieren' zu 'nicht zu verwenden'
  if (arrBaumData[14] == "inaktiv" || standortID != arrBaumData[3]) {
    saveSqlQuerry("UPDATE Standort SET Aktivinaktiv=0 WHERE Aktivinaktiv=3 AND StandortID=" + standortID);
    executeSqlStatement();
  }

  sqlBuilderUpdateBaum();
  runDialog(window, "\nDaten erfolgreich geändert\n");
  popup_destroy_clicked(widget, NULL);

  return 1;
}

void baeumeAnzeigen_refresh(GtkWidget *widget, gpointer data) {

  baeume_auswahl[0] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(art_kirsche));
  baeume_auswahl[1] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(art_apfel));
  baeume_auswahl[2] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(art_birne));
  baeume_auswahl[3] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(art_quitte));
  baeume_auswahl[4] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(art_reneklode));
  baeume_auswahl[5] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(art_pflaume));
  baeume_auswahl[6] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(art_mirabelle));
  baeume_auswahl[7] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(art_zwetschge));
  baeume_auswahl[8] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(art_mandel));
  baeume_auswahl[9] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(art_pfirsich));
  baeume_auswahl[10] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(art_nektarine));
  baeume_auswahl[11] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(art_aprikose));
  baeume_auswahl[12] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(art_wallnuss));
  baeume_auswahl[13] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(art_pekanuss));
  baeume_auswahl[14] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(art_feige));
  baeume_auswahl[15] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(art_mango));
  baeume_auswahl[16] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(art_papaya));
  baeume_auswahl[17] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(art_avocado));
  baeume_auswahl[18] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(art_olive));
  baeume_auswahl[19] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(art_orange));
  baeume_auswahl[20] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(art_clementine));
  baeume_auswahl[21] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(art_mandarine));
  baeume_auswahl[22] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(art_grapefruit));
  baeume_auswahl[23] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(art_maulbeere));
  baeume_auswahl[24] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(art_tafeltraube));
  baeume_auswahl[25] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(wuchsform_buschbaum));
  baeume_auswahl[26] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(wuchsform_viertelstamm));
  baeume_auswahl[27] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(wuchsform_halbstamm));
  baeume_auswahl[28] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(wuchsform_hochstamm));
  baeume_auswahl[29] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(wuchsform_spindel));
  baeume_auswahl[30] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(wuchsform_na));
  baeume_auswahl[31] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(veredelt_ja));
  baeume_auswahl[32] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(veredelt_nein));
  baeume_auswahl[33] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(veredelt_na));
  baeume_auswahl[34] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(befruchtung_selbstbefruchter));
  baeume_auswahl[35] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(befruchtung_teilselbstbefruchter));
  baeume_auswahl[36] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(befruchtung_fremdbefruchter));
  baeume_auswahl[37] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(befruchtung_na));
  baeume_auswahl[38] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(status_bestellt));
  baeume_auswahl[39] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(status_geliefert));
  baeume_auswahl[40] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(status_gepflanzt));
  baeume_auswahl[41] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(status_aktiv));
  baeume_auswahl[42] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(status_inaktiv));
  baeume_auswahl[43] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(status_archiv));
  baeume_auswahl[44] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(status_na));
  baeume_auswahl[45] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(anlieferung_selbst));
  baeume_auswahl[46] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(anlieferung_extern));
  baeume_auswahl[47] = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(anlieferung_na));

  gtk_widget_destroy(GTK_WIDGET(data));
  GtkWidget *vbox;
  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

  baeumeAnzeigen_clicked(window, vbox);
}

void btn_baumHinzu_clicked(GtkWidget *widget, gpointer data) {

  gtk_widget_destroy(GTK_WIDGET(data));

  int maxLengthInp = 255;
  string *arrBaumData;
  arrBaumData = getArrayBaumData();
  for (int i = 2; i < 20; i++) { arrBaumData[i] = ""; } // Resets array

  GtkWidget   *hbox_BaumHinzu, *vbox, *row1, *rc11, *rc12, *rc13, *rc14, *rc15, *row2, *rc21, *rc22, *rc23, *rc24, *row3, *rc31, *rc32, *rc33, *row4, *rc41, *rc42, *rc43, *rc44, *row5, *rc51, *rc52, *rc53, *rc54, *bottom_menue, *btn_aktualisieren, *btn_reset, *btn_speichern, *btn_abbrechen,
              *label_titel, *label_art, *label_sorte, *label_standort, *label_wuchsform, *label_herkunft, *label_lieferdatum, *label_pflanzjahr, *label_veredelt, *label_befruchtung, *label_anmerkungen, *label_pflanzbodenzusatz, *label_unterlage, *label_rfid, *label_status, *label_anlieferung, *label_fruchtanfang, *label_fruchtende, *label_blueteanfang, *label_blueteende, *label_empty,
              *comboBox_art, *comboBox_sorte, *comboBox_standort, *combobox_wuchsform, *entrybox_herkunft, *entrybox_lieferdatum, *entrybox_pflanzjahr, *combobox_veredelt, *combobox_befruchtung, *entrybox_anmerkungen, *entrybox_pflanzbodenzusatz, *entrybox_unterlage, *entrybox_rfid, *combobox_status, *combobox_anlieferung, *entrybox_fruchtanfang, *entrybox_fruchtende, *entrybox_blueteanfang, *entrybox_blueteende,
              *image_reload;

  // Label
  const gchar *str = "<b>Neuen Baum Anlegen</b>";
  label_titel = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_titel), str);
  label_sorte = gtk_label_new("Sorte"); 
  label_standort = gtk_label_new("Standort"); 
  label_wuchsform = gtk_label_new("Wuchsform"); 
  label_herkunft = gtk_label_new("Herkunft"); 
  label_lieferdatum = gtk_label_new("Lieferdatum"); 
  label_pflanzjahr = gtk_label_new("Pflanzjahr"); 
  label_veredelt = gtk_label_new("Veredelt"); 
  label_befruchtung = gtk_label_new("Befruchtung"); 
  label_anmerkungen = gtk_label_new("Anmerkungen"); 
  label_pflanzbodenzusatz = gtk_label_new("Pflanzbodenzusatz"); 
  label_unterlage = gtk_label_new("Unterlage"); 
  label_rfid = gtk_label_new("RFID Kennung"); 
  label_status = gtk_label_new("Status"); 
  label_anlieferung = gtk_label_new("Anlieferung"); 
  label_fruchtanfang = gtk_label_new("Fruchtreife Anfang"); 
  label_fruchtende = gtk_label_new("Fruchtreife Ende"); 
  label_blueteanfang = gtk_label_new("Bluete Anfang"); 
  label_blueteende = gtk_label_new("Bluete Ende"); 
  label_empty = gtk_label_new(""); 

  image_reload = gtk_image_new_from_file("images/reload_small.png");

  // ComboBox 'Art'
  comboBox_art = gtk_combo_box_text_new();  
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Apfel");    
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Avokado");    
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Aprikose");    
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Birne");    
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Clementine");    
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Feige");    
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Grapefruit");    
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Kirsche");           
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Mandarine");           
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Mandel");           
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Mango");           
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Maulbeere");           
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Mirabelle");           
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Nektarine");           
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Olive");           
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Orange");           
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Papaya");           
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Pekanuss");           
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Pflaume");           
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Pfirsich");           
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Quitte");           
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Reneklode");           
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Tafeltraube");           
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Wallnuss");           
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_art), NULL, "Zwetschge");           

  // ComboBox 'Sorte'
  comboBox_sorte = gtk_combo_box_text_new();  
  if (arrBaumData[1] == "") {
    saveSqlQuerry("SELECT DISTINCT UnterArt FROM Sorte ORDER BY UnterArt");
    label_art = gtk_label_new("Art"); 
    gtk_widget_set_sensitive(GTK_WIDGET(comboBox_sorte), FALSE);
  }else {
    saveSqlQuerry("SELECT DISTINCT UnterArt FROM Sorte WHERE OberArt = '" + arrBaumData[1] + "' ORDER BY UnterArt");
    label_art = gtk_label_new(arrBaumData[1].c_str()); 
  }
  vector<vector<string> > vecSorten = executeSqlStatement();
  for (int i = 0; i < vecSorten.size(); i++) {
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_sorte), NULL, (vecSorten[i].at(0)).c_str());
    // cout << vecSorten[i].at(0) << "!" << endl;
  }
  
  // ComboBox 'Standort'
  comboBox_standort = gtk_combo_box_text_new();  
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_standort), NULL, "Kein Standort"); 
  saveSqlQuerry("SELECT StandortID FROM Standort WHERE AktivInaktiv = 2 OR AktivInaktiv = 4");
  vector<vector<string> > vecStandorte = executeSqlStatement();
  for (int i = 0; i < vecStandorte.size(); i++) {
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(comboBox_standort), NULL, (modStandortUF(vecStandorte[i].at(0))).c_str());
  }

  // ComboBox 'Wuchsform'
  combobox_wuchsform = gtk_combo_box_text_new();  
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_wuchsform), NULL, "Buschbaum"); 
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_wuchsform), NULL, "Viertelstamm"); 
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_wuchsform), NULL, "Halbstamm"); 
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_wuchsform), NULL, "Hochstamm"); 
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_wuchsform), NULL, "Spindel"); 

  // EntryBox 'Herkunft'
  entrybox_herkunft = gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(entrybox_herkunft), maxLengthInp);

  // EntryBox 'Lieferdatum'
  entrybox_lieferdatum = gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(entrybox_lieferdatum), 4);
  gtk_entry_set_placeholder_text(GTK_ENTRY(entrybox_lieferdatum), "JJJJ");
  gtk_entry_set_overwrite_mode(GTK_ENTRY(entrybox_lieferdatum), TRUE);

  // EntryBox 'Pflanzjahr'
  entrybox_pflanzjahr = gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(entrybox_pflanzjahr), 4);
  gtk_entry_set_placeholder_text(GTK_ENTRY(entrybox_pflanzjahr), "JJJJ");
  gtk_entry_set_overwrite_mode(GTK_ENTRY(entrybox_pflanzjahr), TRUE);

  // ComboBox 'Veredelt'
  combobox_veredelt = gtk_combo_box_text_new();  
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_veredelt), NULL, "Ja");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_veredelt), NULL, "Nein");

  // ComboBox 'Befruchtung'
  combobox_befruchtung = gtk_combo_box_text_new();  
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_befruchtung), NULL, "Selbstbefruchter");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_befruchtung), NULL, "Teilselbstbefruchter");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_befruchtung), NULL, "Fremdbefruchtung");

  // EntryBox 'Anmerkungen'
  entrybox_anmerkungen = gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(entrybox_anmerkungen), maxLengthInp);

  // EntryBox 'Pflanzbodenzusatz'
  entrybox_pflanzbodenzusatz = gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(entrybox_pflanzbodenzusatz), maxLengthInp);

  // EntryBox 'Unterlage'
  entrybox_unterlage = gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(entrybox_unterlage), maxLengthInp);

  // EntryBox 'RFID'
  entrybox_rfid = gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(entrybox_rfid), maxLengthInp);

  // ComboBox 'Status'
  combobox_status = gtk_combo_box_text_new();  
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_status), NULL, "bestellt");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_status), NULL, "geliefert");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_status), NULL, "gepflanzt");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_status), NULL, "aktiv");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_status), NULL, "inaktiv");

  // ComboBox 'Anlieferung'
  combobox_anlieferung = gtk_combo_box_text_new();  
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_anlieferung), NULL, "selbst");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_anlieferung), NULL, "extern");

  // EntryBox 'Fruchtreife Anfang'
  entrybox_fruchtanfang = gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(entrybox_fruchtanfang), 2);
  gtk_entry_set_placeholder_text(GTK_ENTRY(entrybox_fruchtanfang), "KW");

  // EntryBox 'Fruchtreife Ende'
  entrybox_fruchtende = gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(entrybox_fruchtende), 2);
  gtk_entry_set_placeholder_text(GTK_ENTRY(entrybox_fruchtende), "KW");
  gtk_widget_set_tooltip_text(GTK_WIDGET(entrybox_fruchtende), "Wenn Feld leer gelassen wird, dann wird die KW von Fruchtreife Anfang übernommen");

  // EntryBox 'Bluete Anfang'
  entrybox_blueteanfang = gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(entrybox_blueteanfang), 2);
  gtk_entry_set_placeholder_text(GTK_ENTRY(entrybox_blueteanfang), "KW");

  // EntryBox 'Bluete Ende'
  entrybox_blueteende = gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(entrybox_blueteende), 2);
  gtk_entry_set_placeholder_text(GTK_ENTRY(entrybox_blueteende), "KW");
  gtk_widget_set_tooltip_text(GTK_WIDGET(entrybox_blueteende), "Wenn Feld leer gelassen wird, dann wird die KW von Bluete Anfang übernommen");

  //Boxes
  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  hbox_BaumHinzu = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  row1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  row2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  row3 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  row4 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  row5 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  rc11 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc12 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc13 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc14 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc15 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc21 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc22 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc23 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc24 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc31 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc32 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc33 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc41 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc42 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc43 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc44 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc51 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc52 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc53 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  rc54 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  bottom_menue = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  btn_aktualisieren = gtk_button_new();
  gtk_button_set_image(GTK_BUTTON(btn_aktualisieren), image_reload);
  btn_reset = gtk_button_new_with_label("\t   Formular\t\n\tzurücksetzen\t");
  btn_speichern = gtk_button_new_with_label("\t\tSpeichern\t\t");
  btn_abbrechen = gtk_button_new_with_label("\t\tAbbrechen\t\t");

  //Assign all boxes to parent box
  gtk_box_pack_start(GTK_BOX(rc11), label_art, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(rc11), comboBox_art, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc12), label_empty, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(rc12), btn_aktualisieren, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc13), label_sorte, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(rc13), comboBox_sorte, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc14), label_standort, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(rc14), comboBox_standort, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc15), label_wuchsform, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(rc15), combobox_wuchsform, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(row1), rc11, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(row1), rc12, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(row1), rc13, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(row1), rc14, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(row1), rc15, FALSE, TRUE, 10);

  gtk_box_pack_start(GTK_BOX(rc21), label_herkunft, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc21), entrybox_herkunft, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc22), label_lieferdatum, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc22), entrybox_lieferdatum, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc23), label_pflanzjahr, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc23), entrybox_pflanzjahr, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc24), label_veredelt, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc24), combobox_veredelt, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(row2), rc21, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(row2), rc22, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(row2), rc23, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(row2), rc24, FALSE, TRUE, 10);
  
  gtk_box_pack_start(GTK_BOX(rc31), label_befruchtung, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc31), combobox_befruchtung, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc32), label_anmerkungen, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc32), entrybox_anmerkungen, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc33), label_pflanzbodenzusatz, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc33), entrybox_pflanzbodenzusatz, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(row3), rc31, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(row3), rc32, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(row3), rc33, FALSE, TRUE, 10);

  gtk_box_pack_start(GTK_BOX(rc41), label_unterlage, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc41), entrybox_unterlage, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc42), label_rfid, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc42), entrybox_rfid, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc43), label_status, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc43), combobox_status, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc44), label_anlieferung, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc44), combobox_anlieferung, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(row4), rc41, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(row4), rc42, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(row4), rc43, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(row4), rc44, FALSE, TRUE, 10);

  gtk_box_pack_start(GTK_BOX(rc51), label_fruchtanfang, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc51), entrybox_fruchtanfang, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc52), label_fruchtende, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc52), entrybox_fruchtende, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc53), label_blueteanfang, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc53), entrybox_blueteanfang, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc54), label_blueteende, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc54), entrybox_blueteende, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(row5), rc51, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(row5), rc52, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(row5), rc53, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(row5), rc54, FALSE, TRUE, 10);


  gtk_box_pack_start(GTK_BOX(vbox), label_titel, FALSE, FALSE, 20);
  gtk_box_pack_start(GTK_BOX(vbox), row1, FALSE, FALSE, 20);
  gtk_box_pack_start(GTK_BOX(vbox), row2, FALSE, FALSE, 20);
  gtk_box_pack_start(GTK_BOX(vbox), row3, FALSE, FALSE, 20);
  gtk_box_pack_start(GTK_BOX(vbox), row4, FALSE, FALSE, 20);
  gtk_box_pack_start(GTK_BOX(vbox), row5, FALSE, FALSE, 20);
  gtk_box_pack_start(GTK_BOX(bottom_menue), btn_reset, FALSE, FALSE, 10);
  gtk_box_pack_end(GTK_BOX(bottom_menue), btn_abbrechen, FALSE, FALSE, 10);
  gtk_box_pack_end(GTK_BOX(bottom_menue), btn_speichern, FALSE, FALSE, 10);
  gtk_box_pack_end(GTK_BOX(vbox), bottom_menue, FALSE, FALSE, 10);
  gtk_box_pack_start(GTK_BOX(hbox_BaumHinzu), vbox, TRUE, TRUE, 10);
  gtk_container_add(GTK_CONTAINER(window), hbox_BaumHinzu);
  gtk_widget_show_all(window);

  g_signal_connect(G_OBJECT(comboBox_art), "changed", 
        G_CALLBACK(comboBox_artBaum_changed), NULL);

  g_signal_connect(G_OBJECT(comboBox_sorte), "changed", 
        G_CALLBACK(comboBox_sorteBaum_selected), NULL);

  g_signal_connect(G_OBJECT(comboBox_standort), "changed", 
        G_CALLBACK(comboBox_standortBaum_changed), NULL);

  g_signal_connect(G_OBJECT(combobox_wuchsform), "changed", 
        G_CALLBACK(comboBox_wuchsformBaum_changed), NULL);

  g_signal_connect(G_OBJECT(entrybox_herkunft), "changed", 
        G_CALLBACK(entryBox_herkunft_changed), NULL);

  g_signal_connect(G_OBJECT(entrybox_lieferdatum), "changed", 
        G_CALLBACK(entryBox_lieferdatumBaum_changed), (gpointer) window);

  g_signal_connect(G_OBJECT(entrybox_pflanzjahr), "changed", 
        G_CALLBACK(entryBox_pflanzjahrBaum_changed), (gpointer) window);

  g_signal_connect(G_OBJECT(combobox_veredelt), "changed", 
        G_CALLBACK(comboBox_veredeltBaum_changed), NULL);

  g_signal_connect(G_OBJECT(combobox_befruchtung), "changed", 
        G_CALLBACK(comboBox_befruchtungBaum_changed), NULL);

  g_signal_connect(G_OBJECT(entrybox_anmerkungen), "changed", 
        G_CALLBACK(entryBox_anmerkungenBaum_changed), NULL);

  g_signal_connect(G_OBJECT(entrybox_pflanzbodenzusatz), "changed", 
        G_CALLBACK(entryBox_pflanzbodenzusatzBaum_changed), NULL);

  g_signal_connect(G_OBJECT(entrybox_unterlage), "changed", 
        G_CALLBACK(entryBox_unterlageBaum_changed), (gpointer) window);

  g_signal_connect(G_OBJECT(entrybox_rfid), "changed", 
        G_CALLBACK(entryBox_rfidBaum_changed), NULL);

  g_signal_connect(G_OBJECT(combobox_status), "changed", 
        G_CALLBACK(comboBox_statusBaum_changed), NULL);

  g_signal_connect(G_OBJECT(combobox_anlieferung), "changed", 
        G_CALLBACK(comboBox_anlieferungBaum_changed), NULL);

  g_signal_connect(G_OBJECT(entrybox_fruchtanfang), "changed", 
        G_CALLBACK(entryBox_fruchtanfangBaum_changed), (gpointer) window);

  g_signal_connect(G_OBJECT(entrybox_fruchtende), "changed", 
        G_CALLBACK(entryBox_fruchtendeBaum_changed), (gpointer) window);

  g_signal_connect(G_OBJECT(entrybox_blueteanfang), "changed", 
        G_CALLBACK(entryBox_blueteanfangBaum_changed), (gpointer) window);

  g_signal_connect(G_OBJECT(entrybox_blueteende), "changed", 
        G_CALLBACK(entryBox_blueteendeBaum_changed), (gpointer) window);

  g_signal_connect(G_OBJECT(btn_aktualisieren), "clicked",
        G_CALLBACK(btn_baumHinzu_clicked), hbox_BaumHinzu);

  g_signal_connect(G_OBJECT(btn_reset), "clicked",
        G_CALLBACK(btn_baumHinzu_clicked), hbox_BaumHinzu);

  g_signal_connect(G_OBJECT(btn_speichern), "clicked",
        G_CALLBACK(btn_baumHinzuSpeichern_clicked), hbox_BaumHinzu);

  g_signal_connect(G_OBJECT(btn_abbrechen), "clicked",
       G_CALLBACK(stammd_baeume_activated), hbox_BaumHinzu);
}

void btn_baumHinzuSpeichern_clicked(GtkWidget *widget, gpointer data) {

  string *arrBaumData;
  arrBaumData = getArrayBaumData();

  if (arrBaumData[1] == "" || arrBaumData[2] == "" || arrBaumData[3] == "" || arrBaumData[14] == "") { // Check if Art, Sorte, Standort and Status are choosen
    runDialog(window, "Bitte geben Sie mindestens Art, Sorte, Standort und Status an");
  }else {
    int sortenMatch = sqlBuilderInsertBaum(arrBaumData);
    if (sortenMatch == 0) // Check if Oberart and Unterart are match
    {
      runDialog(window, "Kombination von Art und Sorte ist nicht vorhanden\n\nBitte eine gültige Kombination angeben oder neue Sorte anlegen");
    }else {
      executeSqlStatement();
      runDialog(window, "Eingaben erfolgreich gespeichert");
      
      gtk_widget_destroy(GTK_WIDGET(data));
      GtkWidget *vbox;
      vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
      
      // Set AktivInaktiv to 1 for StandortID
      if (arrBaumData[3] != "Kein Standort") {
        saveSqlQuerry("UPDATE Standort SET AktivInaktiv = 1 WHERE StandortID = " + arrBaumData[3]);
        executeSqlStatement(); 
      }

      stammd_baeume_activated(widget, vbox);
    }
  }
}



//Sortenverwaltung
void stammd_sortenVerwLv1_activated(GtkWidget *widget, gpointer data) {

  gtk_widget_destroy(GTK_WIDGET(data));

  //Delete Data in case of coming back from Lv2
  string* arrSortVerwData = getArraySortenVerwData();
  for (int i = 0; i < 5; i++) {
    arrSortVerwData[i] = "";
  }
  

  GtkWidget *scrollWindow, *vbox, *hbox, *bottom_menue, *btn_apfel, *btn_avocado, *btn_aprikose, *btn_birne, *btn_clementien, *btn_feige, *btn_grapefruit, *btn_kirsche, *btn_mandarine, *btn_mandel, *btn_mango, *btn_maulbeere, *btn_mirabelle, *btn_nektarine, *btn_olive, *btn_orange, *btn_papaya, *btn_pekanuss, *btn_pflaume, *btn_pfirsich, *btn_quitte, *btn_reneklode, *btn_tafeltraube, *btn_wallnuss, *btn_zwetschge, *btn_zurueck,
            *label_titel,
            *grid;

  const gchar *str = "<b>SORTEN VERWALTEN</b>";
  label_titel = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_titel), str);

  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  bottom_menue = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  btn_apfel = gtk_button_new_with_label("Apfel");
  btn_avocado = gtk_button_new_with_label("Avocado");
  btn_aprikose = gtk_button_new_with_label("Aprikose"); 
  btn_birne = gtk_button_new_with_label("Birne"); 
  btn_clementien = gtk_button_new_with_label("Clementine"); 
  btn_feige = gtk_button_new_with_label("Feige"); 
  btn_grapefruit = gtk_button_new_with_label("Grapefruit"); 
  btn_kirsche = gtk_button_new_with_label("Kirsche"); 
  btn_mandarine = gtk_button_new_with_label("Mandarine"); 
  btn_mandel = gtk_button_new_with_label("Mandel"); 
  btn_mango = gtk_button_new_with_label("Mango"); 
  btn_maulbeere = gtk_button_new_with_label("Maulbeere"); 
  btn_mirabelle = gtk_button_new_with_label("Mirabelle"); 
  btn_nektarine = gtk_button_new_with_label("Nektarine"); 
  btn_olive = gtk_button_new_with_label("Olive"); 
  btn_orange = gtk_button_new_with_label("Orange"); 
  btn_papaya = gtk_button_new_with_label("Papaya"); 
  btn_pekanuss = gtk_button_new_with_label("Pekanuss"); 
  btn_pflaume = gtk_button_new_with_label("Pflaume"); 
  btn_pfirsich = gtk_button_new_with_label("Pfirsich"); 
  btn_quitte = gtk_button_new_with_label("Quitte"); 
  btn_reneklode = gtk_button_new_with_label("Reneklode"); 
  btn_tafeltraube = gtk_button_new_with_label("Tafeltraube"); 
  btn_wallnuss = gtk_button_new_with_label("Walnuss"); 
  btn_zwetschge = gtk_button_new_with_label("Zwetschge");
  btn_zurueck = gtk_button_new_with_label("\t\tZurück\t\t");

  // Grid 
  grid = gtk_grid_new();
  gtk_grid_set_column_spacing(GTK_GRID(grid), 20);
  gtk_grid_set_row_spacing(GTK_GRID(grid), 30);
  gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
  gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);

  //Scroll Window
  scrollWindow = gtk_scrolled_window_new(NULL, NULL);
  gtk_container_add(GTK_CONTAINER(scrollWindow), grid);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollWindow), GTK_POLICY_NEVER, GTK_POLICY_NEVER);

  gtk_grid_attach(GTK_GRID(grid), btn_apfel, 0, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), btn_avocado, 1, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), btn_aprikose, 2, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), btn_birne, 3, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), btn_clementien, 4, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), btn_feige, 0, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), btn_grapefruit, 1, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), btn_kirsche, 2, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), btn_mandarine, 3, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), btn_mandel, 4, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), btn_mango, 0, 2, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), btn_maulbeere, 1, 2, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), btn_mirabelle, 2, 2, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), btn_nektarine, 3, 2, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), btn_olive, 4, 2, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), btn_orange, 0, 3, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), btn_papaya, 1, 3, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), btn_pekanuss, 2, 3, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), btn_pflaume, 3, 3, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), btn_pfirsich, 4, 3, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), btn_quitte, 0, 4, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), btn_reneklode, 1, 4, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), btn_tafeltraube, 2, 4, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), btn_wallnuss, 3, 4, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), btn_zwetschge, 4, 4, 1, 1);
  
  gtk_box_pack_start(GTK_BOX(hbox), scrollWindow, TRUE, TRUE, 20);
  gtk_box_pack_start(GTK_BOX(vbox), label_titel, FALSE, TRUE, 20);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 20);
  gtk_box_pack_end(GTK_BOX(bottom_menue), btn_zurueck, FALSE, FALSE, 20);
  gtk_box_pack_end(GTK_BOX(vbox), bottom_menue, FALSE, FALSE, 10);
  gtk_container_add(GTK_CONTAINER(window), vbox);
  gtk_widget_show_all(window);
  
  g_signal_connect(G_OBJECT(btn_apfel), "clicked",
      G_CALLBACK(sortenVerwLv1_btn_sorte_clicked), vbox);

  g_signal_connect(G_OBJECT(btn_avocado), "clicked",
      G_CALLBACK(sortenVerwLv1_btn_sorte_clicked), vbox);
  
  g_signal_connect(G_OBJECT(btn_aprikose), "clicked",
      G_CALLBACK(sortenVerwLv1_btn_sorte_clicked), vbox);

  g_signal_connect(G_OBJECT(btn_birne), "clicked",
      G_CALLBACK(sortenVerwLv1_btn_sorte_clicked), vbox);

  g_signal_connect(G_OBJECT(btn_clementien), "clicked",
      G_CALLBACK(sortenVerwLv1_btn_sorte_clicked), vbox);

  g_signal_connect(G_OBJECT(btn_feige), "clicked",
      G_CALLBACK(sortenVerwLv1_btn_sorte_clicked), vbox);

  g_signal_connect(G_OBJECT(btn_grapefruit), "clicked",
      G_CALLBACK(sortenVerwLv1_btn_sorte_clicked), vbox);

  g_signal_connect(G_OBJECT(btn_kirsche), "clicked",
      G_CALLBACK(sortenVerwLv1_btn_sorte_clicked), vbox);

  g_signal_connect(G_OBJECT(btn_mandarine), "clicked",
      G_CALLBACK(sortenVerwLv1_btn_sorte_clicked), vbox);

  g_signal_connect(G_OBJECT(btn_mandel), "clicked",
      G_CALLBACK(sortenVerwLv1_btn_sorte_clicked), vbox);

  g_signal_connect(G_OBJECT(btn_mango), "clicked",
      G_CALLBACK(sortenVerwLv1_btn_sorte_clicked), vbox);

  g_signal_connect(G_OBJECT(btn_maulbeere), "clicked",
      G_CALLBACK(sortenVerwLv1_btn_sorte_clicked), vbox);

  g_signal_connect(G_OBJECT(btn_mirabelle), "clicked",
      G_CALLBACK(sortenVerwLv1_btn_sorte_clicked), vbox);

  g_signal_connect(G_OBJECT(btn_nektarine), "clicked",
      G_CALLBACK(sortenVerwLv1_btn_sorte_clicked), vbox);

  g_signal_connect(G_OBJECT(btn_olive), "clicked",
      G_CALLBACK(sortenVerwLv1_btn_sorte_clicked), vbox);

  g_signal_connect(G_OBJECT(btn_orange), "clicked",
      G_CALLBACK(sortenVerwLv1_btn_sorte_clicked), vbox);

  g_signal_connect(G_OBJECT(btn_papaya), "clicked",
      G_CALLBACK(sortenVerwLv1_btn_sorte_clicked), vbox);

  g_signal_connect(G_OBJECT(btn_pekanuss), "clicked",
      G_CALLBACK(sortenVerwLv1_btn_sorte_clicked), vbox);

  g_signal_connect(G_OBJECT(btn_pflaume), "clicked",
      G_CALLBACK(sortenVerwLv1_btn_sorte_clicked), vbox);

  g_signal_connect(G_OBJECT(btn_pfirsich), "clicked",
      G_CALLBACK(sortenVerwLv1_btn_sorte_clicked), vbox);

  g_signal_connect(G_OBJECT(btn_quitte), "clicked",
      G_CALLBACK(sortenVerwLv1_btn_sorte_clicked), vbox);

  g_signal_connect(G_OBJECT(btn_reneklode), "clicked",
      G_CALLBACK(sortenVerwLv1_btn_sorte_clicked), vbox);
      
  g_signal_connect(G_OBJECT(btn_tafeltraube), "clicked",
      G_CALLBACK(sortenVerwLv1_btn_sorte_clicked), vbox);

  g_signal_connect(G_OBJECT(btn_wallnuss), "clicked",
      G_CALLBACK(sortenVerwLv1_btn_sorte_clicked), vbox);

  g_signal_connect(G_OBJECT(btn_zwetschge), "clicked",
      G_CALLBACK(sortenVerwLv1_btn_sorte_clicked), vbox);

  g_signal_connect(G_OBJECT(btn_zurueck), "clicked",
      G_CALLBACK(hauptmenue), vbox); // Back to Main Menu
}

void stammd_sortenVerwLv2_activated(GtkWidget *widget, gpointer data) {

  gtk_widget_destroy(GTK_WIDGET(data));

  GtkWidget *vbox, *vbox_grid, *hbox, *hbox_comboBtn, *grid, *combobox_sorten, *bottom_menue, *btn_reset, *btn_submit, *btn_hinzufuegen, *btn_bearbeiten, *btn_loeschen, *btn_zurueck, *label_titel, *label_sorte, *label_infoUser;

  string* arrSortenVerwData = getArraySortenVerwData();
  //Delete Data in case of coming back from Lv3
  for (int i = 2; i < 5; i++) {
    arrSortenVerwData[i] = "";
  }
  
  //Titel Label
  string titel = "<b>(" + arrSortenVerwData[0] + ") Sorten</b>";
  const gchar *str = titel.c_str();
  label_titel = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_titel), str);

  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
  vbox_grid = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
  hbox_comboBtn = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
  bottom_menue = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  GtkWidget *image_submit = gtk_image_new_from_file("images/submit_right_medium.png");
  btn_submit = gtk_button_new();
  gtk_button_set_image(GTK_BUTTON(btn_submit), image_submit);
  GtkWidget *image_reset = gtk_image_new_from_file("images/reset_medium.png");
  btn_reset = gtk_button_new();
  gtk_button_set_image(GTK_BUTTON(btn_reset), image_reset);
  btn_hinzufuegen = gtk_button_new_with_label("Sorte hinzufügen");
  gtk_widget_set_sensitive(btn_hinzufuegen, FALSE);
  btn_bearbeiten = gtk_button_new_with_label("Sorte beartbeiten");
  gtk_widget_set_sensitive(btn_bearbeiten, FALSE);
  btn_loeschen = gtk_button_new_with_label("Sorte löschen");
  gtk_widget_set_sensitive(btn_loeschen, FALSE);
  btn_zurueck = gtk_button_new_with_label("\t\tZurück\t\t");

  //Combobox 'Sorten'
  combobox_sorten = gtk_combo_box_text_new_with_entry();
  saveSqlQuerry("SELECT UnterArt FROM Sorte WHERE OberArt = \"" + arrSortenVerwData[0] + "\" ORDER By UnterArt");
  vector<vector<string> > vecSorten = executeSqlStatement();
  for (int i = 0; i < vecSorten.size(); i++) {
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_sorten), NULL, (vecSorten[i].at(0)).c_str());
  }
  
  // Grid 
  grid = gtk_grid_new();
  gtk_grid_set_column_spacing(GTK_GRID(grid), 40);
  gtk_grid_set_row_spacing(GTK_GRID(grid), 30);
  gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
  gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);

  //Label
  label_sorte = gtk_label_new("Sorte");
  label_infoUser = gtk_label_new("");
  
  gtk_box_pack_start(GTK_BOX(vbox_grid), label_sorte, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(vbox_grid), combobox_sorten, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(hbox_comboBtn), btn_reset, TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(hbox_comboBtn), btn_submit, TRUE, TRUE, 2);

  //User Info
  string inserStatus = getInsertStatus();
  if (inserStatus == "nicht vorhanden") {
    string msg2 = "Ausgewählte Sorte: <b>" + arrSortenVerwData[1] + "</b>";
    gtk_label_set_markup(GTK_LABEL(label_sorte), msg2.c_str());
    gtk_widget_set_sensitive(btn_hinzufuegen, TRUE);
    gtk_widget_set_sensitive(btn_bearbeiten, FALSE);
    gtk_widget_set_sensitive(btn_loeschen, FALSE);
    gtk_widget_set_sensitive(combobox_sorten, FALSE);
  }
  else if (inserStatus == "vorhanden") {
    string msg2 = "Ausgewählte Sorte: <b>" + arrSortenVerwData[1] + "</b>";
    gtk_label_set_markup(GTK_LABEL(label_sorte), msg2.c_str());
    gtk_widget_set_sensitive(btn_hinzufuegen, FALSE);
    gtk_widget_set_sensitive(btn_bearbeiten, TRUE);
    gtk_widget_set_sensitive(btn_loeschen, TRUE);
    gtk_widget_set_sensitive(combobox_sorten, FALSE);
  }
  else if ( inserStatus == "Sorte leer") {
    string msg = "<span foreground=\"#b71c1c\">Bitte Sorte angeben</span>";
    gtk_label_set_markup(GTK_LABEL(label_infoUser), msg.c_str());
  }
  else if ( inserStatus == "hinzugefuegt") {
    string msg = "<span foreground=\"#388e3c\">Sorte <b>" + arrSortenVerwData[1] + "</b> erfolgreich hinzugefügt</span>";
    gtk_label_set_markup(GTK_LABEL(label_infoUser), msg.c_str());
    string msg2 = "Ausgewählte Sorte: <b>" + arrSortenVerwData[1] + "</b>";
    gtk_label_set_markup(GTK_LABEL(label_sorte), msg2.c_str());
    gtk_widget_set_sensitive(btn_hinzufuegen, FALSE);
    gtk_widget_set_sensitive(btn_bearbeiten, TRUE);
    gtk_widget_set_sensitive(btn_loeschen, TRUE);
    gtk_widget_set_sensitive(combobox_sorten, FALSE);
  }
  else if ( inserStatus == "Daten bearbeitet") {
    string msg = "<span foreground=\"#388e3c\">Sorte <b>" + arrSortenVerwData[1] + "</b> erfolgreich bearbeitet</span>";
    gtk_label_set_markup(GTK_LABEL(label_infoUser), msg.c_str());
  }
  else if ( inserStatus == "nicht geloescht") {
    string count = getTransfer();
    string msg;
    if (count == "1") {
      msg = "<span foreground=\"#d32f2f\">Sorte <b>" + arrSortenVerwData[1] + "</b> konnte nicht gelöscht werden, da sie <b>einem</b> anderen Baum zugeordnet ist\nBitte diesen Baum erst einer anderen Sorte zuteilen</span>";
    }else {
      msg = "<span foreground=\"#d32f2f\">Sorte <b>" + arrSortenVerwData[1] + "</b> konnte nicht gelöscht werden, da sie <b>" + count + "</b> anderen Bäumen zugeordnet ist\nBitte diese Bäume erst einer anderen Sorte zuteilen</span>";
    }
    gtk_label_set_markup(GTK_LABEL(label_infoUser), msg.c_str());
    arrSortenVerwData[1] = "";
  }
  else if ( inserStatus == "Daten geloescht") {
    string msg = "<span foreground=\"#388e3c\">Sorte <b>" + arrSortenVerwData[1] + "</b> erfolgreich gelöscht</span>";
    gtk_label_set_markup(GTK_LABEL(label_infoUser), msg.c_str());
    arrSortenVerwData[1] = "";
  }

  gtk_grid_attach(GTK_GRID(grid), vbox_grid, 0, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), hbox_comboBtn, 0, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), btn_hinzufuegen, 1, 0, 1, 2);
  gtk_grid_attach(GTK_GRID(grid), btn_bearbeiten, 1, 2, 1, 2);
  gtk_grid_attach(GTK_GRID(grid), btn_loeschen, 1, 4, 1, 2);
  gtk_box_pack_start(GTK_BOX(vbox), label_titel, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(vbox), label_infoUser, FALSE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(hbox), grid, TRUE, TRUE, 20);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 20);
  gtk_box_pack_end(GTK_BOX(bottom_menue), btn_zurueck, FALSE, FALSE, 20);
  gtk_box_pack_end(GTK_BOX(vbox), bottom_menue, FALSE, FALSE, 10);
  gtk_container_add(GTK_CONTAINER(window), vbox);
  gtk_widget_show_all(window);

  g_signal_connect(G_OBJECT(combobox_sorten), "changed", 
        G_CALLBACK(comboBox_sorteSorten_changed), NULL);

  g_signal_connect(G_OBJECT(btn_submit), "clicked", 
        G_CALLBACK(sortenVerwLv2_btn_submit_clicked), vbox);

  g_signal_connect(G_OBJECT(btn_reset), "clicked", 
        G_CALLBACK(stammd_sortenVerwLv2_activated), vbox);

  g_signal_connect(G_OBJECT(btn_hinzufuegen), "clicked", 
        G_CALLBACK(sortenVerwLv2_btn_hinzufuegen_clicked), vbox);

  g_signal_connect(G_OBJECT(btn_bearbeiten), "clicked", 
        G_CALLBACK(stammd_sortenVerwLv3_activated), vbox);

  g_signal_connect(G_OBJECT(btn_loeschen), "clicked", 
        G_CALLBACK(sortenVerwLv2_btn_loeschen_clicked), vbox);

  g_signal_connect(G_OBJECT(btn_zurueck), "clicked",
      G_CALLBACK(stammd_sortenVerwLv1_activated), vbox);
}

void stammd_sortenVerwLv3_activated(GtkWidget *widget, gpointer data) {

  gtk_widget_destroy(GTK_WIDGET(data));

  GtkWidget *vbox, *row1, *rc11, *rc12, *row2, *rc21, *hbox, *combobox_lateinName, *combobox_befruchtung, *entrybox_befruchtersorte, *bottom_menue, *btn_speichern, *btn_abbrechen, *label_titel, *label_latein, *label_befruchtung, *label_befruchtersorte;
  string* arrSortenVerwData = getArraySortenVerwData();

  //Label
  string titel = "<b> (" + arrSortenVerwData[1] + ") bearbeiten</b>";
  const gchar *str = titel.c_str();
  label_titel = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_titel), str);

  //SQL querry for present values
  saveSqlQuerry("SELECT LateinName, Befruchtung2, BefruchterSorte FROM Sorte WHERE OberArt='" + arrSortenVerwData[0] + "' AND UnterArt='" + arrSortenVerwData[1] + "'", 3);
  vector<vector<string> > vecSortenData = executeSqlStatement();

  string latein = "<b>lat. Name:</b> " + vecSortenData[0].at(0);
  label_latein =gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_latein), latein.c_str());
  string befruchtung = "<b>Befruchtung:</b> " + vecSortenData[0].at(1);
  label_befruchtung =gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_befruchtung), befruchtung.c_str());
  string befruchtersorte = "<b>Befruchtersorte</b>";
  label_befruchtersorte =gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_befruchtersorte), befruchtersorte.c_str());

  //Combobox 'Lateinname'
  combobox_lateinName = gtk_combo_box_text_new();
  int start,end;
  string arrLatein[] = {"Prunus persica var platycarpa", "Prunus persica", "Prunus cerasus acida", "Prunus cerasus","Prunus avium juliana", "Prunus avium duracina",
                        "Malus domestica",//
                        "Pyrus communis",//
                        "Cydonia oblonga",//
                        "Prunus domestica subsp. italica",//
                        "Prunus domestica",//
                        "Prunus domestica subsp. syriaca",//
                        "Prunus domestica subsp. domestica",//
                        "Prunus dulcis", "Prunus dulcis var. fragilis", "Prunus dulcis var. dulcis",//
                        "Prunus persica", "Prunus persica var platycarpa",//
                        "Prunus persica var. nucipersica",//
                        "Prunus armeniaca",//
                        "Juglans regia",//
                        "Carya illinoinensis",//
                        "Ficus carica", "Ficus carica subsp. carica", "Ficus carica subsp. rupestris", "Ficus palmata",//
                        "Mangifera indica", "Mangifera zeylanica",//
                        "Carica papaya",//
                        "Persea americana", "Persea americana var. americana", "Persea americana var. drymifolia", "Persea americana var. guatemalensis",//
                        "Olea europaea", "Olea europaea subsp. maroccana", "Olea europaea subsp. laperrinei", "Olea europaea subsp. guanchica", "Olea europaea subsp. cuspidata", "Olea europaea subsp. cerasiformis", "Olea europaea subsp. africana", "Olea europaea subsp. europaea var. sylvestris", "Olea europaea subsp. europaea var. europaea",
                        "Citrus sinensis",
                        "Citrus clementina",
                        "Citrus reticulata",
                        "Citrus maxima", "Citrus x paradisi",
                        "Morus", "Morus alba", "Morus nigra", "Morus rubra", "Morus insignis", "Morus celtidifolia",
                        "Vitis vinifera subsp. vinifera"};

  if (arrSortenVerwData[0] == "Kirsche") { start = 0; end = 5;  }
  else if (arrSortenVerwData[0] == "Apfel") { start = 6; end = 6;  }
  else if (arrSortenVerwData[0] == "Birne") { start = 7; end = 7;  }
  else if (arrSortenVerwData[0] == "Quitte") { start = 8; end = 8;  }
  else if (arrSortenVerwData[0] == "Reneklode") { start = 9; end = 9;  }
  else if (arrSortenVerwData[0] == "Pflaume") { start = 10; end = 10;  }
  else if (arrSortenVerwData[0] == "Mirabelle") { start = 11; end = 11;  }
  else if (arrSortenVerwData[0] == "Zwetschge") { start = 12; end = 12;  }
  else if (arrSortenVerwData[0] == "Mandel") { start = 13; end = 15;  }
  else if (arrSortenVerwData[0] == "Pfirsich") { start = 16; end = 17;  }
  else if (arrSortenVerwData[0] == "Nektarine") { start = 18; end = 18;  }
  else if (arrSortenVerwData[0] == "Aprikose") { start = 19; end = 19;  }
  else if (arrSortenVerwData[0] == "Walnuss") { start = 20; end = 20;  }
  else if (arrSortenVerwData[0] == "Pekannuss") { start = 21; end = 21;  }
  else if (arrSortenVerwData[0] == "Feige") { start = 22; end = 25;  }
  else if (arrSortenVerwData[0] == "Mango") { start = 26; end = 27;  }
  else if (arrSortenVerwData[0] == "Papaya") { start = 28; end = 28;  }
  else if (arrSortenVerwData[0] == "Avocado") { start = 29; end = 32;  }
  else if (arrSortenVerwData[0] == "Olive") { start = 33; end = 41;  }
  else if (arrSortenVerwData[0] == "Orange") { start = 42; end = 42;  }
  else if (arrSortenVerwData[0] == "Clementine") { start = 43; end = 43;  }
  else if (arrSortenVerwData[0] == "Mandarine") { start = 44; end = 44;  }
  else if (arrSortenVerwData[0] == "Grapefruit") { start = 45; end = 46;  }
  else if (arrSortenVerwData[0] == "Maulbeere") { start = 47; end = 52;  }
  else if (arrSortenVerwData[0] == "Tafeltraube") { start = 53; end = 53;  }
  
  for (int x = start; x <= end; x++) {
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_lateinName), NULL, arrLatein[x].c_str());
  }
  
  //Combobox 'Befruchtung'
  combobox_befruchtung = gtk_combo_box_text_new();
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_befruchtung), NULL, "Selbstbefruchter");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_befruchtung), NULL, "Teilselbstbefruchter");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox_befruchtung), NULL, "Fremdbefruchtung");

  //Entrybox 'Befruchtersorte'
  entrybox_befruchtersorte = gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(entrybox_befruchtersorte), (vecSortenData[0].at(2)).c_str());


  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
  row1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
  row2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
  rc11 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
  rc12 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
  rc21 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
  bottom_menue = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  btn_speichern = gtk_button_new_with_label("\t\tSpeichern\t\t");
  btn_abbrechen = gtk_button_new_with_label("\t\tAbbrechen\t\t");

  gtk_box_pack_start(GTK_BOX(vbox), label_titel, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(rc11), label_latein, FALSE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc11), combobox_lateinName, FALSE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc12), label_befruchtung, FALSE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc12), combobox_befruchtung, FALSE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(row1), rc11, TRUE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(row1), rc12, TRUE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(rc21), label_befruchtersorte, FALSE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(rc21), entrybox_befruchtersorte, FALSE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(row2), rc21, TRUE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(vbox), row1, FALSE, FALSE, 60);
  gtk_box_pack_start(GTK_BOX(vbox), row2, FALSE, FALSE, 20);
  gtk_box_pack_end(GTK_BOX(bottom_menue), btn_abbrechen, TRUE, FALSE, 30);
  gtk_box_pack_end(GTK_BOX(bottom_menue), btn_speichern, TRUE, FALSE, 30);
  gtk_box_pack_end(GTK_BOX(vbox), bottom_menue, FALSE, FALSE, 10);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 20);
  gtk_container_add(GTK_CONTAINER(window), hbox);
  gtk_widget_show_all(window);

  g_signal_connect(G_OBJECT(combobox_lateinName), "changed", 
        G_CALLBACK(comboBox_lateinSorten_changed), NULL);
  
  g_signal_connect(G_OBJECT(combobox_befruchtung), "changed", 
        G_CALLBACK(comboBox_befruchtungSorten_changed), NULL);

  g_signal_connect(G_OBJECT(entrybox_befruchtersorte), "changed", 
        G_CALLBACK(entrybox_befruchtersorteSorten_changed), NULL);

  g_signal_connect(G_OBJECT(btn_speichern), "clicked", 
        G_CALLBACK(sortenVerwLv3_btn_speichern_clicked), hbox);

  g_signal_connect(G_OBJECT(btn_abbrechen), "clicked", 
        G_CALLBACK(stammd_sortenVerwLv2_activated), hbox);
}

void sortenVerwLv1_btn_sorte_clicked(GtkWidget *widget, gpointer data) {

  string* arrSortenVerwData = getArraySortenVerwData();
  int i = 0;
  arrSortenVerwData[i] = gtk_button_get_label(GTK_BUTTON(widget));
  stammd_sortenVerwLv2_activated(widget, data);
}

void sortenVerwLv2_btn_submit_clicked (GtkWidget *widget, gpointer data) {

  string* arrSortenVerwData = getArraySortenVerwData();

  //Checks if Feld 'Sorte' has valid input and sets 'Insert Status' for Button to activate/deactivate
  if (arrSortenVerwData[1] != "") {
    string sqlSelect = "SELECT OberArt, UnterArt FROM Sorte WHERE OberArt = '" + arrSortenVerwData[0] + "'AND UnterArt = '" + arrSortenVerwData[1] + "'";
    saveSqlQuerry(sqlSelect, 2);
    vector<vector<string> > vecObUnArt = executeSqlStatement();
    if (vecObUnArt.size() == 0) {
        setInsertStatus("nicht vorhanden");
    }else {
        setInsertStatus("vorhanden");
    }
  }
  
  stammd_sortenVerwLv2_activated(widget, data);
}

void sortenVerwLv2_btn_hinzufuegen_clicked (GtkWidget *widget, gpointer data) {

  string* arrSortenVerwData = getArraySortenVerwData();

  string sqlInsert = "INSERT INTO Sorte (OberArt, UnterArt) VALUES ('" + arrSortenVerwData[0] + "', '" + arrSortenVerwData[1] + "')";
  saveSqlQuerry(sqlInsert);
  executeSqlStatement();
  setInsertStatus("hinzugefuegt");
  
  stammd_sortenVerwLv2_activated(widget, data);
}

void sortenVerwLv2_btn_loeschen_clicked (GtkWidget *widget, gpointer data) {

  string* arrSortenVerwData = getArraySortenVerwData();
  
  //Checks if there are active Sorten in 'Baum' and gives user feedback about number of trees in conflict if any
  saveSqlQuerry("SELECT SortenID FROM Sorte WHERE OberArt='" + arrSortenVerwData[0] + "' AND UnterArt='" + arrSortenVerwData[1] + "'");
  vector<vector<string> > vecSortenIDSorte = executeSqlStatement();
  saveSqlQuerry("SELECT * FROM Baum WHERE SortenID=" + vecSortenIDSorte[0].at(0) + "");
  vector<vector<string> > vecSortenIDBaum = executeSqlStatement();
  if(vecSortenIDBaum.size() > 0) {
    setInsertStatus("nicht geloescht");
    string count = int_to_string(vecSortenIDBaum.size());
    setTransfer(count);
  }else{
    saveSqlQuerry("DELETE FROM Sorte WHERE OberArt='" + arrSortenVerwData[0] + "' AND UnterArt='" + arrSortenVerwData[1] + "'");
    executeSqlStatement();
    setInsertStatus("Daten geloescht");
  }
  
  stammd_sortenVerwLv2_activated(widget, data);
}

void sortenVerwLv3_btn_speichern_clicked (GtkWidget *widget, gpointer data) {

  sqlBuilderUpdateSorten();
  executeSqlStatement();
  setInsertStatus("Daten bearbeitet");
  stammd_sortenVerwLv2_activated(widget, data);
}



//Wirtschaftsjahr
void stammd_neuesWjahr_activated(GtkWidget *widget, gpointer data) {

  gtk_widget_destroy(GTK_WIDGET(data));

  GtkWidget *vbox, *hbox, *hbox_entry, *bottom_menue, *btn_speichern, *btn_abbrechen, *image_info, *label_titel, *label_hinweis, *label_wjahrAlt, *label_wjahrNeu, *label_warning, *entrybox_wjahr;

  image_info = gtk_image_new_from_file("./images/info_mini.png");
  saveSqlQuerry("SELECT DISTINCT(WJahr) FROM WirtschaftsJahr");
  vector<vector<string> > vecWjahre = executeSqlStatement();
  string infoText = "angelegte Wirtschaftsjahre:\n\n" + string_formater(vecWjahre, 5);
  gtk_widget_set_tooltip_text(GTK_WIDGET(image_info), infoText.c_str());

  //Label
  string titel = "<span size=\"large\"><b>Neues Wirtschaftsjahr anlegen</b></span>";
  const gchar *str = titel.c_str();
  label_titel = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_titel), str);

  label_hinweis = gtk_label_new("         Bitte geben Sie eine Jahreszahl für das\nneue Wirtschaftsjahr zwischen 2020 und 2050 an");

  titel = "<span foreground=\"red\"><b>Hinweis:</b> Das alte Wirtschaftsjahr kann nach dem Anlegen des Neuen nicht mehr bearbeitet werden</span>";
  const gchar *str3 = titel.c_str();
  label_warning = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_warning), str3);

  int aktWjahr = getWjahr();
  int neuesWjahr = (aktWjahr == 0) ? 2020 : aktWjahr+1;
  string* arrWjahrData = getArrayWjahrData();
  arrWjahrData[0] = int_to_string(neuesWjahr);

  string str2;
  str2 = (aktWjahr != 0) ? "<b>aktuelles Wirtschaftsjahr:</b> " + int_to_string(aktWjahr) : "<b>aktuelles Wirtschaftsjahr:</b> keins";
  label_wjahrAlt =gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_wjahrAlt), str2.c_str());

  titel = "<b>neues Wirtschaftsjahr:</b>";
  const gchar *str4 = titel.c_str();
  label_wjahrNeu = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_wjahrNeu), str4);

  //Entrybox Wjahr
  entrybox_wjahr = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(entrybox_wjahr), "JJJJ");
  gtk_entry_set_max_length(GTK_ENTRY(entrybox_wjahr), 4);
  gtk_entry_set_width_chars(GTK_ENTRY(entrybox_wjahr), 5);
  gtk_entry_set_text(GTK_ENTRY(entrybox_wjahr), int_to_string(neuesWjahr).c_str());

  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  hbox_entry = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  bottom_menue = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  btn_speichern = gtk_button_new_with_label("\t\tSpeichern\t\t");
  btn_abbrechen = gtk_button_new_with_label("\t\tAbbrechen\t\t");

  gtk_box_pack_start(GTK_BOX(vbox), label_titel, FALSE, TRUE, 40);
  gtk_box_pack_start(GTK_BOX(vbox), label_hinweis, FALSE, TRUE, 20);
  gtk_box_pack_start(GTK_BOX(vbox), label_wjahrAlt, FALSE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(hbox), label_wjahrNeu, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(hbox), entrybox_wjahr, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox), image_info, FALSE, TRUE, 5);
  gtk_box_pack_start(GTK_BOX(hbox_entry), hbox, TRUE, FALSE, 10);
  gtk_box_pack_start(GTK_BOX(vbox), hbox_entry, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), label_warning, FALSE, FALSE, 50);
  gtk_box_pack_start(GTK_BOX(bottom_menue), btn_speichern, FALSE, FALSE, 50);
  gtk_box_pack_end(GTK_BOX(bottom_menue), btn_abbrechen, FALSE, FALSE, 50);
  gtk_box_pack_end(GTK_BOX(vbox), bottom_menue, FALSE, FALSE, 10);
  gtk_container_add(GTK_CONTAINER(window), vbox);
  gtk_widget_show_all(window);

  g_signal_connect(G_OBJECT(entrybox_wjahr), "changed", 
        G_CALLBACK(entrybox_wjahrWjahr_changed), window);

  g_signal_connect(G_OBJECT(btn_speichern), "clicked", 
        G_CALLBACK(stammd_neuesWjahrSpeichern_activated), vbox);

  g_signal_connect(G_OBJECT(btn_abbrechen), "clicked", 
        G_CALLBACK(hauptmenue), vbox);
}

void stammd_neuesWjahrSpeichern_activated(GtkWidget *widget, gpointer data) {

  string* arrWjahrData = getArrayWjahrData();
  if (string_to_int(arrWjahrData[0]) < 2020 || string_to_int(arrWjahrData[0]) > 2050) {
    runDialog(window, "Bitte Jahr zwischen 2020 und 2050 angeben");
    return;
  }
  sqlBuilderNeuesWjahr();
  setWjahr(string_to_int(arrWjahrData[0]));
  saveSqlQuerry("SELECT count(WJahr) FROM WirtschaftsJahr WHERE WJahr=" + arrWjahrData[0]);
  string count = executeSqlStatement_singleHit();
  runDialog(window, "Wirtschaftsjahr '" + arrWjahrData[0] + "' wurde angelegt.\n         Bäume hinzugefügt: " + count);
  hauptmenue(widget, data);

}

void stammd_wjahrAktualisieren_activated(GtkWidget *widget, gpointer data) {

  // Check if Wirtschaftsjahr exists
  string aktWjahr = int_to_string(getWjahr());
  if (aktWjahr == "0") {
    runDialog(window, "\nEs ist kein Wirtschaftsjahr zum aktualisieren vorhanden\n");
    return;
  }

  int counter = 0;
  // Get all active trees
  saveSqlQuerry("SELECT BaumID, StandortID FROM Baum WHERE Status='aktiv'", 2);
  vector<vector<string> > vecBaumData = executeSqlStatement();
  // Check if active tree is in this Wirtschaftsjahr
  for (int i = 0; i < vecBaumData.size(); i++) {
    saveSqlQuerry("SELECT BaumID FROM WirtschaftsJahr WHERE BaumID=" + vecBaumData[i].at(0) + " AND WJahr=" + aktWjahr + "");
    vector<vector<string> > baumIDWjahr = executeSqlStatement();
    // Insert missing trees
    if (baumIDWjahr.size() == 0) {
      if (vecBaumData[i].at(1) == "") { vecBaumData[i].at(1) = "NULL"; }
      saveSqlQuerry("INSERT INTO WirtschaftsJahr (WJahr, BaumID, StandortID) VALUES (" + aktWjahr + ", " + vecBaumData[i].at(0) + ", " + vecBaumData[i].at(1) + ")");
      executeSqlStatement();
      counter++;
    }
  }
  
  runDialog(window, "\nWirtschaftsjahr wurde aktualisiert.\n       Bäume hinzugefügt: " + int_to_string(counter));
}



// Laufende Pflege
void pflege_baumProfil_activated(GtkWidget *widget, gpointer data) {

  gtk_widget_destroy(GTK_WIDGET(data));

  GtkWidget *vbox, *vbox1, *hbox, *hbox1, *hbox2, *hbox3, *hbox4, *bottom_menue,
  *photo, 
  *btn_refresh ,*btn_speichern, *btn_abbrechen, *btn_photo,
  *entrybox_baumID, *entrybox_ertrag, *entrybox_hoehe, 
  *grid2, *grid3,
  *label_titel, *label_sorte, *label_pflanzjahr, *label_photoWjahr, *label_status,
  *label_baumID, *label_ernteLetJahr, *label_ernteAktJahr, *label_ertragLetJahr, *label_ertragAktJahr, *label_ertragHinzu, *label_gewicht,
  *label_hoeheLetJahr, *label_hoeheAktJahr, *label_wachsLetJahr, *label_wachsAktJahr, *label_hoeheHinzu, *label_laenge;

  //get photo
  string latest_photo = get_latest_photo();
  int n = latest_photo.length();
  char latest_photo_path[n + 1]; 
  strcpy(latest_photo_path, latest_photo.c_str());
  string photoJahr = get_photoJahr();

  //display latest photo
  photo = gtk_image_new_from_file(latest_photo_path);

  //Label
  string titel = "<span size=\"large\"><b>Baum Profil anzeigen und bearbeiten</b></span>";
  label_titel = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_titel), titel.c_str());

  titel = "<b>Baum Nr.</b>";
  label_baumID = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_baumID), titel.c_str());
  
  titel = "<b>Sorte:</b>";
  label_sorte = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_sorte), titel.c_str());
  
  titel = "<b>Pflanzjahr:</b>";
  label_pflanzjahr = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_pflanzjahr), titel.c_str());

  titel = "<b>Status:</b>";
  label_status = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_status), titel.c_str());

  titel = "<span size=\"small\">" + photoJahr + "</span>";
  label_photoWjahr = gtk_label_new(NULL);
  gtk_label_set_xalign(GTK_LABEL(label_photoWjahr), 0.025);
  gtk_label_set_markup(GTK_LABEL(label_photoWjahr), titel.c_str());

  titel = "<b>Ernte letztes Jahr:</b>";
  label_ernteLetJahr = gtk_label_new(NULL);
  gtk_label_set_xalign(GTK_LABEL(label_ernteLetJahr), 0);
  gtk_label_set_markup(GTK_LABEL(label_ernteLetJahr), titel.c_str());
  
  label_ertragLetJahr = gtk_label_new("");
  
  titel = "<b>Ernte aktuelles Jahr:</b>";
  label_ernteAktJahr = gtk_label_new(NULL);
  gtk_label_set_xalign(GTK_LABEL(label_ernteAktJahr), 0);
  gtk_label_set_markup(GTK_LABEL(label_ernteAktJahr), titel.c_str());
  
  label_ertragAktJahr = gtk_label_new("");
  
  titel = "<b>Ertrag hinzufügen:</b>";
  label_ertragHinzu = gtk_label_new(NULL);
  gtk_label_set_xalign(GTK_LABEL(label_ertragHinzu), 0);
  gtk_label_set_markup(GTK_LABEL(label_ertragHinzu), titel.c_str());

  titel = "<b>Baumhöhe letztes Jahr:</b>";
  label_hoeheLetJahr = gtk_label_new(NULL);
  gtk_label_set_xalign(GTK_LABEL(label_hoeheLetJahr), 0);
  gtk_label_set_markup(GTK_LABEL(label_hoeheLetJahr), titel.c_str());
  
  label_wachsLetJahr = gtk_label_new("");
  
  titel = "<b>Baumhöhe aktuelles Jahr:</b>";
  label_hoeheAktJahr = gtk_label_new(NULL);
  gtk_label_set_xalign(GTK_LABEL(label_hoeheAktJahr), 0);
  gtk_label_set_markup(GTK_LABEL(label_hoeheAktJahr), titel.c_str());
  
  label_wachsAktJahr = gtk_label_new("");
  
  titel = "<b>Aktuelle Höhe hinzufügen:</b>";
  label_hoeheHinzu = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(label_hoeheHinzu), titel.c_str());

  label_gewicht = gtk_label_new("kg/stk");
  gtk_label_set_xalign(GTK_LABEL(label_gewicht), 0);

  label_laenge = gtk_label_new("cm");
  gtk_label_set_xalign(GTK_LABEL(label_laenge), 0);

  // Entrybox BaumID
  entrybox_baumID = gtk_entry_new();
  gtk_entry_set_width_chars(GTK_ENTRY(entrybox_baumID), 7);
  gtk_entry_set_max_length(GTK_ENTRY(entrybox_baumID), 6);

  // Entrybox Ertrag
  entrybox_ertrag = gtk_entry_new();
  gtk_entry_set_width_chars(GTK_ENTRY(entrybox_ertrag), 5);
  gtk_entry_set_max_length(GTK_ENTRY(entrybox_ertrag), 4);

  // Entrybox Höhe
  entrybox_hoehe = gtk_entry_new();
  gtk_entry_set_width_chars(GTK_ENTRY(entrybox_hoehe), 6);
  gtk_entry_set_max_length(GTK_ENTRY(entrybox_hoehe), 5);

  grid2 = gtk_grid_new();
  gtk_grid_set_column_spacing(GTK_GRID(grid2), 20);
  gtk_grid_set_row_spacing(GTK_GRID(grid2), 10);
  gtk_grid_set_column_homogeneous(GTK_GRID(grid2), FALSE);
  gtk_grid_set_row_homogeneous(GTK_GRID(grid2), FALSE);
  gtk_grid_attach(GTK_GRID(grid2), label_ernteLetJahr, 0, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid2), label_ertragLetJahr, 1, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid2), label_ernteAktJahr, 0, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid2), label_ertragAktJahr, 1, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid2), label_ertragHinzu, 0, 2, 1, 1);
  gtk_grid_attach(GTK_GRID(grid2), entrybox_ertrag, 1, 2, 1, 1);
  gtk_grid_attach(GTK_GRID(grid2), label_gewicht, 2, 2, 1, 1);

  grid3 = gtk_grid_new();
  gtk_grid_set_column_spacing(GTK_GRID(grid3), 20);
  gtk_grid_set_row_spacing(GTK_GRID(grid3), 10);
  gtk_grid_set_column_homogeneous(GTK_GRID(grid3), FALSE);
  gtk_grid_set_row_homogeneous(GTK_GRID(grid3), FALSE);
  gtk_grid_attach(GTK_GRID(grid3), label_hoeheLetJahr, 0, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid3), label_wachsLetJahr, 1, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid3), label_hoeheAktJahr, 0, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid3), label_wachsAktJahr, 1, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid3), label_hoeheHinzu, 0, 2, 1, 1);
  gtk_grid_attach(GTK_GRID(grid3), entrybox_hoehe, 1, 2, 1, 1);
  gtk_grid_attach(GTK_GRID(grid3), label_laenge, 2, 2, 1, 1);


  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  vbox1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
  hbox1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  hbox2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
  hbox3 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
  bottom_menue = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  GtkWidget *image_reload = gtk_image_new_from_file("images/reload_small.png");
  btn_refresh = gtk_button_new();
  gtk_button_set_image(GTK_BUTTON(btn_refresh), image_reload);
  btn_photo = gtk_button_new_with_label("\tPhoto hinzufügen\t");
  btn_abbrechen = gtk_button_new_with_label("\t\tAbbrechen\t\t");
  btn_speichern = gtk_button_new_with_label("\t\tSpeichern\t\t");

   // deactivate entry fields
  gtk_widget_set_sensitive(GTK_WIDGET(btn_photo), FALSE);
  gtk_widget_set_sensitive(GTK_WIDGET(entrybox_ertrag), FALSE);
  gtk_widget_set_sensitive(GTK_WIDGET(entrybox_hoehe), FALSE);
  gtk_widget_set_sensitive(GTK_WIDGET(btn_speichern), FALSE);

  // Insert Data from Database
  string* arrBaumProfil = getArrayBaumProfil();
  for (int i = 1; i < 3; i++) { 
    arrBaumProfil[i] = "";
  }
  
  string baumID = arrBaumProfil[0];
  gtk_entry_set_text(GTK_ENTRY(entrybox_baumID), baumID.c_str()); // Set entrybox_baumID after page refresh
  int aktWjahr = getWjahr();
  if (aktWjahr > 0) {

    // Check if 'BaumID' was inserted
    if (baumID != "") { 

      // Check if tree exists
      saveSqlQuerry("SELECT Sorte.OberArt, Sorte.UnterArt, Baum.PflanzJahr, Baum.Status From Baum Join Sorte On Baum.SortenID=Sorte.SortenID WHERE BaumID=" + baumID + "", 4);
      vector<vector<string> > vecBaum = executeSqlStatement();
      if (vecBaum.size() > 0) { 
        titel = "<b>Sorte: </b>" + vecBaum[0].at(0) + ", " + vecBaum[0].at(1);
        gtk_label_set_markup(GTK_LABEL(label_sorte), titel.c_str());
        titel = "<b>Pflanzjahr: </b>" + vecBaum[0].at(2);
        gtk_label_set_markup(GTK_LABEL(label_pflanzjahr), titel.c_str());

        // Check status of tree
        if (vecBaum[0].at(3) == "aktiv") {
          titel = "<b>Status: </b><span foreground=\"#029300\">" + vecBaum[0].at(3) + "</span>";
          gtk_label_set_markup(GTK_LABEL(label_status), titel.c_str());
        }
        else if (vecBaum[0].at(3) == "inaktiv") {
          titel = "<b>Status: </b><span foreground=\"#e27900\">" + vecBaum[0].at(3) + "</span>";
          gtk_label_set_markup(GTK_LABEL(label_status), titel.c_str());
        }
        else if (vecBaum[0].at(3) == "archiv") {
          titel = "<b>Status: </b><span foreground=\"#e23400\">" + vecBaum[0].at(3) + "</span>";
          gtk_label_set_markup(GTK_LABEL(label_status), titel.c_str());
        }
        // Check for 'Mango' or 'Feige'
        if (vecBaum[0].at(0) == "Mango" || vecBaum[0].at(0) == "Feige") {
          gtk_label_set_text(GTK_LABEL(label_gewicht), "stk");
        }else {
          gtk_label_set_text(GTK_LABEL(label_gewicht), "kg");
        }

        // Check if tree exists in aktuelles Wirtschaftsjahr 
        saveSqlQuerry("SELECT * FROM (SELECT WJahr, Baum.BaumID, WirtschaftsJahr.Ertrag, WirtschaftsJahr.AktuelleHoehe FROM Baum LEFT Join WirtschaftsJahr On Baum.BaumID=WirtschaftsJahr.BaumID JOIN Sorte ON Baum.SortenID=Sorte.SortenID) WHERE WJahr=" + int_to_string(aktWjahr) + " AND BaumID=" + baumID, 4);
        vector<vector<string> > vecBaumAktJahr = executeSqlStatement();
        if (vecBaumAktJahr.size() > 0) { 
          titel = vecBaumAktJahr[0].at(2);
          gtk_label_set_markup(GTK_LABEL(label_ertragAktJahr), titel.c_str());
          titel = vecBaumAktJahr[0].at(3);
          gtk_label_set_markup(GTK_LABEL(label_wachsAktJahr), titel.c_str());
          gtk_widget_set_sensitive(GTK_WIDGET(btn_photo), TRUE);
          gtk_widget_set_sensitive(GTK_WIDGET(entrybox_ertrag), TRUE);
          gtk_widget_set_sensitive(GTK_WIDGET(entrybox_hoehe), TRUE);
          gtk_widget_set_sensitive(GTK_WIDGET(btn_speichern), TRUE);
        }

        // Check if tree exists in letztes Wirtschaftsjahr 
        saveSqlQuerry("SELECT * FROM (SELECT WJahr, Baum.BaumID, WirtschaftsJahr.Ertrag, WirtschaftsJahr.AktuelleHoehe FROM Baum LEFT Join WirtschaftsJahr On Baum.BaumID=WirtschaftsJahr.BaumID JOIN Sorte ON Baum.SortenID=Sorte.SortenID) WHERE WJahr=" + int_to_string(aktWjahr-1) + " AND BaumID=" + baumID, 4);
        vector<vector<string> > vecBaumLetJahr = executeSqlStatement();
        if (vecBaumLetJahr.size() > 0) {
          titel = vecBaumLetJahr[0].at(2);
          gtk_label_set_markup(GTK_LABEL(label_ertragLetJahr), titel.c_str());
          titel = vecBaumLetJahr[0].at(3);
          gtk_label_set_markup(GTK_LABEL(label_wachsLetJahr), titel.c_str());
        }
      }else {
      runDialog(window, "\nDieser Baum existiert leider nicht\n");
      }
    }
  }else {
      runDialog(window, "\nEs wurde noch kein Wirschaftsjahr angelegt\n");
    }

  gtk_box_pack_start(GTK_BOX(vbox), label_titel, TRUE, TRUE, 20);
  gtk_box_pack_start(GTK_BOX(hbox1), label_baumID, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(hbox1), entrybox_baumID, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(hbox1), btn_refresh, FALSE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), hbox1, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(hbox3), photo, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(vbox1), label_sorte, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(vbox1), label_pflanzjahr, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(vbox1), label_status, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(vbox1), btn_photo, FALSE, TRUE, 20);
  gtk_box_pack_start(GTK_BOX(hbox3), vbox1, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(vbox), hbox3, FALSE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(vbox), label_photoWjahr, FALSE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(hbox2), grid2, TRUE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(hbox2), grid3, TRUE, TRUE, 100);
  gtk_box_pack_start(GTK_BOX(vbox), hbox2, FALSE, TRUE, 50);
  gtk_box_pack_start(GTK_BOX(bottom_menue), btn_speichern, FALSE, FALSE, 100);
  gtk_box_pack_end(GTK_BOX(bottom_menue), btn_abbrechen, FALSE, FALSE, 100);
  gtk_box_pack_end(GTK_BOX(vbox), bottom_menue, FALSE, FALSE, 10);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 20);
  gtk_container_add(GTK_CONTAINER(window), hbox);
  gtk_widget_show_all(window);

  g_signal_connect(G_OBJECT(entrybox_baumID), "changed", 
        G_CALLBACK(entrybox_baumIDProfil_changed), (gpointer) window);

  g_signal_connect(G_OBJECT(entrybox_ertrag), "changed", 
        G_CALLBACK(entrybox_ertragProfil_changed), (gpointer) window);

  g_signal_connect(G_OBJECT(entrybox_hoehe), "changed", 
        G_CALLBACK(entrybox_hoeheProfil_changed), (gpointer) window);

  g_signal_connect(G_OBJECT(btn_refresh), "clicked", 
        G_CALLBACK(pflege_baumProfil_activated), hbox);

  g_signal_connect(G_OBJECT(btn_speichern), "clicked", 
        G_CALLBACK(pflege_baumProfilSpeichern_activated), hbox);

  g_signal_connect(G_OBJECT(btn_photo), "clicked", 
        G_CALLBACK(photo_open_dialog), hbox);

  g_signal_connect(G_OBJECT(btn_abbrechen), "clicked", 
        G_CALLBACK(hauptmenue), hbox);
}

void pflege_baumProfilSpeichern_activated(GtkWidget *widget, gpointer data) {

  string* baumProfilData = getArrayBaumProfil();
  string baumID = baumProfilData[0];
  int aktWjahr = getWjahr();

  saveSqlQuerry("SELECT Ertrag, AktuelleHoehe FROM WirtschaftsJahr WHERE WJahr=" + int_to_string(aktWjahr) + " AND BaumID=" + baumID, 2);
  vector<vector<string> > vecBaumAktJahr = executeSqlStatement();
  for (int i = 0; i < 2; i++) {
    if (vecBaumAktJahr[0].at(i) == "") {
      vecBaumAktJahr[0].at(i) = "0";
    }
  }
  for (int i = 1; i < 3; i++) {
    if (baumProfilData[i] == "") {
      baumProfilData[i] = "0";
    }
  }
  
  int ertragNeuInt = string_to_int(vecBaumAktJahr[0].at(0)) + string_to_int(baumProfilData[1]);
  int hoeheNeuInt = string_to_int(vecBaumAktJahr[0].at(1)) + string_to_int(baumProfilData[2]);

  if (ertragNeuInt < 0 || ertragNeuInt > 999 ) {
    runDialog(window, "\nDer Jahresertrag darf nicht kleiner als 0 sein\nund nicht größer als 999\n");
    return;
  }
  if (hoeheNeuInt < 0) {
    runDialog(window, "\nDie Baumhöhe darf nicht kleiner als 0 sein\n");
    return;
  }

  string ertragNeuStr = int_to_string(ertragNeuInt);
  string hoeheNeuStr = int_to_string(hoeheNeuInt);
  
  saveSqlQuerry("UPDATE Wirtschaftsjahr SET Ertrag=" + ertragNeuStr + ", AktuelleHoehe=" + hoeheNeuStr + " WHERE BaumID=" + baumID + " AND Wjahr=" + int_to_string(aktWjahr));
  executeSqlStatement();

  runDialog(window, "\nDaten erfolgreich eingefügt\n");
  pflege_baumProfil_activated(widget, data);
}

void photo_open_dialog(GtkWidget* button, gpointer window){

  GtkWidget *dialog;
  GtkWidget *dialog_window;
  GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
  gint res;
  string picture_complete_path;               // location of the database file

  dialog_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  app_icon = create_pixbuf("./images/menueiconsmall.png");  
  gtk_window_set_icon(GTK_WINDOW(dialog_window), app_icon);
  dialog = gtk_file_chooser_dialog_new ("Bild auswählen", GTK_WINDOW (dialog_window), action, 
                                        "_Cancel", GTK_RESPONSE_CANCEL, "_Open", 
                                        GTK_RESPONSE_ACCEPT, NULL);
  gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),"/");
  res = gtk_dialog_run (GTK_DIALOG (dialog));
  if(res == GTK_RESPONSE_ACCEPT){
    picture_complete_path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

    char image_folder[256];
    getcwd(image_folder, 256);
    char subfolder[16] = "/images/baeume/";
    strcat(image_folder, subfolder);

    string* arrBaumProfil = getArrayBaumProfil();
    string baumID = arrBaumProfil[0];
    int n = baumID.length(); 
    char char_baumID[n + 1]; 
    strcpy(char_baumID, baumID.c_str());

    strcat(image_folder, char_baumID);
    std::filesystem::create_directory(image_folder);

    cout << image_folder << "\n";

    string slash = "/";
    int year_nr = getWjahr();
    string year = int_to_string(year_nr);
    string image_path = image_folder + slash + year;

    cout << image_path << "\n";

    // std::filesystem::copy_options::overwrite_existing does not workt because of a bug in gcc since 2020.
    // Solution: first try to delete an existing file, then copy the new one.
    try{
      std::filesystem::remove(image_path);
    }
    catch (...){}

    try{
      std::filesystem::copy_file(picture_complete_path, image_path, std::filesystem::copy_options::overwrite_existing);
    }
    catch (...){}
  }
  else
    g_print("You pressed Cancel\n");
    gtk_widget_destroy(dialog);
}



//Hilfe
void hilfeVersion_clicked(GtkWidget *widget, gpointer data) {
  runDialog(window, "\n\t\tVersion: 0.8\t\t\n");
}
void hilfeLizenz_clicked(GtkWidget *widget, gpointer data) {
  runDialog(window, "\nCopyrigt © 2022: Steven Franke, Christian Emmelmann, Benjamin Borns\n");
}

void popup_destroy_clicked(GtkWidget *widget, gpointer data) {

  //Reset all made inserts
  string* arrStaOrtData = getArrayStaOrtData();
  for (int i = 3; i < 9; i++) {
    arrStaOrtData[i] = "";
  }
  
  string* arrBaumData = getArrayBaumData();
  for (int j = 2; j < 20; j++) {
    arrBaumData[j] = "";
  }
  

  gtk_widget_set_sensitive(GTK_WIDGET(window), TRUE);
  gtk_widget_destroy(GTK_WIDGET(window_pu));



}
