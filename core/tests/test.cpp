#include "raylib.h"
#include "linux_ff.h"
#include <iostream>
#include <string>

int main()
{
    InitWindow(800, 450, "Raylib - DFGT Force Feedback");

    // Mapping - Driving Force GT
    SetGamepadMappings("030000006d0400009ac2000011010000,Logitech Driving Force GT,platform:Linux,a:b0,b:b1,x:b2,y:b3,back:b8,start:b9,guide:b20,leftshoulder:b4,rightshoulder:b5,leftstick:b10,rightstick:b11,leftx:a0,lefttrigger:a1,righttrigger:a2,dpup:h0.1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,");
    
    SetTargetFPS(60);

    RaylibLinuxFF ff;
    
    bool ffAktif = ff.Init("/dev/input/by-id/usb-046d_Driving_Force_GT-event-joystick");

    while (!WindowShouldClose())
    {
        // 1. INPUT
        float steer = 0.0f;
        if (IsGamepadAvailable(0))
        {
            steer = GetGamepadAxisMovement(0, 0); // Direksiyon (-1.0 ile 1.0)
        }

        if (ffAktif)
        {
            if(steer < -0.5) ff.SetForce(-6.5f, 50);
            else ff.SetForce(0, 50);
        }
        else ff.SetForce(0, 50);

        BeginDrawing();
            ClearBackground(RAYWHITE);

            DrawText("FORCE FEEDBACK TEST", 250, 20, 30, DARKGRAY);

            if (ffAktif)
            {
                DrawText("FF DURUMU: AKTIF (event15 baglandi)", 50, 100, 20, GREEN);
            }
            else
            {
                DrawText("FF DURUMU: HATA!", 50, 100, 20, RED);
            }

            DrawText(TextFormat("Direksiyon Acisi: %.2f", steer), 50, 150, 20, BLACK);
            
            DrawRectangle(50, 200, 400, 40, LIGHTGRAY);
            DrawRectangle(50 + (int)((steer + 1.0f) * 200), 200, 10, 40, RED);

        EndDrawing();
    }

    ff.Close();
    CloseWindow();
    return 0;
}