#pragma once
#ifndef Simulador
#define Simulador

#include<vector>
#include<list>
#include"Person.h"

using namespace std;

class Simulator {
private:
	vector<vector<int>> world; //Vector de vectores de enteros de la stl. Tiene la cantidad de personas en determinada posicion
	vector<int> v;
	vector<Person> peopleVec; //Vector que contiene a las personas
public:
	int initialize(int, double, double, int, double, int, int, int); //Inicializa el mundo y los parametros
	double update(string, int); //Actualiza el mundo por tic
	int movePos(int, int); //Mueve a la persona de posicion
	bool clear(int, string); //Crea el archivo
	int checkVec(int, int, int);
};

#endif // !Simulador