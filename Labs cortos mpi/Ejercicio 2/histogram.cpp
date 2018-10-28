/* Archivo:      mpi_plantilla.cpp
* Propósito:   ....
*
* Compilación:   mpicxx -g -Wall -o mpi_plantilla mpi_plantilla.cpp
* Ejecución:     mpiexec -n <num_proc> ./mpi_plantilla <secuencia de valores de parámetros>
*
* Entradas:     ...
* Salidas:    ...
*
* Notas:
* 1.  bandera DEBUG produce salida detallada para depuración.
*
*/

#include <mpi.h> 
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <string>
#include<time.h>
using namespace std;

//#define DEBUG

void usage(string nombre_prog);

void obt_args(
	char*    argv[]        /* in  */,

	int&     bin_count_p   /* out */,

	float&   min_meas_p    /* out */,

	float&   max_meas_p    /* out */,

	int&     data_count_p);

void gen_data(
	float   min_meas    /* in  */,
	float   max_meas    /* in  */,
	vector<float>&   data /* out */,
	int     data_count  /* in  */);

void gen_bins(
	float min_meas      /* in  */,
	float max_meas      /* in  */,
	vector<float>& bin_maxes   /* out */,
	vector<int>&   bin_counts  /* out */,
	int   bin_count     /* in  */);

int which_bin(
	float    data         /* in */,
	vector<float>&    bin_maxes  /* in */,
	int      bin_count    /* in */,
	float    min_meas     /* in */);

void print_histo(
	vector<float>    bin_maxes   /* in */,
	vector<int>     bin_counts   /* in */,
	int      bin_count     /* in */,
	float    min_meas      /* in */);

int main(int argc, char* argv[]) {
	int bin_count, bin, block;		  // cantidad de bins, bin actual, bin == rango
	float min_meas, max_meas; // valor inferior de datos, valor superior de datos
	vector<float> bin_maxes;  // vector de m�ximos por bin
	vector<int> bin_counts;   // vector para contar valores por bin
	vector<int> local_int;
	vector<int> total_int;
	int data_count;			  // cantidad de datos
	vector<float> data;		  // vector de datos
	int mid; // id de cada proceso
	int cnt_proc; // cantidad de procesos
	MPI_Status mpi_status; // para capturar estado al finalizar invocación de funciones MPI

	/* Arrancar ambiente MPI */
	MPI_Init(&argc, &argv);             		/* Arranca ambiente MPI */
	MPI_Comm_rank(MPI_COMM_WORLD, &mid); 		/* El comunicador le da valor a id (rank del proceso) */
	MPI_Comm_size(MPI_COMM_WORLD, &cnt_proc);  /* El comunicador le da valor a p (número de procesos) */

#  ifdef DEBUG 
	if (mid == 0)
		cin.ignore();
	MPI_Barrier(MPI_COMM_WORLD);
#  endif

	/* ejecución del proceso principal */
	if (mid == 0) {
		if (argc != 5) usage(argv[0]);
	}
	obt_args(argv, bin_count, min_meas, max_meas, data_count);
	/* Allocate arrays needed */
	bin_maxes.resize(bin_count);
	bin_counts.resize(bin_count);
	local_int.resize(bin_count);
	total_int.resize(bin_count);
	data.resize(data_count);

	/* Generate the data */
	gen_data(min_meas, max_meas, data, data_count);

	/* Create bins for storing counts */
	gen_bins(min_meas, max_meas, bin_maxes, bin_counts, bin_count);

	block = data_count / cnt_proc;
	for (int i = mid * block; i < mid*block + block - 1; i++) {
		int bin = which_bin(data[i], bin_maxes, bin_count, min_meas);
		bin_counts[bin]++;
	}

#  ifdef DEBUG
	cout << "bin_counts = ";
	for (int i = 0; i < bin_count; i++)
		cout << " " << bin_counts[i];
	cout << endl;
#  endif
	cout << endl << endl;
	if (mid == 0) {
		print_histo(bin_maxes, bin_counts, bin_count, min_meas);
	}
	/* finalización de la ejecución paralela */

	if (mid == 0)
		cin.ignore();
	MPI_Barrier(MPI_COMM_WORLD); // para sincronizar la finalización de los procesos

	MPI_Finalize();
	return 0;
}  

void usage(string prog_name /* in */) {
	cerr << "usage: " << prog_name << "<bin_count> <min_meas> <max_meas> <data_count>\n" << endl;
} 

void obt_args(
	char*    argv[]        /* in  */,

	int&     bin_count_p   /* out */,

	float&   min_meas_p    /* out */,

	float&   max_meas_p    /* out */,

	int&     data_count_p) {

	bin_count_p = strtol(argv[1], NULL, 10);
	min_meas_p = strtof(argv[2], NULL);
	max_meas_p = strtof(argv[3], NULL);
	data_count_p = strtol(argv[4], NULL, 10);

#  ifdef DEBUG
	cout << "bin_count = " << bin_count_p << endl;
	cout << "min_meas = " << min_meas_p << "max_meas = " << max_meas_p << endl;
	cout << "data_count = " << data_count_p << endl;
#  endif
}  

void gen_data(
	float   min_meas    /* in  */,
	float   max_meas    /* in  */,
	vector<float>&   data /* out */,
	int     data_count  /* in  */) {

	srand(0);
	for (int i = 0; i < data_count; i++)
		data[i] = min_meas + (max_meas - min_meas)*rand() / ((double)RAND_MAX);

#  ifdef DEBUG
	cout << "data = ";
	for (int i = 0; i < data_count; i++)
		cout << " ", data[i]);
		cout << endl;
#  endif
}  

void gen_bins(
	float min_meas      /* in  */,
	float max_meas      /* in  */,
	vector<float>& bin_maxes   /* out */,
	vector<int>&   bin_counts  /* out */,
	int   bin_count     /* in  */) {
	float bin_width;

	bin_width = (max_meas - min_meas) / bin_count;

	for (int i = 0; i < bin_count; i++) {
		bin_maxes[i] = min_meas + (i + 1)*bin_width;
		bin_counts[i] = 0;
	}

#  ifdef DEBUG
	cout << "bin_maxes = ";
	for (i = 0; i < bin_count; i++)
		cout << " " << bin_maxes[i]);
		cout << endl);
#  endif
}  

int which_bin(
	float    data         /* in */,
	vector<float>&    bin_maxes  /* in */,
	int      bin_count    /* in */,
	float    min_meas     /* in */) {
	int bottom = 0, top = bin_count - 1;
	int mid;
	float bin_max, bin_min;

	while (bottom <= top) {
		mid = (bottom + top) / 2;
		bin_max = bin_maxes[mid];
		bin_min = (mid == 0) ? min_meas : bin_maxes[mid - 1];
		if (data >= bin_max)
			bottom = mid + 1;
		else if (data < bin_min)
			top = mid - 1;
		else
			return mid;
	}
	cerr << "Data = " << data << " doesn't belong to a bin!" << endl;
	cerr << "Quitting" << endl;
	exit(-1);
}  

void print_histo(
	vector<float>    bin_maxes   /* in */,
	vector<int>     bin_counts   /* in */,
	int      bin_count     /* in */,
	float    min_meas      /* in */) {
	float bin_max, bin_min;

	for (int i = 0; i < bin_count; i++) {
		bin_max = bin_maxes[i];
		bin_min = (i == 0) ? min_meas : bin_maxes[i - 1];
		printf("%.3f-%.3f:\t", bin_min, bin_max);
		for (int j = 0; j < bin_counts[i]; j++)
			cout << "X";
		cout << endl;
	}
} 