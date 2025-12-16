#ifndef _PARTICLE_H
#define _PARTICLE_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#define WARNING(...)        fprintf(stdout, __VA_ARGS__)
#define ERROR_BREAK(...)    fprintf(stderr, __VA_ARGS__); exit(-1)
#define LOG(...)            fprintf(stdout, __VA_ARGS__)
#define CLAMP(X, LOW, HIGH) {if((X) < (LOW)) (X) = (LOW); if((X) > (HIGH)) (X) = (HIGH);}
#define ASSERT(msg) {fprintf(stderr, "assert in:\n\tFILE %s\n\tLINE %d\n\tmsg: %s" , __FILE__, __LINE__, msg); exit(-1);}
#define DROP(var) {(void)var;}

#define DA_SIZE 64
#define da_append(da, item)                                                            \
    do {                                                                                 \
        if ((da)->count >= (da)->capacity) {                                               \
            (da)->capacity = (da)->capacity == 0 ? DA_SIZE : (da)->capacity*2;               \
            (da)->items = realloc((da)->items, (da)->capacity*sizeof(*(da)->items));         \
            assert((da)->items != NULL && "Realloc fail !!!");                               \
        }                                                                                \
        (da)->items[(da)->count++] = (item);                                               \
    } while (0)

struct kahan { double sum, err; };


//gcc ver for anon struct



typedef struct {
    float x, y, z;  
    float w;        
} Particle;

typedef struct {
    size_t count;
    size_t capacity;
    Particle* items;
} Particle_DA;


typedef struct {
    float x, y, z;   
} Beacon;

typedef struct {
    Particle_DA p;
    Beacon beacons[3];  
    float sig_meas;     
    float sig_pro;      
    float x_min, x_max; 
    float y_min, y_max;
    float z_min, z_max;
    uint64_t s[4];      
} ParticleFilter3D;





//Func interfaces 
//RAND gaus
double rand_GAUS(uint64_t s[4]);


//CALC dist in 3dim 
float distance_3d(float x1, float y1, float z1, float x2, float y2, float z2);

//setup basic filter
ParticleFilter3D* init_PF3D(int N, 
                                         const Beacon beacons[3],
                                         float sig_meas, float sig_pro,
                                         float x_min, float x_max,
                                         float y_min, float y_max,
                                         float z_min, float z_max);


                                       
//ONE time run of PF                                         
void step_PF3D(ParticleFilter3D *pf, float d1, float d2, float d3);

//get POS estim 
void estimate_PF3D(ParticleFilter3D *pf, float *x, float *y, float *z);

//Trilat using sphere intersect 
int trilaterate_sphere(const Beacon beacons[3], const float distances[3], float result[3]);






#endif