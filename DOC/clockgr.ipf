.*----------------------------------------------------------
.* WPDCR77 -- Systemuhr mit DCF77-Untersttzung
.*
.* IPF Help File
.*
.*----------------------------------------------------------

:userdoc.

.*----------------------------------------------------------
.*  Systemuhr mit DCF77-Untersttzung
.*----------------------------------------------------------
:h1 res=10.Systemuhr mit DCF77/GPS-Untersttzung
:i1.Systemuhr mit DCF77/GPS-Untersttzung

:p.Mit dieser modifizierten :hp2.Systemuhr:ehp2. k”nnen Sie die Systemzeit einstellen,
eine Uhrzeit fr die Alarmfunktion vorgeben sowie die Einstellungen fr
den DCF77/GPS-Treiber vornehmen.
Die Uhr kann in verschiedenen Formaten angezeigt 
werden (z. B. analog oder digital mit oder ohne 
Systemdatum). Gehen Sie folgendermaáen vor&colon. 

:ol compact.
:li. Das Kontextmen fr die Systemuhr anzeigen. 
:li. Den Pfeil rechts neben ™ffnen ausw„hlen. 
:li. Einstellungen ausw„hlen. 
:eol.

:p.Zugeh”rige Informationen&colon.
:ul compact.
:li.:link reftype=hd res=10001.Anzeigen von Kontextmens:elink.
:li.:link reftype=hd res=10000.Anzeigen von Notizbuchseiten:elink.
:eul.    

:p.Ist in Ihrem System ein DCF77/GPS-Empf„nger angesteckt, erh„lt die Systemuhr zus„tzliche
Einstellseiten sowie eine Seite zur Anzeige des Status dieses Ger„tes.
Dazu muá der entsprechende Treiber :hp1.ZEIT.SYS:ehp1.
in der Datei :hp1.CONFIG.SYS:ehp1. eingetragen sein.

:p.N„here Hinweise zur Technik des DCF77-Empfangs siehe
:link reftype=hd res=20000.DCF77-Systembeschreibung:elink.; Hinweise zum GPS-Empfang finden sich unter
:link reftype=hd res=20001.GPS-Systembeschreibung:elink..

.*----------------------------------------------------------
.*  DCF77 Technik
.*----------------------------------------------------------
:h1 res=20000.DCF77 Systembeschreibung
:i1.DCF77 Systembeschreibung

:p.Die verwendeten DCF77-Funkuhren empfangen das Signal des gleichnamigen Langwellensenders in Mainflingen
bei Frankfurt am Main. Dieser der Verbreitung der gesetzlichen Zeit der Bundesrepublik Deutschland
MEZ (englisch: Central European Time, CET) bzw. die Mitteleurop„ische Sommerzeit MESZ (englisch: Central European Summer
Time, CEST). Der Empfang des Senders DCF77 ist nahezu berall in Deutschland und im angrenzenden Ausland m”glich. 

:p.Der Sender wird durch die Atomuhren der Physikalisch-Technischen Bundesanstalt (PTB) in Braunschweig gesteuert. Der Fachbereich
L„nge und Zeit der PTB kontrolliert die ausgesendeten Signale, welche innerhalb jeder Minute in Sekundenimpulsen codiert die
komplette Zeitinformation einschlieálich Uhrzeit, Datum und Wochentag bertragen.

Die hochkonstante Tr„gerfrequenz des Zeitsignals betr„gt 77,5 kHz. Zu Beginn jeder Sekunde wird die Tr„geramplitude fr 0,1 sec oder
0,2 sec auf ca. 25% abgesenkt. Die so erzeugten Sekundenmarken enthalten bin„r codiert die Zeitinformation. Sekundenmarken mit
einer Dauer von 0,1 sec entsprechen einer bin„ren ÿ0ÿ und solche mit 0,2 sec einer bin„ren ÿ1ÿ. Die Information ber die Uhrzeit und
das Datum sowie einige Parity- und Statusbits finden sich in den Sekundenmarken 15 bis 58 jeder Minute. Durch das Fehlen der 59.
Sekundenmarke wird die Minutenmarke angekndigt. 

:p.:hp2.Decodierschema DCF77:ehp2.
:p.:artwork name='E:\WORK\C\CLOCK53\DOC\dcfcode.bmp' align=left.
:p.:parml compact tsize=15 break=none.
:pt.M
:pd.Minutenmarke (0,1s)
:pt.R
:pd.Aussendung ber Reserveantenne
:pt.A1 
:pd.Ankndigung Beginn/Ende der Sommerzeit
:pt.Z1, Z2 
:pd.Zeitzonenbits
.br
Z1, Z2 = 0, 1 Standardzeit (MEZ)
.br
Z1, Z2 = 1, 0: Sommerzeit (MESZ)
:pt.A2 
:pd.Ankndigung einer Schaltsekunde
:pt.S 
:pd.Startbit der codierten Zeitinformation (0,2s)
:pt.P1, P2, P3 
:pd.Prfbits
:eparml.
:p.:hp1.(entnommen: Meinberg Funkuhren: Systembeschreibung DCF77):ehp1.

.*----------------------------------------------------------
.*  DCF77 Technik
.*----------------------------------------------------------
:h1 res=20001.GPS Systembeschreibung
:i1.GPS Systembeschreibung

:p.Das Global Positioning System (GPS) ist ein satellitengesttztes System zur Radioortung, Navigation und Zeitbertragung. Dieses System
wurde vom Verteidigungsministerium der USA (US Departement Of Defense) installiert und arbeitet mit zwei Genauigkeitsklassen: den
Standard Positioning Services (SPS) und den Precise Positioning Services (PPS). Die Struktur der gesendeten Daten des SPS ist
ver”ffentlicht und der Empfang zur allgemeinen Nutzung freigegeben worden, w„hrend die Zeit- und Navigationsdaten des noch
genaueren PPS verschlsselt gesendet werden und daher nur bestimmten (meist milit„rischen) Anwendern zug„nglich sind.

:p.Das Prinzip der Orts- und Zeitbestimmung mit Hilfe eines GPS-Empf„ngers beruht auf einer m”glichst genauen Messung der
Signallaufzeit von den einzelnen Satelliten zum Empf„nger. 21 aktive GPS-Satelliten und drei zus„tzliche Reservesatelliten umkreisen die
Erde auf sechs Orbitalbahnen in 20000 km H”he einmal in ca. 12 Stunden. Dadurch wird sichergestellt, daá zu jeder Zeit an jedem Punkt
der Erde mindestens vier Satelliten in Sicht sind. Vier Satelliten mssen zugleich zu empfangen sein, damit der Empf„nger seine Position
im Raum (x, y, z) und die Abweichung seiner Uhr von der GPS-Systemzeit ermitteln kann.

:p.Kontrollstationen auf der Erde vermessen die Bahnen der Satelliten und registrieren die Abweichungen der an Bord mitgefhrten
Atomuhren von der GPS-Systemzeit. Die ermittelten Daten werden zu den Satelliten hinaufgefunkt und von diesen als Navigationsdaten
wieder zurck zur Erde gesendet. Die hochpr„zisen Bahndaten der Satelliten, genannt Ephemeriden, werden ben”tigt, damit der
Empf„nger zu jeder Zeit die genaue Position der Satelliten im Raum berechnen kann. Ein Satz Bahndaten mit reduzierter Genauigkeit
wird Almanach genannt. Mit Hilfe der Almanachs berechnet der Empf„nger bei ungef„hr bekannter Position und Zeit, welche der
Satelliten vom Standort aus ber dem Horizont sichtbar sind. Jeder der Satelliten sendet seine eigenen Ephemeriden sowie die Almanachs
aller existierender Satelliten aus.

:p.Die GPS-Systemzeit ist eine lineare Zeitskala, die bei Inbetriebnahme des Satellitensystems im Jahre 1980 mit der internationalen
Zeitskala UTC gleichgesetzt wurde. Seit dieser Zeit wurden jedoch in der UTC-Zeit mehrfach Schaltsekunden eingefgt, um die
UTC-Zeit der nderung der Erddrehung anzupassen. Aus diesem Grund unterscheidet sich heute die GPS-Systemzeit um eine ganze
Anzahl Sekunden von der UTC-Zeit. Die Anzahl der Differenzsekunden ist jedoch im Datenstrom der Satelliten enthalten, so daá der
Empf„nger intern synchron zur internationalen Zeitskala UTC laufen kann. Umrechnung der UTC-Zeit in die Ortszeit sowie Bestimmung
von Beginn und Ende der Sommerzeit werden vom Mikroprozessor des Empf„ngers ausgefhrt, da die dazu ben”tigten Informationen
nicht im Datenstrom der Satelliten enthalten sind.

:p.:hp1.(entnommen: Meinberg Funkuhren: Systembeschreibung GPS):ehp1.

.*----------------------------------------------------------
.*  Analoge Ansicht
.*----------------------------------------------------------
:h1 res=20.Analoge Ansicht
:i1.Analoge Ansicht

:p.Die :hp1.analoge Ansicht:ehp1. entspricht einer Uhr mit rundem Ziffernblatt und
Stunden- und Minuteneinteilung sowie einem Sekundenzeiger.

:p.Bei Auswahl von :hp2.Analog:ehp2. wird eine Uhr mit Stunden-
und Minuteneinteilung und einem Sekundenzeiger dargestellt.
Auáerhalb des Ziffernblattes befinden sich die Statusanzeigen fr den
DCF77/GPS-Empfang&colon.

:p.:hp5.Empfangsstatus:ehp5.
:dl compact tsize=6.
:dt.:artwork name='E:\WORK\C\CLOCK53\DOC\carrnd.bmp' align=center runin.
:dd.Treiber (z. B. :hp2.ZEIT.SYS:ehp2.) oder LAN&colon.&colon.Time-Client (:hp2.TIMECLNT.EXE:ehp2.) nicht installiert.
:dt.:artwork name='E:\WORK\C\CLOCK53\DOC\carrx.bmp' align=center runin.
:dd.Hardware nicht installiert, Einstellungen nicht korrekt, die Empfangsqualit„t ist zu schlecht oder
der LAN&colon.&colon.Time-Server (:hp2.TIMESERV.EXE:ehp2.) sendet nicht.
:dt.:artwork name='E:\WORK\C\CLOCK53\DOC\carrnok.bmp' align=center runin.
:dd.Aktuelle Zeit ist nicht gltig&colon. Empfangsbedingungen schlecht.
:dt.:artwork name='E:\WORK\C\CLOCK53\DOC\carrok.bmp' align=center runin.
:dd.Aktuelle Zeit ist gltig.
:edl.

:p.:hp5.Zeitzone:ehp5.
:dl compact tsize=6.
:dt.:artwork name='E:\WORK\C\CLOCK53\DOC\winter.bmp' align=center runin.
:dd.Winterzeit
:dt.:artwork name='E:\WORK\C\CLOCK53\DOC\summer.bmp' align=center runin.
:dd.Sommerzeit
:edl.

:p.:hp5.Sonstiges:ehp5.
:dl compact tsize=6.
:dt.:artwork name='E:\WORK\C\CLOCK53\DOC\batt.bmp' align=center runin.
:dd.Batterie leer (nur bei HOPF-Empfangsmodul).
:edl.

.*----------------------------------------------------------
.*  Digitale Ansicht
.*----------------------------------------------------------
:h1 res=30.Digitale Ansicht
:i1.Digitale Ansicht

:p.Die :hp2.digitale Ansicht:ehp2. entspricht einer Uhr mit einer alphanumerischen
Anzeige von Datum und Uhrzeit. Im Gegensatz zur 
:link reftype=hd res=20.analogen Ansicht:elink. bietet
diese Darstellung keine Darstellung des DCF77/GPS-Status.

.*----------------------------------------------------------
.*  Anzeigen von Notizbuchseiten
.*----------------------------------------------------------
:h1 res=10000.Anzeigen von Notizbuchseiten
:i1.Anzeigen von Notizbuchseiten

:p.Um weitere Seiten des Notizbuchs anzuzeigen, 
entweder eine Indexzunge oder einen der Pfeile in 
der unteren rechten Ecke ausw„hlen.   

.*----------------------------------------------------------
.*  Anzeigen von Kontextmens
.*----------------------------------------------------------
:h1 res=10001.Anzeigen von Kontextmens
:i1.Anzeigen von Kontextmens

:p.Jedes Objekt besitzt ein Kontextmen, das alle die 
Funktionen enth„lt, die zur Zeit verfgbar sind. Das 
Kontextmen fr ein beliebiges Objekt wird 
folgendermaáen angezeigt&colon.

:ol compact.
:li.Den Mauszeiger zum Uhrensymbol bewegen. 
:li.Maustaste 2 drcken. 
:eol.

:p.Ist das Objekt ge”ffnet, kann das 
Men auch durch Klicken mit Maustaste 2 
durch Klicken in das Titelleistensymbol in der 
oberen linken Ecke angezeigt werden. 

:note.Pfeile rechts neben einer Menauswahl 
zeigen an, daá noch weitere Auswahlm”glichkeiten 
verfgbar sind. Ein Pfeil, der sich auf einem 
stilisierten Druckknopf befindet, bedeutet, daá eine 
weiterfhrende Auswahl als Standardfunktion (durch 
den Haken gekennzeichnet) vorausgew„hlt ist, wenn 
die bergeordnete Auswahl angegeben wird. Ein 
einfacher Pfeil zeigt an, daá noch weitere 
Auswahlm”glichkeiten zur Verfgung stehen. 

.*----------------------------------------------------------
.*  ndern von Objektnamen
.*----------------------------------------------------------
:h1 res=10002.ndern von Objektnamen
:i1.ndern von Objektnamen

:p.Sie k”nnen den Namen von Ordnern und Objekten 
„ndern. 

:ol compact.
:li.Den Mauszeiger zum Titel des Objekts bzw. Ordners bewegen, das bzw. der ge„ndert werden soll. 
:li.Die Taste Alt gedrckt halten, dann mit der Maustaste 1 in den Titel klicken. 
:li.Die nderungen eingeben und den vorigen Namen mit Hilfe der Rcktaste oder der Taste Entf l”schen. 
:li.Den Zeiger vom Namen wegbewegen und nochmals die Maustaste 1 drcken. 
:eol.

.*----------------------------------------------------------
:h2 res=10100.Widerrufen
:i1.Widerrufen

:p.Hiermit werden die Werte wiederverwendet, die vor 
Anzeige dieses aktiven Fensters gltig waren.   

.*----------------------------------------------------------
:h2 res=10101.Standard
:i1.Standard

:p.Hiermit werden die Werte wiederverwendet, die beim 
Installieren des Systems gltig waren.   

.*----------------------------------------------------------
.*  Datum/Uhrzeit
.*----------------------------------------------------------
:h1 res=1100.Datum/Uhrzeit
:i1 id=1100.Datum/Uhrzeit

:p.Ist kein DCF77/GPS-Treiber installiert, k”nnen Sie mit :hp2.Datum/Uhrzeit:ehp2. die aktuellen 
Einstellungen fr Datum und Uhrzeit „ndern. Gehen Sie dazu wie folgt vor&colon.

:ol compact.
:li.Den Bestandteil der Uhrzeit oder des Datums, 
der nicht richtig ist, ausw„hlen und mit Hilfe der 
Pfeile korrigieren. 
:li.Anschlieáend einen anderen Bereich im Fenster 
:hp2.Datum und Uhrzeit:ehp2. ausw„hlen. 
:eol.

:p.Eine genaue Erkl„rung fr jedes Feld wird angezeigt, 
wenn der entsprechende Begriff aus der 
untenstehenden Liste ausgew„hlt wird&colon.

:ul compact.
:li.:link reftype=hd res=1101.Stunde:elink.
:li.:link reftype=hd res=1102.Minuten:elink.
:li.:link reftype=hd res=1103.Sekunden:elink.
:li.:link reftype=hd res=1104.Monat:elink.
:li.:link reftype=hd res=1105.Tag:elink.
:li.:link reftype=hd res=1106.Jahr:elink.
:eul.

:p.Zugeh”rige Information&colon.

:p.:link reftype=hd res=10000.Anzeigen von Notizbuchseiten:elink.

.*----------------------------------------------------------
:h2 res=1101.Stunden
:i2 refid=1100.Stunden

:p.In diesem Feld wird die Stundenangabe der 
Systemuhr angezeigt. Diese Angabe kann mit den 
Pfeilen rechts ge„ndert werden. W„hlen Sie 
anschlieáend einen anderen Bereich in diesem 
Fenster aus.   

.*----------------------------------------------------------
:h2 res=1102.Minuten
:i2 refid=1100.Minuten

:p.In diesem Feld wird die aktuelle Minutenangabe der 
Systemuhr angezeigt. Diese Angabe kann mit den 
Pfeilen rechts ge„ndert werden. W„hlen Sie 
anschlieáend einen anderen Bereich in diesem 
Fenster aus. 

.*----------------------------------------------------------
:h2 res=1103.Sekunden
:i2 refid=1100.Sekunden

:p.In diesem Feld wird die aktuelle Sekundenangabe der 
Systemuhr angezeigt. Diese Angabe kann mit den 
Pfeilen rechts ge„ndert werden. W„hlen Sie 
anschlieáend einen anderen Bereich in diesem 
Fenster aus.   

.*----------------------------------------------------------
:h2 res=1104.Monat
:i2 refid=1100.Monat

:p.In diesem Feld wird die aktuelle Monatsangabe der 
Systemuhr angezeigt. Diese Angabe kann mit den 
Pfeilen rechts ge„ndert werden. W„hlen Sie 
anschlieáend einen anderen Bereich in diesem 
Fenster aus.   


.*----------------------------------------------------------
:h2 res=1105.Tag
:i2 refid=1100.Tag

:p.In diesem Feld wird die aktuelle Wochentagangabe 
der Systemuhr angezeigt. Diese Angabe kann mit den 
Pfeilen rechts ge„ndert werden. W„hlen Sie 
anschlieáend einen anderen Bereich in diesem 
Fenster aus.   


.*----------------------------------------------------------
:h2 res=1106.Jahr
:i2 refid=1100.Jahr

:p.In diesem Feld wird die aktuelle Jahresangabe der 
Systemuhr angezeigt. Diese Angabe kann mit den 
Pfeilen rechts ge„ndert werden. W„hlen Sie 
anschlieáend einen anderen Bereich in diesem 
Fenster aus.   

.*----------------------------------------------------------
.*  Zeitoffset
.*----------------------------------------------------------
:h1 res=1800.Zeitoffset
:i1 id=1800.Zeitoffset

:p.Diese Seite dient zur Einstellung von Zeitoffsets. Ein :hp2.systemweiter:ehp2. Offset betrifft
alle Zeitangaben innerhalb des Systems, w„hrend die :hp2.uhrspezifische:ehp2. Angabe sich
nur auf eine Zeitdifferenz zwischen der ausgew„hlten Uhr und der Systemzeit bezieht.
Es gibt folgende Einstellm”glichkeiten&colon.

:ol compact.
:li.:link reftype=hd res=1801.UTC/Greenwich:elink.
:li.:link reftype=hd res=1802.Systemweiter Offset:elink.
:li.:link reftype=hd res=1803.Uhrspezifischer Offset:elink.
:li.:link reftype=hd res=1804.Zeitzone:elink.
:li.:link reftype=hd res=10100.Widerrufen:elink.
:li.:link reftype=hd res=10101.Standard:elink.
:eol.

.*----------------------------------------------------------
:h2 res=1801.UTC/Greenwich
:i2 refid=1800.UTC/Greenwich

:p.Wird dieses Markierungsfeld ausgew„hlt, wird die vom DCF77-Sender ausgesendete
Zeitzone (Winter-/Sommerzeit) ignoriert und direkt die UTC-Zeit (Greenwich-Zeit)
eingestellt.
Diese Zeit wird vom gesamten System bernommen.

.*----------------------------------------------------------
:h2 res=1802.Systemweiter Offset
:i2 refid=1800.Systemweiter Offset

:p.Die Stellung dieses Schiebereglers gibt an, wieviele Stunden zu der vom DCF77/GPS-Sender
ausgesendeten Zeit addiert werden sollen.
Diese Einstellung wird vom gesamten System bernommen.

.*----------------------------------------------------------
:h2 res=1803.Uhrspezifischer Offset
:i2 refid=1800.Uhrspezifischer Offset

:p.Die Stellung dieses Schiebereglers gibt an, wieviele Stunden
zu der vom DCF77/GPS-Sender ausgesendeten Zeit addiert werden sollen.
Diese Einstellung gilt nur fr die ausgew„hlte Uhr.

.*----------------------------------------------------------
:h2 res=1804.Zeitzone
:i2 refid=1804.Zeitzone

:p.In dieses Eingabefeld kann der Name der Zeitzone, auf die die ausgew„hlte Uhr gestellt ist,
eingegeben werden. Der Text erscheint (nach Schlieáen des Einstellungs-Notizbuches) in der
Titelzeile der Uhr.

.*----------------------------------------------------------
.*  Sommer-/Winterzeitumschaltung
.*----------------------------------------------------------
:h1 res=2700.Sommer-/Winterzeitumschaltung
:i1.Sommer-/Winterzeitumschaltung

:p.Bei Empfang der Uhrzeit mit Hilfe eines GPS-Empf„ngers (z. B. hopf 6038), ist
die Uhrzeit die UTC-Zeit. Diese muá fr die entsprechende Zeitzone auf die Lokalzeit
zurckgerechnet und die Sommer-/Winterzeit bercksichtigt werden.

:p.Basis dieser Berechnungen ist eine Zeitzonen-Datei, wie sie auf UNIX-Systemen verwendet
wird. Standardm„áig wird die Datei :hp1.TZTAB:ehp1. mitgeliefert, die alle weltweit wichtigen
Zeitzonen enth„lt. nderungen an dieser Datei k”nnen bei lokalen nderungen der 
Sommer-/Winterzeit-Umschaltpunkte notwendig werden und sind von Hand mit Hilfe eines einfachen
Texteditors durchzufhren.

:p.Ein Eintrag innerhalb dieser Datei hat folgenden Aufbau&colon.
:ol compact.
:li.Kommentarzeilen&colon. Diese beginnen mit dem Zeichen '#'. Beispiel&colon.
  :cgraphic.# Mitteleuropaeische Zeit, Mitteleuropaeische Sommerzeit:ecgraphic.:p.
:li.Zeitzonenname&colon.Dieser Name ist ein String, der sich zusammensetzt aus dem
Namen der Zeitzone bei Standardzeit gefolgt einer Zahl, die den Unterschied zur UTC-Zeit
angibt. Der String endet mit dem Namen der Zeitzone zur Sommerzeit. Beispiel&colon.
  :cgraphic.
  MEZ-1MESZ
   ³ ³  ³
   ³ ³  ÀÄÄÄÄÄ> mitteleurop„ische Sommerzeit
   ³ ÀÄÄÄÄÄÄÄÄ> Differenz der Winterzeit zu UTC ist -1 Stunde
   ÀÄÄÄÄÄÄÄÄÄÄ> mitteleuro„ische Zeit
  :ecgraphic.
:p.Bei Zeitzonen, bei denen die Differenz zu UTC kein ganzzahliges Vielfaches eine Stunde ist, werden
die Minuten durch einen Doppelpunkt abgetrennt (Beispiel&colon. -1&colon.30).:p.
:li.Umschaltzeitpunkte&colon. Jeder Umschaltzeitpunkt wird in einer Zeile definiert.
Jede dieser Zeilen hat den Aufbau&colon.
  :ol compact.
  :li. Minuten des Umschaltzeitpunktes
  :li. Stunden des Umschaltzeitpunktes
  :li. Tag / Tagesintervall des Umschaltzeitpunktes
  :li. Monat des Umschaltzeitpunktes
  :li. Jahr / Jahresintervall des Umschaltzeitpunktes
  :li. Wochentag / Wochentagesintervall des Umschaltzeitpunktes (0=So, 1=Mo, ..., 6=Sa)
  :li. Name der Zeitzone und Abstand zu UTC
  :eol.
Beispiel&colon.
  :cgraphic.
  0 3 25-31 3  1983-2038 0   MESZ-2
   ³    ³   ³      ³     ³    ³
   ³    ³   ³      ³     ³    ÀÄÄÄÄÄÄÄÄ> mitteleurop„ische Sommerzeit, Abstand zu UTC = -1Stunde
   ³    ³   ³      ³     ÀÄÄÄÄÄÄÄÄÄÄÄÄÄ> Umschalttag ist Sonntag
   ³    ³   ³      ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ> gltig innerhalb der Jahre 1983 bis 2038 einschlieálich
   ³    ³   ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ> Umschaltung ist im M„rz
   ³    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ> Umschaltung erfolgt innerhalb der Tage 25 bis 31 einschlieálich
   ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ> Umschaltung erfolgt um 03&colon.00&colon.00 Uhr
  :ecgraphic.
:eol.

:p.Eine genaue Erkl„rung fr jedes Feld wird angezeigt, 
wenn der entsprechende Begriff aus der 
untenstehenden Liste ausgew„hlt wird&colon.

:ul compact.
:li.:link reftype=hd res=2701.Dateiname:elink.
:li.:link reftype=hd res=2702.Ausw„hlen:elink.
:li.:link reftype=hd res=2703.Zeitzone:elink.
:li.:link reftype=hd res=2704.Sommerzeitdefinition:elink.
:li.:link reftype=hd res=2705.Winterzeitdefinition:elink.
:li.:link reftype=hd res=10100.Widerrufen:elink.
:li.:link reftype=hd res=10101.Standard:elink.
:eul.

:p.Zugeh”rige Information&colon.

:p.:link reftype=hd res=10000.Anzeigen von Notizbuchseiten:elink.

.*----------------------------------------------------------
:h2 res=2701.Dateiname
:i2 refid=1804.Dateiname

:p.Dieses Textfeld zeigt die momentan verwendete Zeitzonendatei an.

.*----------------------------------------------------------
:h2 res=2702.Ausw„hlen
:i2 refid=1804.Ausw„hlen

:p.Ein Drcken dieses Knopfes ”ffnet den Dateiauswahldialog, mit dessen Hilfe sich die
Zeitzonendatei ausw„hlen l„át.

.*----------------------------------------------------------
:h2 res=2703.Zeitzonen
:i2 refid=1804.Zeitzonen

:p.Diese Auswahlliste zeigt die Zeitzonen, die in der Zeitzonendatei aufgefhrt sind 
und l„át die Auswahl eines Eintrages zu. Dieser beeinfluát nach Schlieáen des Notizbuches
die Uhrzeit des Systems.

.*----------------------------------------------------------
:h2 res=2704.Sommerzeitdefinition
:i2 refid=1804.Sommerzeitdefinition

:p.Dieses Textfeld zeigt den n„chsten Umschaltzeitpunkt fr die Sommerzeit.

.*----------------------------------------------------------
:h2 res=2705.Winterzeitdefinition
:i2 refid=1804.Winterzeitdefinition

:p.Dieses Textfeld zeigt den n„chsten Umschaltzeitpunkt fr die Winterzeit.

.*----------------------------------------------------------
.*  Anzeigen Seite 1/3
.*----------------------------------------------------------
:h1 res=1200.Anzeigen Seite 1/3
:i1.Anzeigen Seite 1/3
 
:p.W„hlen Sie die gewnschte Anzeigeart fr die 
Systemuhr mit Hilfe der Radiokn”pfe in den Feldern 
:hp2.Information:ehp2. und :hp2.Modus:ehp2. aus.

:p.Eine genaue Erkl„rung fr jedes Feld wird angezeigt, 
wenn der entsprechende Begriff aus der 
untenstehenden Liste ausgew„hlt wird&colon.

:ul compact.
:li.:link reftype=hd res=1201.Nur Uhrzeit:elink.
:li.:link reftype=hd res=1202.Nur Datum:elink.
:li.:link reftype=hd res=1203.Datum und Uhrzeit:elink.
:li.:link reftype=hd res=1204.Digital:elink.
:li.:link reftype=hd res=1205.Analog:elink.
:li.:link reftype=hd res=1206.Sekundenzeiger:elink.
:li.:link reftype=hd res=1207.Stundeneinteilung:elink.
:li.:link reftype=hd res=1208.Minuteneinteilung:elink.
:li.:link reftype=hd res=1209.Bayrischer Anzeigemodus:elink.
:li.:link reftype=hd res=1210.Titelleiste anzeigen:elink.
:li.:link reftype=hd res=1211.Iconposition sichern:elink.
:li.:link reftype=hd res=10100.Widerrufen:elink.
:li.:link reftype=hd res=10101.Standard:elink.
:eul.
  
:p.Es k”nnen noch weitere Funktionen von :hp2.Anzeigen&colon. Seite 2/3:ehp2.
ausgehend ausgefhrt werden. W„hlen Sie 
den Rechtspfeil unten am Fenster aus, um zu 
:hp2.Anzeigen&colon. Seite 2/3:ehp2. umzuschalten. 

:p.Zugeh”rige Information&colon.

:p.:link reftype=hd res=10000.Anzeigen von Notizbuchseiten:elink.

.*----------------------------------------------------------
:h2 res=1201.Nur Uhrzeit
:i2 refid=1200.Nur Uhrzeit
  
:p.Durch Ausw„hlen von :hp2.Nur Uhrzeit:ehp2. wird die Uhrzeit 
ohne Datum angezeigt.

.*----------------------------------------------------------
:h2 res=1202.Nur Datum
:i2 refid=1200.Nur Datum
  
:p.Durch Ausw„hlen von :hp2.Nur Datum:ehp2. wird das Datum 
ohne Uhrzeit angezeigt. Diese Auswahl steht zur 
Verfgung, wenn die Systemuhr :link reftype=hd res=1204.digital:elink.
angezeigt wird.   

.*----------------------------------------------------------
:h2 res=1203.Datum und Uhrzeit
:i2 refid=1200.Datum und Uhrzeit
 
:p.Durch Ausw„hlen von :hp2.Datum und Uhrzeit:ehp2. werden 
Datum und Uhrzeit angezeigt.
 
.*----------------------------------------------------------
:h2 res=1204.Digital
:i2 refid=1200.Digital
  
:p.Bei Auswahl von :hp2.Digital:ehp2. wird die Uhrzeit in 
:link reftype=hd res=1204.digitaler:elink.
Darstellung angezeigt.   

.*----------------------------------------------------------
:h2 res=1205.Analog
:i2 refid=1200.Analog
   
:p.Bei Auswahl von :hp2.Analog:ehp2. wird eine Uhr mit Stunden-
und Minuteneinteilung und einem Sekundenzeiger 
(d. h. :link reftype=hd res=20.analog:elink.) angezeigt.

.*----------------------------------------------------------
:h2 res=1206.Sekundenzeiger
:i2 refid=1200.Sekundenzeiger
  
:p.Bei Auswahl dieses Markierungsfelds wird eine 
Analoguhr mit Sekundenzeiger angezeigt.   

.*----------------------------------------------------------
:h2 res=1207.Stundeneinteilung
:i2 refid=1200.Stundeneinteilung
  
:p.Bei Auswahl dieses Markierungsfelds wird eine 
Analoguhr mit Stundeneinteilung angezeigt.   

.*----------------------------------------------------------
:h2 res=1208.Minuteneinteilung
:i2 refid=1200.Minuteneinteilung
  
:p.Bei Auswahl dieses Markierungsfelds wird eine 
Analoguhr mit Minuteneinteilung angezeigt. 

.*----------------------------------------------------------
:h2 res=1209.Bayrischer Anzeigemodus
:i2 refid=1200.Bayrischer Anzeigemodus

:p.Einfach ausprobieren! 

.*----------------------------------------------------------
:h2 res=1210.Titelleiste anzeigen
:i2 refid=1200.Titelleiste anzeigen
 
:p.Bei Auswahl dieses Markierungsfelds wird die Uhr mit 
einer Titelleiste, einem Titelleistensymbol, einem 
Druckknopf fr die Symbolgr”áe und einem Druckknopf fr die 
maximale Gr”áe angezeigt. 

Wird :hp2.Titelleiste anzeigen:ehp2. nicht ausgew„hlt, wird 
die Uhr in einem Fenster mit einem Rahmen mit w„hlbarer Gr”áe
angezeigt.

:p.Zugeh”rige Information&colon.

:p.:link reftype=hd res=10002.ndern von Objektnamen:elink.
 
.*----------------------------------------------------------
:h2 res=1211.Iconposition sichern
:i2 refid=1200.Iconposition sichern

:p.Standardm„áig wird die Position von Fenstern im verkleinerten
Zustand (Symbolgr”áe) vom Betriebssystem festgelegt und nach
r„umlichen Gesichtspunkten verwaltet.

:p.Ist dieses Markierungsfeld aktiviert, wird die Position
der Uhr in Symbolgr”áe gesichert.

.*----------------------------------------------------------
.*  Anzeigen Seite 2/3
.*----------------------------------------------------------
:h1 res=1300.Anzeigen Seite 2/3
:i1.Anzeigen Seite 2/3
 
:p.W„hlen Sie die gewnschte Anzeigeart fr die 
Systemuhr mit Hilfe der Radiokn”pfe in den Feldern 
:hp2.Farben:ehp2. und :hp2.Schriftarten:ehp2. aus. 

:p.Eine genaue Erkl„rung fr jedes Feld wird angezeigt, 
wenn der entsprechende Begriff aus der 
untenstehenden Liste ausgew„hlt wird&colon.

:ul compact.
:li.:link reftype=hd res=1302.Hintergrund:elink.
:li.:link reftype=hd res=1301.Zifferblatt:elink.
:li.:link reftype=hd res=1303.Stundenmarkierung:elink. 
:li.:link reftype=hd res=1305.Stundenzeiger:elink.
:li.:link reftype=hd res=1306.Minutenzeiger:elink.
:li.:link reftype=hd res=1304.Datumsfeld:elink.
:li.:link reftype=hd res=1307.Farbe Datum:elink.
:li.:link reftype=hd res=1308.Farbe Uhrzeit:elink.
:li.:link reftype=hd res=1309.Transparentfarbe fr Bitmaps:elink.
:li.:link reftype=hd res=1310.Farbe „ndern:elink.
:li.:link reftype=hd res=1311.Datum:elink.
:li.:link reftype=hd res=1312.Uhrzeit:elink.
:li.:link reftype=hd res=1313.Schriftart „ndern:elink.
:li.:link reftype=hd res=10100.Widerrufen:elink.
:li.:link reftype=hd res=10101.Standard:elink.
:eul.
  
:p.Es k”nnen noch weitere Funktionen von :hp2.Anzeigen&colon.
Seite 1/3:ehp2. aus ausgefhrt werden. W„hlen Sie den 
Linkspfeil unten am Fenster aus, um zu :hp2.Anzeigen&colon.
Seite 1/3:ehp2. umzuschalten. 

:p.Zugeh”rige Information&colon.

:p.:link reftype=hd res=10000.Anzeigen von Notizbuchseiten:elink.

.*----------------------------------------------------------
:h2 res=1301.Zifferblatt
:i2 refid=1300.Zifferblatt
 
:p.Durch Ausw„hlen von :hp2.Zifferblatt:ehp2. geben Sie an, daá 
Sie mit :hp2.Farbe „ndern:ehp2. die Farbe des fr die Uhr 
verwendeten Zifferblatts „ndern wollen. 

.*----------------------------------------------------------
:h2 res=1302.Hintergrund
:i2 refid=1300.Hintergrund
  
:p.Durch Ausw„hlen von :hp2.Hintergrund:ehp2. geben Sie an, 
daá Sie mit :hp2.Farbe „ndern:ehp2. die Farbe des fr die Uhr 
verwendeten Hintergrunds „ndern wollen.   

.*----------------------------------------------------------
:h2 res=1303.Stundenmarkierung
:i2 refid=1300.Stundenmarkierung
  
:p.Durch Ausw„hlen von :hp2.Stundenmarkierung:ehp2. geben Sie 
an, daá Sie mit :hp2.Farbe „ndern:ehp2. die Farbe der fr die 
Uhr verwendeten Stundeneinteilung „ndern wollen.   

.*----------------------------------------------------------
:h2 res=1305.Stundenzeiger
:i2 refid=1300.Stundenzeiger
  
:p.Durch Ausw„hlen von :hp2.Stundenzeiger:ehp2. geben Sie an, daá Sie 
mit :hp2.Farbe „ndern:ehp2. die Farbe des Stundenzeigers „ndern 
wollen.   

.*----------------------------------------------------------
:h2 res=1306.Minutenzeiger
:i2 refid=1300.Minutenzeiger
  
:p.Durch Ausw„hlen von :hp2.Minutenzeiger:ehp2. geben Sie an, daá Sie 
mit :hp2.Farbe „ndern:ehp2. die Farbe des Minutenzeigers „ndern 
wollen.   

.*----------------------------------------------------------
:h2 res=1304.Datumsfeld
:i2 refid=1300.Datumsfeld
  
:p.Durch Ausw„hlen von :hp2.Datumsfeld:ehp2. geben Sie an, daá 
Sie mit :hp2.Farbe „ndern:ehp2. die Farbe des Datumsfeldes
der :link reftype=hd res=1205.analogen:elink. Uhr
„ndern wollen.

.*----------------------------------------------------------
:h2 res=1307.Farbe Datum
:i2 refid=1300.Farbe Datum
 
:p.Durch Ausw„hlen von :hp2.Farbe Datum:ehp2. geben Sie an, daá Sie 
mit :hp2.Farbe „ndern:ehp2. die Farbe der Schrift des Datumstextes
„ndern wollen.

.*----------------------------------------------------------
:h2 res=1308.Farbe Uhrzeit
:i2 refid=1300.Farbe Uhrzeit
 
:p.Durch Ausw„hlen von :hp2.Farbe Datum:ehp2. geben Sie an, daá Sie 
mit :hp2.Farbe „ndern:ehp2. die Farbe der Schrift des Uhrzeittextes
der :link reftype=hd res=1204.digitalen:elink. Uhr „ndern wollen.

.*----------------------------------------------------------
:h2 res=1309.Transparentfarbe fr Bitmaps
:i2 refid=1300.Transparentfarbe fr Bitmaps

:p.Durch Ausw„hlen von :hp2.Transparentfarbe fr Bitmaps:ehp2. geben Sie an, daá
Sie mit :hp2.Farbe „ndern:ehp2. die Transparentfarbe der Bitmapdateien „ndern wollen.

.*----------------------------------------------------------
:h2 res=1310.Farbe „ndern
:i2 refid=1300.Farbe „ndern
  
:p.Mit dem Druckknopf :hp2.Farbe „ndern:ehp2. k”nnen Sie die 
Palette mit den Farben anzeigen, die fr die 
einzelnen Bestandteile der Uhr ausgew„hlt werden 
k”nnen. Im einzelnen sind die folgenden Schritte 
erforderlich&colon.

:ol compact.
:li.Die gewnschten Ausw„hlkn”pfe im Bereich :hp2.Farbe:ehp2. ausw„hlen, um den Bereich der Uhr anzugeben,
dessen Farbe ge„ndert werden soll. 
:li.:hp2.Farbe „ndern:ehp2. ausw„hlen.
:li.Die Schritte 1 und 2 fr andere Bereiche der Uhr wiederholen. 
:eol.

.*----------------------------------------------------------
:h2 res=1311.Datum
:i2 refid=1300.Datum
  
:p.Durch Ausw„hlen von :hp2.Datum:ehp2. geben Sie an, daá Sie 
mit :hp2.Schriftart „ndern:ehp2. die fr das Datum 
verwendete Schriftart „ndern wollen. 

.*----------------------------------------------------------
:h2 res=1312.Uhrzeit
:i2 refid=1300.Uhrzeit
  
:p.Durch Ausw„hlen von :hp2.Uhrzeit:ehp2. geben Sie an, daá Sie 
mit :hp2.Schriftart „ndern:ehp2. die fr die Uhrzeit 
verwendete Schriftart „ndern wollen.   

.*----------------------------------------------------------
:h2 res=1313.Schriftart „ndern
:i2 refid=1300.Schriftart „ndern
 
:p.Mit dem Druckknopf :hp2.Schriftart „ndern:ehp2. wird die 
Palette mit den Schriftarten angezeigt, die fr Datum 
und Uhrzeit ausgew„hlt werden k”nnen. Im einzelnen 
sind die folgenden Schritte erforderlich&colon.

:ol compact.
:li.Einen der Radiokn”pfe im Bereich :hp2.Schriftart ausw„hlen:ehp2.,
um den Bereich der Uhr anzugeben, der ge„ndert werden soll. 
:li.:hp2.Schriftart „ndern:ehp2. ausw„hlen. 
:li.Die Schritte 1 und 2 fr die anderen Bereiche der Uhr wiederholen. 
:eol.

.*----------------------------------------------------------
.*  Font ausw„hlen
.*----------------------------------------------------------
:h1 res=310.Font ausw„hlen
:i1.Font ausw„hlen

:p.Mit Hilfe dieser Dialogbox kann der Font fr den ausgew„hlten Text festgelegt werden.
Die DCF77/GPS-Uhr l„át grunds„tzlich nur die Auswahl von sogenannten Vektorfonts zu, die
in H”he und Breite so skaliert werden, daá sie in die aktuelle Gr”áe des Textbereiches
passen.

.*----------------------------------------------------------
.*  Alarm Seite 1/3
.*----------------------------------------------------------
:h1 res=1900.Alarm / Erinnerung Seite 1/3
:i1.Alarm Seite 1/3
 
:p.Mit der Auswahl :hp2.Alarm:ehp2. k”nnen Sie die Alarm- bzw. 
Erinnerungsfunktion aktivieren oder inaktivieren und vorgeben, 
ob der Alarm akustisch oder visuell (Nachrichtenfenster) 
ausgel”st werden soll.

:p.Um Uhrzeit oder Datum fr die Alarmfunktion zu 
„ndern, muá der entsprechende Bestandteil der 
Zeitangabe ausgew„hlt und mit den Pfeilen rechts 
ge„ndert werden. 
  
:p.Die Erinnerungsfunktion gibt einen Alarm nach Ablauf eines
in Minuten angegebenen Intervalls aus. Sie wird vom Kontextmen
der Uhr aus gestartet.

:p.Eine genaue Erkl„rung fr jedes Feld wird angezeigt, 
wenn der entsprechende Begriff aus der 
untenstehenden Liste ausgew„hlt wird&colon.

:ul compact.
:li.:link reftype=hd res=1901.Alarm aktivieren:elink.
:li.:link reftype=hd res=1902.Erinnerung:elink.
:li.:link reftype=hd res=1903.Alarm inaktivieren:elink.
:li.:link reftype=hd res=1904.Akustisches Signal:elink.
:li.:link reftype=hd res=1905.Nachrichtenfenster:elink.
:li.:link reftype=hd res=1906.Programm starten:elink.
:li.:link reftype=hd res=1907.Stunde:elink.
:li.:link reftype=hd res=1909.Minuten:elink.
:li.:link reftype=hd res=1908.Sekunden:elink.
:li.:link reftype=hd res=1910.Monat:elink.
:li.:link reftype=hd res=1911.Tag:elink.
:li.:link reftype=hd res=1912.Jahr:elink.
:li.:link reftype=hd res=1913.Einmal:elink.
:li.:link reftype=hd res=1914.Wochentag Mo...So:elink.
:li.:link reftype=hd res=10100.Widerrufen:elink.
:eul.
  
:p.Zugeh”rige Information&colon.

:p.:link reftype=hd res=10000.Anzeigen von Notizbuchseiten:elink.

.*----------------------------------------------------------
:h2 res=1901.Alarm aktivieren
:i2 refid=1900.Alarm aktivieren
 
:p.Durch Ausw„hlen von :hp2.Alarm aktivieren:ehp2. wird vom 
System eine Meldung geschickt, wenn die eingestellte 
Uhrzeit und das eingestellte Datum mit der Systemuhr 
bereinstimmen. Durch Ausw„hlen von :hp2.Akustisches Signal:ehp2.
oder :hp2.Nachrichtenfenster:ehp2. kann angegeben werden, 
in welcher Form die Meldung erfolgen soll.   
 
.*----------------------------------------------------------
:h2 res=1902.Erinnerung
:i2 refid=1900.Erinnerung

:p.Durch Ausw„hlen von :hp2.Erinnerung:ehp2. wird vom
System eine Meldung geschickt, wenn das eingestellte
Intervall abgelaufen ist. Das Intervall wird durch den
Menpunkt :hp2.Erinnerung:ehp2. im Systemmen der
Uhr gestartet.

.*----------------------------------------------------------
:h2 res=1903.Alarm deaktivieren
:i2 refid=1900.Alarm deaktivieren
  
:p.Durch Ausw„hlen von :hp2.Alarm inaktivieren:ehp2. erfolgt 
keine Meldung vom System, wenn Datum und Uhrzeit 
der Alarmfunktion mit den Zeitangaben der 
Systemuhr bereinstimmen.   

.*----------------------------------------------------------
:h2 res=1904.Akustisches Signal
:i2 refid=1900.Akustisches Signal
 
:p.Bei Auswahl dieses Markierungsfelds ert”nt zum 
vorgegebenen Zeitpunkt ein akustisches Signal.   
 
.*----------------------------------------------------------
:h2 res=1905.Nachrichtenfenster
:i2 refid=1900.Nachrichtenfenster
 
:p.Bei Auswahl dieses Markierungsfelds wird zum 
vorgegebenen Zeitpunkt eine Nachricht angezeigt.
Der Inhalt dieser Nachricht kann vorgegeben werden
(siehe :link reftype=hd res=2100.Alarm Seite 2/2:elink.).
 
.*----------------------------------------------------------
:h2 res=1906.Programm starten
:i2 refid=1900.Programm starten
 
:p.Bei Auswahl dieses Markierungsfelds wird zum 
vorgegebenen Zeitpunkt das auf der Seite :link reftype=hd res=2300.Alarm Seite 3/3:elink.
angegebene Programm gestartet.
 
.*----------------------------------------------------------
:h2 res=1907.Stunden
:i2 refid=1900.Stunden
:p.In diesem Feld wird die aktuelle Stundenangabe fr 
die Alarmfunktion angezeigt. Diese Angabe kann mit 
den Pfeilen rechts ge„ndert werden. W„hlen Sie 
anschlieáend einen anderen Bereich in diesem 
Fenster aus.   

.*----------------------------------------------------------
:h2 res=1909.Minuten
:i2 refid=1900.Minuten
 
:p.In diesem Feld wird die aktuelle Minutenangabe fr 
die Alarmfunktion angezeigt. Diese Angabe kann mit 
den Pfeilen rechts ge„ndert werden. W„hlen Sie 
anschlieáend einen anderen Bereich in diesem 
Fenster aus.   
 
.*----------------------------------------------------------
:h2 res=1908.Sekunden
:i2 refid=1900.Sekunden
 
:p.In diesem Feld wird die aktuelle Sekundenangabe fr 
die Alarmfunktion angezeigt. Diese Angabe kann mit 
den Pfeilen rechts ge„ndert werden. W„hlen Sie 
anschlieáend einen anderen Bereich in diesem 
Fenster aus.   
 
.*----------------------------------------------------------
:h2 res=1910.Monat
:i2 refid=1900.Monat
  
:p.In diesem Feld wird die aktuelle Monatsangabe fr 
die Alarmfunktion angezeigt. Diese Angabe kann mit 
den Pfeilen rechts ge„ndert werden. W„hlen Sie 
anschlieáend einen anderen Bereich in diesem 
Fenster aus.   

.*----------------------------------------------------------
:h2 res=1911.Tag
:i2 refid=1900.Tag
 
:p.In diesem Feld wird die aktuelle Tagesangabe fr die 
Alarmfunktion angezeigt. Diese Angabe kann mit den 
Pfeilen rechts ge„ndert werden. W„hlen Sie 
anschlieáend einen anderen Bereich in diesem 
Fenster aus. 
 
.*----------------------------------------------------------
:h2 res=1912.Jahr
:i2 refid=1900.Jahr
 
:p.In diesem Feld wird die aktuelle Jahresangabe fr 
die Alarmfunktion angezeigt. Diese Angabe kann mit 
den Pfeilen rechts ge„ndert werden. W„hlen Sie 
anschlieáend einen anderen Bereich in diesem 
Fenster aus.   

.*----------------------------------------------------------
:h2 res=1913.Einmal
:i2 refid=1900.Einmal
 
:p.Mit diesem Auswahlknopf wird festgelegt, daá ein eingestellter Alarm nur einmal
ausgefhrt werden soll. In diesem Fall wechselt die Darstellung dieser Dialogbox
so, daá ein Datum eingegeben werden kann.

.*----------------------------------------------------------
:h2 res=1914.Wochentag Mo...So
:i2 refid=1900.Wochentag MO...So
 
:p.Mit diesen Auswahlkn”pfen wird der Wochentag festgelegt, an dem der eingestellte Alarm
ausgefhrt werden soll.

.*----------------------------------------------------------
.*  Alarm Seite 2/3
.*----------------------------------------------------------
:h1 res=2100.Alarm Seite 2/3
:i1.Alarm Seite 2/3

:p.Auf dieser Seite kann ein Text vorgegeben werden, der zur Alarmzeit im Nachrichtenfenster
erscheint. Wird diese Seite leer gelassen, erscheint ein Standardtext.
Die L„nge des Textes ist auf maximal 128 Zeichen begrenzt (einschlieálich der Zeilenschaltung).

:p.Fr die Texteingabe sind austauschbare Parameter erlaubt. Dies sind&colon.
:dl compact tsize=4.
:dt.%t
:dd.Aktuelle Uhrzeit
:dt.%d
:dd.Aktuelles Datum
:dt.%z
:dd.Eingestellte Zeitzone
:edl.

:p.Eine genaue Erkl„rung fr jedes Feld wird angezeigt, 
wenn der entsprechende Begriff aus der 
untenstehenden Liste ausgew„hlt wird&colon.

:ul compact.
:li.:link reftype=hd res=10100.Widerrufen:elink.
:eul.
  
:p.Zugeh”rige Information&colon.

:p.:link reftype=hd res=10000.Anzeigen von Notizbuchseiten:elink.

.*----------------------------------------------------------
.*  Alarm Seite 3/3
.*----------------------------------------------------------
:h1 res=2300.Alarm Seite 3/3
:i1.Alarm Seite 3/3

:p.Mit der Auswahl :hp2.Pfad und Dateiname:ehp2. kann eine Programmdatei
festgelegt werden, die zu dem auf der Seite :link reftype=hd res=1900.Alarm Seite 1/3:elink.
angegebenen Zeitpunkt gestartet wird.

:p.Eine genaue Erkl„rung fr jedes Feld wird angezeigt, 
wenn der entsprechende Begriff aus der 
untenstehenden Liste ausgew„hlt wird&colon.

:ul compact.
:li.:link reftype=hd res=2301.Pfad und Dateiname:elink.
:li.:link reftype=hd res=267.Ausw„hlen...:elink.
:li.:link reftype=hd res=2302.Parameter:elink.
:li.:link reftype=hd res=2303.Arbeitsverzeichnis:elink.
:li.:link reftype=hd res=10100.Widerrufen:elink.
:eul.
  
:p.Zugeh”rige Information&colon.

:p.:link reftype=hd res=10000.Anzeigen von Notizbuchseiten:elink.

.*----------------------------------------------------------
:h2 res=2301.Pfad und Dateiname
:i2 refid=2300.Pfad und Dateiname
 
:p.Dieses Eingabefeld dient zur Eingabe von Pfad und Programmdateiname des Programmes, das
zum Alarmzeitpunkt gestartete werden soll. Die Angabe von DOS- oder Windows-Programmen
ist erlaubt.

.*----------------------------------------------------------
:h2 res=2302.Parameter
:i2 refid=2300.Parameter
 
:p.Dieses Eingabefeld legt den Aufrufparameter des Programmes fest, das unter
:link reftype=hd res=2301.Pfad und Dateiname:elink. angegeben wurde.
Die Eingabe dieses Parameters ist wahlfrei.

.*----------------------------------------------------------
:h2 res=2303.Arbeitsverzeichnis
:i2 refid=2300.Arbeitsverzeichnis

:p.Dieses Eingabefeld legt das Arbeitsverzeichnis des Programmes fest, das unter
:link reftype=hd res=2301.Pfad und Dateiname:elink. angegeben wurde.
Die Eingabe dieses Parameters ist wahlfrei.

.*----------------------------------------------------------
.*  Programmdatei ausw„hlen
.*----------------------------------------------------------
:h2 res=267.Programmdatei ausw„hlen
:i2 refid=2300.Programmdatei ausw„hlen

:p.Mit Hilfe dieser Dialogbox kann eine Programmdatei ausgew„hlt werden, die zum angegebenen Alarmzeitpunkt
gestartet wird, wenn der Auswahlknopf :link reftype=hd res=1906.Programm starten:elink.
gedrckt wurde. An dieser Stelle ist auch die Auswahl von DOS- und Windows-Programmen m”glich.
Der ausgew„hlte Dateiname erscheint im Feld :link reftype=hd res=2301.Pfad und Dateiname:elink..

.*----------------------------------------------------------
.*  DCF77-Einstellungen Seite 1/3
.*----------------------------------------------------------
:h1 res=1400.DCF77/GPS-Einstellungen Seite 1/3
:i1.DCF77/GPS-Einstellungen Seite 1/3

:p.Ist ein DCF77/GPS-Treiber installiert, kann auf der Notizbuchseite
:hp2.DCF77/GPS-Einstellungen Seite 1/3:ehp2. die vom Uhrenmodul verwendete
Schnittstelle ge„ndert werden. Folgende Schnittstellen werden
angeboten&colon.

:ul compact.
:li.:link reftype=hd res=1401.Serielle n:elink.
:li.:link reftype=hd res=1402.Spieleadapter (Standard):elink.
:li.:link reftype=hd res=1403.Seriell, Port:elink.
:li.:link reftype=hd res=1404.Spieleadapter:elink.
:eul.   

:p.Darber hinaus sind folgende, weitere Auswahlelemente verfgbar&colon.

:ul compact.
:li.:link reftype=hd res=10100.Widerrufen:elink.
:li.:link reftype=hd res=10101.Standard:elink.
:eul.

:p.Zugeh”rige Information&colon.

:p.:link reftype=hd res=10000.Anzeigen von Notizbuchseiten:elink.

.*----------------------------------------------------------
:h2 res=1401.Serielle n
:i2 refid=1400.Serielle n

:p.Mit diesen Radiokn”pfen kann eine der vier m”glichen seriellen Schnittstellen
des PC mit Standardadresse ausgew„hlt werden (COM1...COM4).

.*----------------------------------------------------------
:h2 res=1402.Spieleadapter (Standard)
:i2 refid=1400.Spieleadapter (Standard)

:p.Mit diesem Radioknopf kann ein Standard-Spieleadapter ausgew„hlt werden.

.*----------------------------------------------------------
:h2 res=1403.Seriell, Port
:i2 refid=1400.Seriell, Port

:p.Mit diesem Radioknopf kann eine serielle Schnittstelle ausgew„hlt werden, die nicht
auf einer Standard-I/O-Adresse liegt. Wird diese Einstellung ausgew„hlt, erscheint
abh„ngig vom installierten Treibertyp entweder eine Dialogbox, in der die gewnschte
:link reftype=hd res=2200.I/O-Adresse:elink. oder die
:link reftype=hd res=2500.COM-Port Nummer:elink.
eingegeben werden kann.

.*----------------------------------------------------------
:h2 res=1404.Spieleadapter
:i2 refid=1400.Spieleadapter

:p.Mit diesem Radioknopf kann ein Spiele- oder „hnliches Adapter ausgew„hlt werden, das
nicht auf der Standard-I/O-Adresse liegt. Wird diese Einstellung ausgew„hlt, erscheint eine
Dialogbox, in der die gewnschte :link reftype=hd res=2200.I/O-Adresse:elink.
eingegeben werden muá.

.*----------------------------------------------------------
.*  DCF77-I/O-Adresse
.*----------------------------------------------------------
:h1 res=2200.I/O-Adresse
:i1.I/O-Adresse

:p.In dieser Dialogbox kann die Portadresse (hexadezimal) eines seriellen oder Spieleadapters
eingegeben werden, das nicht dem Standard entspricht.
Es muá jedoch beachtet werden, daá solche Adapter nur dann mit dem Treiber zusammenarbeiten, wenn
sie ein im wesentlichen kompatibles Registerinterface besitzen.
Das Eingabefeld wird ben”tigt, wenn die Radiokn”pfe :link reftype=hd res=1403.Seriell, Port:elink.
bzw. :link reftype=hd res=1404.Spieleadapter:elink. ausgew„hlt wurden.

.*----------------------------------------------------------
.*  DCF77-COMx
.*----------------------------------------------------------
:h1 res=2500.COM-Port
:i1.COM-Port

:p.In dieser Dialogbox kann die Nummer der seriellen Schnittstelle (COMx) im Bereich von 
1 bis 9 eingegeben werden.
Dieses Eingabefeld wird ben”tigt, wenn der Radioknopf :link reftype=hd res=1403.Seriell, Nummer:elink.
ausgew„hlt wurde.

.*----------------------------------------------------------
.* DCF77-Einstellungen Seite 2/3
.*----------------------------------------------------------
:h1 res=1500.DCF77-Einstellungen Seite 2/3
:i1.DCF77-Einstellungen Seite 2/3

:p.Ist ein DCF77-Treiber installiert, k”nnen auf der Notizbuchseite
:hp2.DCF77-Einstellungen Seite 2/3:ehp2. verschiedene, vom Treiber
verwendete Intervallzeiten angegeben werden.
Es handelt sich hierbei um&colon.

:ul compact.
:li.:link reftype=hd res=1501.Updateintervall:elink.
:li.:link reftype=hd res=1502.Schwellwert Impulsl„nge:elink.
:li.:link reftype=hd res=1503.Wiederholprfung:elink.
:li.:link reftype=hd res=1504.Hochaufl”sender Zeitgeber:elink.
:li.:link reftype=hd res=10100.Widerrufen:elink.
:li.:link reftype=hd res=10101.Standard:elink.
:eul.

:p.Zugeh”rige Information&colon.

:p.:link reftype=hd res=10000.Anzeigen von Notizbuchseiten:elink.

.*----------------------------------------------------------
:h2 res=1501.Updateintervall
:i2 refid=1500.Updateintervall

:p.Mit diesem Schieberegler wird eingestellt, nach welcher Zeit frhestens die Uhr
im System bei einer Gangabweichung wieder nachgestellt werden soll.
Diese Zeit ist im Bereich von 1 Minute bis 1 Tag w„hlbar.

.*----------------------------------------------------------
:h2 res=1502.Schwellwert Impulsl„nge
:i2 refid=1500.Schwellwert Impulsl„nge

:p.Die digitale Zeitinformation wird ber den DCF77-Sender durch Impulse mit 2 verschiedenen
L„ngen bertragen, die den :hp2.0:ehp2. und :hp2.1:ehp2. Zustand kennzeichnen.
Die Unterscheidung, welchen Zustand das gerade empfangene Bit hat, wird durch Vergleich
mit der mit diesem Schieberegler einstellbaren Zeit getroffen.
Diese Zeit ist in Millisekunden angegeben.
Da die Zeit fr ein :hp2.0:ehp2.-Bit etwa 100ms, die fr ein :hp2.1:ehp2.-Bit etwa 200ms
betr„gt, ist eine Einstellung von 128ms-160ms fr die meisten Uhrenmodule normal.

:p.Der Treiber untersttzt 2 
:link reftype=hd res=1504.verschiedene Zeitgebertypen:elink.&colon.
Den Systemtimer von OS/2 mit seinem Takt von 32ms, sowie einen hochaufl”senden Zeitgeber, der 
auf eine Aufl”sung von 10ms eingestellt ist. Wird dieser hochaufl”sende Zeitgeber verwendet, 
bestimmt der Treiber die optimale Einstellung des Schiebereglers automatisch. 
Bei Benutzung des Systemtimers ist dies jedoch aufgrund der
zu geringen Aufl”sung nicht m”glich. In diesem Fall muá der Schwellwert von Hand eingestellt werden.
Wird ein Wert ausgew„hlt, der durch eine Beschriftung an der Tickmarke gekennzeichnet ist, ist
dies direkt ein Vielfaches des Systemtaktes.

:p.Es gibt jedoch Module bestimmter Hersteller, die die Impulse innerhalb ihrer elektronischen
Auswertung verl„ngern. In einem solchen Fall ist es notwendig, von diesem Standardwert abzuweichen.
Werden die Impulse unterschiedlich verl„ngert, so daá die Differenz zwischen :hp2.0:ehp2. und
:hp2.1:ehp2. geringer wird, kann es notwendig werden, bei Verwendung des Systemtimers
Bruchteile von Ticks einzustellen.

:note.Wenn es nicht unbedingt notwendig ist, sollte man davon Abstand nehmen, nicht ganzzahlige
Ticks einzustellen, da in einem solchen Fall zur Zeitmessung CPU-Zeit verloren geht.

.*----------------------------------------------------------
:h2 res=1503.Wiederholprfung
:i2 refid=1500.Wiederholprfung

:p.Der DCF77-Sender ist zur Erkennung von šbertragungsfehler aufgrund atmosph„rischer St”rungen
mit einer Parit„tssicherung ausgestattet.
Diese arbeitet jedoch nicht zuverl„ssig, da damit nur 1-Bit-Fehler erkannt werden k”nnen.
Werden 2 Bits falsch detektiert, kann dies der Treiber nicht aufgrund dieser Parit„tsprfung feststellen.

:p.Aus diesem Grund wurde eine weitere Sicherung vorgesehen. Dazu wird die aktuell empfangene
Zeit mit den letzten :hp1.N-1:ehp1. Zeiten verglichen. Weicht diese mehr als :hp1.N-1:ehp1. Minuten
von diesen Zeiten ab, wird sie verworfen.

:p.:hp2.Beispiel&colon.:ehp2. Wird hier eine :hp1.N=2:ehp1. eingestellt, darf die letzte empfangene Minute
maximal :hp1.N-1=1:ehp1. Minute von der aktuell empfangenen Zeit abweichen.

:note.Die Vorgabe eines Wertes fr die Wiederholprfung gr”áer als 1 fhrt dazu, daá
die Sommer-/Winterzeitumschaltung einige Minuten sp„ter erkannt wird.

.*----------------------------------------------------------
:h2 res=1504.Hochaufl”sender Zeitgeber
:i2 refid=1500.Hochaufl”sender Zeitgeber

:p.Der Ger„tetreiber fr externe DCF77-Module ist in der Lage, anstelle des Systemtaktes
von 32ms einen hochaufl”senden Zeitgeber zu verwenden. Dieser wird ab OS/2 Warp 4.0 standardm„áig
mit dem Betriebssystem ausgeliefert. Der Treiber heiá TIMER0.SYS und geh”rt zum Multimedia-Paket.
Der Treiber kann auch fr „ltere Betriebssystemversionen verwendet werden, vorausgesetzt der 
Treiber CLOCK0x.SYS wird ebenfalls ausgetauscht.

:p.Dieser hochaufl”sende Timer hat einige Vorteile&colon.
:ul compact.
:li.Auch bei gew„hlten Zeiteinheiten, die nicht ein Vielfaches des Systemtaktes sind, wird
keine CPU-Zeit verbraucht.
:li.Der Treiber ist bei Verwendung des hochaufl”senden Timers in der Lage, den optimalen
Schwellwert fr die Impulsl„nge automatisch zu bestimmen.
:eul.

:p.Da in der aktuellen Implementierung des hochaufl”senden Zeitgebers Probleme mit anderen
Zeitgeber-Treibern auftreten k”nnen (beispielsweise CPPOPA3.SYS von Visual Age C++), l„át
sich der Treiber auf den Systemtakt umstellen. Als Schwellwert wird dann der bei Verwendung
des hochaufl”senden Treibers automatisch bestimmte Schwellwert bernommen. Beim Umschalten
auf den Standard-Timer wird der Schieberegler mit einem Knopf und einer neuen Skala versehen.
Hierbei ist zu beachten, daá alle Einstellungswerte, die zwischen den beschrifteten Ticks liegen,
CPU-Zeit verbrauchen.

.*----------------------------------------------------------
:h1 res=1600.DCF77-Einstellungen Seite 3/3
:i1.DCF77-Einstellungen Seite 3/3

:p.Ist ein DCF77-Treiber installiert, k”nnen auf der Notizbuchseite
:hp2.DCF77-Einstellungen Seite 3/3:ehp2. die šbertragungsleitungen 
fr die Stromversorgung des Uhrenmoduls sowie dessen Signalleitung
ausgew„hlt werden.
Es gibt folgende Einstellm”glichkeiten&colon.


:ul compact.
:li.:link reftype=hd res=1601.Leitung 1 und 2; Aktiv:elink.
:li.:link reftype=hd res=1602.Steuerleitung:elink.
:li.:link reftype=hd res=1603.Seriell:elink.
:li.:link reftype=hd res=10100.Widerrufen:elink.
:li.:link reftype=hd res=10101.Standard:elink.
:eul.

:p.Zugeh”rige Information&colon.

:p.:link reftype=hd res=10000.Anzeigen von Notizbuchseiten:elink.

.*----------------------------------------------------------
:h2 res=1601.Leitung 1 und 2; Aktiv
:i2 refid=1600.Leitung 1 und 2; Aktiv

:p.Uhrenmodule fr serielle Schnittstellen werden dadurch mit Strom versorgt, daá 
auf der verwendeten Schnittstelle eine oder zwei Steuerleitungen auf ein definiertes
Potential gelegt werden (negative Spannung = :hp2.1:ehp2., positive Spannung = :hp2.0:ehp2.).
Es gibt verschiedene Typen solcher Module. Einige kommen mit einer Leitung aus und versorgen
sich mit einer unipolaren Spannung, andere ben”tigen zwei Leitungen fr eine bipolare Spannung.
Es gibt auch Typen, die zwei Leitungen auf demselben Potential ben”tigen, um einen h”heren
Strom entnehmen zu k”nnen.

:p.Werden die Uhrenmodule zusammen mit einer Maus auf derselben Schnittstelle verwendet,
darf die Versorgungsspannung nicht vom DCF77-Treiber angelegt werden, dies muá durch den 
Maustreiber geschehen.

:p.Diese drei F„lle k”nnen durch die Einstellungen mit Hilfe der Kn”pfe :hp1.Leitung 1/2:ehp1.
festgelegt werden. Der jeweilige Zustand am Ausgang der Leitung wird durch das Symbol im Knopf dargestellt.
Es bedeuten&colon.

:dl compact tsize=4.
:dt.:hp8.+:ehp8.
:dd.Positive Spannung
:dt.:hp8.-:ehp8.
:dd.Negative Spannung
:edl.

:p.Ist der Knopf :hp1.Aktiv:ehp1. nicht markiert, wird keine Ausgangsleitung beeinfluát.

.*----------------------------------------------------------
:h2 res=1602.Steuerleitung
:i2 refid=1600.Steuerleitung

:p.Der Signaleingang des Uhrenmoduls befindet sich auf einer Steuerleitung.
Wird eine serielle Schnittstelle verwendet, ist hier eine Angabe von 4 bis 7 sinnvoll,
fr nicht standardisierte I/O-Karten kann der gesamte Bereich von 0 bis 7 verwendet werden.

:p.Wird im Kombinationsfeld :hp1.Auto:ehp1. ausgew„hlt, wird die benutzte Steuerleitung
vom Treiber automatisch gesucht. Dieses Verfahren funktioniert mit den meisten Uhrenmodulen.
Probleme treten nur mit solchen Modulen auf, die zwei verschiedenen Signale auf unterschiedlichen
Signalleitungen ausgeben.

.*----------------------------------------------------------
:h2 res=1603.Seriell
:i2 refid=1600.Seriell

:p.Die Daten eines Uhrenmoduls am seriellen Eingang werden ber die RxD-Leitung bertragen.

.*----------------------------------------------------------
.*  DCF77-Status
.*----------------------------------------------------------
:h1 res=1700.DCF77/GPS-Status
:i1.DCF77/GPS-Status

:p.Ist ein DCF77/GPS-Treiber installiert, k”nnen Sie mit
:hp2.DCF77/GPS-Status:ehp2. den aktuellen Status des Empf„ngers abfragen.
Folgende Statusanzeigen sind verfgbar&colon.

:ul compact.
:li.:link reftype=hd res=1701.Polarit„t:elink.
:li.:link reftype=hd res=1702.Treiberversion:elink.
:li.:link reftype=hd res=1703.Tr„ger:elink.
:li.:link reftype=hd res=1704.Ersatzantenne:elink.
:li.:link reftype=hd res=1705.Stundensprung folgt:elink.
:li.:link reftype=hd res=1712.Zeit gltig:elink.
:li.:link reftype=hd res=1706.Schaltsekunde folgt:elink.
:li.:link reftype=hd res=1707.Zeitzonenbit 1:elink.
:li.:link reftype=hd res=1708.Zeitzonenbit 2:elink.
:li.:link reftype=hd res=1713.Auto-Scan l„uft:elink.
:li.:link reftype=hd res=1709.Richtig erkannt:elink.
:li.:link reftype=hd res=1710.Verworfen:elink.
:li.:link reftype=hd res=1711.Uhr gesetzt:elink.
:eul.   

:p.Zugeh”rige Information&colon.

:p.:link reftype=hd res=10000.Anzeigen von Notizbuchseiten:elink.

.*----------------------------------------------------------
:h2 res=1701.Polarit„t
:i2 refid=1700.Polarit„t

:p.Die Information des DCF77-Senders wird in Form kurzer Impulse im Sekundenabstand bertragen.
Dieses Textfeld gibt an, welche Polarit„t diese Impulse am Ausgang des Empf„ngermoduls besitzen.
Die Information betrifft nur den Treiber bei der Auswertung dieser Signale.
 
.*----------------------------------------------------------
:h2 res=1702.Treiberversion
:i2 refid=1700.Treiberversion

:p.Dieses Textfeld zeigt die Versionsnummer des installierten Treibers an.
Diese Systemuhr arbeitet nur korrekt mit einem Treiber ab Version 2.0.

.*----------------------------------------------------------
:h2 res=1703.Tr„ger
:i2 refid=1700.Tr„ger

:p.Ein ausgeflltes Markierungssymbol gibt an, daá das Uhrenmodul momentan einen
Tr„ger empf„ngt.

.*----------------------------------------------------------
:h2 res=1704.Ersatzantenne
:i2 refid=1700.Ersatzantenne

:p.Ein ausgeflltes Markierungssymbol gibt an, daá der DCF77-Sender gegenw„rtig mit
der Ersatzantenne bertr„gt.

.*----------------------------------------------------------
:h2 res=1705.Stundensprung folgt
:i2 refid=1700.Stundensprung folgt

:p.Ein ausgeflltes Markierungssymbol gibt an, daá mit der n„chsten Minute ein Stundensprung
erfolgt.

.*----------------------------------------------------------
:h2 res=1712.Zeit gltig
:i2 refid=1700.Zeit gltig

:p.Ein ausgeflltes Markierungssymbol gibt an, daá die zuletzt empfangene Minute 
aufgrund der Parit„ts- und Wiederholprfung als gltig befunden wurde.
 
.*----------------------------------------------------------
:h2 res=1706.Schaltsekunde folgt
:i2 refid=1700.Schaltsekunde folgt
 
:p.Ein ausgeflltes Markierungssymbol gibt an, daá die n„chste Sekunde eine Schaltsekunde ist.

.*----------------------------------------------------------
:h2 res=1707.Zeitzone 1
:i2 refid=1700.Zeitzone 1
 
:p.Ein ausgeflltes Markierungssymbol gibt an, daá die aktuelle Zeit zur Zeitzone 1 geh”rt
(Sommerzeit).

.*----------------------------------------------------------
:h2 res=1708.Zeitzone 2
:i2 refid=1700.Zeitzone 2
 
:p.Ein ausgeflltes Markierungssymbol gibt an, daá die aktuelle Zeit zur Zeitzone 2 geh”rt
(Winterzeit).

.*----------------------------------------------------------
:h2 res=1713.Auto-Scan l„uft
:i2 refid=1700.Auto-Scan l„uft
 
:p.Ein ausgeflltes Markierungssymbol gibt an, daá der Treiber gerade die Steuerleitung
sucht, auf der das Signal bertragen wird (siehe auch die Information zur Einstellseite
:link reftype=hd res=1602.Steuerleitung:elink.).
 
.*----------------------------------------------------------
:h2 res=1709.Richtig erkannt
:i2 refid=1700.Richtig erkannt
 
:p.Dieses Textfeld zeigt die Zahl der seit dem Systemstart korrekt empfangenen Minuten.

.*----------------------------------------------------------
:h2 res=1710.Verworfen
:i2 refid=1700.Verworfen

:p.Dieses Textfeld zeigt die Zahl der seit dem Systemstart empfangenen Minuten an, die nicht
erkannt werden konnten.
 
.*----------------------------------------------------------
:h2 res=1711.Uhr gesetzt
:i2 refid=1700.Uhr gesetzt
 
:p.Dieses Textfeld zeigt die Zahl der seit dem Systemstart vorgenommenen Setzvorg„nge der Systemuhr
(siehe dazu :link reftype=hd res=1503.Prfintervall:elink.).

.*----------------------------------------------------------
.*  Time C/S Server
.*----------------------------------------------------------
:h1 res=2400.TIME C/S Server
:i1.Time C/S Server

:p.Ist die Server-Komponente des LAN&colon.&colon.Time-Paketes installiert, erscheint
diese Seite im Einstellungsnotizbuch. Der Serverprozeá ist zust„ndig fr die šbertragung
der aktuellen Uhrzeit sowie des Empfangsstatus ber das lokale Netzwerk.
Diese šbertragung geschieht durch regelm„áige Rundsendenachrichten ("broadcasts"), deren
zeitlichen Abst„nde in dieser Einstellungsseite festgelegt werden k”nnen.

:p.Standard ist ein Intervall von 10 Minuten.

:p.Eine genaue Erkl„rung fr jedes Feld wird angezeigt, 
wenn der entsprechende Begriff aus der 
untenstehenden Liste ausgew„hlt wird&colon.

:ul compact.
:li.:link reftype=hd res=2401.šbertragungsintervall:elink.
:li.:link reftype=hd res=2402.Fehlerverhalten:elink.
:li.:link reftype=hd res=10100.Widerrufen:elink.
:li.:link reftype=hd res=10101.Standard:elink.
:eul.
  
:p.Zugeh”rige Information&colon.

:p.:link reftype=hd res=10000.Anzeigen von Notizbuchseiten:elink.

.*----------------------------------------------------------
:h2 res=2401.šbertragungsintervall
:i2 refid=2400.šbertragungsintervall

:p.Mit Hilfe dieses Schiebereglers wird das Intervall festgelegt, mit dem
die Uhrzeit sowie der Empfangsstatus ber das Netz bertragen wird.

:p.Die šbertragung erfolgt bei guten Empfangsbedingungen mit diesem Intervall.
Ist der Empfang l„ngere Zeit gest”rt, werden bei Erkennen einer gltigen Zeit
die Daten auch vor Ablauf dieses Intervalles bertragen.

.*----------------------------------------------------------
:h2 res=2402.Fehlerverhalten
:i2 refid=2400.Fehlerverhalten

:p.Ist dieser Knopf nicht markiert, wird bei schlechtem Empfang keine Zeit versandt.
Ein Client zeigt daher nur den Status des Servers an, seine lokale Uhrzeit wird nicht
korrigiert.

:p.In Umgebungen, in denen es auf hohe Synchronit„t aller Rechner im Netzwerk ankommt,
kann durch Drcken dieses Knopfes bewirkt werden, daá der Server im Falle einer ungltig
empfangenen Zeit seine lokale Uhrzeit verschickt.

.*----------------------------------------------------------
.*  Statistik-Panel
.*----------------------------------------------------------
:h1 res=2600.DCF77/GPS Statistik
:i1.DCF77/GPS Statistik

:p.Dieses Fenster erfllt zwei Zwecke&colon.
:ul compact.
:li.Die Darstellung einer Langzeitstatistik (max. ca. 20 Tage) ber die Empfangsqualit„t, sowie
:li.die Darstellung des Empfangssignals zum Einrichten der Antenne (dabei h„ngt die Darstellungsform
vom Typ des Empfangsmodules ab).
:eul.

:p.Im ersten Fall zeigt die Grafik die Zahl der erfolgreichen Empfangsereignisse an.
Je geringer die Zahl der Empfangsfehler, desto h”her die Gte. Eine zu schlechte Gte kann
viele Grnde haben. Beispiele sind&colon.
:ul compact.
:li.Die Antenne ist schlecht ausgerichtet. Im Idealfall zeigt die Achse der Ferritantenne
nach Frankfurt/Main (genauer Mainflingen).
:li.Die Wetterbedingungen sind schlecht. Vor allem Gewitterwetter fhrt oft zu schlechtem
Empfang.
:li.Der :link reftype=hd res=1502.Schwellwert fr die Impulsl„nge:elink.
ist falsch eingestellt. Es ist zu beachten, daá der Optimalwert fr diese Schwelle von der
Empfangsfeldst„rke abh„ngig ist.
:eul.

:p.Die Aufzeichnung dieser Grafik erfolgt nur, wenn eine der vier Sichten der Uhr offen ist
(Analog, Digital, Status oder die Grafik selbst). Die Skalierung der x-Achse paát sich an 
die Zahl der bereits aufgezeichneten Meápunkte an und reicht von 10 bis 4000 Stunden.

:p.Im Modus :hp1.Antenne einrichten:ehp1. wird das demodulierte AM-Signal des Empf„ngers 
dargestellt. Die Darstellung ist hierbei vom Empf„ngertyp abh„ngig.
:ul compact.
:li.:hp1.hopf 6036-Einsteckkarte&colon.:ehp1. Die dargestellte Zeitdauer betr„gt etwa 2s, 
so daá im ungnstigsten Fall immer noch mindestens 1 Impuls zu sehen ist. 
Die Antenne sollte so eingerichtet werden, daá der Abstand zwischen
der minimalen Amplitude (:hp1.der Impuls:ehp1.) und der maximalen Amplitude (:hp1.der Tr„ger:ehp1.)
maximal ist.
:li.:hp1.externes Empfangsmodul&colon.:ehp1. Die dargestellte Zeitdauer betr„gt 1s, es werden
insgesamt 10 Impulse bereinandergezeichnet ("Augenmuster"). Da der Low- und der High-Impuls
unterschiedliche L„ngen haben (nominal 100 bzw. 200 ms), bildet der Bereich zwischen diesen
Werten ein sogenanntes "Auge". Fr die Auswertung ist es optimal, wenn dieses Auge m”glichst
weit "offen" ist, das heiát, daá keine Ausreiáer zwischen diesen Werten liegen. Im Optimalfall
gibt es nur zwei Linien, die ihre fallende Flanke bei 100 bzw. 200 ms haben.
:eul.

.*----------------------------------------------------------
.*  Datum/Uhrzeit
.*----------------------------------------------------------
:h1 res=2800.Anzeigen Seite 3/3
:i1 id=2800.Anzeigen Seite 3/3

:p.Wenn in eines der Eingabefelder dieser Seite eine gltige Bitmap-Datei eingegeben wird,
wird in der analogen Ansicht der Uhr anstelle der Default-Darstellung die Bitmap-Datei wiedergegeben.
Bei den Bitmap-Dateien muá es sich um gltige Bitmap-Dateien (Endung "BMP") fr die
Betriebssysteme OS/2 oder Windows handeln. Fr die Darstellung ist folgendes zu beachten&colon.

:ul compact.
:li.Die Bitmapdatei fr das Ziffernblatt sollte quadratisch sein, um Verzerrungen zu vermeiden.
:li.Fr die Zeiger-Bitmaps kann eine Transparentfarbe verwendet werden, die auf der Einstellungsseite
:link reftype=hd res=1300.Anzeigen Seite 2/3:elink. eingestellt werden kann.
:li.Zweifarb-Bitmaps werden nicht schwarz/weiá dargestellt, sondern es wird anstelle von 
"weiá" die Farbe des Ziffernblattes verwendet (d. h. "weiá" ist die Transparentfarbe). Falls ein
Zeiger mit den Farben "schwarz" und "weiá" gewnscht wird, sollte man eine Bitmapdatei erzeugen,
das mehr Farben mit einer Palette besitzt (z. B. 16 oder 256 Palettenfarben), von denen eine
Farbe schwarz, eine weiá ist. Ein dritte Farbe kann dann als Transparentfarbe definiert werden
(rechteckige Zeiger drften wohl kaum erwnscht sein).
:li.Die Zeiger-Bitmaps besitzen einen "Hotspot"-Wert, der den Punkt der Zeigerachse definiert. Die
meisten Bitmap-Bearbeitungsprogramme schreiben an die Stelle dieses Wertes im Bitmap-Header
die Koordinaten (0/0). Zur Modifikation dieser Werte kann das mitgelieferte Programm
:hp2.HOTSPOT.EXE:ehp2. verwendet werden. Wird dieses Programm ohne Argumente aufgerufen,
gibt es auf der Konsole eine kurze Hilfe aus.
:eul.

:p.Eine genaue Erkl„rung fr jedes Feld wird angezeigt, 
wenn der entsprechende Begriff aus der 
untenstehenden Liste ausgew„hlt wird&colon.

:ul compact.
:li.:link reftype=hd res=2802.Stundenzeiger:elink.
:li.:link reftype=hd res=2801.Minutenzeiger:elink.
:li.:link reftype=hd res=2803.Ziffernblatt:elink.
:li.:link reftype=hd res=2804.Suchen:elink.
:eul.

:p.Zugeh”rige Information&colon.

:p.:link reftype=hd res=10000.Anzeigen von Notizbuchseiten:elink.

.*----------------------------------------------------------
:h2 res=2802.Stundenzeiger
:i2 refid=2800.Stundenzeiger

:p.In diesem Feld wird der komplette Pfad der Bitmapdatei
angegeben, die als Stundenzeiger verwendet werden soll.
Die Datei kann von Hand eingegeben, ber 
:link reftype=hd res=2804.Suchen:elink.
ausgew„hlt oder per Drag&amp.Drop auf das Eingabefeld
fallengelassen werden.

.*----------------------------------------------------------
:h2 res=2801.Minutenzeiger
:i2 refid=2800.Minutenzeiger

:p.In diesem Feld wird der komplette Pfad der Bitmapdatei
angegeben, die als Minutenzeiger verwendet werden soll.
Die Datei kann von Hand eingegeben, ber 
:link reftype=hd res=2804.Suchen:elink.
ausgew„hlt oder per Drap&amp.Drop auf das Eingabefeld
fallengelassen werden.

.*----------------------------------------------------------
:h2 res=2803.Ziffernblatt
:i2 refid=2800.Ziffernblatt

:p.In diesem Feld wird der komplette Pfad der Bitmapdatei
angegeben, die als Ziffernblatt verwendet werden soll.
Die Datei kann von Hand eingegeben, ber 
:link reftype=hd res=2804.Suchen:elink.
ausgew„hlt oder per Drag&amp.Drop auf das Eingabefeld
fallengelassen werden.

.*----------------------------------------------------------
:h2 res=2804.Suchen
:i2 refid=2800.Suchen

:p.Die Auswahl eines dieser Kn”pfe ”ffnet den Dateiauswahldialog,
mit dessen Hilfe eine Bitmapdatei auf der Festplatte selektiert
werden kann.

.*----------------------------------------------------------
.*  Stundengong
.*----------------------------------------------------------
:h1 res=2900.Stundengong
:i1 id=2900.Stundengong

:p.Auf dieser Seite werden die Einstellungen fr den Stundengong vorgenommen. Dies erfolgt durch 
Auswahl einer Wave-Datei, die entweder jeweils zu Beginn einer Stunde einmal abgespielt wird oder
aber 1 bis 12 mal entsprechend der Uhrzeit.

:p.Werden mehrere Uhren auf dem Rechner instanziert, kann in diesem Auswahldialog nur in 
einer Instanz eine Einstellung gemacht werden. Dadurch wird dafr gesorgt, daá nur genau 1 Uhr
den Stundengong spielt.

:p.Eine genaue Erkl„rung fr jedes Feld wird angezeigt, 
wenn der entsprechende Begriff aus der 
untenstehenden Liste ausgew„hlt wird&colon.

:ul compact.
:li.:link reftype=hd res=2901.Pfad und Dateiname:elink.
:li.:link reftype=hd res=2902.Ausw„hlen:elink.
:li.:link reftype=hd res=2903.1 mal pro Stunde:elink.
:li.:link reftype=hd res=2904.Anzahl entsprechend der Uhrzeit:elink.
:li.:link reftype=hd res=10100.Widerrufen:elink.
:li.:link reftype=hd res=10101.Standard:elink.
:eul.

:p.Zugeh”rige Information&colon.

:p.:link reftype=hd res=10000.Anzeigen von Notizbuchseiten:elink.

.*----------------------------------------------------------
:h2 res=2901.Pfad und Dateiname
:i2 refid=2900.Pfad und Dateiname

:p.In diesem Feld wird der komplette Pfad der Audiodatei
angegeben, die als Stundengong verwendet werden soll.
Die Datei kann von Hand eingegeben, ber 
:link reftype=hd res=2902.Ausw„hlen:elink.
ausgew„hlt oder per Drap&amp.Drop auf das Eingabefeld
fallengelassen werden.

.*----------------------------------------------------------
:h2 res=2902.Ausw„hlen
:i2 refid=&PANEL_SELECT..Ausw„hlen

:p.Die Auswahl eines dieser Kn”pfe ”ffnet den Dateiauswahldialog,
mit dessen Hilfe eine Bitmapdatei auf der Festplatte selektiert
werden kann.

.*----------------------------------------------------------
:h2 res=2903.1 mal pro Stunde
:i2 refid=&PANEL_SELECT..1 mal pro Stunde

:p.Wird dieser Knopf gedrckt, so wird die unter
:link reftype=hd res=2901.Pfad und Dateiname:elink.
ausgew„hlte Datei zu Beginn jeder Stunde genau einmal gespielt.

.*----------------------------------------------------------
:h2 res=2904.Anzahl entsprechend der Uhrzeit
:i2 refid=&PANEL_SELECT..Anzahl entsprechend der Uhrzeit

:p.Wird dieser Knopf gedrckt, so wird die unter
:link reftype=hd res=2901.Pfad und Dateiname:elink.
ausgew„hlte Datei zu Beginn jeder Stunde 1 bis 12 mal gespielt, entsprechend der 
jeweiligen Uhrzeit.


:euserdoc.

