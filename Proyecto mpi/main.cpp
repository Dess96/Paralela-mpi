#include<iostream>
#include<string>
#include<thread>
#include<mpi.h> 
#include<random>
#include<fstream>

using namespace std;

//#define DEBUG

/* Funciones */
void obt_args(char* argv[], int&, double&, double&, int&, int&, int&, int&);
int movePos(int, int);
bool write(int, string);
/* Funciones */

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

	int* world;
	int* rec;
	int** num_sick;
	int world_size, death_duration, tic, number_people, infected, perc, healthy, pos1, pos2, block1, sick, x, y, state, sick_time; 
	int healthy_people, sick_people, dead_people, inmune_people;
	int new_sim = 1;
	int sims = 1;
	int actual_tic = 1;
	double infectiousness, chance_recover, arch_time;
	double prob_rec, prob_infect;
	bool stable = false;
	string number;
	string name = " ";
	random_device rd;
	random_device generator;
	uniform_real_distribution<double> distribution(0.0, 1.0);

	obt_args(argv, number_people, infectiousness, chance_recover, death_duration, infected, world_size, tic);

	/* Matriz con enfermos y arreglo con personas y sus atributos*/
	num_sick = new int*[world_size];
	for (int i = 0; i < world_size; i++) {
		num_sick[i] = new int[world_size]();
	}
	world = new int[number_people * 4 / cnt_proc];
	rec = new int[number_people * 4];
	/* Matriz con enfermos y arreglo con personas y sus atributos*/

	/* Inicializacion */
	perc = number_people * infected / 100; //Cantidad correspondiente al porcentaje dado
	healthy = number_people - perc; //Gente sana
	block1 = number_people / cnt_proc;

	for (int i = mid * block1; i < mid * block1 + block1; i++) { //Cambiamos a los infectados
		if (i < perc) {
			pos1 = rd() % world_size;
			pos2 = rd() % world_size;
			world[4 * i] = pos1;
			world[4 * i + 1] = pos2;
			world[4 * i + 2] = 1;
			world[4 * i + 3] = 1;
		}
		else {
			pos1 = rd() % world_size;
			pos2 = rd() % world_size;
			world[4 * i] = pos1;
			world[4 * i + 1] = pos2;
			world[4 * i + 2] = 0;
			world[4 * i + 3] = 0;
		}
	}
	/* Inicializacion */

	/* Actualizaciones por tic */
	healthy_people = healthy;
	sick_people = number_people - healthy; //Los enfermos son el resto
	dead_people = inmune_people = 0;
	do {
		MPI_Allgather(world, number_people * 4 / cnt_proc, MPI_INT, rec, number_people * 4 / cnt_proc, MPI_INT, MPI_COMM_WORLD);
		sick = num_sick[0][0];
		cout << "do" << endl;
		actual_tic++;
	} while (!stable && (actual_tic <= tic));

	if (mid == 0) {
		name = "report_"; //Nos encargamos de crear el nombre del futuro archivo por simulacion
		number = to_string(sims);
		name.append(number);
		name.append(".txt");
	}
	/* Actualizaciones por tic */

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

