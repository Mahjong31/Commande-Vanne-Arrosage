
// Les deux variables suivantes sont � changer pour adapteer le temps d'ouveture et fermeture de l'electrovanne et le temps de pause avant nouveau cycle :

byte pauseMultiplicateur = 1;// Multiplie la variable " pauseEntreDeuxArrosages ":
float pauseEntreDeuxArrosages = 60; // = 60: en minutes; 60 = 3600000 millisecondes:
float dureePourArrosage = 1; // = 1: en minutes, - Dur�e initiale en minutes,il sert de base au calcul du temps effectif d'arrosage, celui-ci sera fonction du nombre d'arrosages d�ja effectu�s cette nuit:
float dureeArrosage = 0; //Contiendra la dur�e en secondes effectivement utilis�e pour l'arrosage:
const int limiteSolHumide = 600; // limiteSolHumide sperieure à 600  il y aura arrosage:

// Variables fixes:
byte nombreArrosages = 9; // cette valeur qui ne sera jamais atteinte nous permet de fixer suivant l'humidit� du sol et uniquement la premiere fois le nombre exact darroges compris entre 1 et 3  pour chaque  nuit:
int compteurCyclesArrosage = 0; // Enregistre le nombre total d'Arrosages depuis le lancement du programme:
byte compteurDArrosagesParNuit = 1; // Enregistre le nombre d'arrosages �ffectu�s dans la nuit:
int sondeDHumiditeA1 = 2; // Alim de la sonde d'humudit�e pin D2:
int sondeDHumiditeA2 = 3; // Alim de la sonde d'humudit�e pin D3:
int signalRelay = 10;  //Alim de du signal du relais Pin D10:
bool sondeA1A2 = 0; // receuille la plus grande valeur des deux sondes:
int sensorDHumiditeValueA1 = 0;// variable recevant la valeur de la sonde dhumidit�e:
int sensorDHumiditeValueA2 = 0;// variable recevant la valeur de la sonde dhumidit�e:
int sensorHumidValue = 0;// reçoit la plus grande valeur des deux sondes d'humiditée:
int sensorNuitValue = 0; // variable recevant la valeur du photoresistor:
float dureePause = 0;// Valeur qui contiendra un temps de pause servant �; rechercher s'il fait nuit, le delais entre deux arrosages sucsessifs, le long laps de temps apr�s le nombre maximum d'arrosages atteint:
int photoresistor = 13; //Alim du photoresistor pin D13 :
bool itsNight = 0; // Apr�s le test du photoresistor sera mis � 0 ou 1: pour la nuit O/N:
boolean nightTrue = 0; // Sera mis � 1 s'il y a d�tection de la nuit une premi�re fois:
bool vanneOpen = 0; //vanne  ON/OF:

unsigned long relayOnMillis = 0; // compteur debut ouverture de la vanne d'arrosage:
unsigned long currentMillis = 0 ; // garde en secondes le temps �coul� depuis le demarragedu programme, incr�ment� � chaque it�ration de la boucle principale:
unsigned long tempsAuTestNuitMillis = 0; // variable de temps reinitialis� en pr�sence de la nuit:
unsigned long tempsCourrant = 0; // Re�oit les minutges:
unsigned long tempsCourrantD = 0; // contiendra "currentMillis" en Days:
unsigned long tempsCourrantH = 0; // contiendra "currentMillis" en Heures:
unsigned int tempsCourrantMn = 0; // contiendra "currentMillis" en minutes:
unsigned long tempsCourrantSc = 0; // contiendra "currentMillis" en millisecondes:
unsigned long relaisOn = 0; // Dur�e d'ouverture du relais,  qui est d�finit suivant le numero de la sonde par  "dureeArrosage" correspondant:
unsigned long tempsfinArrosage = 0; // Garde l'heure de la fin du dernier arrosage:

void setup() {

  pauseEntreDeuxArrosages = pauseEntreDeuxArrosages * 60000; // transforme les minutes en millisecondes:
  dureePourArrosage = dureePourArrosage * 60000; // idem ci dessus:
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

  //contr�le et ouvre la vanne si c'est la nuit avec un sol sec si non on ferme la vanne si le temps "relaisOn" pour l'ouverture est d�pass�:

  currentMillis = millis();
  
  if ((compteurDArrosagesParNuit > nombreArrosages) && ((currentMillis - tempsfinArrosage) >= dureePause)) { //  Delais avant nouvel arrosage du lendemain (ici dureePause = 50400000=14-~1 heures, 43200000 = (3600000*12) = 12 heures):
    compteurDArrosagesParNuit = 1; // Reinitialise a 0 la variable contenant le nombre d'arrosages permis dans la nouvelle journ�e:
    dureePause = 0; // (compteurDArrosagesParNuit �tant superieur � nombreArrosages) on �tabli une longue pause en sortant du "else" de la fonction relayON, en modifiant "dureePause = pauseApresXArrosages", ici on le remet 0 pour rentrer imediatement dans la boucle du "if" ci dessous:
    sondeA1A2 = 0; // sondeA1 < sondeA2:
    nombreArrosages = 9; // Le nombre d'arrosages estait compris entre 1 et 3, on lui assigne le 9 pour rentrer dans le if de adapteNombreArrosages():
  }

  if ((vanneOpen == 0) && (currentMillis - tempsAuTestNuitMillis >= dureePause )) { //Si vanne ferm�e et l'intervale de test "dureePause" �coul�:

    testePresenceNuit(); // Vérifie s'il fait nuit;
    transformersoixante(currentMillis);// transformation des millisecondes en jours, heures et minutes:
    affichageDonneesArrosage();// Du nombre d'arrosages et de la valeur du photoresistor:

    if (itsNight == 1) {

      lectureSondesHumidite(sondeDHumiditeA1,sondeDHumiditeA2);
      affichageDonneesSondesHumidite();
      adapteNombreArrosages();

      if (sensorHumidValue >= limiteSolHumide) {

        // Le sol est sec, si sensorHumidValue supérieure à limitesolHumide initial = 600, appel de la fonction relayOn pour ouverir la vanne d'eau:
        AdapteDureeArrosage(); // Etabli la durée de la pause entre deux arrosages et leur durée respective:
        relaisOn = dureeArrosage; // Le relais restera ferm� pour Arrosage, relayOF sera appel� a la fin de ce laps de temps:
        relayOn();
      }
      else {
        // La Nuit et sol humide, pas d'arrosage, et test des sondes d'humidit� toutes les 4 heures :
        dureePause =  (pauseEntreDeuxArrosages * 4 * pauseMultiplicateur);
        transformersoixante(dureePause);
        Serial.println("  ");
        Serial.print("       Nuit et sol encore humide, entre 2 tests une pause de:   ");
        afficheTemps();
      }
    } 
    else {
      // Test du photoresistor, LDR, le jour, toutes les 4 heures:
      dureePause =  (pauseEntreDeuxArrosages * 4 * pauseMultiplicateur);
      // Mais en abcense d'une premi�re d�tection de la  nuit:
      dejaNuit(); // Si c'est le premier jour, on fera  1 heure de pause entre deux tests, si non on garde la valeur pr�c�dente:

    } // Fin de la boucle du if itsNight = 1:

     transformersoixante(dureePause); // appel pour transformer les millisecondes en jours, heures et minutes:
    Serial.print("       L'Arrosage plus le temps de pause vont durer:            ");
    afficheTemps(); //Affichages des temps:
    
  }      
  else {
    // Si temps d'arrosage "relaisOn" �coul�, appel de la fonction relayOF pour la fermeture de la vanne d'eau:
    if ((vanneOpen == 1) && (currentMillis - relayOnMillis >= relaisOn )) {
      relayOF();
    }
  } // Fin de la boucle du if sensorHumidValue > 559  // limitesolHumide >= 600:

} // Fin Loop et retour pour une nouvelle lecture,on verifie dabord si c'est la nuit avant de tester si le sol est sec:

void dejaNuit() {

  if (nightTrue != 1) {
    // Si la nuit n'a pas encore �t� detect�e une premi�re fois:
    // ceci le premier jour et uniquement jusqu'a la nuit tomb�e, la pause entre deux tests sera de de 1 heure:
    dureePause = (pauseEntreDeuxArrosages);

  }
}

void testePresenceNuit() {

  // Variable pour pouvoir definir le temps entre deux tests:
  tempsAuTestNuitMillis = millis(); // reinitialyse cette variable apr�s que le test de la nuit soit effectu�:
  digitalWrite(photoresistor, HIGH);  	// Power photoresistor ON:
  delay(1000); // Un delais de une seconde permet une lecture plus pr�cise de la valeur de la sonde:
  // read the input on analog pin A0:
  sensorNuitValue = analogRead(A0);
  delay(500);
  digitalWrite(photoresistor, LOW);    // Power photoresistor OFF:
  if (sensorNuitValue > 850) { // S'il fait nuit, un faible �clairement implique une grande valeur proche de 850 :
    itsNight = 1; // Si la nuit a �t� detect�e:
    if (nightTrue != 1) {
      nightTrue = 1; // D�s la premiere detection de la nuit, on suprime pour les journ�es suivantes la pause de 1 heure le jour;
    }
  }
  else {
    itsNight = 0;
    if (compteurDArrosagesParNuit > 0) { // il fait jour remise � zero du compteur d'arrosages par nuit:
      compteurDArrosagesParNuit = 0;
    }
  }

}

void transformersoixante( unsigned long i) { // Transforme les millisecondessecondes en Heures et Minutes:

  if (i < 60000) { //les milisecondes:
    tempsCourrantSc = i ; 
  }
  else{
    tempsCourrantSc = (i % 60000);
  }
  tempsCourrant = (i / 60000); // transforme les miiillisecondes en minutes:
  tempsCourrantH = (tempsCourrant / 60); // transforme les minutes les heures:
  tempsCourrantD = (tempsCourrantH / 24); // Re�oit les jours:
  tempsCourrantH = (tempsCourrantH % 24); // re�oit les heures:
  tempsCourrantMn = (tempsCourrant % 60); // re�oit les minutes:
}

void afficheTemps() {

  Serial.print(tempsCourrantD);
  Serial.print( " D : ");
  Serial.print(tempsCourrantH);
  Serial.print( " H : ");
  Serial.print(tempsCourrantMn);
  Serial.print(" Mn : ");
  Serial.print(tempsCourrantSc);
  Serial.println(" MiliSec : ");
  Serial.println("  ");
}

void affichageDonneesArrosage() {

  //Serial.println("  ");
  Serial.print("   Le nombre d'arrosages dèjà effectu�s cette nuit:             ");
  if (compteurDArrosagesParNuit > 0){ //Ce compteur debute a 1 pour 0 arrosages:
    Serial.println(compteurDArrosagesParNuit - 1);
  }
  else{
    Serial.println(compteurDArrosagesParNuit);
  }
  Serial.print("   Nombres d'arrosages effectu�s depuis le début du programme:  ");
  Serial.println(compteurCyclesArrosage);
  Serial.println("  ");
  Serial.println("  ");
  Serial.print("   Temps dèjà écoulé:                                           ");
  afficheTemps();// Affichage du temps en heures et minutes:
  Serial.println("   Nouveau test de la sonde d'humidité:                       ");
  Serial.print("   Valeur du Photoresistor la nuit, sera sup�rieure �  850:     ");
  Serial.println (sensorNuitValue);
  Serial.println("  ");
}

void lectureSondesHumidite(int sHA1 , int sHA2){
   
   int compteur = 0;
   int moyenneA1 = 0;
   int moyenneA2 = 0;
   sondeA1A2 = 0;
   digitalWrite(sHA1, HIGH); // sondeDHumiditeA1 pin d2 high:
   digitalWrite(sHA2, HIGH); // sondeDHumiditeA1 pin d2 high:
   delay (1000); // Un delais de une seconde permet une lecture plus pr�cise de la valeur de la sonde:
   for( compteur = 0 ; compteur < 3 ; compteur++){
     // lit sonde humidit� sur analog pin A1:
     sensorDHumiditeValueA1 = analogRead(A1);
     moyenneA1 = moyenneA1 + sensorDHumiditeValueA1; //moyenne des 3 valeurs de lecture:
     //Serial.println(moyenneA1);
     sensorDHumiditeValueA2 = analogRead(A2);
     moyenneA2 = moyenneA2 + sensorDHumiditeValueA2; //ajoute les 3 valeurs de lecture:
     //Serial.println(moyenneA2);
     delay(500);
   }
   digitalWrite(sondeDHumiditeA1, LOW); // pin d2 low:
   digitalWrite(sondeDHumiditeA2, LOW); // pin d3 low:
   sensorDHumiditeValueA1 = moyenneA1 / compteur; //calcule la moyenne des 3 valeurs lues:
   sensorDHumiditeValueA2 = moyenneA2 / compteur;
   sensorDHumiditeValueA1 = map(sensorDHumiditeValueA1,420,1023,0,1023); // Redistribu la valeur de la sonde lue dans l'intervale (0,1023)
   sensorDHumiditeValueA2 = map(sensorDHumiditeValueA2,350,1023,0,1023);
  
  if ((sensorDHumiditeValueA1) > sensorDHumiditeValueA2) { // Trouve la plus grande valeur de la sonde d'humidite
    sondeA1A2 = 1; // sondeA1 > sondeA2:
    sensorHumidValue  = sensorDHumiditeValueA1;
  }
  else{
    sondeA1A2 = 0; // sondeA1 < sondeA2:
    sensorHumidValue  = sensorDHumiditeValueA2;
  }  
}

void adapteNombreArrosages() { // Si le sol est sec:fixe le nombre d'arrosages par nuit suivant de sec à tr�s sec:

  if (nombreArrosages == 9){  // cette valeur qui ne sera jamais atteinte nous permet de ne rentrer dans le if uniquement la premiere fois pour determiner le nombre exact d'arroges pour la  nuit:
    switch (sensorHumidValue) {
      case limiteSolHumide ... 700: //sol sec et 1 arrosage:
        nombreArrosages = 1;
        break;
      case 701 ... 850:
        nombreArrosages = 2; //sol très sec et 2 arrosages:
        break;
      case 851 ... 1023:
        nombreArrosages = 3; //sol extremement sec et 3 arrosages:
        break;
      default: // Sol humide:
        break;
    }
  }
}


void AdapteDureeArrosage() { // Adapte la durée d'arrosage et du nombre d'arrosages dèja �ffectu�s en fonction de la sonde :
  if (sondeA1A2) { // La sonde de plus grande valeur est la sondeA1:
    switch (nombreArrosages) {
      case 1:
         dureeArrosage = dureePourArrosage * 7 ; // temps d'arrosage = dureePourArrosage en minutes * 4*1 :
         dureePause =  (dureeArrosage + (pauseEntreDeuxArrosages * 12)); //   **********************************dans (pauseEntreDeuxArrosages en heures * 12) on testera a nouveau l'humidit� du sol dans:
        break;
      case 2:
         dureeArrosage = dureePourArrosage * 4 ; // *3*2:
         dureePause =  (dureeArrosage + (pauseEntreDeuxArrosages * 6 ));// *6 :*60000 *****************:          
        break;
      case 3:
         dureeArrosage = dureePourArrosage * 3 ; // *3*3:
         dureePause =  (dureeArrosage+ (pauseEntreDeuxArrosages * 3 )); // *3 : *60000 *******************:
        break;
    }
  }
  else{
    switch (nombreArrosages) {// La sonde de plus grande valeur est la sondeA1:
      case 1:
        dureeArrosage = dureePourArrosage * 9 ; // *9*1 :
        dureePause =  (dureeArrosage + (pauseEntreDeuxArrosages * 12)); // *12 :*60000  *********************:
        break;
      case 2:
        dureeArrosage = dureePourArrosage * 5 ; // *5*2:
        dureePause =  (dureeArrosage + (pauseEntreDeuxArrosages * 6 )); // *6 :*60000  *****************:
        break;
      case 3:
        dureeArrosage = dureePourArrosage * 4 ; // *4*3
        dureePause =  (dureeArrosage + (pauseEntreDeuxArrosages * 3 )); // *3:*60000  *************:
        break;
    }
  }
}

void affichageDonneesSondesHumidite(){

  // * On doit remplacer les deux lignes suivantes en fonction de la carte Arduino:float voltageA2 = sensorDHumiditeValueA2 * (3.3 / 1023.0) / pour une lecture exacte de la tention fournie par la Sonde:
  float voltageA1 = sensorDHumiditeValueA1 * (3.3 / 1023.0); //* pour 5V, 5 / 1023 carte Arduino 5 olts:
  // Affichage de la valeur de A1:
  Serial.print("       Lecture de la valeur lue sur la sonde A1= 0~1023 :       ");
  Serial.print(sensorDHumiditeValueA1);
  // et la tention pr�sente aux bornes de la sonde A1:
  Serial.println("  ");
  Serial.print("       La tention mesur�e aux bornes de la SondeA1 en volts:    ");
  Serial.print(voltageA1);
  Serial.println("  ");
  
  float voltageA2 = sensorDHumiditeValueA2 * (3.3 / 1023.0);//* pour 5V, 5 / 1023 carte Arduino 5 volts :
  // Affichage de la valeur de A2:
  Serial.print("       Lecture valeur lue sur la sonde A2= 0~1023 :             ");
  Serial.print(sensorDHumiditeValueA2);
  // et la tention pr�sente aux bornes de la sonde A2:
  Serial.println("  ");
  Serial.print("       Tention pr�sente aux bornes de la SondeA2 en volts:      ");
  Serial.print(voltageA2);
  Serial.println("  ");
}

void relayOn() {
  if (compteurDArrosagesParNuit < nombreArrosages + 1) { // ********************************
    // Il y aura  x Arrosages si le nombre x d'arrosages choisi n'a pas encore �t� atteint:
    vanneOpen = 1;
    digitalWrite(signalRelay, HIGH);  // turn the signalRelay ON	:
    relayOnMillis = millis(); // Memorise le temps de d�but d'arrosage:
    transformersoixante(relayOnMillis);
    Serial.println(" ");
    Serial.print("       La vanne d'arrosage a été ouverte à:                     ");
    afficheTemps();

  }
  else {
    // Le nombre d'arrosages maximal à �t� atteint, on fait une pause de: (pauseEntreDeuxArrosages * 10):
    // Pour de reprendre le test des sondes en fin de journée suivante:
    dureePause =  (pauseEntreDeuxArrosages * 10 * pauseMultiplicateur); // 10 heures:
    transformersoixante(dureePause);
    Serial.println(" ");
    Serial.println("  ------------------------------------------------------------");
    Serial.print("   Cette Nuit ");
    Serial.print(nombreArrosages); 
    Serial.println(" arrosage(s)  ont eu lieu:            ");
    Serial.print("  On va patienter et attendre la nuit prochaine, dans environ:  ");
    afficheTemps();
  }
}

void relayOF() {
  // On arr�te la vanne si le temps d'arrosage a �r� effctu�:
  compteurCyclesArrosage += 1; // Augmentation du compteur d'Arrosages total depuis le d�but du fonctionnement:
  vanneOpen = 0;// M�morise l'�tat de l'�lectrovanne:
  compteurDArrosagesParNuit += 1; // Augmentation du compteur d'arrosages durant les dernieres 24 heures:
  digitalWrite(signalRelay, LOW);  	// turn the signalRelay OFF:
  tempsfinArrosage = millis(); // Memorise le temps pris � la fin du dernier arrosage:
  transformersoixante(tempsfinArrosage);
  Serial.println(" ");
  Serial.print("       La vanne d'arrosage a été fermée à:                      "); 
  afficheTemps();

  }
