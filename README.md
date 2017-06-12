p### NEWS
<p>14.11.16 - Das KS-GEN-Shield-V1-final_rev02.lay6 Platinenlayout wurde minimal an der Brücke zum T1 überarbeitet. TNX Thor!</p>
<p>18.01.17 - Neue Version mit Methodenauswahl PPM oder Zeit / ppm-counter mittels Q = I * t. 
Neues Layout KS-GEN-Shield-V1-final_shunt.lay6 mit shunt zur Strommessung. TNX Thor!</p>
<p>16.05.17 Hard u. Software patch 1 - Verbesserung der Schaltung. K2, D2, T1 werden durch einen BC327-25 ersetzt. Ausserdem ist ein 100nF Kondensator am AREF nach Masse dazu gekommen. Die Messwerte sind dadurch ruhiger und störungsfreier. Die Logik des Einschaltens muss invertiert werden digitalWrite(START, HIGH); -->ist jetzt STOP und umgekehrt ! Der patch muss nicht zwingend angewendet werden. Danke an Markus W.</p>
<p>12.06.2017 ACHTUNG FEHLER in der Bestelliste: die 1,6kOhm sind keine 0.1% sondern normale Metalfilm wie die anderen Widerstände. Fehler tut der SHUNT Widerstand 51 Ohm und der sollte 0.1% haben und die Zenerdiode 4,7Volt parallel zum Shunt(dient nur als Überspannungsschutz für den ADC Eingang)  Danke an Toby</p>

## **KS-GEN-SHIELD**
Kolloidal Silber Generator Arduino Uno shield</br>
##############################################

Die Informationen zu diesem Projekt sind mit größter Sorgfalt erarbeitet.Dennoch können Fehler nicht vollständig ausgeschlossen werden. Die Authoren/Entwickler übernehmen keinerlei juristische Verantwortung oder Haftung für eventuell verbliebende Fehler und deren Folgen.

Das KS-Shield ist aus dem Selbstbauprojekt 7 von Hans-Dieter Teuteberg entstanden.
http://www.kolloidal-silber.de/koll_silber_selbstbauprojekt_7_III_4_baustufe.htm
Das Layout der Platine sowie die Front und Seitenbeschriftung stammt von Thor Larsson Lundberg (Nickname). Beiden sei gedankt!

Die Erweiterungen umfassen einen Timer mit automatischer Ein-/Aus-schaltung, wobei die Wassermenge und die gewünschte ppm Stärke frei gewählt werden kann. Die daraus resultierende Zeit, wird nach der Faradayschen Formel  m = M * I * t / (z * F) ausgerechnet. Es handelt sich um ein rein privates und nicht kommerzielles Projekt. Der Nachbau erfolgt auf eigene Gefahr.

Die beiden Platinenlayouts sind im Abacom Sprint-Layoutformat. Den Viewer zum ausdrucken gibt es hier:
http://www.abacom-online.de/updates/Sprint-Layout60_Viewer.exe
 
Bitte unbedingt beachten:  
1.) Kurzschlüsse oder sonstige defekte am Shield, können den USB-Port zerstören und damit auch ihren PC oder Laptop. Deshalb empfehle ich, den Arduino zunächst ++OHNE++ Shield zu programmieren und danach das Gerät nur noch mit einem externen Netzteil (9V Steckernetzteil direkt am Arduino) mit dem aufgestecktem Shield zu betreiben. 

2.) Das Shield kann leicht falsch aufgesteckt werden, einen oder mehrere Pins nach oben oder unten, was zur Zerstörung des Arduinos und des Shields führen kann. Beachten sie daher genauestens, wie das Shield aufgesteckt wird.

3.) Beim ersten Einschalten kann es sein, das die beleuchtete LCD-Anzeigen nichts anzeigt, aber trotzdem alles funktioniert. Drehen sie am Spindelpoti für den Kontrast, bis die Zeichen erkennbar werden.

4.) Dies ist kein Elektronik-Einsteiger Projekt !!

Bekannte Probleme (Stand 18.01.2017):
- <s>Die Spannungsmessung ist ungenau, kann aber durch nachmessen der 5V Betriebsspannung (4,8 - 5,2V) und eintragen des realen Multiplizierwertes in das Sketch ( die 0,0049V setzen sich aus der gemessenen Spannung/ Referenzspannung geteilt durch 1024 zusammen) verbessert werden.</s>
- Zur Strommessung muss die Variable shunt auf den Wert des genommenen Wertes gesetzt werden (50 oder 51 Ohm)
- Zur Stabilisierung der Messung wurde der ADC auf INTERN Referenz 1.1V gesetzt, was unabhängig von der Versorgungsspannung macht. Deshalb sind auch die Spannungsteilerwiderstände verändert worden.
