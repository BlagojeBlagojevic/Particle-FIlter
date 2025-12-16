#include "particle.h"
#include "kalman.h"
#include "raylib.h"

#include<string.h>


const int width = 1200, height = 600;
uint64_t s[4];  //Used for gaus gener


static inline float distance(int rssi, int ref, float enviroment_factor){
	const double temp = (ref - rssi) / (double)(10.0f * enviroment_factor);
	return pow(10.0f, temp);
}

static inline float RSSI(int ref, int distance, float enviroment_factor){
    const float temp = ref - 10.0f * enviroment_factor * log10(distance);
    return temp;

}  



void input_select(int *what){
    if(IsKeyDown(KEY_A)){
        *what = 1;

   
    }
    else if(IsKeyDown(KEY_S)){
        *what = 2;

   

    }
    else if(IsKeyDown(KEY_D)){
        *what = 3;

   
    }
    else if(IsKeyDown(KEY_F)){
        *what = 4;

    }
    
}

void text_what_seleceted(int what){
    
    if(what == 1){
        DrawText("Beacon 1", width/4, 10, 20, WHITE);
    }
    else if(what == 2){
       DrawText("Beacon 2", width/4, 10, 20, WHITE);
    }
    else if(what == 3){
        DrawText("Beacon 3", width/4, 10, 20, WHITE);
    }
    else if(what == 4){
       DrawText("Transmiter", width/4, 10, 20, WHITE);
    }
}



void render_beacons_and_transmiter(Beacon reciver[3], Beacon transmiter, float posNoParticle[3], float posParticle[3], float distances[3]){
    
    const Color c = (Color){255, 10, 10, 25};
    DrawCircle(reciver[0].x, reciver[0].y, distances[0], c);
    DrawCircle(reciver[1].x, reciver[1].y, distances[1], c);
    DrawCircle(reciver[2].x, reciver[2].y, distances[2], c);

    DrawRectangle(transmiter.x, transmiter.y, 5, 5, BLUE);
    DrawRectangle(posNoParticle[0], posNoParticle[1], 5, 5, GREEN);
    DrawRectangle(posParticle[0], posParticle[1], 5, 5, YELLOW);
    
    DrawRectangle(reciver[0].x, reciver[0].y, 5, 5, RED);
    DrawRectangle(reciver[1].x, reciver[1].y, 5, 5, RED);
    DrawRectangle(reciver[2].x, reciver[2].y, 5, 5, RED);

}

void change_position(Beacon reciver[3], Beacon *transmiter, int what){
    if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
        const Vector2 mouse = GetMousePosition();
    
        switch (what)
        {
            case 1:
            case 2: 
            case 3:
                reciver[what-1].x = mouse.x;
                reciver[what-1].y = mouse.y;
                break;
            case 4:
                transmiter->x = mouse.x;
                transmiter->y = mouse.y;
                break;
            default:
                ASSERT("Not posible");
                break;
        }
    } 


}

void calculate_all_distances(Beacon reciver[3], Beacon transmiter, float dist[3]){
    dist[0] =  distance_3d(reciver[0].x, reciver[0].y, reciver[0].z, transmiter.x, transmiter.y, transmiter.z );
    dist[1] =  distance_3d(reciver[1].x, reciver[1].y, reciver[1].z, transmiter.x, transmiter.y, transmiter.z );
    dist[2] =  distance_3d(reciver[2].x, reciver[2].y, reciver[2].z, transmiter.x, transmiter.y, transmiter.z );

}


void render_distances(Beacon reciver[3], Beacon transmiter, float posNoParticle[3], float posParticle[3], float posNoEst[3], float dist[3], float RSSIs[3]){
    char text[100];
    int x = width*1 / 2, y = 0, inc = 20;
    memset(text, 0, 100);
    snprintf(text, 100, "Dist1(%d, %d, %d) = %.2f, RSSI = %.2f", (int)reciver[0].x, (int)reciver[0].y, (int)reciver[0].z, dist[0], RSSIs[0]);
    DrawText(text, x, y+=inc, 20, WHITE);

    memset(text, 0, 100);
    snprintf(text, 100, "Dist2(%d, %d, %d) = %.2f, RSSI = %.2f", (int)reciver[1].x, (int)reciver[1].y, (int)reciver[1].z, dist[1], RSSIs[1]);
    DrawText(text, x, y+=inc, 20, WHITE);
    
    memset(text, 0, 100);
    snprintf(text, 100, "Dist3(%d, %d, %d) = %.2f, RSSI = %.2f", (int)reciver[2].x, (int)reciver[2].y, (int)reciver[2].z, dist[2] , RSSIs[2]);
    DrawText(text, x, y+=inc, 20, WHITE);

    memset(text, 0, 100);
    snprintf(text, 100, "Real(%d, %d, %d)", (int)transmiter.x, (int)transmiter.y, (int)transmiter.z);
    DrawText(text, x, y+=inc, 20, WHITE);


    memset(text, 0, 100);
    snprintf(text, 100, "Est(%d, %d, %d)", (int)posNoEst[0], (int)posNoEst[1], (int)posNoEst[2]);
    DrawText(text, x, y+=inc, 20, WHITE);



    memset(text, 0, 100);
    snprintf(text, 100, "Est no particle(%d, %d, %d)", (int)posNoParticle[0], (int)posNoParticle[1], (int)posNoParticle[2]);
    DrawText(text, x, y+=inc, 20, WHITE);


    memset(text, 0, 100);
    snprintf(text, 100, "Est particle(%d, %d, %d)", (int)posParticle[0], (int)posParticle[1], (int)posParticle[2]);
    DrawText(text, x, y+=inc, 20, WHITE);



}



void calculate_RSSIs(float RSSIs[3], float distances[3], float RSSI1m, float envFactor){
    for(int i = 0; i < 3; i++){
        RSSIs[i] = RSSI(RSSI1m, distances[i], envFactor)  + ( -5.0f * rand_GAUS(s));
        CLAMP(RSSIs[i], -120, -40);
    }

}


int main() {
    //Change seed
    srand(time(0));
    s[0] = rand();   
    s[1] = rand();
    s[2] = rand(); 
    s[3] = rand();
    InitWindow(width, height, "Nesto");
    SetTargetFPS(10);
    Beacon reciver[3];
    memset(reciver, 0, 3*sizeof(Beacon));
    Beacon transmiter;
    memset(&transmiter, 0, sizeof(Beacon));
    float distances[3];
    int selecetForPlacment = 1;
    //reciver[0].z = 10;
    //reciver[1].z = 10;
    //reciver[2].z = 10;
    float RSSIs[3];
    Kalman1D kf[3];
    kalman1d_init(&kf[0], 0, VAR0, MESURMENT_VAR, PROCES_VAR);
    kalman1d_init(&kf[1], 0, VAR0, MESURMENT_VAR, PROCES_VAR);
    kalman1d_init(&kf[2], 0, VAR0, MESURMENT_VAR, PROCES_VAR);

    float posNoParticle[3], posParticle[3], posNoEst[3], distNo[3], disParticle[3];



    // Parametri
    const int N = 600;         
    
    const float sig_pro = 100.0f;  // ~10m/s movement uncertainty
    const float sig_mea = 30.1f;
    
    // Granice prostora
    const float x_min = 0, x_max = width;
    const float y_min = 0, y_max = height;
    const float z_min = -0.0f, z_max = 00.0f;
    
    
    
  
    ParticleFilter3D *pf = init_PF3D(N, reciver, sig_mea, sig_pro, 
                                    x_min, x_max, y_min, y_max, z_min, z_max);
    
    




    const Color b_color = (Color){0x18, 0x18, 0x18, 255};
    while(!WindowShouldClose()){
        BeginDrawing();
           ClearBackground(b_color);
           input_select(&selecetForPlacment);
           change_position(reciver, &transmiter, selecetForPlacment);
           text_what_seleceted(selecetForPlacment);
           render_beacons_and_transmiter(reciver, transmiter, posNoParticle, posParticle, disParticle);
           calculate_all_distances(reciver, transmiter, distances);
           calculate_RSSIs(RSSIs, distances, -59, 2.01f);
           distNo[0] = distance(RSSIs[0], -59, 2.01f);
           distNo[1] = distance(RSSIs[1], -59, 2.01f);
           distNo[2] = distance(RSSIs[2], -59, 2.01f);
           trilaterate_sphere(reciver, distNo, posNoEst);

           kalman1d_update(&kf[0], RSSIs[0]);
           RSSIs[0] = kf[0].x;
           distances[0] = distance(RSSIs[0], -59, 2.01f);
           
           kalman1d_update(&kf[1], RSSIs[1]);
           RSSIs[1] = kf[1].x;
           distances[1] = distance(RSSIs[1], -59, 2.01f);
           
           kalman1d_update(&kf[2], RSSIs[2]);
           RSSIs[2] = kf[2].x;
           distances[2] = distance(RSSIs[2], -59, 2.01f);
           
           disParticle[0] = distances[0];
           disParticle[1] = distances[1];
           disParticle[2] = distances[2];
           trilaterate_sphere(reciver, distances, posNoParticle);
           CLAMP(posNoParticle[0], 0, width);
           CLAMP(posNoParticle[1], 0, height);
           //CLAMP(posNoParticle[2], -2, 0);
           CLAMP(posParticle[0], 0, width);
           CLAMP(posParticle[1], 0, height);
           //CLAMP(posParticle[2], -2, 0);
                   


           for(int i = 0; i < 1000; i++){
                step_PF3D(pf, disParticle[0], disParticle[1], disParticle[2]);
                
                
           }
           estimate_PF3D(pf, &posParticle[0], &posParticle[1], &posParticle[2]);
           trilaterate_sphere(reciver, disParticle, posParticle);
           render_distances(reciver, transmiter, posNoParticle, posParticle, posNoEst, distances, RSSIs);
        
        EndDrawing();
    }


    return 0;
}
