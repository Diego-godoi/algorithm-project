#pragma once

typedef unsigned long long comparison_count;

int binary_search(int *v, int n, int target, comparison_count *comparisons);

int ordered_sequential_search(int *v, int n, int target,
                              comparison_count *comparisons);
