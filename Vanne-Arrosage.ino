/* Modification du sketch "ReadAnalogVoltage" sur http://www.arduino.cc/en/Tutorial/ReadAnalogVoltage
  _Signal photoresistor (LDR) pin A0:
  _Signal sonde dhumidité N°1 to pin A1, and the outside pins to +5V and ground:
  _Signal sonde dhumidité N°2 to pin A2, and the outside pins to +5V and ground:
  _Envoi signal commande du relais vanne d'eau pin d10:
  _Le relais consomme 70 mA et ne peut pas être alimenté par une broche numerique
   qui fournie au maximun 40mA sera alimenté directement depuis la broche +5V:
  _Power LDR pin d13:
  _Power sonde d'humidité N°1 pin d2:
  _Power sonde d'humidité N°2 pin d3:
  _Les sondes et la LDR auront leur patte commune au potentiel moins et:
   l'autre par une broche numerique HIGH:
  _Le jour et avec sol humide (au repos) le signal prelevé sur la LDR et les sondes:
  sera proche de zero  > Res sera faible:
  _La nuit et avec un sol sec (au travail) Res très grande > que le signal sera HIGH proche du +5V:
*/

/* Sur cette version 21 pour modifier le temps de pause entre deux arrosages, j'ai effectué ces changements: 
 comenté la ligne 120 et ajouté les lignes 288 292 296 300:
 j'ai remplacé le  4 par un 2 aux lignes 31 et 319:
/*

/* valeurs actuelles: arrosage 6 minutes, 10x6 minutes soit 1 heure de pause entre chaque arrosage, 4 arrosages par nuit, arrosage si la valeur de la sonde est superieure a 650: (sensorNuitValue > 850) dans la fonction " testePresenceNuit() " determine le seuil de luminosité minimum audessus du quel on considére qu'il fait nuit:
 Convertion minutes/secondes: 60mn=3600000,45mn=2700000, 30mn=1800000, 20mn=1200000, 15mn=900000, 12mn=720000, 10mn=600000, 5mn=300000, 1mn=60000, Durée d'ouverture electrovanne  en millisecondes:
 3mn 15mn,pause entre 2 arrosages =20mn,pause après 5 arrosages =150mn, pause nuit arrosage =20mn, pause jour =40mn, nuit sans arrosage 80mn:
*/

// Ces variables peuvent être changés pour adapteer le programme a nos souhaits de comportement sur le laps de temps d'ouveture et fermeture de l'electrovanne et l'attente avant nouveau cycle :

int dureePourArrosage = 3; // Durée initiale en minutes,il sert de base au calcul du temps effectif d'arrosage, celui-ci sera fonction du nombre d'arrosages dèja effectués cette nuit:
unsigned long dureeArrosage = 0; //Contiendra la durée en secondes effectivement utilisée pour l'arrosage:
byte ajusteSondeA1 = 60; // la Sonde A1 présente toujours une valeur differente de celle de A2 qui ici était de moins 60:
byte ajusteSondeA2 = -0; // la Sonde A2 présente toujours une valeur différente deà celle de A1:
byte nombreArrosages = 4; // Le nombre d'arrosages par nuit sera au maximum de :
byte pauseMultiplicateur = 2;//2; // Multiplie la variable " pauseEntreDeuxArrosages " pour obtenire le temps d'attente avant le prochain test sur le photoresistor durant le jour, ici toutes les 4 heures:
int limiteValeurSonde = 650; // Valeur limite de la sonde au dessus de laquelle le sol est consideré sec et acctionnera l'ouverture de l'electrovanne:
unsigned long pauseEntreDeuxArrosages = 7200000; // ici 2 heure  qui est le temps de la pause entre deux arrosages ":

// Variables fixes

int compteurCyclesArrosage = 0; // Enregistre le nombre total d'Arrosages depuis le lancement du programme:
byte compteurDArrosagesParNuit = 0; // Enregistre le nombre d'arrosages éffectués dans la nuit:
int sondeDHumiditeA1 = 2; // Alim de la sonde d'humuditée pin D2:
int sondeDHumiditeA2 = 3; // Alim de la sonde d'humuditée pin D3:
int signalRelay = 10;  //Alim de du signal du relais Pin D10:
int sensorDHumiditeValueA1 = 0;// variable recevant la valeur de la sonde dhumiditée:
int sensorDHumiditeValueA2 = 0;// variable recevant la valeur de la sonde dhumiditée:
int sensorNuitValue = 0; // variable recevant la valeur du photoresistor:
int photoresistor = 13; //Alim du photoresistor pin D13 :
bool vanneOpen = 0; //vanne  ON/OF:
bool itsNight = 0; // Après le test du photoresistor sera mis à 0 ou 1: pour la nuit O/N:
boolean nightTrue = 0; // Sera mis à 1 s'il y a détection de la nuit une premiére fois:
bool solHumideA1 = 1; // sol humide pas d'arrosage:
bool solHumideA2 = 1; // sol humide pas d'arrosage:
unsigned long relayOnMillis = 0; // compteur debut ouverture de la vanne d'arrosage:
unsigned long currentMillis = 0 ; // garde en secondes le temps écoulé depuis le demarragedu programme, incrémenté à chaque itération de la boucle principale:
unsigned long tempsAuTestNuitMillis = 0; // variable de temps reinitialisé en présence de la nuit:
int tempsCourrant = 0; // Reçoit les minutges:
int tempsCourrantD = 0; // contiendra "currentMillis" en Days:
int tempsCourrantH = 0; // contiendra "currentMillis" en Heures:
int tempsCourrantMn = 0; // contiendra "currentMillis" en minutes:
unsigned long dureePause = 0;// Valeur qui contiendra un temps de pause servant à; rechercher s'il fait nuit, le delais entre deux arrosages sucsessifs, le long laps de temps après le nombre maximum d'arrosages atteint:
unsigned long relaisOn = 0; // Durée d'ouverture du relais,  qui est définit suivant le numero de la sonde par  "dureeArrosage" correspondant:
unsigned long tempsfinArrosage = 0; // Garde l'heure de la fin du dernier arrosage:

void setup() {

  pinMode(photoresistor, OUTPUT); //Define port attribute as output:
  digitalWrite(photoresistor, LOW);  	// turn photoresistor OFF:
  pinMode(sondeDHumiditeA1, OUTPUT); //Define port attribute as output:
  digitalWrite(sondeDHumiditeA1, LOW);  	// turn soil moisture OFF:
  pinMode(sondeDHumiditeA2, OUTPUT); //Define port attribute as output:
  digitalWrite(sondeDHumiditeA2, LOW);  	// turn soil moisture OFF:
  pinMode(signalRelay, OUTPUT);  	// Define the port attribute as output:
  digitalWrite(signalRelay, LOW);  	// turn signalRelay OFF:
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);

}

// Boucle sans fin:

void loop() {

  //contrôle et ouverture de la vanne si c'est la nuit et si le sol est sec si non on ferme la vanne si le temps "relaisOn" pour l'ouverture est dépassé:

  currentMillis = millis();

  if ((compteurDArrosagesParNuit >= nombreArrosages) && ((currentMillis - tempsfinArrosage) >= dureePause)) { //  Delais avant nouvel arrosage du lendemain (ici dureePause = 50400000=14-~1 heures, 43200000 = (3600000*12) = 12 heures):
    compteurDArrosagesParNuit = 0; // Reinitialise a 0 la variable contenant le nombre d'arrosages permis dans la nouvelle journée:
    dureePause = 0; // (compteurDArrosagesParNuit étant superieur à nombreArrosages) on établi une longue pause en sortant du "else" de la fonction relayON, en modifiant "dureePause = pauseApresXArrosages", ici on le remet 0 pour rentrer imediatement dans la boucle du "if" ci dessous:

  }

  if ((vanneOpen == 0) && (currentMillis - tempsAuTestNuitMillis >= dureePause )) { //Si vanne fermée et l'intervale de test "dureePause" écoulé:

    testePresenceNuit(); // Test s'il fait nuit;
    transformersoixante(currentMillis);// transformation des millisecondes en jours, heures et minutes:
    affichageDonnees();// Du nombre d'arrosages et de la valeur du photoresistor:


    if (itsNight == 1) {

      testeSondeHumiditeA1();
      testeSondeHumiditeA2();

      if ((solHumideA1 == 0) || (solHumideA2 == 0)) {
        // Le sol est sec, appel de la fonction relayOn et ouverture de la vanne d'eau:

        if (solHumideA2 == 0) {
          changeDureedArrosage(); // Si le sol est humide sur la sonde A2 on fixe la durée d'arrosage en fonction du nombre d'arrosages éffectués:
          dureeArrosage = (dureeArrosage * 1) ;// On augmente ou on diminue la valeur de duree d'arrosage en fontion de la sonde qui detecte le sol sec:
        }
        else {
          changeDureedArrosage(); // Si le sol est humide sur la la sonde A1 uniquement on fixe différament  la durée d'arrosage en fonction aussi du nombre d'arrosages éffectués:
          dureeArrosage = (dureeArrosage * 1);// On augmente ou on diminue la valeur de duree d'arrosage en fontion de la sonde qui detecte le sol sec:
        }

        // comme dureeArrosage est fournie en minutes, on le transforme en milisecondes:
        dureeArrosage = dureeArrosage * 60000;
        relaisOn = dureeArrosage; // Le relais restera fermé pour Arrosage, relayOF sera appelé a la fin de ce laps de temps:
        // La nuit avec un sol sec, on arrose puis on attend "pauseEntreDeuxArrosages avant de recommencer un nouveau cycle:
        //dureePause =  (dureeArrosage + pauseEntreDeuxArrosages); // On testera a nouveau l'humidité du sol dans; la durée d'arrosage + 2 heures

        relayOn();
      }
      else {
        // Il fait nuit,mais le sol est encore humide, donc pas d'arrosage dans ce cas mais on testera de nouveau l'humidité du sol dans:
        dureePause =  (pauseEntreDeuxArrosages + (pauseEntreDeuxArrosages / 2)); // soit ici 3 heures
        transformersoixante(dureePause);
        Serial.println("  ");
        Serial.print("Nuit et sol encore humide, entre 2 tests une pause de:   ");
        afficheTemps();
      }
    } // Fin de la boucle du if itsNight = 1:
    else {   // modifier pour realiser  un test de 10 heures une seule fois puis toutes les heures ********************************************************
      // Pause normale de jour, 3 heures entre deux tests du photoresistor:
      dureePause =  (pauseEntreDeuxArrosages + (pauseEntreDeuxArrosages / 2));
      // Mais en abcense d'une premiére détection de la  nuit:
      dejaNuit(); // Si c'est le premier jour, on fera  1 heure de pause entre deux tests, si non on garde la valeur précédente:

    }

    transformersoixante(dureePause); // appel pour transformer les millisecondes en jours, heures et minutes
    Serial.print("Ce nouveau cycle va durer  :                             ");
    afficheTemps(); //Affichages des temps:

  }      // Fin de la grande boucle du if vanneOpen = 0:
  else {
    // Si temps d'arrosage "relaisOn" écoulé, appel de la fonction relayOF pour la fermeture de la vanne d'eau:
    if ((vanneOpen == 1) && (currentMillis - relayOnMillis >= relaisOn )) {
      relayOF();
    }
  }

} // Fin Loop et retour pour une nouvelle lecture,on verifie dabord si c'est la nuit avant de tester si le sol est sec:

void dejaNuit() {

  if (nightTrue != 1) {
    // Si la nuit n'a pas encore été detectée une premiére fois:
    // ceci le premier jour et uniquement jusqu'a la nuit tombée, la pause entre deux tests sera de de 1 heure:
    dureePause = (pauseEntreDeuxArrosages / 2);

  }
}


void testePresenceNuit() {

  // Variable pour pouvoir definir le temps entre deux tests
  tempsAuTestNuitMillis = millis(); // reinitialyse cette variable aprés que le test de la nuit soit effectué:
  digitalWrite(photoresistor, HIGH);  	// Power photoresistor ON:
  delay(1000); // Un delais de une seconde permet une lecture plus précise de la valeur de la sonde:
  // read the input on analog pin A0:
  sensorNuitValue = analogRead(A0);
  delay(500);
  digitalWrite(photoresistor, LOW);    // Power photoresistor OFF:
  if (sensorNuitValue > 850) { // Un faible éclairement implique une grande valeur proche de 850 :
    itsNight = 1; // Si la nuit a été detectée;
    if (nightTrue != 1) {
      nightTrue = 1; // Dès la premiere detection de la nuit, on suprime pour les journées suivantes la pause de 1 heure le jour:
    }
  }
  else {
    itsNight = 0;
    if (compteurDArrosagesParNuit > 0) {
      compteurDArrosagesParNuit = 0;
    }
  }

}

void transformersoixante( unsigned long i) {

  // Transforme les secondes en Heures et Minutes,i= currentMillis:
  tempsCourrant = (i / 60000); // transforme les secondes en minutes:
  tempsCourrantH = (tempsCourrant / 60); // transforme les minutes les heures
  tempsCourrantD = (tempsCourrantH / 24); // Reçoit les jours
  tempsCourrantH = (tempsCourrantH % 24); // reçoit les heures
  tempsCourrantMn = (tempsCourrant % 60); // reçoit les minutes:

}

void afficheTemps() {

  Serial.print(tempsCourrantD);
  Serial.print( " D : ");
  Serial.print(tempsCourrantH);
  Serial.print( " H : ");
  Serial.print(tempsCourrantMn);
  Serial.println(" Mn  ");
  Serial.println("  ");
}


void affichageDonnees() {

  Serial.print("### Début d'un NOUVEAU cycle et nombres d'arrosages effectués");
  Serial.println(compteurCyclesArrosage);
  Serial.print("                  ----------------------------------------   ");
  Serial.println("  ");
  Serial.print("          Le nombre d'arrosages effectués cette nuit         ");
  Serial.print(compteurDArrosagesParNuit);
  Serial.println("  ");
  Serial.print(" *    Le temps écoulé depuis le début de fonctionnement  *   ");
  afficheTemps();// Affichage du temps en heures et minutes:
  Serial.print("  Valeur du Photoresistor la nuit, sera supérieure à 850:    ");
  Serial.println (sensorNuitValue);
  Serial.println("  ");
}


void testeSondeHumiditeA1() {

  digitalWrite(sondeDHumiditeA1, HIGH); // pin d2 high, power soil moisture:
  delay (1000); // Un delais de une seconde permet une lecture plus précise de la valeur de la sonde:
  // lit sonde humidité sur analog pin A1:
  sensorDHumiditeValueA1 = analogRead(A1);
  //Valeur fournie a l'initialisaton des variables, valeur lue proche de 600+60 = 660 pour un sol sec pour A1:
  if (sensorDHumiditeValueA1 > (limiteValeurSonde + ajusteSondeA1 )) { 
    solHumideA1 = 0;
  }
  else {
    solHumideA1 = 1;
  }
  delay(500);
  digitalWrite(sondeDHumiditeA1, LOW); // pin d2 low, power-off soil moisture:
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 3.3V):
  float voltage1 = sensorDHumiditeValueA1 * (3.3 / 1023.0); // pour 5V, 5 / 1023 a adapter suivant la carte Arduino pour une lecture exacte de la tention fournie par la Sonde: ********:
  // Affichage de la valeur de A1:
  Serial.print("Lecture de la valeur lue sur la sonde A1= 0~1023 :       ");
  Serial.print(sensorDHumiditeValueA1);
  // et la tention présente aux bornes de la sonde A1:
  Serial.println("  ");
  Serial.print("La tention mesurée aux bornes de la SondeA1 en volts:    ");
  Serial.print(voltage1);
  Serial.println("  ");
}

void testeSondeHumiditeA2() {

  digitalWrite(sondeDHumiditeA2, HIGH); // pin d3 high, power soil moisture:
  delay (1000);// Un delais de une seconde permet une lecture plus précise de la valeur de la sonde:
  // lit sonde humidité sur analog pin A2:
  sensorDHumiditeValueA2 = analogRead(A2);
  //Valeur fournie a l'initialysaton des variables, valeur lue proche de 600+0 pour un sol sec:
  if (sensorDHumiditeValueA2 >(limiteValeurSonde + ajusteSondeA2 )) { 
    solHumideA2 = 0;
  }
  else {
    solHumideA2 = 1;
  }
  delay(500);
  digitalWrite(sondeDHumiditeA2, LOW); // pin d3 low, power-off soil moisture:
  float voltage2 = sensorDHumiditeValueA2 * (3.3 / 1023.0);// pour 5V, 5 / 1023 a adapter suivant la carte Arduino pour une lecture exacte de la tention fournie par la Sonde: ********:
  // Affichage de la valeur de A2:
  Serial.print("Valeur lue sur la sonde A2= 0~1023 :                     ");
  Serial.print(sensorDHumiditeValueA2);
  // et la tention présente aux bornes de la sonde A2:
  Serial.println("  ");
  Serial.print("Tention présente aux bornes de la SondeA2 en volts:      ");
  Serial.print(voltage2);
  Serial.println("  ");
}

void changeDureedArrosage() { // Calcule et adapte la durée d'arrosage en fonction du nombre d'arrosages éffectués:
  switch (compteurDArrosagesParNuit) {
    case 0:
      dureeArrosage = dureePourArrosage * 2 ; // 6mn  _ ? a l'origine le temps d'arrosage était différent en fonction de la sondes qui detecte le sol sec; A1 - 25 - A2 - 32 (pour 32 mn)
      dureePause =  (dureeArrosage + (pauseEntreDeuxArrosages *1.5)); // On testera a nouveau l'humidité du sol dans; la durée d'arrosage + 2 heures
      break;
    case 1:
      dureeArrosage = dureePourArrosage * 2 ; // 6 mn  _ ? 3: A1 - 8,5 - A2 - 10
      dureePause =  (dureeArrosage + (pauseEntreDeuxArrosages *1.5)); // On testera a nouveau l'humidité du sol dans; la durée d'arrosage + 2 heures
      break;
    case 2:
      dureeArrosage = dureePourArrosage * 1 ; // 3 mn  _ ? 3: A1 - 8,5 - A2 - 10
      dureePause =  (dureeArrosage + (pauseEntreDeuxArrosages * 0.5 )); // On testera a nouveau l'humidité du sol dans; la durée d'arrosage + 2 heures
      break;  
    default:
      dureeArrosage = dureePourArrosage / 3 ;  // 1 mn  _ ?  ; A1 - 5 - A2 - 6
      dureePause =  (dureeArrosage + (pauseEntreDeuxArrosages * 0.5 )); // On testera a nouveau l'humidité du sol dans; la durée d'arrosage + 2 heures
      break;
  }
}

void relayOn() {
  if (compteurDArrosagesParNuit < nombreArrosages) {
    // Il y aura  x Arrosages si le nombre x d'arrosages choisi n'a pas encore été atteint:
    vanneOpen = 1;
    digitalWrite(signalRelay, HIGH);  // turn the signalRelay ON	:
    relayOnMillis = millis(); // Memorise le temps de début d'arrosage:
    transformersoixante(relayOnMillis);
    Serial.println(" ");
    Serial.print("      *_Ouverture de la vanne d'arrosage:_*            ");
    afficheTemps();

  }
  else {
    // Si le nombre d'arrodages choisi a été atteint, on attend environ 12 heures  avant de reprendre le déroulement du programme :
    dureePause =  (pauseEntreDeuxArrosages * (pauseMultiplicateur + 4)); // 2*(2+4)=12
    transformersoixante(dureePause);
    Serial.println(" ");
    Serial.print("Après les 4 arrosages il y a maintenant une pause de:  ");
    afficheTemps();
  }
}

void relayOF() {
  // On arrête la vanne si le temps d'arrosage a éré effctué:
  compteurCyclesArrosage += 1; // Augmentation du compteur d'Arrosages total depuis le début du fonctionnement:
  vanneOpen = 0;// Mémorise l'état de l'électrovanne:
  compteurDArrosagesParNuit += 1; // Augmentation du compteur d'arrosages durant les dernieres 24 heures:
  digitalWrite(signalRelay, LOW);  	// turn the signalRelay OFF:
  tempsfinArrosage = millis(); // Memorise le temps pris à la fin du dernier arrosage:
  transformersoixante(tempsfinArrosage);
  Serial.print("      *_Fermeture de la vanne d'arrosage:_*              ");
  afficheTemps();

  }
