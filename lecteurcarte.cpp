#include "lecteurcarte.h"
#include "voyants.h"
#include "boutons.h"
#include "base_clients.h"
#include "generateur_save.h"
#include "prise.h"
#include "timer.h"
#include <lcarte.h>


enum Etat {CARTE_INSERE, AUTHENTIFICATION_CORRECT, AUTHENTIFICATION_INCORRECT,STOP_PENDANT_CHARGEMENT, DEFAUT, ETAT_1, ETAT_2, ETAT_3, ETAT_4};
Etat etatCourant;

Prise prise;
Generateur_save generateur;

int duree=0;
int appui_stop = 0;
int appui_charge=0;
int prise_branchee = 0;

void LecteurCarte::initialiser()
{
	initialisations_ports();
}

void LecteurCarte::lire_carte()
{
	
	
	switch (etatCourant)
		{
			case CARTE_INSERE :
			
				cout<<"INSERER VOTRE CARTE"<<endl;
				attente_insertion_carte();
				numero_carte=lecture_numero_carte();  // lecture de la carte
				
				cout<<"Numéro de la carte : "<<numero_carte<<endl;
				cout<<endl;				
		
				// Check de la carte 
				if(baseclient.authentifier(numero_carte) && voyants.dispo()) // si client reconnu et borne disponible
				{	
					etatCourant = AUTHENTIFICATION_CORRECT;
				}
				else
				{
					etatCourant = AUTHENTIFICATION_INCORRECT;
				}
				
			break;
		
				
		 	case AUTHENTIFICATION_CORRECT : 
		 	
		 	timer.timer_raz(); // initialisation du timer
			
		 	while((duree <= 16) && (appui_charge!=1) && (appui_stop!=1))//tant que timer <16 et qu'il n'y a pas d'appui sur bouton charge ou stop
		 	{
				duree = timer.timer_valeur();   // on récupère la valeur du timer
				cout<<"Temps = "<<duree<<endl;
				
				appui_stop = bouton.bouton_stop();  // regarder l'état du bouton stop
		    	appui_charge = bouton.bouton_charge();  // regarder l'état du bouton charge
				
				// clignotement de la led charge pendant 8s
				if (duree <= 8)
				{
					voyants.blink_charge();
				}
				else {voyants.set_charge(OFF);}
				
				// si appui bouton charge, led dispo éteinte et led charge rouge
				if (appui_charge==1)
				{
					voyants.set_dispo(OFF);
					voyants.set_charge(ROUGE);
						
					etatCourant = ETAT_1;
				}
				// si appui bouton stop, fin
				if (appui_stop==1 || duree >= 10)
				{						
					etatCourant = DEFAUT;    		
				}
		 	}
		 				
			break;
			
			case AUTHENTIFICATION_INCORRECT : 
				timer.timer_raz(); // initialisation du timer
				//clignotement de la led defaut pendant 8s
		 		while(duree <= 8)
		 		{
					duree = timer.timer_valeur(); 
					
					if (duree <= 8)
					{
						voyants.blink_defaut();
						cout<<"Blink defaut pdt 8s "<<endl;
					}
					else {voyants.set_defaut(OFF);}
		 		}
		 		cout<<endl;
		 		etatCourant = DEFAUT; 		
				
			break;									
				
			case ETAT1 : 
			
	 			prise.deverouiller_trappe();
	  			generateur.generer_PWM(DC); 
	  			//tant que la tension est à 9V et qu'il n'y a pas d'appui bouton stop
				while( (valeur_tension != 9) && (appui_stop != 1))
				{
					valeur_tension = generateur.tension(); 
					appui_stop = bouton.bouton_stop();
					// allumage led prise en vert et déverrouillage trappe
					if (valeur_tension==9)
					{
						voyants.led_prise(VERT);
						prise.verouiller_trappe();
						generateur.generer_PWM(AC_1K);    // Generer tension AC
						usleep(1000000);
						prise_branchee = 1;
						
						etatCourant = ETAT_2;
					} 
					else if (appui_stop==1)  
					{						
						etatCourant = DEFAUT;    		
					}
					
				}
			break;
			
			
			case ETAT_2 : 
				// Fermer contacteur AC et recuperer son etat
				etatContacteur_AC = generateur.ouvrir_fermer_AC(1);
				
				if (etatContacteur_AC==1)
				{
					generateur.generer_PWM(AC_CL); // Generer un signal PWM de 1KHz à rapport cyclique variable
					cout<<"Tension = "<< valeur_tension<<endl;
					etatCourant = ETAT_3;	
				}
				valeur_tension=0; // Réinitialiser la valeur de tension récupérée
				
			break ;	
			
			
			case ETAT3 : 						
				
			// Attente de la charge batterie et sans stop
				while ((valeur_tension!=9) && (appui_stop != 1))
				{
					cout<<"Rechargement de la batterie : tension AC = "<<valeur_tension<<endl;
					valeur_tension = generateur.tension();   
					appui_stop = bouton.bouton_stop();     
	
					// Si tension = 9 (chargement terminé)
					if ( valeur_tension==9)
					{
         					etatContacteur_AC = generateur.ouvrir_fermer_AC(0); // ouvrir contacteur AC
         					numero_carte = 0; // réinitialiser num_carte pour entrer dans boucle while en dessous
         					
         					cout<<endl;
							cout<<"Réinsérer votre carte pour récuperer votre véhicule"<<endl;
    						cout<<endl;
         											
    						while(!baseclient.authentifier(numero_carte)) // tant que carte pas reconnue
    						{
								attente_insertion_carte(); // attente insertion
								numero_carte = lecture_numero_carte(); // lecture de la carte
								cout<<"Numéro de la carte : "<<numero_carte<<endl;
							}
							
							cout<<endl;
         					
         					generateur.generer_PWM(DC); // generer tension DC
         					usleep(1000000);
         					voyants.set_charge(VERT);
         					
         					etatCourant = ETAT_4;
					}
					// si un appui sur stop est détécté pendant le chargement, on doit s'authentifier de nouveau pour l'arrêter
					else if (appui_stop == 1) 
					{
						
						etatCourant = STOP_PENDANT_CHARGEMENT;    		
					}
				}
				break ;
				
				
				case ETAT4 : 	
		
					prise.deverouiller_trappe(); // deverouiller trappe
					// Attente de débrancher 
					while ((valeur_tension!=12 ) && (prise_branchee == 1))
					{	
						valeur_tension= generateur.tension();
						cout<<"DEBRANCHER LA PRISE !"<<endl;
						
						if (valeur_tension==12)
						{	
							voyants.led_prise(OFF);
							voyants.set_charge(OFF);
							voyants.set_dispo(VERT);
						}
					}
					cout<<endl;
					etatCourant = DEFAUT;
				
				break;
				
			
			case STOP_PENDANT_CHARGEMENT :
	
				cout<<"Réauthentifier vous pour arrêter chargement"<<endl;
				cout<<endl;
         					
				numero_carte=0;
					
				cout<<endl;
				cout<<"Réinsérer votre carte pour récuperer votre véhicule"<<endl;
    			cout<<endl;
         								
    			while(!baseclient.authentifier(numero_carte)) // tant que numéro carte n'est pas reconnu
    			{
					attente_insertion_carte(); // attente insertion
					numero_carte = lecture_numero_carte(); // lecture de la carte
					cout<<"Numéro de la carte : "<<numero_carte<<endl;
				}
				cout<<endl;	
						
				etatContacteur_AC = generateur.ouvrir_fermer_AC(0); // ouvrir contacteur AC
				generateur.generer_PWM(DC); // generer tension DC
         		usleep(1000000);
         		voyants.set_charge(VERT);
			
				etatCourant = ETAT_4;
			break;		
			
			case DEFAUT :
				
				generateur.generer_PWM(STOP);
				cout<<"FIN !"<<endl;
				cout<<endl;
				usleep(2000000); 
				
				voyants.voyants_initialiser();
				bouton.boutons_initialiser();
				
				duree=0;
				appui_stop = 0;
				appui_charge=0;
				
				etatCourant = CARTE_INSERE;		
			break;	
			
		}
}

