#define RAYGUI_IMPLEMENTATION
#include "raylib.h"
#include "raygui.h" 

#include "../src/input/Input.h" 

#include <iostream>
#include <iomanip>

int main()
{
    InitWindow(1000, 700, "Inertia Sim - Input & FFB Tester");
    SetTargetFPS(60);

    SetGamepadMappings("030000006d0400009ac2000011010000,Logitech Driving Force GT,platform:Linux,a:b0,b:b1,x:b2,y:b3,back:b8,start:b9,guide:b20,leftshoulder:b4,rightshoulder:b5,leftstick:b10,rightstick:b11,leftx:a0,lefttrigger:a1,righttrigger:a2,dpup:h0.1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,");
    SetGamepadMappings("03000000412300003680000000010000,Arduino Leonardo v1,platform:Linux,a:b0,leftx:a0,");
    SetGamepadMappings("03000000412300003680000001000000,Arduino Leonardo v2,platform:Linux,a:b0,leftx:a0,");

    clb::InputAxis dummyAxis(1.0f, -1.0f); 
    HardwareInput::InputManager inputManager(dummyAxis, dummyAxis, dummyAxis);

    float limitLeft = -1.0f;
    float limitRight = 1.0f;
    bool softLockActive = true;
    float constantForce = 0.0f;     
    bool selfCenterActive = false; 
    float vibrationStr = 0.0f;      
    float vibrationFreq = 0.5f;     
    bool showDebugText = true;

    GuiSetStyle(DEFAULT, TEXT_SIZE, 18);

    while (!WindowShouldClose())
    {
        if (IsGamepadAvailable(0))
            inputManager.updateInputs();
        
        inputManager.setWheelLimits(limitLeft, limitRight);
        
        if (selfCenterActive) inputManager.activateSelfCenter();
        else inputManager.deactivateSelfCenter();

        if (!selfCenterActive) {
            inputManager.setWheelForce(constantForce, 30);
        } else {
            inputManager.setWheelForce(0, 30);
        }

        inputManager.setWheelVibration(vibrationStr, vibrationFreq);

        // 3. FFB Güncelle
        inputManager.updateFeedBacks();

        // --- DRAWING ---
        BeginDrawing();
            ClearBackground(GetColor(0x181818FF)); 

            // SOL PANEL: GÖRSELLEŞTİRME
            // Düzeltme 1: Değişken ismini steerVal olarak kullandık
            float steerVal = inputManager.getWheelSteer(); 
            
            DrawText("STEERING WHEEL", 50, 40, 20, LIGHTGRAY);
            
            Rectangle steerBarRect = { 50, 70, 400, 40 };
            DrawRectangleRec(steerBarRect, DARKGRAY);
            DrawLine(250, 60, 250, 120, WHITE);

            int leftPixel = 250 + (int)(limitLeft * 200);
            int rightPixel = 250 + (int)(limitRight * 200);
            DrawRectangle(leftPixel, 65, 4, 50, RED);   
            DrawRectangle(rightPixel, 65, 4, 50, RED);  

            // Düzeltme 1 (Devam): Burada 'steer' yerine 'steerVal' kullanıyoruz
            int steerPixel = 250 + (int)(steerVal * 200);
            DrawRectangle(steerPixel - 5, 70, 10, 40, SKYBLUE);
            DrawText(TextFormat("%.2f", steerVal), steerPixel - 10, 115, 15, SKYBLUE);

            // PEDALLAR
            float gasVal = inputManager.getGasPedal();     
            float brakeVal = inputManager.getBrakePedal(); 

            // Gaz
            DrawText("GAS", 100, 180, 20, GREEN);
            // Düzeltme 2: &gasVal (Adres operatörü eklendi)
            GuiProgressBar((Rectangle){ 100, 210, 40, 200 }, NULL, NULL, &gasVal, 0.0f, 1.0f);
            DrawText(TextFormat("%.2f", gasVal), 105, 420, 20, GREEN);

            // Fren
            DrawText("BRAKE", 200, 180, 20, RED);
            // Düzeltme 2: &brakeVal (Adres operatörü eklendi)
            GuiProgressBar((Rectangle){ 200, 210, 40, 200 }, NULL, NULL, &brakeVal, 0.0f, 1.0f);
            DrawText(TextFormat("%.2f", brakeVal), 205, 420, 20, RED);

            // SAĞ PANEL: KONTROLLER
            int startX = 550;
            int startY = 50;

            GuiGroupBox((Rectangle){ (float)startX - 20, (float)startY - 10, 400, 550 }, "FFB SETTINGS");

            GuiLabel((Rectangle){ (float)startX, (float)startY + 20, 200, 20 }, "Soft Lock Limits");
            GuiSlider((Rectangle){ (float)startX, (float)startY + 50, 200, 20 }, "-1.0", "0.0", &limitLeft, -1.0f, 0.0f);
            GuiSlider((Rectangle){ (float)startX, (float)startY + 80, 200, 20 }, "0.0", "1.0", &limitRight, 0.0f, 1.0f);
            
            startY += 140;
            GuiLabel((Rectangle){ (float)startX, (float)startY, 200, 20 }, "Manual Force Test");
            
            GuiCheckBox((Rectangle){ (float)startX, (float)startY + 30, 20, 20 }, "Activate Self-Center", &selfCenterActive);

            if (!selfCenterActive) {
                GuiSlider((Rectangle){ (float)startX, (float)startY + 70, 200, 20 }, "Left (-1)", "Right (1)", &constantForce, -1.0f, 1.0f);
                if (GuiButton((Rectangle){ (float)startX + 220, (float)startY + 70, 100, 20 }, "RESET FORCE")) {
                    constantForce = 0.0f;
                }
            } else {
                GuiLabel((Rectangle){ (float)startX, (float)startY + 70, 300, 20 }, "Self-Center Active (Auto Force)");
            }

            startY += 140;
            GuiLabel((Rectangle){ (float)startX, (float)startY, 200, 20 }, "Vibration / Rumble");
            GuiSlider((Rectangle){ (float)startX, (float)startY + 30, 200, 20 }, "Strength", NULL, &vibrationStr, 0.0f, 1.0f);
            GuiSlider((Rectangle){ (float)startX, (float)startY + 60, 200, 20 }, "Frequency", NULL, &vibrationFreq, 1.f, 20.0f);
            
            if (GuiButton((Rectangle){ (float)startX + 220, (float)startY + 45, 100, 30 }, "STOP VIB")) {
                vibrationStr = 0.0f;
            }

            if (showDebugText) {
                DrawText("DEBUG INFO:", 50, 550, 20, YELLOW);
                DrawText(TextFormat("Limit Left: %.2f | Limit Right: %.2f", limitLeft, limitRight), 50, 580, 20, GRAY);
                DrawText(TextFormat("Applied Force: %.2f", constantForce), 50, 610, 20, GRAY);
                
                if(!IsGamepadAvailable(0)) DrawText("DIREKSIYON BULUNAMADI!", 50, 640, 20, MAROON);
                else DrawText("DIREKSIYON BAGLI", 50, 640, 20, DARKGREEN);
            }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}