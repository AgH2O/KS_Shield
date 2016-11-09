/*******************************************
 *
 * Name.......:  ks_gen
 * Description:  Arduino sketch for the kolloidal silver generator shield 
 * Author.....:  Peter Schmidt
 * Version....:  0.3
 * Date.......:  04.11.16 (Start 14.2.2016)
 * Status.....:  beta version/ works, voltage meter inaccurate  
 * Project....:  https://www.facebook.com/groups/kolloidalessilbergeraetetechnik/permalink/1328210467210327/
 * Contact....:  https://www.facebook.com/peter.schmidt.52831
 * License....:  This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
 *               To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/3.0/ or send a letter to
 *               Creative Commons, 171 Second Street, Suite 300, San Francisco, California, 94105, USA.
 *
 ********************************************/

#include <LiquidCrystal.h>

//LCD Size 16x2
#define COLS 16
#define ROWS 2

//LCD PINs
#define RS 13
#define E  12
#define D4 11
#define D5 10
#define D6 9
#define D7 8

// Schalter def.
#define SWL 7
#define SWM 6
#define SWR 5

// Steuerleitungen def.
#define START 4
#define AUDIO 3
#define POLW  2


// Startwerte setzen
float liter = 0.4; // liter
float ppm = 50;    // ppm
float strom = 5 ;  // mA
boolean polaritaet = true;

int kszeit;
unsigned long startzeit;
unsigned int taste, i, eine_minute, Position, adc_wert;
unsigned int polwechselzeit = 2;
float spannung;

LiquidCrystal lcd(RS, E, D4, D5, D6, D7);

void setup() {

  // Setup Schalter + PullUp
  pinMode(SWL, INPUT);
  digitalWrite(SWL, HIGH);
  pinMode(SWM, INPUT);
  digitalWrite(SWM, HIGH);
  pinMode(SWR, INPUT);
  digitalWrite(SWR, HIGH);
  
  // Setup Steuerleitungen
  pinMode(START, OUTPUT);
  digitalWrite(START, LOW); // StartwertLow
  //pinMode(AUDIO,OUTPUT);
  pinMode(POLW, OUTPUT);
  digitalWrite(POLW, LOW); // StartwertLow

  // Setup LCD
  lcd.begin(COLS, ROWS);
  lcd.print("Silver 7 PRO");
  lcd.setCursor(0, 1);
  lcd.print("Version 03");
  delay(2000);
}

// 3 Tasten einlesen und return 1 2 oder 4
uint8_t lese_tasten(void) {
  uint8_t zwi_speich = 0;
  if (digitalRead(SWL) == LOW)
    zwi_speich = 1;
  if (digitalRead(SWM) == LOW)
    zwi_speich = zwi_speich + 2;
  if (digitalRead(SWR) == LOW)
    zwi_speich = zwi_speich + 4;
  return zwi_speich;
}
/*
Funktion zum ausrechnen der Zeit
Formel dazu: m = M * I * t / (z * F)
m = molare Masse Ag 107.8682 g/mol
i = Strom in MilliAmpere
t = Zeit in Sekunden
z = Ladungszahl Ag ist 1
f = Faraday-konstante
*/
const float  m = 107.87;
const float  z = 1.0;
const float  f = 96485.0;
int zeit(float liter, float strom, float ppm)
{
  return (int) ((13400 * ((m / (strom * z * f)) * liter * ppm )) + 0.5);
  // plus 0.5 zum runden !(int) schneidet ab
}

// 1. Ausgabe
void print_wassermenge(float liter) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Wasser:     l");
  lcd.setCursor(0, 1);
  lcd.print("Auf   Ab  Weiter");
  lcd.setCursor(9, 0);
  lcd.print(liter);
}

// 2. Ausgabe
void print_ppm(float ppm) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PPM:  ");
  lcd.setCursor(0, 1);
  lcd.print("Auf   Ab  Weiter");
  lcd.setCursor(6, 0);
  lcd.print((int)ppm);
}

// 3. Ausgabe
void print_polw(unsigned int polwechselzeit) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Umpolzeit:   Min");
  lcd.setCursor(0, 1);
  lcd.print("Auf   Ab  Weiter");
  lcd.setCursor(11, 0);
  lcd.print(polwechselzeit);
}
void erste_zeile_clean() {
  lcd.home();
  lcd.print("                ");
}

void zweiSekunden(void) {
  for (i = 0; i < 100; i ++)
    if ((lese_tasten() == 4) || (lese_tasten() == 1) )
      break;
    else
      delay(20);
}
// *** M A I N L O O P ***
void loop() {
  do { //zureck Schleife
    // Wassermenge waehlen
    liter = 0.4; // ruecksetzen auf Standart f. neuen Durchlauf
    print_wassermenge(liter);
    do {
      if (lese_tasten() == 1) {
        if (liter < 1)
          liter = liter + 0.1 ;
        lcd.setCursor(9, 0);
        lcd.print(liter);
      }
      if (lese_tasten() == 2) {
        if (liter > 0.2)
          liter = liter - 0.1 ;
        lcd.setCursor(9, 0);
        lcd.print(liter);
      }
      delay(300);
    } while (lese_tasten() != 4);
    //delay(1000);
    //PPM waehlen
    ppm = 50;
    print_ppm(ppm);
    do {
      if (lese_tasten() == 1) {
        if (ppm < 200)
          ppm = ppm + 5 ;
        lcd.setCursor(6, 0);
        lcd.print ("   ");// Korrektur 100derter Stelle
        lcd.setCursor(6, 0);
        lcd.print((int)ppm);
      }
      if (lese_tasten() == 2) {
        if (ppm > 11)
          ppm = ppm - 5 ;
        lcd.setCursor(6, 0);
        lcd.print ("   ");// Korrektur 100derter Stelle
        lcd.setCursor(6, 0);
        lcd.print((int) ppm);
      }
      delay(300);
    } while (lese_tasten() != 4);

    print_polw(polwechselzeit);
    do {
      if (lese_tasten() == 1) {
        if (polwechselzeit < 6)
          polwechselzeit = polwechselzeit + 1 ;
        lcd.setCursor(11, 0);
        lcd.print(polwechselzeit);
      }
      if (lese_tasten() == 2) {
        if (polwechselzeit)
          polwechselzeit = polwechselzeit - 1 ;
        lcd.setCursor(11, 0);
        lcd.print(polwechselzeit);
      }
      delay(300);
    } while (lese_tasten() != 4);
    
    //delay(1000);

    // benoetigte Zeit ausrechnen
    kszeit = zeit(liter, strom, ppm);
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("zurueck");
    lcd.setCursor(10, 1);
    lcd.print("START");
    // B e n u t z e r   letzter check
    delay(1000);
    do {
      lcd.setCursor(0, 0);
      lcd.print((int)strom);
      lcd.setCursor(3, 0);
      lcd.print("mA");
      lcd.setCursor(9, 0);
      lcd.print((int)ppm);
      lcd.setCursor(13, 0);
      lcd.print("ppm");
      //erste 2 Werte -Pause- zweite 2 Werte in diegleiche Zeile
      taste = lese_tasten();
      zweiSekunden();
      erste_zeile_clean();

      lcd.setCursor(0, 0);
      lcd.print(kszeit);
      lcd.setCursor(3, 0);
      lcd.print("Min.");
      lcd.setCursor(9, 0);
      lcd.print(liter);
      lcd.setCursor(14, 0);
      lcd.print("l");
      taste = lese_tasten();
      zweiSekunden();
      erste_zeile_clean();
    } while (taste != 4 && taste != 2 && taste != 1); //Reagiere auf Tastendruck
  } while ( taste != 4 && taste != 2); //nur ++NICHT++ zurueck, wenn 4 == weiter
  // Taste 2 Mitte ist der TEST-Schnelldurchlauf
  if (taste == 2) {
    eine_minute = 1000;
    tone(AUDIO, 2000);
    delay(500);
    noTone(AUDIO);
  } else {
    eine_minute = 60000;
  }
  // *** KS Prozess beginnt ***
  lcd.clear();
  // Start KS
  lcd.clear();
  digitalWrite(START, HIGH); //Relais einschalten!

  for (i = 0; i <= (kszeit - 1); i ++){
    lcd.setCursor(0, 0);
    lcd.print("noch     Min.");
    if ((kszeit - i) < 10) // Korrektur, weil beim Uebergang von
      Position = 6;      // Zehner auf EIner eine Null stehen bleibt
    else
      Position = 5;
    lcd.setCursor(Position, 0);
    lcd.print(kszeit - i);

    if (!(i % polwechselzeit)) //Polaritätswechsel standard auf 2
    { polaritaet = !polaritaet;
      digitalWrite(POLW, polaritaet);
    }
    if (polaritaet) {
      lcd.setCursor(14, 0);
      lcd.print("+");
    }
    else {
      lcd.setCursor(14, 0);
      lcd.print("-");
    }
    // Spannung messen und Ausgabe 2.Zeile sowie eine Minute und weiter
    startzeit = millis();
    while(millis() - startzeit < eine_minute){
      lcd.setCursor(0, 1);
      adc_wert = analogRead(0);
      spannung = (float) adc_wert * 0.0049 ; // 0.0049 (5V /1024)
      lcd.print((int)(spannung * 13));
      lcd.print(" Volt");
    }
    //delay(60000); // 1 eine_minute == 60000
    //delay(eine_minute);
  }
  digitalWrite(START, LOW); // Fertig !

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("KS fertig ");
  lcd.setCursor(10, 0);
  lcd.print(kszeit);
  lcd.setCursor(13, 0);
  lcd.print("Min");

  lcd.setCursor(0, 1);
  lcd.print((int)ppm);
  lcd.print(" PPM  ");
  lcd.print(liter);
  lcd.print(" l");
  // Ton ausgeben
  for (i = 0; i < 5; i++)
  {
    tone(AUDIO, 1500);
    delay(500);
    noTone(AUDIO);
    delay(500);
  }
  //Warteschleife bis irgend eine Taste
  do {
    taste = lese_tasten();
  } while (taste != 4 && taste != 2 && taste != 1);

  delay(1000);
}


