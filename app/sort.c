#include "sort.h"

#define COUNT_COMPARISON(expr) (++(*comparisons), (expr))

static void heapify(int *v, int n, int i, comparison_count *comparisons) {
  int largest = i;
  int left = 2 * i + 1;
  int right = 2 * i + 2;
  if (left < n && COUNT_COMPARISON(v[left] > v[largest]))
    largest = left;

  if (right < n && COUNT_COMPARISON(v[right] > v[largest]))
    largest = right;

  if (largest != i) {
    int temp = v[i];
    v[i] = v[largest];
    v[largest] = temp;

    heapify(v, n, largest, comparisons);
  }
}

static int partition(int *v, int start, int end, comparison_count *comparisons) {
  int left = start, right = end, pivot = v[start], temp;
  while (left < right) {
    while (left <= end && COUNT_COMPARISON(v[left] <= pivot)) {
      left++;
    }
    while (right >= 0 && COUNT_COMPARISON(v[right] > pivot)) {
      right--;
    }
    if (left < right) {
      temp = v[left];
      v[left] = v[right];
      v[right] = temp;
    }
  }
  v[start] = v[right];
  v[right] = pivot;
  return right;
}

void quick_sort(int *v, int start, int end, comparison_count *comparisons) {
  if (start > end)
    return;

  int pivot = partition(v, start, end, comparisons);
  quick_sort(v, start, pivot - 1, comparisons);
  quick_sort(v, pivot + 1, end, comparisons);
}

void heap_sort(int *v, int n, comparison_count *comparisons) {
  for (int i = n / 2 - 1; i >= 0; i--)
    heapify(v, n, i, comparisons);

  for (int i = n - 1; i > 0; i--) {
    int temp = v[0];
    v[0] = v[i];
    v[i] = temp;

    heapify(v, i, 0, comparisons);
  }
}
