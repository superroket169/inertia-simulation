#ifndef LINUX_FF_H
#define LINUX_FF_H

// linux kütühaneleri
#include <fcntl.h>
#include <unistd.h> 
#include <sys/ioctl.h>
#include <linux/input.h>

#include <iostream>
#include <cstring>
#include <cmath>

class RaylibLinuxFF
{
    int fd = -1;
    struct ff_effect effect;
    bool isInit = false;

public:
    RaylibLinuxFF()
    {
        // kontrol için -1 default value
        memset(&effect, 0, sizeof(effect));
        effect.id = -1; 
    }

    // init
    bool Init(const char* path)
    {
        // hem okuma hem yazma (O_RDWR)
        fd = open(path, O_RDWR);
        
        if (fd == -1)
        {
            std::cerr << "dosya açilamadi: " << path << std::endl;
            perror("Erişim reddi : ");
            return false;
        }

        std::cout << "cihaza hağlandi, başarili : " << path << std::endl;
        
        // eski efektleri temizleme
        ioctl(fd, EVIOCRMFF, -1);

        isInit = true;
        return true;
    }

    // güç ayarlar
    void SetForce(float force, unsigned short replayLenght)
    {
        if (!isInit || fd == -1) return;

        // Güvenlik: clamp
        if (force > 1.0f) force = 1.0f;
        if (force < -1.0f) force = -1.0f;

        // force ayarları
        effect.type = FF_CONSTANT;

        // yön Ayarı: sağa : 90(0x4000) / sola : 270(0xC000)
        // tam tersi de olabilir
        effect.direction = (force >= 0) ? 0x4000 : 0xC000; 

        // güç ayarı yapılır
        effect.u.constant.level = (short)(std::abs(force) * 0x7FFF);
        
        // Süre Ayarı:
        // replayLentgh kadar süreyi dosyaya yazar : 
        effect.replay.length = replayLenght;
        effect.replay.delay = 0;

        // yüklenemezse hata alır
        if (ioctl(fd, EVIOCSFF, &effect) == -1)
        {
            perror("FF Upload Error");
            return;
        }

        // oynatma / dosyaya yazdırma kısmı : 
        struct input_event play;
        play.type = EV_FF;
        play.code = effect.id;
        play.value = 1;

        if (write(fd, &play, sizeof(play)) == -1)
        {
            perror("FF Play Error");
        }
    }

    // herşeyi şiit
    void Close()
    {
        if (fd != -1)
        {
            // efekti sil ve kapat
            ioctl(fd, EVIOCRMFF, effect.id);
            close(fd);
            std::cout << "Cihaz baglantisi kesildi." << std::endl;
        }
    }
};

#endif