# laveuse-de-fut
Programme de base pour piloter une machine DIY de lavage de fûts de bière
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

Un programme réalisé par Antony Le Goïc-Auffret sous licence GPL
 */
