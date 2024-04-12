#ifndef LECTEURCARTE_H
#define LECTEURCARTE_H

#include <lcarte.h>
#include <iostream>

#include "base_clients.h"
#include "voyants.h"
#include "boutons.h"
#include "timer.h"
#include "prise.h"
#include "generateur_save.h"

using namespace std;


class LecteurCarte
{

  public : 
  
	void initialiser();
	void lire_carte();

	Base_clients baseclient; 
	Voyants voyants;		
	Boutons bouton;
	Timer timer;
	Prise prise;
	Generateur_save generateur;
	
	int choix_menu;
	int numero_carte;

	float valeur_tension;
	int etatContacteur_AC;

};

#endif // LECTEURCARTE_H
