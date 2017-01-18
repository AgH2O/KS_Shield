/*******************************************

  Name.......:  ks_gen
  Description:  Arduino sketch for the kolloidal silver generator shield
  Author.....:  Peter S.
  Version....:  0.5
  Date.......:  17.01.17 (Start 14.2.2016)
  Status.....:  OK
  Project....:  https://www.facebook.com/groups/kolloidalessilbergeraetetechnik/permalink/1328210467210327/
  Contact....:  https://www.facebook.com/peter.schmidt.52831
  License....:  This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
                To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/3.0/ or send a letter to
                Creative Commons, 171 Second Street, Suite 300, San Francisco, California, 94105, USA.

********************************************/
// PetS 9.11.16 New feature - Menü zur Wahl der Zeit des Polaritätswechsels von 0 bis 10 minuten
// PetS 9.11.16 New feature - Software Reset mittels assembler jump nach Addresse 0
// PetS 15.11.16 New feature - measure current
// PetS 28.11.16 Code clean to find problem with suddenly reset/ Software reset press 3 keys
// PetS 25.12.16 Test ppm-counter-system
// PetS 26.12.16 Interruft Routine
// PetS 30.12.16 TIMER_OFF / ON changedd, new function masse2ppm()
// PetS 02.01.16 lcd.print ppm, Q_gesamt & i reset after finish KSS
// PetS 17.01.17 lcd formatierter output fuer methode ppm
// PetS 17.01.17 remove unused code
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

#define TIMER_START TCCR1B |= (1 << CS12) | (0 << CS11) | (0 << CS10);// Bits setzen
#define TIMER_STOP  TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10));// Bits löschen


// Startwerte setzen
float liter = 0.4; // liter
float ppm = 50;    // ppm
float akt_ppm = 0; 
float strom = 5 ;  // mA
boolean polaritaet = true;

char text[32];

int kszeit;
unsigned long startzeit, anfangszeit, endzeit;
unsigned int taste, i, eine_minute, Position, adc_wert, adc_wert_a1;
unsigned int polwechselzeit = 2;
float spannung;
float strom_mess;
float shunt = 51; // 51 Ohm 0.1%

// neue Variable fuer ppm methode
float Q_gesamt = 0;
float Q_messung = 0;
float faktor = 0.001118083; // = M / z * F , 107,8782/1*96485
float zielmasse;
float masse;
unsigned int intervall = 1; // Ein Messzyklus in Sekunden
unsigned int messzeit = 1000;
long unsigned int sek = 0;
char stringbuf[16];
int stunde, minute, sekunde;

LiquidCrystal lcd(RS, E, D4, D5, D6, D7);

void setup() {
  //Serial.begin(9600);

  // Interrupt Routine Setup Timer1 1 Hz
  // TIMER 1 for interrupt frequency 1 Hz:
  cli(); // stop interrupts
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  TCNT1  = 0; // initialize counter value to 0
  // set compare match register for 1 Hz increments
  OCR1A = 62499; // = 16000000 / (256 * 1) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12, CS11 and CS10 bits for 256 prescaler
  TCCR1B |= (1 << CS12) | (0 << CS11) | (0 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  // disable timer compare interrupt
  // TIMSK1 |= (0 << OCIE1A);
  sei(); // allow interrupts
  TIMER_STOP

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
  lcd.print("Version 05");
  delay(2000);
  // Analoge Referenz auf 1.1V - bei ATMega168/328
  analogReference(INTERNAL);
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
  m = Masse des Stoffes
  M = molare Masse Ag 107.8682 g/mol
  i = Strom in MilliAmpere
  t = Zeit in Sekunden
  z = Ladungszahl Ag ist 1
  f = Faraday-konstante
*/
const float  M = 107.87;
const float  z = 1.0;
const float  f = 96485.0;
int zeit(float liter, float strom, float ppm)
{
  return (int) ((13400 * ((M / (strom * z * f)) * liter * ppm )) + 0.5);
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

// 4. Ausgabe
void print_methode(void) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Methode waehlen:");
  lcd.setCursor(0, 1);
  lcd.print("ppm        zeit");
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

void software_Reset() {
  // Restarts program from beginning but
  // does not reset the peripherals and registers

  asm volatile ("  jmp 0");
}

float errechne_zielmasse(float ppm, float wasser) {
  return (wasser * 1000 / 1000000) * ppm;
  // masse = (wassermenge * 1000 (gramm)/ 1000000) * ppm
}

float masse2ppm(float masse, float liter){
  return (masse /(liter * 1000 / 1000000));
  }
// PetS 17.01.17
void sek2hhmmss(long int zeit){
  if (zeit > 59){
    minute++;
    sek = 0;}
  if (minute > 59){
    stunde++;
    minute = 0;} 
  if (stunde > 9){
    stunde = 0;} // da einstellig 9 max. 
  }

ISR(TIMER1_COMPA_vect) {
  //Serial.print("Int: "); Serial.println(millis());
  
  // MESSUNG
  adc_wert_a1 = analogRead(1);
  strom_mess = (float) (adc_wert_a1 * 1.055) ;//Hier noch Spannung am 51 Ohm Shunt *1000
  strom_mess = (strom_mess / shunt) ;

  Q_messung =  strom_mess  * intervall;// I * t
  Q_gesamt = Q_gesamt + Q_messung;// Q aufaddiert
  masse = (Q_gesamt / 1000) * faktor; // Qgesamt von mC nach C
  sek2hhmmss(sek);
  akt_ppm = masse2ppm(masse, liter);
    // Ausgabe 1. LCD Zeile
  lcd.setCursor(0,0); 
  sprintf(stringbuf,"%01d:%02d:%02d      mA",stunde,minute,sek);
  lcd.print(stringbuf);
  lcd.setCursor(9,0); lcd.print(strom_mess);
    // Ausgabe 2. LCD Zeile
  ppm = masse2ppm(masse,liter);
  lcd.setCursor(0,1);
  lcd.print(ppm); lcd.setCursor(7,1); lcd.print("ppm");
  lcd.setCursor(11,1); lcd.print(liter);lcd.setCursor(15,1);lcd.print("l");
  
  if (!(i % (polwechselzeit * 60))) //Polaritätswechsel standard auf 2
  { polaritaet = !polaritaet;
    digitalWrite(POLW, polaritaet);
    if (polaritaet){
        lcd.setCursor(15,0); lcd.print("*");
    }else{
       lcd.setCursor(15,0); lcd.print(" "); 
    }
  }
  sek++;
  i++; // Intervall mal i ergibt Gesamtzeit
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
          liter = liter + 0.05 ; //geändert
        lcd.setCursor(9, 0);
        lcd.print(liter);
      }
      if (lese_tasten() == 2) {
        if (liter > 0.2)
          liter = liter - 0.05 ; //geändert
        lcd.setCursor(9, 0);
        lcd.print(liter);
      }
      delay(300);
    } while (lese_tasten() != 4);
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
        if (polwechselzeit < 10)
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
    lcd.print("Weiter");
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

  // Menüausgabe
  delay(1000);
  print_methode();
  do {
    taste = lese_tasten();
  } while ( taste != 1 && taste != 2 && taste != 4);// 1 = nach ppm , 4 = nach zeit

  // Auswahl Methode Standard oder ppm-count
  // switch case
  switch (taste)
  {
    case 1:
      { zielmasse = errechne_zielmasse(ppm, liter);
        digitalWrite(START, HIGH);
        TIMER_START
          do {
            delay(100); //0,1 sek
            if (lese_tasten() == 7)
              software_Reset();
          } while (masse <= zielmasse);
        digitalWrite(START, LOW);
        TIMER_STOP
        // Ton ausgeben
        for (i = 0; i < 5; i++)
        {
          tone(AUDIO, 1500);
          delay(500);
          noTone(AUDIO);
          delay(500);
        }
      }
      break;
    case 2:
    case 4:
      {
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
        anfangszeit = millis(); // zum errechnen der Gesamtzeit
        // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
        for (i = 0; i <= (kszeit - 1); i ++) {
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
          while (millis() - startzeit < eine_minute) {
            lcd.setCursor(0, 1);
            adc_wert = analogRead(0);
            adc_wert_a1 = analogRead(1);
            spannung = (float) adc_wert * 0.00107 ; // 0.0010742 (1,1V /1024) genaue Wert wichtig!!!
            strom_mess = (float) (adc_wert_a1 * 1.055) ;//Hier noch Spannung am 51 Ohm Shunt *1000
            strom_mess = (strom_mess / shunt) ;
            // Serial.println(strom_mess);
            lcd.print((int)(spannung * 67.76)); // Spannnungsteilerverhältnis 66,79 gemessen
            lcd.print(" V "); // ein Leerzeichen hinten dran, wegen einstellige V.
            lcd.setCursor(8, 1);
            lcd.print(strom_mess);
            lcd.print(" mA");
            if (lese_tasten() == 7)
              software_Reset();
          }
        }
        digitalWrite(START, LOW); // Fertig !
        endzeit = millis(); // zum errechnen der Gesamtzeit

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
      }
      break;
  }
  //Warteschleife bis irgend eine Taste
  do {
    taste = lese_tasten();
  } while (taste != 4 && taste != 2 && taste != 1);
  Q_gesamt=0; i=0; sek=0; stunde=0; minute=0;// Werte zurücksetzen
  delay(1000);
}


