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

static struct kahan kahan(struct kahan k, double x){
    double y = x - k.err;
    double t = k.sum + y;
    k.err = t - k.sum - y;
    k.sum = t;
    return k;
}

static uint64_t sfc64(uint64_t s[4]){
    uint64_t r = s[0] + s[1] + s[3]++;
    s[0] = (s[1] >> 11) ^ s[1];
    s[1] = (s[2] <<  3) + s[2];
    s[2] = r + (s[2]<<24 | s[2] >>40);
    return r;
}

static double rand_GAUS(uint64_t s[4]){
    uint64_t u = sfc64(s);
    double x = __builtin_popcount(u>>32);
    x += (uint32_t)u * (1 / 4294967296.);
    x -= 16.5;
    x *= 0.3517262290563295;
    return x;
}

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


static inline float distance_3d(float x1, float y1, float z1, float x2, float y2, float z2) {
    float dx = x1 - x2;
    float dy = y1 - y2;
    float dz = z1 - z2;
    return sqrtf(dx*dx + dy*dy + dz*dz);
}


static inline ParticleFilter3D* init_PF3D(int N, 
                                         const Beacon beacons[3],
                                         float sig_meas, float sig_pro,
                                         float x_min, float x_max,
                                         float y_min, float y_max,
                                         float z_min, float z_max) {
    ParticleFilter3D *pf = malloc(sizeof(ParticleFilter3D));
    
    for(int i = 0; i < 3; i++) {
        pf->beacons[i] = beacons[i];
    }
    
    pf->sig_meas = sig_meas;
    pf->sig_pro = sig_pro;
    pf->x_min = x_min;
    pf->x_max = x_max;
    pf->y_min = y_min;
    pf->y_max = y_max;
    pf->z_min = z_min;
    pf->z_max = z_max;
    
    pf->p.count = 0;
    pf->p.capacity = 0;
    pf->p.items = NULL;


    pf->s[0] = rand();
    pf->s[1] = rand();
    pf->s[2] = rand();
    pf->s[3] = rand();

    for(int i = 0; i < N; i++) {
        Particle part;
        part.x = x_min + (float)rand()/RAND_MAX * (x_max - x_min);
        part.y = y_min + (float)rand()/RAND_MAX * (y_max - y_min);
        part.z = z_min + (float)rand()/RAND_MAX * (z_max - z_min);
        part.w = 1.0f / (float)N;
        da_append(&pf->p, part);
    }
    return pf;
}


void step_PF3D(ParticleFilter3D *pf, float d1, float d2, float d3) {
    float w_sum = 0.0f;
    
    for(size_t i = 0; i < pf->p.count; i++) {

        pf->p.items[i].x += pf->sig_pro * rand_GAUS(pf->s);
        pf->p.items[i].y += pf->sig_pro * rand_GAUS(pf->s);
        pf->p.items[i].z += pf->sig_pro * rand_GAUS(pf->s);
        

        CLAMP(pf->p.items[i].x, pf->x_min, pf->x_max);
        CLAMP(pf->p.items[i].y, pf->y_min, pf->y_max);
        CLAMP(pf->p.items[i].z, pf->z_min, pf->z_max);
        float pred_d1 = distance_3d(pf->p.items[i].x, pf->p.items[i].y, pf->p.items[i].z,
                                   pf->beacons[0].x, pf->beacons[0].y, pf->beacons[0].z);
        float pred_d2 = distance_3d(pf->p.items[i].x, pf->p.items[i].y, pf->p.items[i].z,
                                   pf->beacons[1].x, pf->beacons[1].y, pf->beacons[1].z);
        float pred_d3 = distance_3d(pf->p.items[i].x, pf->p.items[i].y, pf->p.items[i].z,
                                   pf->beacons[2].x, pf->beacons[2].y, pf->beacons[2].z);
        

        float error1 = d1 - pred_d1;
        float error2 = d2 - pred_d2;
        float error3 = d3 - pred_d3;
        

        float total_error = (error1*error1 + error2*error2 + error3*error3);
        //float total_error = 1/ (error1 + error2 + error3);
        //LOG("Total error %f\n", total_error);
        float w = expf(-0.5f * total_error / (pf->sig_meas * pf->sig_meas));
        pf->p.items[i].w = w;
        w_sum += w;
    }
    

    if (w_sum > 0) {
        for(size_t i = 0; i < pf->p.count; i++) {
            pf->p.items[i].w /= w_sum;
        }
    } else {
      
        for(size_t i = 0; i < pf->p.count; i++) {
            pf->p.items[i].w = 1.0f / pf->p.count;
        }
    }
    
    // RESAMPLING:
    const float N_Treshold =  0.9;
    if(1.0f / (w_sum*w_sum)  < N_Treshold){
	    
	    Particle_DA newp = {0};
	    float step = 1.0f / pf->p.count;
	    float r = (2*rand_GAUS(pf->s) - 0.5) * step;  //
	    float c = pf->p.items[0].w;
	    size_t i = 0;
	    
	    for(size_t m = 0; m < pf->p.count; m++) {
		float U = r + m * step;
		while(U > c && i < pf->p.count - 1) {
		    i++;
		    c += pf->p.items[i].w;
		}
		da_append(&newp, pf->p.items[i]);
		newp.items[m].w = 1.0f / pf->p.count;
 	    }
	    
	    free(pf->p.items);
	    pf->p = newp;
    }
}


static inline void estimate_PF3D(ParticleFilter3D *pf, float *x, float *y, float *z) {
    float sum_x = 0.0f, sum_y = 0.0f, sum_z = 0.0f;
    
    for(size_t i = 0; i < pf->p.count; i++) {
        sum_x += pf->p.items[i].x * pf->p.items[i].w;
        sum_y += pf->p.items[i].y * pf->p.items[i].w;
        sum_z += pf->p.items[i].z * pf->p.items[i].w;
    }
    
    *x = sum_x;
    *y = sum_y;
    *z = sum_z;
}
//Trilateration using sphere intersections
int trilaterate_sphere(const Beacon beacons[3], const float distances[3], float result[3]) {
    // Vector between beacon 1 and beacon 2
    float dx12 = beacons[1].x - beacons[0].x;
    float dy12 = beacons[1].y - beacons[0].y;
    float dz12 = beacons[1].z - beacons[0].z;
    
    // Vector between beacon 1 and beacon 3
    float dx13 = beacons[2].x - beacons[0].x;
    float dy13 = beacons[2].y - beacons[0].y;
    float dz13 = beacons[2].z - beacons[0].z;
    
    // Distance between beacon 1 and beacon 2
    float d12 = sqrtf(dx12*dx12 + dy12*dy12 + dz12*dz12);
    

    float ex_x = dx12 / d12;
    float ex_y = dy12 / d12;
    float ex_z = dz12 / d12;
    

    float i = ex_x * dx13 + ex_y * dy13 + ex_z * dz13;
    
    float ey_x = dx13 - i * ex_x;
    float ey_y = dy13 - i * ex_y;
    float ey_z = dz13 - i * ex_z;
    float ey_len = sqrtf(ey_x*ey_x + ey_y*ey_y + ey_z*ey_z);
    
    if (ey_len < 1e-6f) {
        return -1; // Beacons are colinear
    }
    
    ey_x /= ey_len;
    ey_y /= ey_len;
    ey_z /= ey_len;
    
    
    float ez_x = ex_y * ey_z - ex_z * ey_y;
    float ez_y = ex_z * ey_x - ex_x * ey_z;
    float ez_z = ex_x * ey_y - ex_y * ey_x;
    
    
    float j = ey_x * dx13 + ey_y * dy13 + ey_z * dz13;
    
    
    float x = (distances[0]*distances[0] - distances[1]*distances[1] + d12*d12) / (2*d12);
    float y = (distances[0]*distances[0] - distances[2]*distances[2] + i*i + j*j) / (2*j) - (i/j)*x;
    
    
    float z_sq = distances[0]*distances[0] - x*x - y*y;
    if (z_sq < 0) {
        z_sq = 0; // Use the plane solution
    }
    float z = sqrtf(z_sq);
    
    // Convert back to original coordinate system
    result[0] = beacons[0].x + x * ex_x + y * ey_x + z * ez_x;
    result[1] = beacons[0].y + x * ex_y + y * ey_y + z * ez_y;
    result[2] = beacons[0].z + x * ex_z + y * ey_z + z * ez_z;
    
    return 0;
}






int main() {
    srand(time(0));
    
    LOG("\t\t\t3D Particle Filter za Lokalizaciju\n");
    LOG("\n --------------------------------------------------- \n");
    
    
    Beacon beacons[3] = {
        {0.0f, 0.0f,  0.0f},    // Beacon 1
        {10.0f, 0.0f, 0.0f},   // Beacon 2  
        {5.0f, 8.66f, 0.0f}   // Beacon 3 
    };
    
    // Parametri
    const int N = 6000;         
    const float sig_pro = 0.10f; 
    const float sig_mea = 0.5f; 
    
    // Granice prostora
    const float x_min = -2.0f, x_max = 12.0f;
    const float y_min = -2.0f, y_max = 10.0f;
    const float z_min = -2.0f, z_max = 10.0f;
    
    //This shoud be calculated using triang
    float true_x = 5.0f, true_y = 4.0f, true_z = 0.0f;
    
  
    ParticleFilter3D *pf = init_PF3D(N, beacons, sig_mea, sig_pro, 
                                    x_min, x_max, y_min, y_max, z_min, z_max);
    
    const int num_steps = 5000;
    FILE *f = fopen("3d_localization.py", "w+");
    
    fprintf(f, "import numpy as np\n");
    fprintf(f, "import matplotlib.pyplot as plt\n");
    fprintf(f, "from mpl_toolkits.mplot3d import Axes3D\n\n");
    
    fprintf(f, "true_positions = [\n");
    
    LOG("Time | True pos | Estim poz | Error\n");
    LOG("--------------------------------------------------------\n");
    
    for(int t = 0; t < num_steps; t++) {
        
        if(rand_GAUS(pf->s) < 0.01){
            true_x += -0.1 + 0.2f * (float)rand() / (float)RAND_MAX;
            true_y += -0.1 + 0.2f * (float)rand() / (float)RAND_MAX;
            CLAMP(true_x, pf->x_min, pf->x_max);
            CLAMP(true_y, pf->y_min, pf->y_max);
        
        }
        
        
        float true_d1 = distance_3d(true_x, true_y, true_z, 
                                   beacons[0].x, beacons[0].y, beacons[0].z);
        float true_d2 = distance_3d(true_x, true_y, true_z, 
                                   beacons[1].x, beacons[1].y, beacons[1].z);
        float true_d3 = distance_3d(true_x, true_y, true_z, 
                                   beacons[2].x, beacons[2].y, beacons[2].z);
        
        
        float measured_d1 = true_d1 + sig_mea * rand_GAUS(pf->s);//((float)rand()/RAND_MAX - 0.5f);
        float measured_d2 = true_d2 + sig_mea * rand_GAUS(pf->s);
        float measured_d3 = true_d3 + sig_mea * rand_GAUS(pf->s);
        float trilat_pos[3];
        const float mes[] = {true_d1, true_d2, true_d3};
      


        step_PF3D(pf, measured_d1, measured_d2, measured_d3);
        
        float est_x, est_y, est_z;
        estimate_PF3D(pf, &est_x, &est_y, &est_z);
        
        float error = (distance_3d(true_x, true_y, true_z, est_x, est_y, est_z));
 	    if (trilaterate_sphere(beacons, mes, trilat_pos) == 0) {
           LOG("Sfere trilateration: (%5.2f, %5.2f, %5.2f)\n", 
                 trilat_pos[0], trilat_pos[1], trilat_pos[2]);
        }
        
        LOG("%5d | (%5.2f,%5.2f,%5.2f) | (%5.2f,%5.2f,%5.2f) | %5.3f\n", 
             t, true_x, true_y, true_z, est_x, est_y, est_z, error);
        
        fprintf(f, "    [%f, %f, %f, %f, %f, %f, %f, %f, %f],  # True position at step %d\n", true_x, true_y, true_z, 
            est_x, est_y, est_z,
            trilat_pos[0], trilat_pos[1], trilat_pos[2], t);
    }
    
    fprintf(f, "]\n\n");
    
    float final_est_x, final_est_y, final_est_z;
    estimate_PF3D(pf, &final_est_x, &final_est_y, &final_est_z);
    
    fprintf(f, "final = np.array([\n");
    for(int i = 0; i < 3; i++) {
        fprintf(f, "    [%f, %f, %f],\n", final_est_x, final_est_y, final_est_z);
    }
    fprintf(f, "])\n\n");
    fprintf(f, "plt.plot(true_positions)\n");
    fprintf(f, "plt.show()\n");   
 	
    
    fclose(f);
    
    LOG("\nRuning 3d_localization.py\n");
    system("python 3d_localization.py");
    free(pf->p.items);
    free(pf);
    
    return 0;
}
