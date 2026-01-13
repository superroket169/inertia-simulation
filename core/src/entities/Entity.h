#ifndef ENTITY_H
#define ENTITY_H

#include "raylib.h"
#include "raymath.h"

class Entity
{
public:
    Vector3 position;
    Vector3 rotation; // Euler açıları (Pitch, Yaw, Roll) veya Quaternion
    Vector3 scale;
    
    // Görünürlük kontrolü
    bool isActive;

    Entity()
    {
        position = {0, 0, 0};
        rotation = {0, 0, 0};
        scale = {1, 1, 1};
        isActive = true;
    }

    virtual ~Entity() {}

    // Her frame'de çalışacak mantık (Şimdilik boş, sonra fiziği buraya bağlayacağız)
    virtual void Update(float deltaTime) = 0;

    // Çizim fonksiyonu
    virtual void Draw() = 0;

    // Transform matrisini almak için yardımcı fonksiyon (Raylib DrawModel için lazım olacak)
    Matrix GetTransformMatrix()
    {
        Matrix matScale = MatrixScale(scale.x, scale.y, scale.z);
        Matrix matRot = MatrixRotateXYZ({DEG2RAD*rotation.x, DEG2RAD*rotation.y, DEG2RAD*rotation.z});
        Matrix matTrans = MatrixTranslate(position.x, position.y, position.z);
        
        return MatrixMultiply(MatrixMultiply(matScale, matRot), matTrans);
    }
};

#endif