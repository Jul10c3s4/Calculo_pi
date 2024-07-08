#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

#define N 1000000000 // 1 bilhão de termos

typedef struct {
    int thread_id;
    long num_terms;
    int num_threads;
    double *results;
} thread_data_t;

void *calculate_pi(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    double sum = 0.0;
    long start = data->thread_id * data->num_terms / data->num_threads;
    long end = (data->thread_id + 1) * data->num_terms / data->num_threads;
    int Sinal = (start % 2 == 0) ? 1 : -1;

    for (long i = start; i < end; i++) {
        sum += Sinal / (2.0 * i + 1.0);
        Sinal = -Sinal;
    }

    data->results[data->thread_id] = sum;
    pthread_exit(NULL);
}

double compute_pi(int num_threads) {
    pthread_t threads[num_threads];
    thread_data_t thread_data[num_threads];
    double results[num_threads];
    double pi = 0.0;

    for (int i = 0; i < num_threads; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].num_terms = N;
        thread_data[i].num_threads = num_threads;
        thread_data[i].results = results;

        int rc = pthread_create(&threads[i], NULL, calculate_pi, (void *)&thread_data[i]);
        if (rc) {
            fprintf(stderr, "Error ao criar a thread %d\n", i);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    for (int i = 0; i < num_threads; i++) {
        pi += results[i];
    }

    return pi * 4.0;
}

double get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

int main(int argc, char *argv[]) {
    int thread_counts[] = {1, 2, 4, 8, 16, 32};
    int num_configs = sizeof(thread_counts) / sizeof(thread_counts[0]);

    for (int i = 0; i < num_configs; i++) {
        int num_threads = thread_counts[i];
        printf("Executando com %d threads:\n", num_threads);

        double times[5];
        double sum_time = 0.0;
        double sum_time_sq = 0.0;

        for (int j = 0; j < 5; j++) {
            double start_time = get_time();

            double pi = compute_pi(num_threads);

            double end_time = get_time();
            double elapsed = end_time - start_time;
            times[j] = elapsed;
            sum_time += elapsed;
            sum_time_sq += elapsed * elapsed;

            printf("Executando %d: π = %.15f, Tempo = %.6f segundos\n", j + 1, pi, elapsed);
        }

        double mean_time = sum_time / 5;
        double variance = (sum_time_sq / 5) - (mean_time * mean_time);
        double stddev = sqrt(variance);
        double coeff_var = (stddev / mean_time) * 100;

        printf("Tempo Médio = %.6f segundos, Coeficiente de Variação = %.2f%%\n\n", mean_time, coeff_var);
    }

    return 0;
}
