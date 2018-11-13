#include"Simulator.h"
#include"Person.h"
#include<iostream>
#include<random>
#include<fstream>

using namespace std;

/* world_size: Tamaño del espacio bidimensional
* number_people: Cantidad total de personas
* death_duration: Cuanto pasa antes de que se mueran las personas
* tic: Cantidad de tics que dura la simulacion
* infectiousness: Potencia infecciosan
* chance_recover: Probabilidad de recuperacion
* infected: Personas infectadas inicialmente*/
int world_size, death_duration, tic, thread_count, number_people;
int healthy_people, dead_people, sick_people, inmune_people;
double infected, infectiousness, chance_recover;

int Simulator::initialize(int number_peopleM, double infectiousnessM, double chance_recoverM, int death_durationM, double infectedM, int world_sizeM, int ticM, int thread_countM) {
	random_device rd;
	int perc;
	int healthy;
	int pos1, pos2;
	number_people = number_peopleM;
	infectiousness = infectiousnessM;
	chance_recover = chance_recoverM;
	death_duration = death_durationM;
	infected = infectedM;
	world_size = world_sizeM;
	tic = ticM;
	thread_count = thread_countM;
	Person p;
#pragma omp single
	{
		v.resize(world_size); //Vector de vectores de tamaño world_size*world_size
		world.resize(world_size, v);
		peopleVec.resize(number_people);
	}
	sick_people = 0;
	perc = number_people * infected / 100; //Cantidad correspondiente al porcentaje dado
	healthy = number_people - perc; //Gente sana
	//Personas enfermas
#pragma omp parallel for num_threads(thread_count)
	for (int i = 0; i < perc; i++) { //Cambiamos a los infectados
		Person p; //Se crean sanos y con su x y y
		pos1 = rd() % world_size;
		pos2 = rd() % world_size;
		p.setX(pos1);
		p.setY(pos2);
		p.setState(1);
		p.setSick(1);
#pragma omp atomic
		world[pos1][pos2]++; //Metemos a la persona en la lista de la posicion correspondiente	
		peopleVec[i] = p;
	}
	//Personas sanas
#pragma omp parallel for num_threads(thread_count)
	for (int j = perc; j < peopleVec.size(); j++) {
		Person p;
		pos1 = rd() % world_size;
		pos2 = rd() % world_size;
		p.setX(pos1);
		p.setY(pos2);
#pragma omp atomic
		world[pos1][pos2]++;
		peopleVec[j] = p;
	}
	return healthy;
}

double Simulator::update(string name, int healthy) {
	healthy_people = healthy; //Inicializar nos dio el numero de sanos 
	sick_people = number_people - healthy; //Los enfermos son el resto
	random_device generator;
	uniform_real_distribution<double> distribution(0.0, 1.0);
	int pos1, pos2, state, x, y, sick_time, sick;
	int actual_tic = 1; //Tics actuales
	double prob_infect, prob_rec;
	double out_time = 0; //Variable que toma el tiempo de salida de datos para restarlo posteriormente
	bool isSick = 0; //Indica si la persona se enfermo
	bool stable = 0; //Termina el while cuando todo se estabiliza
	do {
#pragma omp parallel for num_threads(thread_count) reduction(+:dead_people, sick_people, inmune_people, healthy_people)
		for (int i = 0; i < peopleVec.size(); i++) { //Usamos reduction para hacer las sumas por hilo en las variables globales
			x = peopleVec[i].getX();
			y = peopleVec[i].getY();
#pragma omp atomic
			world[x][y]--;
			if (world[x][y] > 0) { //Como el mundo tiene la cantidad de personas, si despues de procesar a alguien hay mas, hay que revisar por enfermos
#pragma omp critical
				sick = checkVec(x, y, i);
			}
			state = peopleVec[i].getState();
			if (state == 1) {
				sick_time = peopleVec[i].getSick();
				if (sick_time >= death_duration) {
					prob_rec = distribution(generator); //Decidimos si la persona se enferma o se hace inmune
					if (prob_rec < chance_recover) {
						peopleVec[i].setState(2);
						sick_people--;
						inmune_people++;
					}
					else {
						peopleVec[i].setState(3);
						sick_people--;
						dead_people++;
					}
				}
				else { //Si todavia no le toca, aumentamos el tiempo que lleva enferma
#pragma omp atomic
					sick_time++;
					peopleVec[i].setSick(sick_time);
				}
			}
			else if (state == 0) {
				for (int j = 0; j < sick; j++) { //Hacemos un for por cada enfermo en la misma posicion de la persona
					prob_infect = distribution(generator);
					if (prob_infect < infectiousness) {
						isSick = 1;
					}
				}
				if (isSick) { //Si la persona se enfermo, cambiamos su estado y empezamos el conteo de tics
					peopleVec[i].setState(1);
					peopleVec[i].setSick(1);
					healthy_people--;
					sick_people++;
				}
			}
			pos1 = movePos(x, world_size);
			pos2 = movePos(y, world_size);
			peopleVec[i].setX(pos1);
			peopleVec[i].setY(pos2);
			isSick = 0;
		}
		stable = clear(actual_tic, name); //*
		actual_tic++;
	} while (!stable && (actual_tic <= tic));
	cout << std::endl;
	cout << "Archivo generado" << endl;
	peopleVec.clear(); //Se limpian las ed's para nuevas simulaciones. Ademas se limpian las variables
	world.clear();
	dead_people = sick_people = healthy_people = inmune_people = 0;
	return out_time;
}

int Simulator::checkVec(int pos1, int pos2, int j) {
	int pos3, pos4, state;
	int sick = 0;
	for (int i = j; i < peopleVec.size(); i++) {
		pos3 = peopleVec[i].getX();
		pos4 = peopleVec[i].getY();
		if ((pos1 == pos3) && (pos2 == pos4)) { //Si hay otra persona en la misma posicion de los parametros y esta enferma aumentamos la cantidad de enfermos
			state = peopleVec[i].getState();
			if (state == 1) {
				sick++;
			}
		}
	}
	return sick;
}

int Simulator::movePos(int pos, int world_size) {
	random_device rd;
	int movX;
	movX = rd() % 2;
#pragma omp atomic
	movX -= 1;
#pragma omp atomic
	pos += movX;
	if (pos < 0) { //Para que no se salga de la matriz
		pos = world_size - 1;
	}
	else if (pos >= world_size) {
		pos = 0;
	}
	return pos;
}

bool Simulator::clear(int actual_tic, string name) {
	int x, y;
	bool stable = 0;
	ofstream file;
	file.open(name, ios_base::app);
	file << "Reporte del tic " << actual_tic << endl
		<< " Personas muertas total " << dead_people << ", promedio " << dead_people / actual_tic << ", porcentaje " << number_people * dead_people / 100 << endl
		<< " Personas sanas total " << healthy_people << ", promedio " << healthy_people / actual_tic << ", porcentaje " << number_people * healthy_people / 100 << endl
		<< " Personas enfermas total " << sick_people << ", promedio " << sick_people / actual_tic << ", porcentaje " << number_people * sick_people / 100 << endl
		<< " Personas inmunes total " << inmune_people << ", promedio " << inmune_people / actual_tic << ", porcentaje " << number_people * inmune_people / 100 << endl;

	file.close();//Hacer archivo

	if (sick_people == 0) {
		stable = 1;
	}
	else {
#pragma omp parallel for num_threads(thread_count)
		for (int i = 0; i < peopleVec.size(); i++) { //Volvemos a llenar la matriz despues de haber procesado a todos en el tic anterior
			x = peopleVec[i].getX();
			y = peopleVec[i].getY();
#pragma omp atomic
			world[x][y]++;
		}
	}
	return stable;
}