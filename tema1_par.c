#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

char *in_filename_julia;
char *in_filename_mandelbrot;
char *out_filename_julia;
char *out_filename_mandelbrot;

pthread_barrier_t barrier;
int number_threads;
int  width, height;
 int **result;
// structura pentru un numar complex
typedef struct _complex {
	double a;
	double b;
} complex;

// structura pentru parametrii unei rulari
typedef struct _params {
	int is_julia, iterations;
	double x_min, x_max, y_min, y_max, resolution;
	complex c_julia;
} params;

params par;
int minimum(int x, int y) {
    if(x < y) {
        return x;
    } else {
        return y;
    }
}
// citeste argumentele programului
void get_args(int argc, char **argv)
{
	if (argc < 6) {
		printf("Numar insuficient de parametri:\n\t"
				"./tema1 fisier_intrare_julia fisier_iesire_julia "
				"fisier_intrare_mandelbrot fisier_iesire_mandelbrot P\n");
		exit(1);
	}

	in_filename_julia = argv[1];
	out_filename_julia = argv[2];
	in_filename_mandelbrot = argv[3];
	out_filename_mandelbrot = argv[4];
	// salvarea numarului de thread-uri
	number_threads = atoi(argv[5]);

}

// citeste fisierul de intrare
void read_input_file(char *in_filename, params* par)
{
	FILE *file = fopen(in_filename, "r");
	if (file == NULL) {
		printf("Eroare la deschiderea fisierului de intrare!\n");
		exit(1);
	}

	fscanf(file, "%d", &par->is_julia);
	fscanf(file, "%lf %lf %lf %lf",
			&par->x_min, &par->x_max, &par->y_min, &par->y_max);
	fscanf(file, "%lf", &par->resolution);
	fscanf(file, "%d", &par->iterations);

	if (par->is_julia) {
		fscanf(file, "%lf %lf", &par->c_julia.a, &par->c_julia.b);
	}

	fclose(file);
}

// scrie rezultatul in fisierul de iesire
void write_output_file(char *out_filename, int **result, int width, int height)
{
	int i, j;

	FILE *file = fopen(out_filename, "w");
	if (file == NULL) {
		printf("Eroare la deschiderea fisierului de iesire!\n");
		return;
	}

	fprintf(file, "P2\n%d %d\n255\n", width, height);
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			fprintf(file, "%d ", result[i][j]);
		}
		fprintf(file, "\n");
	}

	fclose(file);
}

// aloca memorie pentru rezultat
int **allocate_memory(int width, int height)
{
	int **result;
	int i;

	result = malloc(height * sizeof(int*));
	if (result == NULL) {
		printf("Eroare la malloc!\n");
		exit(1);
	}

	for (i = 0; i < height; i++) {
		result[i] = malloc(width * sizeof(int));
		if (result[i] == NULL) {
			printf("Eroare la malloc!\n");
			exit(1);
		}
	}

	return result;
}

// elibereaza memoria alocata
void free_memory(int **result, int height)
{
	int i;

	for (i = 0; i < height; i++) {
		free(result[i]);
	}
	free(result);
}

// ruleaza algoritmul Julia
void run_julia(int thread_id) 
{
	int w, h, i;
	int start1 = thread_id * (double) width / number_threads;
	int end1 = minimum((thread_id + 1) * (double)width / number_threads, width);
	// paralelizarea calcului multimii Julia
	for (w = start1; w < end1; w++) {
		for (h = 0; h < height; h++) {
			int step = 0;
			complex z = { .a = w * par.resolution + par.x_min,
							.b = h * par.resolution + par.y_min };

			while (sqrt(pow(z.a, 2.0) + pow(z.b, 2.0)) < 2.0 && step < par.iterations) {
				complex z_aux = { .a = z.a, .b = z.b };

				z.a = pow(z_aux.a, 2) - pow(z_aux.b, 2) + par.c_julia.a;
				z.b = 2 * z_aux.a * z_aux.b + par.c_julia.b;

				step++;
			}

			result[h][w] = step % 256;
		}
	}
	pthread_barrier_wait(&barrier);

	// recalcularea indicilor pentru paralelizarea transformarii rezultatului
	start1 = thread_id * (double) height / 2*number_threads;
	end1 = minimum((thread_id + 1) * (double)height / 2*number_threads, height/2);
	// transforma rezultatul din coordonate matematice in coordonate ecran
	for (i = start1; i < end1; i++) {
			int *aux = result[i];
			result[i] = result[height - i - 1];
			result[height - i - 1] = aux;
	}
	
	pthread_barrier_wait(&barrier);
}

// ruleaza algoritmul Mandelbrot
void run_mandelbrot(int thread_id)
{
	int w, h, i;
	int start1 = thread_id * (double) width / number_threads;
	int end1 = minimum((thread_id + 1) * (double)width / number_threads, width);
	// paralelizarea calcului multimii Mandelbord
	for (w = start1; w < end1; w++) {
		for (h = 0; h < height; h++) {
			complex c = { .a = w * par.resolution + par.x_min,
							.b = h * par.resolution + par.y_min };
			complex z = { .a = 0, .b = 0 };
			int step = 0;

			while (sqrt(pow(z.a, 2.0) + pow(z.b, 2.0)) < 2.0 && step < par.iterations) {
				complex z_aux = { .a = z.a, .b = z.b };

				z.a = pow(z_aux.a, 2.0) - pow(z_aux.b, 2.0) + c.a;
				z.b = 2.0 * z_aux.a * z_aux.b + c.b;

				step++;
			}

			result[h][w] = step % 256;
		}
	}
	pthread_barrier_wait(&barrier);
	
	// recalcularea indicilor pentru paralelizarea transformarii rezultatului
	start1 = thread_id * (double) height / 2*number_threads;
	end1 = minimum((thread_id + 1) * (double)height / 2*number_threads, height/2);
	// transforma rezultatul din coordonate matematice in coordonate ecran
	for (i = start1; i < end1; i++) {
		int *aux = result[i];
		result[i] = result[height - i - 1];
		result[height - i - 1] = aux;
	}
	pthread_barrier_wait(&barrier);
}
void *f_threads(void *arg)
{
	int thread_id = *(int *)arg;
	if(par.is_julia) {
		// se ruleaza algoritmul Julia
		run_julia(thread_id);
	}
	if (thread_id == 0) {
		// Julia: 
		//se scrie rezultatul in fisierul de iesire
		// se elibereaza memoria alocata
		write_output_file(out_filename_julia, result, width, height);
		free_memory(result, height);
		// Mandelbrot:
		// se citesc parametrii de intrare
		// se aloca tabloul cu rezultatul
		
		read_input_file(in_filename_mandelbrot, &par);
		width = (par.x_max - par.x_min) / par.resolution;
		height = (par.y_max - par.y_min) / par.resolution;
		result = allocate_memory(width, height);
	}
	// sincronizarea thread-urilor
	pthread_barrier_wait(&barrier);
	if (!par.is_julia) {
		// se ruleaza algoritmul Mandelbrot
		run_mandelbrot(thread_id);
	}
	if (thread_id == 0) {
		// Mandelbrot:
		// se scrie rezultatul in fisierul de iesire
		// se elibereaza memoria alocata
		write_output_file(out_filename_mandelbrot, result, width, height);
		free_memory(result, height);
	}
	pthread_exit(NULL);
}
int main(int argc, char *argv[])
{
	int i, r;
	void *status;

	// se citesc argumentele programului
	get_args(argc, argv);
	pthread_barrier_init(&barrier, NULL, number_threads);
	pthread_t threads[number_threads];
	int arguments[number_threads];
	// Julia:
	// - se citesc parametrii de intrare
	// - se aloca tabloul cu rezultatul
	read_input_file(in_filename_julia, &par);
	width = (par.x_max - par.x_min) / par.resolution;
	height = (par.y_max - par.y_min) / par.resolution;
	result = allocate_memory(width, height);
	// crearea thread-urilor si lansarea lor in executie
	for (i = 0; i < number_threads; i++) {
		arguments[i] = i;
		r = pthread_create(&threads[i], NULL, f_threads, &arguments[i]);

		if (r) {
			printf("Eroare la crearea thread-ului %d\n", i);
			exit(-1);
		}
	}
	// asteaptarea ca toate thread-urile sa isi termine executia
	for (i = 0; i < number_threads; i++) {
		r = pthread_join(threads[i], &status);

		if (r) {
			printf("Eroare la asteptarea thread-ului %d\n", i);
			exit(-1);
		}
	}
	
	pthread_barrier_destroy(&barrier);
	return 0;
}
