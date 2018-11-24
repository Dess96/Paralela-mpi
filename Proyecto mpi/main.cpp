#include<iostream>
#include<string>
#include<thread>
#include<mpi.h> 
#include<random>
#include<fstream>

using namespace std;

/* Funciones */
bool validateProb(double, double);
bool validatePeople(int);
int initialize(int, double, int);
double update(string, int);
int movePos(int, int);
bool write(int, string);
/* Funciones */

/* Variables globales */
int world_size, death_duration, tic, thread_count, number_people;
int healthy_people, dead_people, sick_people, inmune_people;
double infected, infectiousness, chance_recover;
int** world; //Tiene las personas en si con sus cuatro atributos
int** num_sick; //Numero enfermos en x y y
/* Variables globales */


int main(int argc, char * argv[]) {
	/* Ambiente mpi */
	int mid; // id de cada proceso
	int cnt_proc; // cantidad de procesos
	MPI_Status mpi_status; // para capturar estado al finalizar invocación de funciones MPI
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &mid); //El comunicador le da valor a id (rank del proceso) */
	MPI_Comm_size(MPI_COMM_WORLD, &cnt_proc); //El comunicador le da valor a p (número de procesos) 

 /* Ambiente mpi */
	unsigned n = std::thread::hardware_concurrency(); //Saca la cantidad de nucleos en la computadora
	int thread_countM = 2 * n;
	int new_sim = 1;
	int sims = 1;
	double arch_time;
	double local_start, local_finish, local_elapsed, elapsed;
	string number;
	string name = " ";

	do {
		/* Para llevar a cabo las validaciones */
		infectiousness = -1;
		chance_recover = -1;
		number_people = -1;

		while (!validateProb(infectiousness, chance_recover) || !validatePeople(number_people)) { //Pedir y validar datos
			if (mid == 0) {
				cout << "Ingrese el numero de personas en el mundo (de 0 a 10000000)" << endl;
				cin >> number_people;
				cout << "Ingrese la potencia infecciosa del mundo (decimal entre 0 y 1)" << endl;
				cin >> infectiousness;
				cout << "Ingrese la probabilidad de recuperacion (decimal entre 0 y 1)" << endl;
				cin >> chance_recover;
				cout << "Ingrese el tiempo maximo que puede vivir una persona infectada" << endl;
				cin >> death_duration;
				cout << "Ingrese el porcentaje de personas infectadas inicialmente" << endl;
				cin >> infected;
				cout << "Ingrese el tamano del espacio bidimensional" << endl;
				cin >> world_size;
				cout << "Ingrese la cantidad de tics" << endl;
				cin >> tic;
			}
		}

		if (mid == 0) {
			cout << "Se usaran " << thread_count << " hilos en esta simulacion" << endl;
			name = "report_"; //Nos encargamos de crear el nombre del futuro archivo por simulacion
			number = to_string(sims);
			name.append(number);
			name.append(".txt");
		}

		MPI_Barrier(MPI_COMM_WORLD);
		local_start = MPI_Wtime();

		healthy_people = initialize(number_people, infected, world_size); //Metodo inicializador
		arch_time = update(name, healthy_people); //Metodo que actualiza el mundo por tic

		local_finish = MPI_Wtime();
		local_elapsed = local_finish - local_start;
		MPI_Reduce(&local_elapsed, &elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

		if (mid == 0) {
			//			cout << "Tiempo transcurrido = " << elapsed - arch_time << endl;
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

/*En la matriz world
/* 0: Posicion x
/* 1: Posicion y
/* 2: Estado
/* 3: Tiempo enfermo*/
int initialize(int number_people, double infected, int world_size) {
	int mid;
	MPI_Comm_rank(MPI_COMM_WORLD, &mid); /* El comunicador le da valor a id (rank del proceso) */
	random_device rd;
	int perc;
	int healthy;
	int pos1, pos2;

	if (mid == 0) {
		world = new int*[number_people];
		for (int i = 0; i < number_people; ++i) {
			world[i] = new int[4];
		}

		num_sick = new int*[world_size]();
		for (int i = 0; i < world_size; i++) {
			num_sick[i] = new int[world_size]();
		}
	}
	sick_people = 0;
	perc = number_people * infected / 100; //Cantidad correspondiente al porcentaje dado
	healthy = number_people - perc; //Gente sana
	//Personas sanas
	for (int i = 0; i < perc; i++) { //Cambiamos a los infectados
		pos1 = rd() % world_size;
		pos2 = rd() % world_size;
		world[i][0] = pos1;
		world[i][1] = pos2;
		world[i][2] = 1;
		world[i][3] = 1;
		num_sick[pos1][pos2]++;
	}
	for (int j = perc; j < number_people; j++) {
		pos1 = rd() % world_size;
		pos2 = rd() % world_size;
		world[j][0] = pos1;
		world[j][1] = pos2;
		world[j][2] = 0;
		world[j][3] = 0;
	}
	return healthy;
}

double update(string name, int healthy) {
	int mid;
	MPI_Comm_rank(MPI_COMM_WORLD, &mid); 		/* El comunicador le da valor a id (rank del proceso) */

	double local_start, local_finish, local_elapsed, elapsed;
	double local_start2, local_finish2, local_elapsed2, elapsed2;
	double local_start3, local_finish3, local_elapsed3, elapsed3;
	double prob_infect, prob_rec;
	double out_time = 0; //Variable que toma el tiempo de salida de datos para restarlo posteriormente
	int pos1, pos2, state, x, y, sick_time, sick;
	int actual_tic = 1; //Tics actuales
	bool isSick = 0; //Indica si la persona se enfermo
	bool stable = 0; //Termina el while cuando todo se estabiliza
	healthy_people = healthy; //Inicializar nos dio el numero de sanos 
	sick_people = number_people - healthy; //Los enfermos son el resto
	random_device generator;
	uniform_real_distribution<double> distribution(0.0, 1.0);

	do {
		for (int i = 0; i < number_people; i++) {
			sick = 0;
			x = world[i][0];
			y = world[i][1];
			state = world[i][2];
			if (state == 1) {
				sick_time = world[i][3];
				if (sick_time >= death_duration) {
					prob_rec = distribution(generator); //Decidimos si la persona se enferma o se hace inmune
					if (prob_rec < chance_recover) {
						world[i][2] = 2;
						num_sick[x][y]--;
						sick_people--;
						inmune_people++;
					}
					else {
						world[i][2] = 3;
						num_sick[x][y]--;
						sick_people--;
						dead_people++;
					}
				}
				else { //Si todavia no le toca, aumentamos el tiempo que lleva enferma
					world[i][3]++;
				}
			} else if (state == 0) {
				sick = num_sick[x][y];
				for (int j = 0; j < sick; j++) { //Hacemos un for por cada enfermo en la misma posicion de la persona
					prob_infect = distribution(generator);
					if (prob_infect < infectiousness) {
						isSick = 1;
					}
				}
				if (isSick) { //Si la persona se enfermo, cambiamos su estado y empezamos el conteo de tics
					world[i][2] = 1;
					num_sick[x][y]++;
					healthy_people--;
					sick_people++;
				}
				isSick = 0;
			}
		}
		stable = write(actual_tic, name); 
		actual_tic++;
	} while (!stable && (actual_tic <= tic));

	if (mid == 0) {
		cout << std::endl;
		cout << "Archivo generado" << endl;
	}
	dead_people = sick_people = healthy_people = inmune_people = 0;
	return out_time;
}

int movePos(int pos, int world_size) {
	random_device rd;
	int movX;
	movX = rd() % 2;
	movX -= 1;
	pos += movX;
	if (pos < 0) { //Para que no se salga de la matriz
		pos = world_size - 1;
	}
	else if (pos >= world_size) {
		pos = 0;
	}
	return pos;
}

bool write(int actual_tic, string name) {
	int x, y, pos1, pos2, state;
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
		for (int i = 0; i < number_people; i++) { //Volvemos a llenar la matriz despues de haber procesado a todos en el tic anterior
			x = world[i][0];
			y = world[i][1];
			state = world[i][2];
			pos1 = movePos(x, world_size);
			pos2 = movePos(y, world_size);
			if (state == 1) {
				num_sick[x][y]--;
				num_sick[pos1][pos2]++;
			}
			world[i][0] = pos1;
			world[i][1] = pos2;
		}
	}
	return stable;
}