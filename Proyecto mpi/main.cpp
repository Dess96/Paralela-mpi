#include<stdlib.h>
#include<iostream>
#include<string>
#include<thread>
#include <mpi.h> 
#include"Person.h"
#include"Simulator.h"

using namespace std;

bool validateProb(double, double);
bool validatePeople(int);

int main(int argc, char * argv[]) {
	int mid; // id de cada proceso
	int cnt_proc; // cantidad de procesos
	MPI_Status mpi_status; // para capturar estado al finalizar invocación de funciones MPI
	MPI_Init(&argc, &argv);             		/* Arranca ambiente MPI */
	MPI_Comm_rank(MPI_COMM_WORLD, &mid); 		/* El comunicador le da valor a id (rank del proceso) */
	MPI_Comm_size(MPI_COMM_WORLD, &cnt_proc);  /* El comunicador le da valor a p (número de procesos) */
	unsigned n = std::thread::hardware_concurrency(); //Saca la cantidad de nucleos en la computadora
	int thread_countM = 2 * n;
	int world_sizeM, death_durationM, ticM, number_peopleM, healthy_people;
	int new_sim = 1;
	int sims = 1;
	double infectedM, infectiousnessM, chance_recoverM, arch_time;
	double local_start, local_finish, local_elapsed, elapsed;
	string number;
	string name = " ";
	Simulator sim;
	do {
		infectiousnessM = -1;
		chance_recoverM = -1;
		number_peopleM = -1;
		while (!validateProb(infectiousnessM, chance_recoverM) || !validatePeople(number_peopleM)) { //Pedir y validar datos
			if (mid == 0) {
				cout << "Ingrese el numero de personas en el mundo (de 0 a 10 000)" << endl;
				cin >> number_peopleM;
				cout << "Ingrese la potencia infecciosa del mundo (decimal entre 0 y 1)" << endl;
				cin >> infectiousnessM;
				cout << "Ingrese la probabilidad de recuperacion (decimal entre 0 y 1)" << endl;
				cin >> chance_recoverM;
				cout << "Ingrese el tiempo maximo que puede vivir una persona infectada" << endl;
				cin >> death_durationM;
				cout << "Ingrese el porcentaje de personas infectadas inicialmente" << endl;
				cin >> infectedM;
				cout << "Ingrese el tamano del espacio bidimensional" << endl;
				cin >> world_sizeM;
				cout << "Ingrese la cantidad de tics" << endl;
				cin >> ticM;
			}
		}
		if (mid == 0) {
			cout << "Se usaran " << thread_countM << " hilos en esta simulacion" << endl;
			name = "report_"; //Nos encargamos de crear el nombre del futuro archivo por simulacion
			number = to_string(sims);
			name.append(number);
			name.append(".txt");
		}
		MPI_Barrier(MPI_COMM_WORLD);
		local_start = MPI_Wtime();

		healthy_people = sim.initialize(number_peopleM, infectiousnessM, chance_recoverM, death_durationM, infectedM, world_sizeM, ticM, thread_countM); //Metodo inicializador
		arch_time = sim.update(name, healthy_people); //Metodo que actualiza el mundo por tic

		local_finish = MPI_Wtime();
		local_elapsed = local_finish - local_start;
		MPI_Reduce(&local_elapsed, &elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

		if (mid == 0) {
			cout << "Tiempo transcurrido = " << elapsed << endl;
			cout << endl;
			cout << "Desea ver otra simulacion?" << endl;
			cout << "1. Si   2. No" << endl;
			cin >> new_sim;
		}
		sims++;
		name = " ";
	} while (new_sim == 1);

	if (mid == 0)
		cin.ignore();
	MPI_Barrier(MPI_COMM_WORLD); // para sincronizar la finalización de los procesos
	MPI_Finalize();
	return 0;
}

bool validateProb(double infectiousness, double chance_recover) {
	return infectiousness >= 0 && infectiousness <= 1 && chance_recover >= 0 && chance_recover <= 1;
}

bool validatePeople(int number_people) {
	return number_people >= 0 && number_people <= 10000000;
}