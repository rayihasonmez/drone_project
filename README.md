# Emergency Drone Coordination System

**Acil durumlarda droneların, hayatta kalanları (survivor) bulup kurtarmasını sağlayan, gerçek zamanlı ve çoklu-thread destekli bir koordinasyon sistemi.**  
Bu proje, ODTÜ CENG 334/583 "Distributed Systems" dersi için geliştirilmiştir.

---

## 🚁 Proje Hakkında

Bu sistem, gerçek dünyadaki drone sürülerinin (swarm) acil yardım operasyonlarını modellemek için tasarlanmıştır.  
Amaç, merkezi bir sunucu ile haberleşen çok sayıda drone’un, rastgele ortaya çıkan survivor’lara en hızlı şekilde yardım etmesini sağlamak ve tüm süreci gerçek zamanlı olarak görselleştirmektir.

---

## 🔑 Temel Özellikler

- **Çoklu Thread & Senkronizasyon:**  
  - Survivor ve drone listeleri thread-safe olarak yönetilir (mutex ile korunur).
  - Her drone ve survivor kendi thread’inde çalışır (Phase 1).
- **Gerçek Zamanlı Görselleştirme:**  
  - SDL2 ile harita üzerinde canlı drone ve survivor hareketleri.
  - Görev atamaları, kurtarma animasyonları ve istatistik paneli.
- **Gerçek Client-Server Mimarisi (Phase 2):**  
  - Dronelar ayrı process (client) olarak çalışır ve TCP ile sunucuya bağlanır.
  - JSON tabanlı mesajlaşma protokolü.
- **AI Destekli Görev Atama:**  
  - En eski yardımsız survivor’a en yakın idle drone atanır.
- **Kolay Genişletilebilirlik:**  
  - Kod modülerdir, yeni özellikler ve fazlar kolayca eklenebilir.

---

## 🚦 Kurulum

### Gereksinimler

- GCC veya uyumlu bir C derleyicisi
- SDL2 ve SDL2_ttf kütüphaneleri
- json-c kütüphanesi

#### Ubuntu/Debian için:
```sh
sudo apt-get install build-essential libsdl2-dev libsdl2-ttf-dev libjson-c-dev
```

---

## ⚙️ Derleme

```sh
gcc -Wall -Wextra -pedantic -std=c11 -g `sdl2-config --cflags` \
    -o server server.c list.c drone.c survivor.c ai.c map.c view.c \
    `sdl2-config --libs` -lSDL2_ttf -lpthread -lm -ljson-c

gcc -Wall -Wextra -pedantic -std=c11 -g `sdl2-config --cflags` \
    -o client client.c \
    `sdl2-config --libs` -lSDL2_ttf -lpthread -lm -ljson-c
```

---

## 🚀 Çalıştırma

### Sunucu (Server)
```sh
./server
```
Sunucu başlatılır ve görsel arayüz açılır.

### Drone (Client)
Farklı terminallerde, her biri farklı bir drone_id ile:
```sh
./client D1
./client D2
./client D3
```
Her client, sunucuya bağlanır ve konum/görev güncellemelerini gönderir.

---

## 🗺️ Arayüz ve Kullanım

- **Kırmızı Nokta:** Survivor (yardım bekliyor)
- **Mavi Daire:** Drone (idle veya görevde)
- **Yeşil Çizgi:** Drone aktif olarak bir survivor’a gidiyor
- **Gri Survivor:** Kurtarıldıktan sonra kısa süre ekranda kalan kişi
- **İstatistik Paneli:** Toplam drone, survivor ve renklerin anlamı

---

## 🧩 Kod Yapısı

- `server.c` : Sunucu ana döngüsü, bağlantı yönetimi, survivor üretimi
- `client.c` : Drone client kodu, sunucuya bağlanma ve mesajlaşma
- `list.c/.h` : Thread-safe generic bağlı liste (Phase 1 için)
- `drone.c/.h` : Drone veri yapısı ve davranışı
- `survivor.c/.h` : Survivor üretimi ve yönetimi
- `ai.c/.h` : AI görev atama mantığı
- `view.c/.h` : SDL2 ile görselleştirme
- `common.h` : Ortak veri yapıları

---

## 🔄 Fazlar ve Gelişim

- **Phase 1:** Thread-safe listeler, simüle edilmiş drone ve survivor thread’leri
- **Phase 2:** Gerçek socket tabanlı client-server mimarisi, JSON mesajlaşma
- **Phase 3:** Yük testi, fault tolerance, performans loglama, isteğe bağlı web dashboard

---

## 📡 İletişim Protokolü

- **Drone → Server:**  
  - STATUS_UPDATE: `{ "drone_id": "D1", "status": "idle", "location": [10, 20] }`
  - MISSION_COMPLETE: `{ "drone_id": "D1", "type": "mission_complete" }`
- **Server → Drone:**  
  - ASSIGN_MISSION: `{ "type": "mission", "target": [x, y] }`
  - HEARTBEAT: `{ "type": "heartbeat" }`

---

## 👩‍💻 Katkı ve Lisans

Bu proje eğitim amaçlıdır.  
Katkı sağlamak veya öneri sunmak için Pull Request gönderebilirsiniz.

---

**Hazırlayanlar:**  
Rayiha Sönmez, Ayşe Betül Güneş, Gülten Aydın  
**Dönem:** 2025 Bahar

---