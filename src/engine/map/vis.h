#ifndef BSPVIS_H
#define BSPVIS_H

typedef struct bspnode_t bspnode_t;

typedef struct visportal_t
{
    int planenum;
    bspnode_t *fronts;
    bspnode_t *backs;
} visportal_t;

#endif
