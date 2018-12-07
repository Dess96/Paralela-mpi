#include<iostream>
#include<sstream>
#include<string>
#include<mpi.h> 
#include<random>
#include<fstream>

using namespace std;

/* Funciones */
static void obt_args(char* argv[], int&, int&, int&, int&, int&, int&);
static int validate(int, int, int, int, int, int);
static void initialize(int, int, int, int*, int);
static int simulate(int*, int, int, int*, int, int*, int*, int, double, double, int*, int, string);
static bool write(int&, string&, int&, int*, int&, int&, int&, int*);
/* Funciones */

/* En world: Primer atributo x, Segundo atributo y, Tercer atributo estado y Cuarto atributo tiempo enfermo */
/* En las personas: Estado 0 es estar sano, Estado 1 es estar enfermo, Estado 2 es ser inmune y Estado 3 es estar muerto igual en el vector variables*/
int main(int argc, char * argv[]) {
	int mid; // id de cada proceso
	int cnt_proc; // cantidad de procesos
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &mid); //El comunicador le da valor a id (rank del proceso) */
	MPI_Comm_size(MPI_COMM_WORLD, &cnt_proc); //El comunicador le da valor a p (número de procesos)
	/* Buffers y estructuras de datos */
	int* world;
	int* rec;
	int* variables;
	int* rec_var;
	int* num_sick;
	/* Parametros y variables utilizadas en la simulacion */
	int world_size, death_duration, number_people, infected, chance, infect, correct, actual_tic;
	double infectiousness, chance_recover, local_start, local_finish, local_elapsed, elapsed;
	bool stable = false;
	string number;
	string name = " ";

	obt_args(argv, number_people, infect, chance, death_duration, infected, world_size);

	if (mid == 0) {
		correct = validate(number_people, infect, chance, death_duration, infected, world_size);
	}
	MPI_Bcast(&correct, 1, MPI_INT, 0, MPI_COMM_WORLD); //Hacemos un Bcast para que los demas procesos sepan cuando no se debe correr el programa
	if (correct == -1) { //Si algun parametro es incorrecto, el programa termina
		if (mid == 0) {
			cout << "El programa no inicio correctamente. Presione cualquier tecla para salir del programa" << endl;
			cin.ignore();
		}
		MPI_Barrier(MPI_COMM_WORLD);
		MPI_Finalize();
		return -1;
	}
	else {
		infectiousness = (double)infect / 100; //Sacar probabilidad a partir de los enteros ingresados
		chance_recover = (double)chance / 100;

		name = "report_"; //Nos encargamos de crear el nombre del futuro archivo por simulacion
		number = to_string(1);
		name.append(number);
		name.append(".txt");

		num_sick = new int[world_size*world_size]; //Contendra la cantidad de enfermos. Cada proceso tiene la matriz completa
		world = new int[number_people * 4 / cnt_proc]; //Tendra a las personas y sus cuatro "atributos": x, y, estado, tiempo enfermo. Cada proceso tiene una parte
		rec = new int[number_people * 4]; //Buffer para enviar world
		variables = new int[4](); //Tendra las variables con la cantidad de personas sanas, enfermas, inmunes y muertas
		rec_var = new int[4]; //Buffer para enviar variables

		/* Inicializacion */
		initialize(number_people, cnt_proc, world_size, world, infected);
		MPI_Barrier(MPI_COMM_WORLD);
		local_start = MPI_Wtime();
		/* Simulacion */
		actual_tic = simulate(world, number_people, cnt_proc, rec, world_size, num_sick, variables, death_duration, infectiousness, chance_recover, rec_var, mid, name);
		local_finish = MPI_Wtime();
		local_elapsed = local_finish - local_start;
		MPI_Reduce(&local_elapsed, &elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

		if (mid == 0) {
			cout << "Tiempo transcurrido en total = " << elapsed << endl;
			cout << "Tiempo transcurrido por tic = " << elapsed / actual_tic << endl;
			cout << "La simulacion ha terminado." << endl;
		}
	}
	if (mid == 0) {
		cin.ignore();
	}
	delete[] rec; //Liberar memoria de buffers y estructuras de datos
	delete[] variables;
	delete[] rec_var;
	delete[] world;
	MPI_Barrier(MPI_COMM_WORLD); // para sincronizar la finalización de los procesos
	MPI_Finalize();

	return 0;
}

void obt_args(char* argv[], int& number_people, int& infect, int& chance, int& death_duration, int& infected, int& world_size) { //Metodo que llena parametros
	number_people = strtol(argv[1], NULL, 10);
	infect = strtol(argv[2], NULL, 10);
	chance = strtol(argv[3], NULL, 10);
	death_duration = strtol(argv[4], NULL, 10);
	infected = strtol(argv[5], NULL, 10);
	world_size = strtol(argv[6], NULL, 10);
}

int validate(int number_people, int infect, int chance, int death_duration, int infected, int world_size) { //Hace las validaciones de cada parametro
	int correct = -1;
	if (number_people < 0 || number_people > 10000000) {
		cout << "La cantidad de personas deberia estar entre 0 y 10 000 000. La cantidad ingresada es invalida." << endl;
	} if (infect < 0 || infect > 100) {
		cout << "La probabilidad de infeccion de personas deberia estar entre 0 y 100 por ciento. La cantidad ingresada es invalida." << endl;
	} if (chance < 0 || chance > 100) {
		cout << "La probabilidad de recuperacion de personas deberia estar entre 0 y 100 por ciento. La cantidad ingresada es invalida." << endl;
	} if (death_duration < 5 || death_duration > 50) {
		cout << "La duracion de muerte de personas deberia estar entre 5 y 50. La cantidad ingresada es invalida." << endl;
	} if (infected < 0 || infected > 10) {
		cout << "La cantidad de personas inicialmente infectadas deberia estar entre 0 y 10 por ciento. La cantidad ingresada es invalida." << endl;
	} if (world_size != 100 && world_size != 500 && world_size != 1000) {
		cout << "El tamano del mundo debe ser 100 x 100, 500 x 500 o 1000 x 1000. La cantidad ingresada es invalida." << endl;
	}
	else {
		correct = 0;
	}
	return correct;
}

void initialize(int number_people, int cnt_proc, int world_size, int* world, int infected) {
	random_device rd;
	int x, y;
	int block = number_people / cnt_proc; //Bloque que le tocara a cada proceso
	int perc = number_people * infected / 100; //Cantidad correspondiente al porcentaje dado
	perc /= cnt_proc;
	for (int i = 0; i < block; ++i) { //Cambiamos a los infectados
		if (i < perc) {
			x = rd() % world_size;
			y = rd() % world_size;
			world[4 * i] = x;
			world[4 * i + 1] = y;
			world[4 * i + 2] = 1;
			world[4 * i + 3] = 1;
		}
		else { //Y a los sanos
			x = rd() % world_size;
			y = rd() % world_size;
			world[4 * i] = x;
			world[4 * i + 1] = y;
			world[4 * i + 2] = 0;
			world[4 * i + 3] = 0;
		}
	}
}

int simulate(int* world, int number_people, int cnt_proc, int* rec, int world_size, int* num_sick, int* variables, int death_duration, double infectiousness, double chance_recover, int* rec_var, int mid, string name) {
	random_device rd;
	uniform_real_distribution<double> distribution(0.0, 1.0);
	int x, y, state, index, sick_time, sick, movX;
	int actual_tic = 1;
	int block = number_people / cnt_proc; //Bloque que le tocara a cada proceso
	double prob_rec, prob_infect;
	bool stable = false;
	do {
		variables[0] = variables[1] = variables[2] = variables[3] = 0; //Arreglo que lleva la cuenta de las variables
		if (rec_var[0] != 0) {
			MPI_Allgather(world, number_people * 4 / cnt_proc, MPI_INT, rec, number_people * 4 / cnt_proc, MPI_INT, MPI_COMM_WORLD); //Hacemos que todos los procesos sepan lo que hay en world
			for (int i = world_size * world_size; i >= 0; --i) { //Hay que limpiar lo que dejamos en num_sick en el tic anterior
				num_sick[i] = 0;
			}
			for (int i = number_people; i >= 0; --i) { //Lo llenamos a partir del buffer del Allgather
				x = rec[4 * i];
				y = rec[4 * i + 1];
				state = rec[4 * i + 2];
				if (state == 1) {
					index = x * world_size + y;
					++num_sick[index];
				}
			}
		}
		//Este ciclo lleva a cabo los cambios de estado, posicion y conteos
		for (int i = 0; i < block; ++i) {
			sick = 0;
			x = world[4 * i];
			y = world[4 * i + 1];
			state = world[4 * i + 2];
			if (state == 1) {
				++variables[1]; //Aumentamos a enfermos
				sick_time = world[4 * i + 3];
				if (sick_time >= death_duration) {
					prob_rec = distribution(rd); //Decidimos si la persona se enferma o se hace inmune
					if (prob_rec < chance_recover) {
						world[4 * i + 2] = 2; //Cambiamos estado segun corresponda
					}
					else {
						world[4 * i + 2] = 3;
					}
				}
				else { //Si todavia no le toca, aumentamos el tiempo que lleva enferma
					world[4 * i + 3]++;
				}
			}
			else if (state == 2) {
				++variables[2]; //Aumentamos inmunes
			}
			else if (state == 3) {
				++variables[3]; //Aumentamos muertos
			}
			if (rec_var[0] != 0) { //Si ya no hay sanos no vale la pena siquiera meterse al if
				if (state == 0) {
					++variables[0]; //Aumentamos sanos
					index = x * world_size + y; //Este sera el indice en el arreglo simulando la matriz de enfermos
					sick = num_sick[index];
					for (int j = 0; j < sick; ++j) { //Hacemos un for por cada enfermo en la misma posicion de la persona
						prob_infect = distribution(rd);
						if (prob_infect < infectiousness) {
							world[4 * i + 2] = 1;
							world[4 * i + 3] = 1;
						}
					}
				}
			}
			/* Cambios de posicion x */
			movX = rd() % 2; //Rand de "-1 a 1"
			movX -= 1;
			x += movX;
			if (x < 0) { //Para que no se salga de la matriz
				x = world_size - 1;
			}
			else if (x >= world_size) {
				x = 0;
			}
			/* Cambios de posicion y */
			movX = rd() % 2; //Rand de "-1 a 1"
			movX -= 1;
			y += movX;
			if (y < 0) { //Para que no se salga de la matriz
				y = world_size - 1;
			}
			else if (y >= world_size) {
				y = 0;
			}
			world[4 * i] = x;
			world[4 * i + 1] = y;
		}
		MPI_Allreduce(variables, rec_var, 4, MPI_INT, MPI_SUM, MPI_COMM_WORLD); //Hacemos reduce de cada variable
		stable = write(actual_tic, name, number_people, world, world_size, mid, cnt_proc, rec_var); //Crear archivo y escribir en la consola
		++actual_tic;
	} while (!stable);
	return actual_tic;
}

bool write(int& actual_tic, string& name, int& number_people, int* world, int& world_size, int& mid, int& cnt_proc, int* rec_var) { //Hace archivo e imprime en consola
	stringstream ss;
	int healthy_people = rec_var[0]; //Obtenemos datos del buffer del reduce
	int sick_people = rec_var[1];
	int inmune_people = rec_var[2];
	int dead_people = rec_var[3];
	bool stable = 0;
	if (mid == 0) {
		ss << "Reporte del tic " << actual_tic << endl
			<< " Personas muertas total " << dead_people << ", promedio " << (dead_people) / actual_tic << ", porcentaje " << 100 * (dead_people) / number_people << endl
			<< " Personas sanas total " << healthy_people << ", promedio " << (healthy_people) / actual_tic << ", porcentaje " << 100 * (healthy_people) / number_people << endl
			<< " Personas enfermas total " << sick_people << ", promedio " << (sick_people) / actual_tic << ", porcentaje " << 100 * (sick_people) / number_people << endl
			<< " Personas inmunes total " << inmune_people << ", promedio " << (inmune_people) / actual_tic << ", porcentaje " << 100 * (inmune_people) / number_people << endl;
		ofstream file;
		file.open(name, ios_base::app);
		file << ss.str() << endl;
		file.close(); //Hacer archivo

		cout << ss.str() << endl;
	}
	if (sick_people == 0) {
		stable = 1;
	}
	return stable;
}
