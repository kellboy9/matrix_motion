#ifndef CALC_H
#define CALC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct point Point;
struct point {
	int x;
	int y;
};

int distance(Point a, Point b);

#endif
