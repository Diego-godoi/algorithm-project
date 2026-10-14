#include "search.h"
#include "sort.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_SIZES 3

typedef struct {
  int n;
  const char *name;
  int pos; /* position for search results: -1 if not search */
  double average_time;
  comparison_count average_comparisons;
} result_t;

static void fill_random(int *v, int n) {
  for (int i = 0; i < n; i++)
    v[i] = rand();
}

static void copy_array(const int *source, int *destination, size_t n) {
  memcpy(destination, source, n * sizeof *source);
}

static void print_array(const int *v, int n) {
  const int sample = 20;

  printf("  ");
  for (int i = 0; i < n && i < sample; i++)
    printf("%d ", v[i]);

  if (n > sample)
    printf("... %d ", v[n - 1]);

  printf("[n = %d]\n", n);
}

static double elapsed_seconds(const struct timespec *start,
                              const struct timespec *end) {
  return (double)(end->tv_sec - start->tv_sec) +
         (double)(end->tv_nsec - start->tv_nsec) / 1e9;
}

/* Mantido apenas porque o quick_sort precisa de (0, n-1) em vez de apenas (n) */
static void quick_sort_adapter(int *v, int n, comparison_count *comparisons) {
  quick_sort(v, 0, n - 1, comparisons);
}

static result_t measure(void (*sort)(int *v, int n, comparison_count *comparisons),
                        const char *name, const int *source, int *work, int n) {
  const int runs = 10;
  double total_time = 0.0;
  comparison_count total_comparisons = 0;

  printf("%s:\n", name);

  for (int i = 0; i < runs; i++) {
    copy_array(source, work, n);
    comparison_count comparisons = 0;
    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);
    sort(work, n, &comparisons);
    clock_gettime(CLOCK_MONOTONIC, &end);

    double seconds = elapsed_seconds(&start, &end);
    total_time += seconds;
    total_comparisons += comparisons;

    printf("  execucao %2d: %10.6f s | %20llu comparacoes\n", i + 1, seconds,
           comparisons);
  }

  printf("  media:          %10.6f s | %20llu comparacoes\n", total_time / runs,
         total_comparisons / runs);

  result_t result = {
      .n = n,
      .name = name,
      .pos = -1,
      .average_time = total_time / runs,
      .average_comparisons = total_comparisons / runs,
  };

  return result;
}

static result_t measure_search_reps(
    int (*search)(int *v, int n, int target, comparison_count *comparisons),
    const char *name, const int *sorted_arr, int n, int target,
    int expected_pos, long long reps) {
  const int runs = 10;
  double total_time = 0.0;
  comparison_count total_comparisons = 0;

  printf("%s (repeticoes = %lld, tempo por busca):\n", name, reps);

  for (int i = 0; i < runs; i++) {
    struct timespec start, end;
    comparison_count comparisons_sum = 0;
    int found = -1;

    clock_gettime(CLOCK_MONOTONIC, &start);
    for (long long k = 0; k < reps; k++) {
      comparison_count c = 0;
      found = search((int *)sorted_arr, n, target, &c);
      comparisons_sum += c;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);

    if (found != expected_pos) {
      fprintf(stderr,
              "erro: busca por %d no vetor ordenado encontrou %d, "
              "esperado %d\n",
              target, found, expected_pos);
      exit(1);
    }

    double seconds_per_search = elapsed_seconds(&start, &end) / (double)reps;
    comparison_count comparisons_per_search =
        comparisons_sum / (comparison_count)reps;

    total_time += seconds_per_search;
    total_comparisons += comparisons_per_search;

    printf("  execucao %2d: %12.2f ns | %20llu comparacoes\n", i + 1,
           seconds_per_search * 1e9, comparisons_per_search);
  }

  printf("  media:          %12.2f ns | %20llu comparacoes\n",
         total_time / runs * 1e9, total_comparisons / runs);

  result_t r = {.n = n,
                    .name = name,
                    .pos = expected_pos,
                    .average_time = total_time / runs,
                    .average_comparisons = total_comparisons / runs};
  return r;
}

int main(void) {
  const int sizes[NUM_SIZES] = {100000, 500000, 1000000};
  result_t results[NUM_SIZES][2];
  result_t search_results[NUM_SIZES][2][4];

  const char *search_pos_label[4] = {"0", "n-1", "n/2", "n/3"};
  const char *search_names[2] = {"binary_search", "ordered_sequential_search"};

  srand(time(NULL));

  for (int i = 0; i < NUM_SIZES; i++) {
    int n = sizes[i];
    int *source = malloc(n * sizeof(*source));
    int *work = malloc(n * sizeof(*work));
    int *sorted_arr = malloc(n * sizeof(*sorted_arr));

    if (!source || !work || !sorted_arr) {
      fprintf(stderr, "falha na alocacao de memoria\n");
      free(source); free(work); free(sorted_arr);
      return 1;
    }

    fill_random(source, n);

    printf("\t\t=== n = %d (10 execucoes por algoritmo) ===\n", n);
    printf("vetor aleatorio:\n");
    print_array(source, n);

    /* Usando o adapter do quick_sort */
    results[i][0] = measure(quick_sort_adapter, "quick_sort", source, work, n);
    printf("\nvetor ordenado:\n");
    print_array(work, n);

    /* Passando heap_sort diretamente */
    results[i][1] = measure(heap_sort, "heap_sort", source, work, n);
    printf("\nvetor ordenado:\n");
    print_array(work, n);

    /* Utilizando o copy_array novamente aqui */
    copy_array(work, sorted_arr,  n);

    printf("\n\t\t=== BUSCAS (apenas a busca e cronometrada) ===\n");
    printf("vetor ordenado (base para as buscas):\n");
    print_array(sorted_arr,  n);

    int target_pos[4] = {0, n - 1, n / 2, n / 3};
    long long binary_reps = 100000;
    long long sequential_reps = 20000000 / n;
    if (sequential_reps < 1) sequential_reps = 1;

    for (int p = 0; p < 4; p++) {
      int pos = target_pos[p];
      int target = sorted_arr[pos];

      printf("\n--- alvo = vetor[%s] = vetor[%d] = %d ---\n",
             search_pos_label[p], pos, target);

      /* Passando binary_search e ordered_sequential_search diretamente */
      search_results[i][0][p] = measure_search_reps(
          binary_search, search_names[0], sorted_arr, n, target, pos, binary_reps);

      search_results[i][1][p] = measure_search_reps(
          ordered_sequential_search, search_names[1], sorted_arr, n, target, pos, sequential_reps);
    }

    printf("\n");

    free(source);
    free(work);
    free(sorted_arr);
  }

  printf("\t\t=== medias de 10 execucoes ===\n");
  printf("\t------ Algoritmos de Ordenação --------\n");
  printf("%-10s %-12s %14s %22s\n", "n", "algoritmo", "tempo (s)", "comparacoes");

  for (int i = 0; i < NUM_SIZES; i++)
    for (int j = 0; j < 2; j++)
      printf("%-10d %-12s %14.6f %22llu\n", results[i][j].n, results[i][j].name,
             results[i][j].average_time, results[i][j].average_comparisons);

  printf("\n\t------ Algoritmos de Busca --------\n");
  printf("%-10s %-26s %-8s %14s %16s %22s\n", "n", "algoritmo", "pos",
         "tempo (ns)", "tempo (s)", "comparacoes");

  for (int i = 0; i < NUM_SIZES; i++)
    for (int p = 0; p < 4; p++)
      for (int j = 0; j < 2; j++) {
        const result_t *r = &search_results[i][j][p];
        printf("%-10d %-26s %-8s %14.2f %16.6f %22llu\n", r->n, search_names[j],
               search_pos_label[p], r->average_time * 1e9, r->average_time,
               r->average_comparisons);
      }

  return 0;
}