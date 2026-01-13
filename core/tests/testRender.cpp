#include "raylib.h"
#include <iostream>

// Senin oluşturduğun başlık dosyaları
#include "../src/entities/Car.h"
#include "../src/input/Input.h"
#include "../src/calibration/calibration.h"

int main()
{
    // 1. Raylib Pencere Kurulumu
    const int screenWidth = 1280;
    const int screenHeight = 720;
    
    SetConfigFlags(FLAG_MSAA_4X_HINT); // Kenar yumuşatma
    InitWindow(screenWidth, screenHeight, "Raylib 3D Simulation - Real Input");

    // --- SENİN KUSURSUZ MAPPING AYARIN ---
    // Bu satır sayesinde Raylib, direksiyonun hangi tuşunun ne olduğunu anlıyor.
    SetGamepadMappings("030000006d0400009ac2000011010000,Logitech Driving Force GT,platform:Linux,a:b0,b:b1,x:b2,y:b3,back:b8,start:b9,guide:b20,leftshoulder:b4,rightshoulder:b5,leftstick:b10,rightstick:b11,leftx:a0,lefttrigger:a1,righttrigger:a2,dpup:h0.1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,");

    // 2. Kamera Kurulumu (Orbit Mod)
    Camera3D camera = { 0 };
    camera.position = (Vector3){ -6.0f, 4.0f, -6.0f }; // Arkadan bakış
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // 3. Input Sistemini Başlatma
    // Senin kodundaki gibi Dummy Axis ile başlatıyoruz, mapping işi çözüyor zaten.
    clb::InputAxis dummyAxis(1.0f, -1.0f);
    HardwareInput::InputManager inputMgr(dummyAxis, dummyAxis, dummyAxis);

    // Force Feedback Ayarları (İstersen değiştirebilirsin)
    inputMgr.setWheelLimits(-1.0f, 1.0f); // 900 derece simülasyonu için -1, 1 arası
    inputMgr.setWheelForce(0, 30);     // Sabit sertlik (Test için)
    // inputMgr.activateSelfCenter();     // İstersen kendini merkeze toplasın
    
    // 4. Arabayı Oluşturma
    Car myCar(&inputMgr);

    SetTargetFPS(60);

    // --- OYUN DÖNGÜSÜ ---
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        // A. Input İşlemleri
        if (IsGamepadAvailable(0)) 
        {
            inputMgr.updateInputs();
            inputMgr.updateFeedBacks(); // Titreşim ve sertlik gönder
        }

        // B. Araba Fiziği ve Hareketi
        myCar.Update(dt);

        // C. Kamera Takibi
        // Kamera arabanın peşinden gelsin ama yumuşak gelsin (Basit takip)
        camera.target = myCar.position;
        camera.position.x = myCar.position.x - 6.0f; // Arka mesafeyi koru (Basit versiyon)
        camera.position.z = myCar.position.z - 6.0f;
        camera.position.y = 4.0f; // Yükseklik sabit
        
        // İstersen mouse ile dönmek için bunu açabilirsin:
        // UpdateCamera(&camera, CAMERA_ORBITAL); 

        // D. Çizim
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                // Zemin (Izgara)
                DrawGrid(100, 1.0f); 
                
                // Referans Eksenler
                DrawLine3D({0,0,0}, {10,0,0}, RED);   // X
                DrawLine3D({0,0,0}, {0,0,10}, BLUE);  // Z
                
                // Arabayı Çiz
                myCar.Draw();

            EndMode3D();

            // UI Bilgileri
            DrawRectangle(10, 10, 320, 130, Fade(BLACK, 0.6f));
            DrawText("SIMULATION ACTIVE", 20, 20, 20, GREEN);
            
            if (IsGamepadAvailable(0)) {
                DrawText(TextFormat("Direksiyon: %.2f", inputMgr.getWheelSteer()), 20, 50, 10, WHITE);
                DrawText(TextFormat("Gaz: %.2f", inputMgr.getGasPedal()), 20, 70, 10, WHITE);
                DrawText(TextFormat("Fren: %.2f", inputMgr.getBrakePedal()), 20, 90, 10, WHITE);
                
                // Basit hız göstergesi
                DrawText(TextFormat("Hiz (Sanal): %.1f km/h", myCar.getSpeed() * 10.0f), 20, 120, 20, YELLOW);
            } else {
                DrawText("CIHAZ BAGLI DEGIL!", 20, 50, 20, RED);
            }

            DrawFPS(screenWidth - 100, 10);

        EndDrawing();
    }

    // Temizlik
    myCar.Unload();
    CloseWindow();

    return 0;
}