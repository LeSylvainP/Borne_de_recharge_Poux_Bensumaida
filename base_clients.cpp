#include <iostream>
#include <vector>
#include "base_clients.h"

using namespace std;


typedef vector<int> vec;

bool Base_clients::authentifier(int num_carte)
{
	vec liste_clients;
	// liste des clients connus
	liste_clients.push_back(20);
	liste_clients.push_back(12);
	liste_clients.push_back(8);
	liste_clients.push_back(3);
	liste_clients.push_back(15);
	

	for(unsigned int i=0; i < liste_clients.size(); i++)
	{
		if(liste_clients[i]==num_carte)
		{
			cout<<"Authentification correcte"<<endl;
			return true;
		}
		
	}
		if (num_carte==0) 
		{
			return false;
		}  
		else 
		{
			cout<<"Authentification incorrecte"<<endl<<endl;
			return false;
		}
		
}


