# ks_shield
Kolloidal Silber Generator Arduino Uno shield
#############################################

Die Informationen zu diesem Projekt sind mit größter Sorgfalt erarbeitet.Dennoch können Fehler nicht vollständig ausgeschlossen werden. Die Authoren/Entwickler übernehmen keinerlei juristische Verantwortung oder Haftung für eventuell verbliebende Fehler und deren Folgen.

Bitte unbedingt beachten:  
1.) Kurzschlüsse oder sonstige defekte am Shield, können den USB-Port zerstören und damit auch ihren PC oder Laptop. Deshalb empfehle ich, den Arduino zunächst ++OHNE++ Shield zu programmieren und danach das Gerät nur noch mit einem externen Netzteil (9V Steckernetzteil direkt am Arduino) mit dem aufgestecktem Shield zu betreiben. 

2.) Das Shield kann leicht falsch aufgesteckt werden, einen oder mehrere Pins nach oben oder unten, was zur Zerstörung des Arduinos und des Shields führen kann. Beachten sie daher genauestens, wie das Shield aufgesteckt wird.

3.) Beim ersten Einschalten kann es sein, das die beleuchtete LCD-Anzeigen nichts anzeigt, aber trotzdem alles funktioniert. Drehen sie am Spindelpoti für den Kontrast, bis die Zeichen erkennbar werden.

4.) Dies ist kein Elektronik-Einsteiger Projekt !!!

Bekannte Probleme (Stand 6.11.2016):
- Die Brücke auf der Bestückungsseite, am T1, ist sehr nahe am Transistor. Hier bitte ein Stück Isolierschlauch benutzen.
- Die Spannungsmessung ist ungenau, kann aber durch nachmessen der 5V Betriebsspannung (4,8 - 5,2V) und eintragen des realen Multiplizierwertes in das Sketch ( die 0,0049V setzen sich aus der gemessenen Spannung/ Referenzspannung geteilt durch 1024 zusammen) verbessert werden.
