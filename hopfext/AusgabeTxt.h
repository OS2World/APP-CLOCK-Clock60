#ifndef AUSGABETXT_H
#define AUSGABETXT_H

// Demo Switch
//#define DEMO

// //////////////////////////////////////////////////////
// Text zur Ausgabe beim Starten
// //////////////////////////////////////////////////////
#ifdef DEMO
char *demotxt[]={"                 DCF77-Programm",
        "                     fÅr 'Hopf clock controller'",
        "                     noller & breining software",
        "                        ShareWare-Programm",
        "                         Version 6.00",
        " ",
        "Dieses  DCF77-ShareWare-Programm setzt die Systemzeit ein einziges Mal.",
        "Es ist  mîglich, da· dies  schon wÑhrend  des Bootvorganges  geschieht.",
        "Um eventuell notwendige énderungen an den  Parametern des Programms fÅr",
        "Ihr Hopf-Modul  bequem und  interaktiv  von der WPS  aus durchfÅhren zu",
        "kînnen, haben wir eine neue  Systemuhr entwickelt, die Sie zusammen mit",
        "diesem Programm installiert haben.",
        " ",
        "Die Vollversion des Programms bietet darÅber hinaus die Mîglichkeit die",
        "Systemuhr in einstellbaren Intervallen zu aktualisieren. Sie verzichtet",
        "au·erdem auf diese lÑstige Informationsausgabe und Tastaturabfrage beim",
        "Booten des Rechners. Sie erhalten die Vollversion im Handel oder bei:",
        " ",
        "       noller & breining software",
        "       Lauffener Str. 4",
        "       71522 Backnang",
        ""};
#endif

#define ERROR_BREAK "Programm kann zur Zeit nicht abgebrochen werden...\n"
char *helptxt[]={
       " Hilfe zu:DCF77-Programm",
       "                     fÅr 'Hopf clock controller'",
       "                     noller & breining software",
       "                        ShareWare-Programm",
       "                         Version 6.00",
       "Aufrufparameter: ",
       " -s$, -S$  $ = serielle Schnittstellennummer (1..9)",
       " -h -H -?    = diese Hilfe",
       ""};
char *aktivtxt[]={"Das Hopf-Programm von ",
       "noller & breining software",
       "ist bereits aktiv",
       " ",
       "Bitte nur einmal starten",
       ""};
char *treibertxt[]={"Der DCF77 - Treiber von ",
       "noller & breining software",
       "ist bereits aktiv",
       "Bitte nur Treiber oder Hopf - PRG starten",
       ""};

// ////////////////////////////////////////////////////
// Initialisierung der öbergabeparameter
// ////////////////////////////////////////////////////
#include "InitClass.hpp"

Initc_Pro inidata[]={GETINT,'s',&serielle,
                     GETINT,'S',&serielle,
                     SETNUL,'?',&showhelp,
                     SETNUL,'h',&showhelp,
                     SETNUL,'H',&showhelp};

#endif
