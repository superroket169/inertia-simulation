#ifndef CAR_H
#define CAR_H

#include "Entity.h"
#include "../input/Input.h"

class Car : public Entity
{
private:
    Model chassisModel; 
    Model wheelModel;   
    Vector3 wheelOffsets[4]; 
    
    float speed = 0.0f;
    float steeringAngle = 0.0f;
    float currentGas = 0.0f;     // Debug için saklayalım
    float currentBrake = 0.0f;   // Debug için saklayalım

    HardwareInput::InputManager* inputManager;

public:
    Car(HardwareInput::InputManager* inputMgr)
    {
        this->inputManager = inputMgr;
        
        // 1. GÖVDEYİ OLUŞTUR (Kırmızı Kutu)
        Mesh chassisMesh = GenMeshCube(2.0f, 1.0f, 4.0f);
        chassisModel = LoadModelFromMesh(chassisMesh);
        chassisModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = RED; // Araba Kırmızı olsun

        // 2. TEKERLEĞİ OLUŞTUR (Siyah Silindir)
        // Silindir dik oluşturulur, onu yatırmak için Transform kullanacağız
        Mesh wheelMesh = GenMeshCylinder(0.4f, 0.4f, 10);
        wheelModel = LoadModelFromMesh(wheelMesh);
        wheelModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = BLACK;
        
        // Tekerlek pozisyonları
        wheelOffsets[0] = { 1.1f, -0.2f,  1.2f}; // Ön Sol
        wheelOffsets[1] = {-1.1f, -0.2f,  1.2f}; // Ön Sağ
        wheelOffsets[2] = { 1.1f, -0.2f, -1.2f}; // Arka Sol
        wheelOffsets[3] = {-1.1f, -0.2f, -1.2f}; // Arka Sağ
        
        position = {0, 1.0f, 0}; // Arabayı havada başlat (yere gömülmesin)
    }

    void Update(float deltaTime) override
    {
        // Input Manager zaten main.cpp'de update ediliyor, burada sadece okuyoruz
        currentGas = inputManager->getGasPedal();     
        float brake = inputManager->getBrakePedal();
        float steer = inputManager->getWheelSteer(); 

        // Hareket Mantığı (Basit Test)
        speed = (currentGas - brake) * 20.0f; // Gazla git, frenle geri gel
        steeringAngle = steer * -300.0f;       // Direksiyon açısı (Ters ise - kaldır)

        // Basit Fizik (Trigonometri)
        // Araba nereye bakıyorsa (rotation.y) oraya git
        position.x += sin(rotation.y * DEG2RAD) * speed * deltaTime;
        position.z += cos(rotation.y * DEG2RAD) * speed * deltaTime;

        // Dönme (Sadece hareket ederken dönsün)
        if (abs(speed) > 0.1f) {
            rotation.y += steeringAngle * deltaTime * (speed > 0 ? 1 : -1); 
        }
    }

    void Draw() override
    {
        // Gövdeyi çiz
        // Not: Raylib koordinat sistemi Y yukarıdır.
        DrawModelEx(chassisModel, position, {0,1,0}, rotation.y, {1,1,1}, WHITE);

        // Tekerlekleri çiz
        for(int i = 0; i < 4; i++)
        {
            // Tekerleğin dünya üzerindeki tahmini konumu (Basit matematik)
            // Doğru dönmesi için Matrix kullanmak gerekir ama şimdilik görsel yeterli.
            
            // Arabanın dönüşüne göre offset'i döndür
            Vector3 rotatedOffset = Vector3RotateByAxisAngle(wheelOffsets[i], {0,1,0}, rotation.y * DEG2RAD);
            Vector3 wheelPos = Vector3Add(position, rotatedOffset);

            // Ön tekerlekler direksiyona göre dönsün
            float wheelAngle = rotation.y;
            if (i < 2) wheelAngle += steeringAngle;

            // Tekerleği 90 derece yatır (Silindir dik gelir) + Arabanın açısı
            // Raylib rotasyon sırası bazen karışıktır, basit yöntem:
            Matrix matScale = MatrixScale(1, 1, 1);

            Matrix matRotWheel = MatrixRotateZ(90 * DEG2RAD);
            Matrix matRotCar = MatrixRotateY(wheelAngle * DEG2RAD);            
            Matrix matTrans = MatrixTranslate(wheelPos.x, wheelPos.y, wheelPos.z);            
            Matrix finalTransform = MatrixMultiply(matScale, MatrixMultiply(matRotWheel, MatrixMultiply(matRotCar, matTrans)));
            
            wheelModel.transform = finalTransform;
            DrawModel(wheelModel, {0,0,0}, 1.0f, WHITE);
            
            // Transformu sıfırla ki diğer çizimler bozulmasın
            wheelModel.transform = MatrixIdentity();
        }
    }
    
    void Unload()
    {
        UnloadModel(chassisModel);
        UnloadModel(wheelModel);
    }
    
    float getSpeed() { return speed; }
};

#endif