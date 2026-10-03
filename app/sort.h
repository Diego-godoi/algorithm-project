#pragma once

typedef unsigned long long comparison_count;

void quick_sort(int *v, int start, int end, comparison_count *comparisons);

void heap_sort(int *v, int n, comparison_count *comparisons);
