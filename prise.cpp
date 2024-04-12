#include "prise.h"
#include "voyants.h"
#include <mem_sh.h>
#include <donnees_borne.h>
#include <memoire_borne.h>

Prise::Prise()
{
	io=acces_memoire(&shmid);
	if (io==NULL) cout<<"Erreur pas de memoire sh\n"<<endl;
}

void Prise::verouiller_trappe()
{
	voyant.led_trappe(OFF);
}


void Prise::deverouiller_trappe()
{

	voyant.led_trappe(VERT);
	cout<<"Trappe déverouillée "<<endl;
}

int Prise::set_prise()
{
	prise_branche=1;
	return prise_branche;
}

