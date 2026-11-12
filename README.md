# maze-bot-c
Maze-solving Arduino robot · Team C · IMU IEEE RAS

## Donanım Listesi ve Kullanım Amaçları

* **Arduino Uno (ATmega328P):** Ana kontrol kartıdır. Sensörlerden gelen verileri toplar, açı ve mesafe hesaplamalarını yapar, motor sürücüye PWM sinyalleri gönderir.
* **Güç Kaynağı (8x AA Pil / 2x4 Seri Bağlantı):** Sisteme ~9.6V - 12V aralığında gerilim sağlar. Motor sürücüsünün güç girişine bağlıdır.
* **L298N Çift H-Köprüsü Motor Sürücü:** Sol ve sağ DC motorları bağımsız olarak kontrol eder.
  * `ENA` ve `ENB` jumper kapakları takılıdır; hız ve yön kontrolü doğrudan `IN1`-`IN4` pinlerine gönderilen PWM sinyalleriyle yapılır.
  * Kart üzerindeki 5V regülatör jumper'ı takılıdır. Pilden gelen gerilim 5V'a düşürülerek klemensin `5V` ucundan Arduino ve sensör hattına besleme sağlar.
* **3x HC-SR04 Ultrasonik Mesafe Sensörü (Sol, Ön, Sağ):** Önündeki ve yanlarındaki duvarlara olan mesafeyi ölçer. Yol ayrımlarını ve çıkmaz sokakları algılar.
* **2x MH-Sensor-Series Optik Enkoder Kartı:** Motor arkasındaki delikli diskleri okur. Arduino'nun donanımsal kesme (interrupt) pinlerine bağlıdır; tekerlek dönüş turunu ve katedilen mesafeyi ölçer.
* **MPU-6050 (GY-521) IMU:** 3 eksen jiroskop ve 3 eksen ivmeölçer içerir. I2C hattı üzerinden haberleşir. Dönüş açılarının (90° ve 180°) tekerlek kaymasından etkilenmeden doğru yapılmasını sağlar.
* **HC-06 Bluetooth Modülü:** Robot ile bilgisayar veya telefon arasında kablosuz seri iletişim kurar. Telemetri verilerini izlemek ve komut göndermek için kullanılır.

---

## Pin Bağlantı Tablosu

Aşağıdaki tablo, `src/main.cpp` dosyasındaki tanımlamalara göre düzenlenmiştir:

| Arduino Pini | Bağlanan Modül / Pin | Yön | Açıklama |
| :--- | :--- | :--- | :--- |
| **5V** | Breadboard `+5V` Dağıtım Hattı | Giriş | L298N `5V` klemensinden gelen düzenlenmiş güç |
| **GND** | Breadboard `GND` Ortak Hat | - | Pil, sürücü ve tüm sensörlerin ortak toprak hattı |
| **D0 (RX)** | HC-06 `TX` | Giriş | Donanımsal Seri Alıcı (3.3V lojik okunabilir) |
| **D1 (TX)** | HC-06 `RX` *(Gerilim Bölücü ile)* | Çıkış | Donanımsal Seri Verici (1kΩ / 2kΩ dirençlerle 3.3V seviyesine indirilir) |
| **D2** | Sol Enkoder `DO` | Giriş | Harici Kesme 0 (`INT0` / `isrEncoderLeft`) |
| **D3** | Sağ Enkoder `DO` | Giriş | Harici Kesme 1 (`INT1` / `isrEncoderRight`) |
| **D4** | Sol HC-SR04 `Trig` | Çıkış | Sol ultrasonik tetikleme pini |
| **D5 (PWM)** | L298N `IN3` | Çıkış | Sağ Motor İleri PWM |
| **D6 (PWM)** | L298N `IN4` | Çıkış | Sağ Motor Geri PWM |
| **D7** | Ön HC-SR04 `Trig` | Çıkış | Ön ultrasonik tetikleme pini |
| **D8** | Ön HC-SR04 `Echo` | Giriş | Ön ultrasonik yankı okuma pini |
| **D9 (PWM)** | L298N `IN1` | Çıkış | Sol Motor İleri PWM |
| **D10 (PWM)**| L298N `IN2` | Çıkış | Sol Motor Geri PWM |
| **D11** | Sol HC-SR04 `Echo` | Giriş | Sol ultrasonik yankı okuma pini |
| **D12** | Sağ HC-SR04 `Trig` | Çıkış | Sağ ultrasonik tetikleme pini |
| **D13** | Sağ HC-SR04 `Echo` | Giriş | Sağ ultrasonik yankı okuma pini |
| **A4 (SDA)** | MPU-6050 `SDA` | Çift Yönlü | I2C Veri Hattı (Adres: `0x68`) |
| **A5 (SCL)** | MPU-6050 `SCL` | Çıkış | I2C Saat Hattı (400 kHz) |

---

## Çalıştırmadan Önce Dikkat Edilmesi Gerekenler

### 1. Pil Gücünün Kesilmesi (Açma / Kapama Anahtarı)
* Robotu kullanmadığınız zamanlarda pil kutusu üzerindeki anahtarı kapatın veya pillerden birini yuvasından çıkarın.
* Sürücü kartı üzerindeki regülatör ve sensörlerin gösterge ışıkları, robot hareket etmese dahi akım çekmeye devam eder ve pilleri zamanla tamamen bitirir.

### 2. Bilgisayara Bağlarken USB ve 5V Çakışması
* **Pil gücü açıkken ve L298N'den gelen 5V kablosu Arduino'nun `5V` pinine takılıyken kesinlikle bilgisayardan USB kablosu bağlamayın.**
* Arduino'nun `5V` pinine harici güç verildiğinde kartın kendi koruma devresi devre dışı kalır. Bu durum bilgisayarın USB portu ile motor sürücüsünün regülatörünü karşı karşıya getirir ve donanıma zarar verebilir.
* **Güvenli Kod Yükleme:** Bilgisayara USB bağlamadan önce pil anahtarını kapatın veya Arduino `5V` pinine giren kabloyu yerinden çıkarın.

### 3. Kod Yüklerken D0 ve D1 Pinlerinin Sökülmesi
* `D0` (RX) ve `D1` (TX) pinleri, Arduino'nun bilgisayarla haberleşmesini sağlayan USB dönüştürücü çip ile ortak çalışır.
* HC-05 modülü bu pinlere takılı durumdayken PlatformIO üzerinden kod yüklenemez; yükleme `avrdude: stk500_getsync(): not in sync` hatası verir.
* **Çözüm:** Kodu karta yüklemeden önce arduino `D0` veya bluetooth HC-06 `TX` pinine giren sarı jumper kabloyu çıkarın. Yükleme tamamlandıktan sonra kabloyu tekrar yerine takın.

### 4. HC-06 RX Pininde Gerilim Bölücü Kullanımı
* HC-05 modülünün beslemesi 5V olsa bile, kartın `RX` pini 3.3V lojik seviyesiyle çalışır.
* Arduino'nun `D1` (TX) pininden çıkan 5V sinyal modülün `RX` pinine doğrudan verilmemelidir. Araya breadboard üzerinde 1kΩ ve 2kΩ dirençlerle gerilim bölücü devre kurulmalıdır. Doğrudan bağlantı Bluetooth çipini bozabilir.

### 5. Başlatma Anında IMU Kalibrasyonu
* Robotun gücü açıldığı anda (`setup()` fonksiyonu çalışırken) MPU-6050 sensörü jiroskop başlangıç değerini sıfırlar.
* Bu sebeple açma anahtarına basıldığı andan itibaren ilk 3-5 saniye boyunca robot düz bir zeminde hareketsiz durmalıdır. Başlatma sırasında robotu hareket ettirmek açı okumalarında sapmaya (drift) yol açar.

### 6. Ultrasonik Sensörlerin Okunma Aralığı
* Üç ultrasonik sensör yazılım içinde aynı anda art arda tetiklenmemelidir.
* Labirent gibi kapalı alanlarda ses dalgalarının duvarlardan sekip diğer sensörlere sahte yankı oluşturmasını engellemek için her sensör okuması arasında en az 25-30 ms süre bırakılmalıdır.
