# Emergency Drone Coordination Simulator

Bu proje, **acil durumlarda droneların survivor'ları (hayatta kalanları) bulup kurtarmasını simüle eden, çoklu-thread ve gerçek zamanlı görselleştirme içeren bir sistemdir**. Proje, ODTÜ CENG 334/583 veya benzeri dersler için "Distributed Systems" ödevi kapsamında geliştirilmiştir.

---

## Özellikler

- **Gerçek zamanlı görselleştirme (SDL2 + SDL2_ttf)**  
  Dronelar ve survivor'lar harita üzerinde canlı olarak gösterilir, görevler ve kurtarma süreçleri anlık olarak izlenebilir.

- **Çoklu thread desteği**  
  - Her drone ve survivor kendi thread'inde çalışır.
  - AI kontrol thread'i en uygun drone ve survivor eşleştirmesini yapar.

- **Thread-safe veri yapıları**  
  Drone ve survivor listeleri mutex ile korunur, veri çakışması engellenir.

- **Gelişmiş arayüz**  
  - Drone ve survivor ID'leri ekranda görünür.
  - Kurtarılan survivor'lar 2 saniye boyunca gri/şeffaf olarak ekranda kalır ve yanında "RESCUED" etiketi belirir.
  - Ekranın üstünde kısa süreli "Survivor S14 rescued!" gibi bilgi mesajları çıkar.
  - İstatistik paneli ve legend (renklerin anlamı) bulunur.

- **Kolayca genişletilebilir altyapı**  
  Kodlar modüler ve Phase 2'de gerçek TCP server/client iletişimine kolayca geçirilebilir.

---

## Kurulum

### Gereksinimler

- C derleyicisi (gcc tavsiye edilir)
- SDL2 ve SDL2_ttf kütüphaneleri

#### Ubuntu/Debian için:

```sh
sudo apt-get install build-essential libsdl2-dev libsdl2-ttf-dev
```

---

## Derleme

Proje kök dizininde:

```sh
gcc -Wall -Wextra -pedantic -std=c11 -g `sdl2-config --cflags` \
    -o server server.c list.c drone.c survivor.c ai.c map.c view.c \
    `sdl2-config --libs` -lSDL2_ttf -lpthread -lm
```

---

## Çalıştırma

```sh
./server
```

Otomatik olarak bir pencere açılır ve simülasyon başlar.

---

## Kullanım ve Arayüz

- **Kırmızı noktalar:** Survivor'lar (yardım bekleyen kişiler)
- **Mavi daireler:** Dronelar
- **Yeşil çizgi:** Drone'un aktif olarak bir survivor'a gidiyor olması
- **Gri/şeffaf survivor:** Kurtarıldıktan sonra 2 saniye ekranda kalan kişi
- **Legend ve istatistikler:** Sol üstte toplam drone, survivor ve renklerin anlamı

---

## Kod Yapısı

- `server.c` : Ana dosya, thread başlatma ve simülasyon döngüsü
- `drone.c/.h` : Drone veri yapısı ve thread fonksiyonu
- `survivor.c/.h` : Survivor oluşturma, yok etme ve survivor thread'i
- `ai.c/.h` : AI kontrol thread'i (görev atama mantığı)
- `view.c/.h` : SDL2 ile görselleştirme ve arayüz çizimi
- `list.c/.h` : Thread-safe generic bağlı liste yapısı
- `map.c/.h` : (İsteğe bağlı) Harita ve koordinat yönetimi

---

## Genişletme/Phase 2

Protokol ve kod altyapısı, ileride gerçek TCP tabanlı client/server mimarisine (ör: farklı bilgisayarlarda gerçek drone ve server processleri) kolayca evrilebilir.

---

## Katkı ve Lisans

Proje eğitim amaçlıdır. Her türlü katkı ve öneri için [Pull Request](https://github.com/) gönderebilirsiniz.

---

**Hazırlayan:** aybett  
**Dönem:** 2025 Bahar  