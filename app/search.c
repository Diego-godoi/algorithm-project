#include "search.h"

#define COUNT_COMPARISON(expr) (++(*comparisons), (expr))

int binary_search(int *v, int n, int target, comparison_count *comparisons) {
  int start = 0, mid, end = n - 1;
  while (start <= end) {
    mid = (start + end) / 2;
    if (COUNT_COMPARISON(target == v[mid]))
      return mid;
    if (COUNT_COMPARISON(target < v[mid]))
      end = mid - 1;
    else
      start = mid + 1;
  }
  return -1;
}

int ordered_sequential_search(int *v, int n, int target,
                              comparison_count *comparisons) {
  for (int i = 0; i < n; i++) {
    if (COUNT_COMPARISON(target == v[i]))
      return i;
    if (COUNT_COMPARISON(target < v[i]))
      return -1;
  }
  return -1;
}
