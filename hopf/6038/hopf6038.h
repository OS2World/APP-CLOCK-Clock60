#define VERSION_MAJOR       6
#define VERSION_MINOR       0

#define OFFSET_SEC          0
#define OFFSET_MIN          1
#define OFFSET_HOUR         2
#define OFFSET_DAY          3
#define OFFSET_DOW          4
#define OFFSET_MON          5
#define OFFSET_YEAR         6
#define OFFSET_STAT         7

// *** nur 6038 ***
#define OFFSET_OHOUR        0x10        // Stundenversatz zu UTC (BCD)
#define OFFSET_OMIN         0x11        // Minutenversatz zu UTC (BCD, nur 0 oder 30)
#define OFFSET_VER_ZE       0x12        // SW-Versionsnummer 10er und 1er (BCD)
#define OFFSET_VER_ZH       0x13        // SW-Versionsnummer 1/10 und 1/100 (BCD)
#define OFFSET_VER_DAY      0x14        // SW-Version: Tag (BCD)
#define OFFSET_VER_MON      0x15        // SW-Version: Tag (BCD)
#define OFFSET_VER_YEAR     0x16        // SW-Version: Tag (BCD)

#define OFFSET_BREITE       0x18        // Breite (BinÑr, sowie 0x19, 0x1A und 0x1B)
#define OFFSET_LAENGE       0x1C        // Breite (BinÑr, sowie 0x1D, 0x1E und 0x1F)
#define OFFSET_NR_SAT1      0x20        // Nummer des 1.Satelliten
#define OFFSET_SN_SAT1      0x21        // SN-Ration des 1.Satelliten
#define OFFSET_NR_SAT2      0x22        // Nummer des 2.Satelliten
#define OFFSET_SN_SAT2      0x23        // SN-Ration des 2.Satelliten
#define OFFSET_NR_SAT3      0x24        // Nummer des 3.Satelliten
#define OFFSET_SN_SAT3      0x25        // SN-Ration des 3.Satelliten
#define OFFSET_NR_SAT4      0x26        // Nummer des 4.Satelliten
#define OFFSET_SN_SAT4      0x27        // SN-Ration des 4.Satelliten
#define OFFSET_NR_SAT5      0x28        // Nummer des 5.Satelliten
#define OFFSET_SN_SAT5      0x29        // SN-Ration des 5.Satelliten
#define OFFSET_NR_SAT6      0x2A        // Nummer des 6.Satelliten
#define OFFSET_SN_SAT6      0x2B        // SN-Ration des 6.Satelliten

// ab hier wieder alle Karten
#define OFFSET_INPUT        0x30        // Puffer fÅr öbertragung zur Karte
#define OFFSET_INPUT_CMD    0x37        // Kommando fÅr öbertragung zur Karte

#define OFFSET_MS_LSB       0x38        // LSB fÅr MIKROSEKUNDEN
#define OFFSET_MS_MSB       0x39        // MSB fÅr MIKROSEKUNDEN
#define OFFSET_MODE         0x3A        // MODE-BYTE (nur 6038)
#define OFFSET_KENNUNG1     0x3B        // Uhrenkennung =0x58
#define OFFSET_KENNUNG2     0x3C        // Uhrenkennung =0x4E
#define OFFSET_QUALITY      0x3D        // GÅtezÑhler
#define OFFSET_DONGLE_LSB   0x3E        // LSB fÅr Dongle-Option
#define OFFSET_DONGLE_MSB   0x3F        // MSB fÅr Dongle-Option

#define HSTAT_ANT           0x0002      // Ersatzantenne
#define HSTAT_HLEAP         0x0004      // Stundensprung
#define HSTAT_SLEAP         0x0020      // Schaltsekunde
#define HSTAT_TZONE1        0x0010      // Zeitzone 1
#define HSTAT_TZONE2        0x0008      // Zeitzone 2
#define HSTAT_DCFTIME       0x0080      // Zeit gÅltig

#define SET_BREITE          0x10        // Setze geografische Breite
#define SET_LAENGE          0x11        // Setze geografische Laenge
#define SET_OFFSET          0x12        // Setze Offset Local <-> UTC
#define SET_MODE            0x15        // Modus
#define SET_QSZ             0x48        // Setze Quarzuhr auf Sommerzeit
#define SET_USZ             0x49        // Setze Umschaltezeitpunkt SZ
#define SET_QWZ             0x50        // Setze Quarzuhr auf Winterzeit
#define SET_UWZ             0x51        // Setze Umschaltezeitpunkt WZ

#define SET_FLAG_BREITE     0x01        // Breite ist zu setzen
#define SET_FLAG_LAENGE     0x02        // Laenge ist zu setzen
#define SET_FLAG_MODE       0x04        // Mode   ist zu setzen
#define SET_IN_WORK         0x80        // Es ist noch nicht alles gesetzt

#define CCHMAXLEN           255         // Maximale ZeilenlÑnge
