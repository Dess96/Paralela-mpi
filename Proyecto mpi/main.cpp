#include<iostream>
#include<string>
#include<thread>
#include<mpi.h> 
#include<random>
#include<fstream>

using namespace std;

#define DEBUG

/* Funciones */
void obt_args(char* argv[], int&, double&, double&, int&, int&, int&, int&);
int initialize(int, double, int, int, int);
double update(string, int);
int movePos(int, int);
bool write(int, string);
/* Funciones */

/* Variables globales */
int healthy_people, dead_people, sick_people, inmune_people;
/* Variables globales */


int main(int argc, char * argv[]) {
	int mid; // id de cada proceso
	int cnt_proc; // cantidad de procesos
	MPI_Status mpi_status; // para capturar estado al finalizar invocación de funciones MPI
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &mid); //El comunicador le da valor a id (rank del proceso) */
	MPI_Comm_size(MPI_COMM_WORLD, &cnt_proc); //El comunicador le da valor a p (número de procesos) 

#  ifdef DEBUG 
	if (mid == 0)
		cin.ignore();
	MPI_Barrier(MPI_COMM_WORLD);
#  endif

	int world_size, death_duration, tic, number_people, infected;
	int new_sim = 1;
	int sims = 1;
	double infectiousness, chance_recover;
	string number;
	string name = " ";

	obt_args(argv, number_people, infectiousness, chance_recover, death_duration, infected, world_size, tic);
	if (mid == 0) {
		name = "report_"; //Nos encargamos de crear el nombre del futuro archivo por simulacion
		number = to_string(sims);
		name.append(number);
		name.append(".txt");
	}
	healthy_people = initialize(number_people, infected, world_size, mid, cnt_proc); //Metodo inicializador
	//arch_time = update(name, healthy_people); //Metodo que actualiza el mundo por tic
	if (mid == 0) {
		cout << endl;
		cout << "Desea ver otra simulacion?" << endl;
		cout << "1. Si   2. No" << endl;
		cin >> new_sim;
	}

	if (mid == 0)
		cin.ignore();
	MPI_Barrier(MPI_COMM_WORLD); // para sincronizar la finalización de los procesos
	MPI_Finalize();

	return 0;
}

void obt_args(char* argv[], int& number_people, double& infectiousness, double& chance_recover, int& death_duration, int& infected, int& world_size, int& tic) {
	number_people = strtol(argv[1], NULL, 10); 
	infectiousness = strtol(argv[2], NULL, 10); 
	chance_recover = strtol(argv[3], NULL, 10); 
	death_duration = strtol(argv[4], NULL, 10); 
	infected = strtol(argv[5], NULL, 10);
	world_size = strtol(argv[6], NULL, 10); 
	tic = strtol(argv[7], NULL, 10); 
}

/*En la matriz world
/* 0: Posicion x
/* 1: Posicion y
/* 2: Estado
/* 3: Tiempo enfermo*/
int initialize(int number_people, double infected, int world_size, int mid, int cnt_proc) {
	random_device rd;
	int perc, healthy, pos1, pos2, block1;
	int** world = 0;
	int** num_sick = 0;
	int l, k;

	world = new int*[number_people];
	for (int i = 0; i < number_people; ++i) {
		world[i] = new int[4];
	}

	num_sick = new int*[world_size]();
	for (int i = 0; i < world_size; i++) {
		num_sick[i] = new int[world_size]();
	}

	perc = number_people * infected / 100; //Cantidad correspondiente al porcentaje dado
	healthy = number_people - perc; //Gente sana
	block1 = perc / cnt_proc;
	for (int i = mid * block1; i < mid * block1 + block1; i++) { //Cambiamos a los infectados
		pos1 = rd() % world_size;
		pos2 = rd() % world_size;
		world[i][0] = pos1;
		world[i][1] = pos2;
		world[i][2] = 1;
		world[i][3] = 1;
		num_sick[pos1][pos2]++;
	}

	for (int j = mid * block1 + perc; j < mid * block1 + block1 + perc; j++) {
		pos1 = rd() % world_size;
		pos2 = rd() % world_size;
		world[j][0] = pos1;
		world[j][1] = pos2;
		world[j][2] = 0;
		world[j][3] = 0;
	}
	return 0;
}

//double update(string name, int healthy) {
//	int mid;
//	MPI_Comm_rank(MPI_COMM_WORLD, &mid); 		/* El comunicador le da valor a id (rank del proceso) */
//	double prob_infect, prob_rec;
//	int pos1, pos2, state, x, y, sick_time, sick;
//	int actual_tic = 1; //Tics actuales
//	bool isSick = 0; //Indica si la persona se enfermo
//	bool stable = 0; //Termina el while cuando todo se estabiliza
//	healthy_people = healthy; //Inicializar nos dio el numero de sanos 
//	sick_people = number_people - healthy; //Los enfermos son el resto
//	random_device generator;
//	uniform_real_distribution<double> distribution(0.0, 1.0);
//
//	do {
//		for (int i = 0; i < number_people; i++) {
//			sick = 0;
//			x = world[i][0];
//			y = world[i][1];
//			state = world[i][2];
//			if (state == 1) {
//				sick_time = world[i][3];
//				if (sick_time >= death_duration) {
//					prob_rec = distribution(generator); //Decidimos si la persona se enferma o se hace inmune
//					if (prob_rec < chance_recover) {
//						world[i][2] = 2;
//						num_sick[x][y]--;
//						sick_people--;
//						inmune_people++;
//					}
//					else {
//						world[i][2] = 3;
//						num_sick[x][y]--;
//						sick_people--;
//						dead_people++;
//					}
//				}
//				else { //Si todavia no le toca, aumentamos el tiempo que lleva enferma
//					world[i][3]++;
//				}
//			}
//			else if (state == 0) {
//				sick = num_sick[x][y];
//				for (int j = 0; j < sick; j++) { //Hacemos un for por cada enfermo en la misma posicion de la persona
//					prob_infect = distribution(generator);
//					if (prob_infect < infectiousness) {
//						isSick = 1;
//					}
//				}
//				if (isSick) { //Si la persona se enfermo, cambiamos su estado y empezamos el conteo de tics
//					world[i][2] = 1;
//					num_sick[x][y]++;
//					healthy_people--;
//					sick_people++;
//				}
//				isSick = 0;
//			}
//		}
//		stable = write(actual_tic, name);
//		actual_tic++;
//	} while (!stable && (actual_tic <= tic));
//
//	if (mid == 0) {
//		cout << std::endl;
//		cout << "Archivo generado" << endl;
//	}
//	dead_people = sick_people = healthy_people = inmune_people = 0;
//}
//
//int movePos(int pos, int world_size) {
//	random_device rd;
//	int movX;
//	movX = rd() % 2;
//	movX -= 1;
//	pos += movX;
//	if (pos < 0) { //Para que no se salga de la matriz
//		pos = world_size - 1;
//	}
//	else if (pos >= world_size) {
//		pos = 0;
//	}
//	return pos;
//}
//
//bool write(int actual_tic, string name) {
//	int x, y, pos1, pos2, state;
//	bool stable = 0;
//	ofstream file;
//	file.open(name, ios_base::app);
//	file << "Reporte del tic " << actual_tic << endl
//		<< " Personas muertas total " << dead_people << ", promedio " << dead_people / actual_tic << ", porcentaje " << number_people * dead_people / 100 << endl
//		<< " Personas sanas total " << healthy_people << ", promedio " << healthy_people / actual_tic << ", porcentaje " << number_people * healthy_people / 100 << endl
//		<< " Personas enfermas total " << sick_people << ", promedio " << sick_people / actual_tic << ", porcentaje " << number_people * sick_people / 100 << endl
//		<< " Personas inmunes total " << inmune_people << ", promedio " << inmune_people / actual_tic << ", porcentaje " << number_people * inmune_people / 100 << endl;
//
//	file.close();//Hacer archivo
//
//	if (sick_people == 0) {
//		stable = 1;
//	}
//	else {
//		for (int i = 0; i < number_people; i++) { //Volvemos a llenar la matriz despues de haber procesado a todos en el tic anterior
//			x = world[i][0];
//			y = world[i][1];
//			state = world[i][2];
//			pos1 = movePos(x, world_size);
//			pos2 = movePos(y, world_size);
//			if (state == 1) {
//				num_sick[x][y]--;
//				num_sick[pos1][pos2]++;
//			}
//			world[i][0] = pos1;
//			world[i][1] = pos2;
//		}
//	}
//	return stable;
//}