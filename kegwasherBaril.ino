////////////////////////////////
//     BRASSERIE DU BARIL     //
//            BREST           //
//  MACHINE A LAVER LES FUTS  //
////////////////////////////////
/*
Ce programme est conçu pour piloter une machine à la ver les fûts de bière DIY.
Elle fait fonctionner :
- 9 relais activant 9 électrovannes (sur les broches numériques 2 à 10).
- 1 buzzer sur la broche A0
- 1 LED d'état sur la broche 11
- 1 encodeur rotatif pour sélevtionner les programmes (broches A1 et A2)
- 1 bouton poussoir pour lancer les action en A3

                               BROCHAGE
                                          +-----+
                      +----[PWR]-------------------| USB |--+
                      |                            +-----+  |
                      |         GND/RST2  [ ][ ]            |
                      |       MOSI2/SCK2  [ ][ ]  A5/SCL[ ] | 
                      |          5V/MISO2 [ ][ ]  A4/SDA[ ] |   
                      |                             AREF[ ] |
                      |                              GND[ ] |
                      | [ ]N/C                    SCK/13[ ] |  
                      | [ ]IOREF                 MISO/12[ ] |   
                      | [ ]RST                   MOSI/11[X]~| - LED   
                      | [ ]3V3    +---+               10[X]~| - Relais Pompe
                      | [X]5v    -| A |-               9[X]~| - Relais Valve vidange
                      | [X]GND   -| R |-               8[X] | - Relais sortie de l'acide
                      | [X]GND   -| D |-                    |
                      | [ ]Vin   -| U |-               7[X] | - Relais sortie de la soude
                      |          -| I |-               6[X]~| - Relais entrée acide
             Buzzer - | [X]A0    -| N |-               5[X]~| - Relais entrée soude
Encodeur Rotatif haut | [X]A1    -| O |-               4[X] | - Relais eau
Encodeur Rotatif bas  | [X]A2     +---+           INT1/3[X]~| - Relais CO2
      Bouton Action - | [X]A3                     INT0/2[X] | - Relais air
      Ecran LCD I2C - | [X]A4/SDA  RST SCK MISO     TX>1[ ] |  
      Ecran LCD I2C - | [X]A5/SCL  [ ] [ ] [ ]      RX<0[ ] |  
                      |            [ ] [ ] [ ]              |
                      |  UNO_R3    GND MOSI 5V  ____________/
                       \_______________________/


Inspiré du projet https://github.com/vieuxsinge/kegwasher

Un programme réalisé par Antony Le Goïc-Auffret sous licence creative commons CC-By-Sa
 */
//------------------------------------------------------
#include <Wire.h>

// bibliothèque anti-rebond pour les boutons
#include <Bounce2.h>

Bounce boutonHaut = Bounce(); // création d'objet anti-renond
Bounce boutonBas = Bounce(); 
Bounce boutonAction = Bounce(); 

// Utilisation d'un ecran LCD I2C de chez grove
#include "rgb_lcd.h"

rgb_lcd lcd;

// On allume l'écran en bleu
const int colorR = 0;
const int colorG = 0;
const int colorB = 255;

//attribution des broches
#define PIN_BUZZER                A0 // Pour les alerte sonores
#define PIN_BOUTON_HAUT           A1 // sélection
#define PIN_BOUTON_BAS            A2 // sélection
#define PIN_BOUTON_ACTION         A3 // lancement des séquences
#define PIN_VALVE_AIR             2  // injecte de l’air comprimé pour purger les futs du reste de bière pouvant rester dedans ou bien pour vider la soude ou l’eau de rinçage (non stérile) 
#define PIN_VALVE_CO2             3  // même chose que pour l’air comprimé sauf que le CO2 est stérile et il permet de vider les futs de la solution de désinfection (ACIDE) et de les pressuriser
#define PIN_VALVE_EAU             4  // Rinçage à l’eau claire des futs
#define PIN_VALVE_SOUDE_IN        5  // envoi d’une solution de soude pour le nettoyage des futs
#define PIN_VALVE_ACIDE_IN        6  // envoi d’une solution acide pour la désinfection des futs
#define PIN_VALVE_SOUDE_OUT       7  // retour de la soude pour faire circuler la solution en circuit fermé
#define PIN_VALVE_ACIDE_OUT       8  // retour de l’acide pour faire circuler en circuit fermé
#define PIN_VALVE_VIDANGE         9  // mise à l’égout du reste de bière contenu dans les futs
#define PIN_POMPE                 10 // mise en route d’une pompe (220v pour faire circuler en circuit fermé les solutions de soude et d’acide) 
#define PIN_LED                   11 // signaux lumineux

void setup()
{
  Serial.begin(9600);
  //initialisation des broches
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_BOUTON_HAUT, INPUT_PULLUP);
  pinMode(PIN_BOUTON_BAS, INPUT_PULLUP);
  pinMode(PIN_BOUTON_ACTION, INPUT_PULLUP);
  pinMode(PIN_VALVE_AIR, OUTPUT);
  pinMode(PIN_VALVE_CO2, OUTPUT);
  pinMode(PIN_VALVE_EAU, OUTPUT);
  pinMode(PIN_VALVE_SOUDE_IN, OUTPUT);
  pinMode(PIN_VALVE_ACIDE_IN, OUTPUT);
  pinMode(PIN_VALVE_SOUDE_OUT, OUTPUT);
  pinMode(PIN_VALVE_ACIDE_OUT, OUTPUT);
  pinMode(PIN_VALVE_VIDANGE, OUTPUT);
  pinMode(PIN_POMPE, OUTPUT);
  pinMode(PIN_LED, OUTPUT);

  digitalWrite(PIN_LED, LOW);    // LED éteinte
  digitalWrite(PIN_BUZZER, LOW); // Buzzer silencieux

  boutonHaut.attach(PIN_BOUTON_HAUT); // les anti-rebons soint attachgés à des broches en mode "INPUT_PULLUP"
  boutonBas.attach(PIN_BOUTON_BAS);
  boutonAction.attach(PIN_BOUTON_ACTION);

  boutonHaut.interval(10); // l'interval anti-rebond est fixé à 10 milisecondes
  boutonBas.interval(10);
  boutonAction.interval(10);

  // intialisation le nombre de colones et lignes de l'écran LCD
  lcd.begin(16, 2);  
  lcd.setRGB(colorR, colorG, colorB); // on éclaire l'écran
}


void loop () {
  lcd.setRGB(colorR, colorG, colorB); // on éclaire l'écran en bleu
  lcd.setCursor(0, 0);
  lcd.print("Brasserie");
  lcd.setCursor(0, 1);
  lcd.print("du Baril");
  
  boutonAction.update(); // mise à jour de l'état de l'antibond "Action".
   
  if ( boutonAction.fell() ) {  // l'action est déclenchée si le bouton passe de l'état HIGH à LOW
    buzzerDebutCycle(); // On sonne
    scenario();         // Le scenario tourne
    buzzerFinCycle();   // On sonne pour signaler la fin de cycle.
   }
  // Cette version du programme ne permet de sélectionner différents programmes ou de créer et enregistrer des séquences originales
  // nous n'utilisons donc pas le bouton de sélection PIN_BOUTON_HAUT et PIN_BOUTON_BAS
  Serial.println( "Attente..." );
}

void scenario() {
  Serial.println( "SCENARIO LANCE..." );
  lcd.setRGB(colorB, colorG, colorR); // on éclaire l'écran en rouge
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Etape de lavage:");
  lcd.setCursor(0, 1);
  lcd.print("vidange         ");
  vidange ();
  lcd.setCursor(0, 1);
  lcd.print("rincage         ");  
  rincage();
  lcd.setCursor(0, 1);
  lcd.print("purge Air       ");
  purgeAir();
  lcd.setCursor(0, 1);
  lcd.print("Soude           ");
  soude();
  lcd.setCursor(0, 1);
  lcd.print("vidange soude   ");
  vidangeSoude();
  lcd.setCursor(0, 1);
  lcd.print("rincage         ");
  rincage();
  lcd.setCursor(0, 1);
  lcd.print("acide           ");
  acide();
  lcd.setCursor(0, 1);
  lcd.print("vidange acide   ");
  vidangeAcide();
  lcd.setCursor(0, 1);
  lcd.print("pression CO2    ");
  pressurisationCO2();
  lcd.clear();
}

void buzzerDebutCycle() {
  //le buzzer sonne 2 coups brefs
  digitalWrite(PIN_BUZZER, HIGH);
  delay(150);
  digitalWrite(PIN_BUZZER, LOW);
  delay(100);
  digitalWrite(PIN_BUZZER, HIGH);
  delay(150);
  digitalWrite(PIN_BUZZER, LOW);
}

void buzzerFinCycle() {
  //le buzzer sonne 3 coups pendant un quart de seconde
  digitalWrite(PIN_BUZZER, HIGH);
  delay(250);
  digitalWrite(PIN_BUZZER, LOW);
  delay(250);
  digitalWrite(PIN_BUZZER, HIGH);
  delay(250);
  digitalWrite(PIN_BUZZER, LOW);
  delay(250);
  digitalWrite(PIN_BUZZER, HIGH);
  delay(250);
  digitalWrite(PIN_BUZZER, LOW);
}
void vidange () {
  digitalWrite(PIN_VALVE_AIR, LOW);
  digitalWrite(PIN_VALVE_CO2, HIGH);
  digitalWrite(PIN_VALVE_EAU, HIGH);
  digitalWrite(PIN_VALVE_SOUDE_IN, HIGH);
  digitalWrite(PIN_VALVE_ACIDE_IN, HIGH);
  digitalWrite(PIN_VALVE_SOUDE_OUT, HIGH);
  digitalWrite(PIN_VALVE_ACIDE_OUT, HIGH);
  digitalWrite(PIN_VALVE_VIDANGE, LOW);
  digitalWrite(PIN_POMPE, HIGH);
  delay(10000);
}

void rincage () {
  digitalWrite(PIN_VALVE_AIR, HIGH);
  digitalWrite(PIN_VALVE_CO2, HIGH);
  digitalWrite(PIN_VALVE_EAU, LOW);
  digitalWrite(PIN_VALVE_SOUDE_IN, HIGH);
  digitalWrite(PIN_VALVE_ACIDE_IN, HIGH);
  digitalWrite(PIN_VALVE_SOUDE_OUT, HIGH);
  digitalWrite(PIN_VALVE_ACIDE_OUT, HIGH);
  digitalWrite(PIN_VALVE_VIDANGE, LOW);
  digitalWrite(PIN_POMPE, HIGH);
  delay(20000);
}
void purgeAir () {
  digitalWrite(PIN_VALVE_AIR, LOW);
  digitalWrite(PIN_VALVE_CO2, HIGH);
  digitalWrite(PIN_VALVE_EAU, HIGH);
  digitalWrite(PIN_VALVE_SOUDE_IN, HIGH);
  digitalWrite(PIN_VALVE_ACIDE_IN, HIGH);
  digitalWrite(PIN_VALVE_SOUDE_OUT, HIGH);
  digitalWrite(PIN_VALVE_ACIDE_OUT, HIGH);
  digitalWrite(PIN_VALVE_VIDANGE, LOW);
  digitalWrite(PIN_POMPE, HIGH);  
  delay(5000);
}
void soude () {
  digitalWrite(PIN_VALVE_AIR, HIGH);
  digitalWrite(PIN_VALVE_CO2, HIGH);
  digitalWrite(PIN_VALVE_EAU, HIGH);
  digitalWrite(PIN_VALVE_SOUDE_IN, LOW);
  digitalWrite(PIN_VALVE_ACIDE_IN, HIGH);
  digitalWrite(PIN_VALVE_SOUDE_OUT, LOW);
  digitalWrite(PIN_VALVE_ACIDE_OUT, HIGH);
  digitalWrite(PIN_VALVE_VIDANGE, HIGH);
  digitalWrite(PIN_POMPE, LOW);  
  delay(240000);
}
void vidangeSoude () {
  digitalWrite(PIN_VALVE_AIR, LOW);
  digitalWrite(PIN_VALVE_CO2, HIGH);
  digitalWrite(PIN_VALVE_EAU, HIGH);
  digitalWrite(PIN_VALVE_SOUDE_IN, HIGH);
  digitalWrite(PIN_VALVE_ACIDE_IN, HIGH);
  digitalWrite(PIN_VALVE_SOUDE_OUT, LOW);
  digitalWrite(PIN_VALVE_ACIDE_OUT, HIGH);
  digitalWrite(PIN_VALVE_VIDANGE, HIGH);
  digitalWrite(PIN_POMPE, HIGH); 
  delay(5000);
}
void acide () {
  digitalWrite(PIN_VALVE_AIR, HIGH);
  digitalWrite(PIN_VALVE_CO2, HIGH);
  digitalWrite(PIN_VALVE_EAU, HIGH);
  digitalWrite(PIN_VALVE_SOUDE_IN, HIGH);
  digitalWrite(PIN_VALVE_ACIDE_IN, LOW);
  digitalWrite(PIN_VALVE_SOUDE_OUT, HIGH);
  digitalWrite(PIN_VALVE_ACIDE_OUT, LOW);
  digitalWrite(PIN_VALVE_VIDANGE, HIGH);
  digitalWrite(PIN_POMPE, LOW);
  delay(240000);
}
void vidangeAcide () {
  digitalWrite(PIN_VALVE_AIR, HIGH);
  digitalWrite(PIN_VALVE_CO2, LOW);
  digitalWrite(PIN_VALVE_EAU, HIGH);
  digitalWrite(PIN_VALVE_SOUDE_IN, HIGH);
  digitalWrite(PIN_VALVE_ACIDE_IN, HIGH);
  digitalWrite(PIN_VALVE_SOUDE_OUT, HIGH);
  digitalWrite(PIN_VALVE_ACIDE_OUT, LOW);
  digitalWrite(PIN_VALVE_VIDANGE, HIGH);
  digitalWrite(PIN_POMPE, HIGH);
  delay(5000);
}
void pressurisationCO2 () {
  digitalWrite(PIN_VALVE_AIR, HIGH);
  digitalWrite(PIN_VALVE_CO2, LOW);
  digitalWrite(PIN_VALVE_EAU, HIGH);
  digitalWrite(PIN_VALVE_SOUDE_IN, HIGH);
  digitalWrite(PIN_VALVE_ACIDE_IN, HIGH);
  digitalWrite(PIN_VALVE_SOUDE_OUT, HIGH);
  digitalWrite(PIN_VALVE_ACIDE_OUT, HIGH);
  digitalWrite(PIN_VALVE_VIDANGE, HIGH);
  digitalWrite(PIN_POMPE, HIGH);
  delay(5000);
}

//--------documentation des séquences---------
/*
ETAPE 1 : VIDANGE (10 SECONDES)

2 ———— FERME
3 ———— OUVERT
4 ———— OUVERT
5 ——— —OUVERT
6   ————OUVERT
7   ————OUVERT
8   ————OUVERT
9   ————FERME
10 ————OUVERT



ETAPE 2 : RINÇAGE ( 20 SECONDES)

2   ————OUVERT
3   ————OUVERT
4   ————FERME
5   ————OUVERT
6   ————OUVERT
7   ————OUVERT
8   ————OUVERT
9   ————FERME
10 ————OUVERT

ETAPE 3 : PURGE AIR (5 SECONDES)

2   ————FERME
3   ————OUVERT
4   ————OUVERT
5   ————OUVERT
6   ————OUVERT
7   ————OUVERT
8   ————OUVERT
9   ————FERME
10 ————OUVERT

ETAPE 4 : SOUDE (240 SECONDES)

2   ————OUVERT
3   ————OUVERT
4   ————OUVERT
5   ————FERME
6   ————OUVERT
7   ————FERME
8   ————OUVERT
9   ————OUVERT
10 ————FERMÉ 

ETAPE 5 :  VIDANGE SOUDE (5 SECONDES)

2   ————FERMÉ
3   ————OUVERT
4   ————OUVERT
5   ————OUVERT
6   ————OUVERT
7   ————FERMÉ
8   ————OUVERT
9   ————OUVERT
10 ————OUVERT

ETAPE 6 :  RINÇAGE (20 SECONDES)

2   ————OUVERT
3   ————OUVERT
4   ————FERME
5   ————OUVERT
6   ————OUVERT
7   ————OUVERT
8   ————OUVERT
9   ————FERME
10 ————OUVERT

ETAPE 7 :  ACIDE (240 SECONDES)

2   ————OUVERT
3   ————OUVERT
4   ————OUVERT
5   ————OUVERT
6   ————FERMÉ
7   ————OUVERT
8   ————FERMÉ
9   ————OUVERT
10 ————FERMÉ

ETAPE 8 : VIDANGE ACIDE (5 SECONDES)
 
2   ————OUVERT
3   ————FERMÉ
4   ————OUVERT
5   ————OUVERT
6   ————OUVERT
7   ————OUVERT
8   ————FERMÉ
9   ————OUVERT
10 ————OUVERT

ETAPE 9 : PRESSURISATION CO2 (5 SECONDES)

2   ————OUVERT
3   ————FERMÉ
4   ————OUVERT
5   ————OUVERT
6   ————OUVERT
7   ————OUVERT
8   ————OUVERT
9   ————OUVERT
10 ————OUVERT

ETAPE 10 : BUZZER
*/
