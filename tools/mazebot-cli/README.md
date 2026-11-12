# MazeBot CLI — HC-06

Robot telemetrisini terminalde canlı gösterir ve otomatik CSV'ye kaydeder.
Python 3.10+; Windows ve Linux için hazırlanmıştır. Önceki BLE/JSON sürümünün yerine geçer.
Bağımlılıklar: Rich (ekran), pyserial (seri port). HC-06 bağlantısı klasik Bluetooth seri portu üzerindendir.

## 1. İlk adım: Windows'ta simülasyon

ZIP'e sağ tık → Tümünü Ayıkla. İçteki `mazebot-cli` klasörünü aç.
`main.py`, `requirements.txt` ve bu dosya aynı klasörde olmalı.
Dosya Gezgini adres çubuğuna `powershell` yazıp Enter'a bas.

Python'un kurulu olduğunu kontrol et:

```powershell
py --version
```

Python 3.10 veya üstü varsa sırayla:

```powershell
py -m venv .venv
.\.venv\Scripts\python.exe -m pip install -r requirements.txt
.\.venv\Scripts\python.exe main.py
```

Sanal ortamı etkinleştirmek gerekmez; PowerShell yürütme ilkesini değiştirmene gerek yok.
`py` tanınmıyorsa Python kurulumunu kontrol et; `python --version` çalışıyorsa ilk komutta `py` yerine `python` kullan.
Programı Windows Terminal/PowerShell terminalinde çalıştır; Python IDLE kabuğu uygun değildir.

Ekranda SİMÜLASYON yazmalı; T ve kayıt sayısı artmalı. Her 3 saniyede bir demo karar değişir.
Bu karar dizisi görsel deneme içindir, labirent çözme algoritması değildir.
Ctrl+C ile durdur. `logs` klasöründeki `sim_...csv` dosyasını aç.

30 saniyede otomatik çıkış:

```powershell
.\.venv\Scripts\python.exe main.py --duration 30
```

## 2. Linux'ta kurulum

Python/venv eksikse Ubuntu/Debian üzerinde:

```bash
sudo apt install python3 python3-venv
```

Ardından proje klasöründe:

```bash
python3 -m venv .venv
.venv/bin/python -m pip install -r requirements.txt
.venv/bin/python main.py
```

Windows `.venv` klasörünü Linux'a kopyalama; her sistemde yeniden oluştur.

## 3. Kesinleşen firmware–CLI sözleşmesi

Kaynak: kullanıcının paylaştığı “7.1 Telemetri satırı” görseli.

```text
T=15230,L=10.2,F=35.0,R=400.0,ACT=FWD
```

| Alan | Anlamı |
|---|---|
| T | Robot açıldığından beri geçen süre, milisaniye; negatif olmayan tam sayı |
| L | Sol mesafe, cm |
| F | Ön mesafe, cm |
| R | Sağ mesafe, cm |
| ACT | FWD, TURN_L, TURN_R, BACK veya STOP |

400, yankı alınmadığını / yolun açık olduğunu belirten özel değerdir.
Ekranda bunun açıklaması gösterilir; CSV'de 400.0 olarak korunur.
ACT etiketi: ileri, sola dön, sağa dön, geri, dur.

Her örnek **tek satır** olmalı ve `\n` veya `\r\n` ile bitmeli.
Alan adları büyük harflidir, virgülle ayrılır; alan sırası değişebilir.
Mesafeler sonlu, negatif olmayan sayılar; ondalık ayırıcı noktadır.
Eksik, tekrarlı veya bilinmeyen alanlar; bozuk sayılar ve bilinmeyen ACT değerleri reddedilir.
Boş satırlar yok sayılır. Başka debug mesajları bu hat üzerinden gönderilirse hatalı satır sayılır.
Robot yeniden başlarsa T sıfırlanabilir; satırlar gelme sırasıyla kaydedilir.

Bir satır birkaç seri okumaya bölünebilir; program satır sonuna kadar parçaları birleştirir.
1024 baytı aşan satır atılır; sonraki satır sonundan sonra okumaya devam edilir.
Enes bu formatı gönderecek, Muhammed okuyacak; format değişikliği önce issue üzerinden kararlaştırılmalı.

## 4. Gerçek HC-06 bağlantısı — robot hazır olunca

Robotun HC-06 üzerinden yayın yapıyor olması ve bilgisayarın Bluetooth seri bağlantısını desteklemesi gerekir.
İşletim sisteminde eşleştirme yap ve seri portu oluştur/seç.
Windows'ta HC-06'nın dışarı giden Bluetooth COM portu kullanılmalı. Linux'ta önce RFCOMM seri aygıtı yapılandırılmalı.
Uygulama eşleştirme veya RFCOMM yapılandırmasını kendisi yapmaz.

Mevcut portları listele:

```powershell
.\.venv\Scripts\python.exe main.py --list-ports
```

Örnekteki COM5 ve 9600 GERÇEK AYARLAR DEĞİLDİR. Robotun portu ve doğrulanmış baud hızıyla değiştir:

```powershell
.\.venv\Scripts\python.exe main.py --mode serial --port COM5 --baud 9600
```

Linux örneği (aygıt önceden yapılandırılmış olmalı):

```bash
.venv/bin/python main.py --mode serial --port /dev/rfcomm0 --baud 9600
```

Baud hızı henüz bilinmiyor; firmware'deki HC-06 UART ayarıyla doğrulanmalı.
Program bilerek otomatik baud tahmini yapmaz ve seri modda `--baud` ister.
Port adı da örnektir. Kullanılan seri biçim 8 veri biti, paritesiz, 1 stop biti (8N1), akış kontrolü yoktur.
Linux'ta port listesi bazı RFCOMM aygıtlarını göstermeyebilir; doğru aygıt yolunu doğrudan verebilirsin.
Program sadece okur; hareket komutu göndermez. Robota hareketi firmware verir.

## 5. Ekran ve CSV davranışı

- CSV, simülasyon başlayınca veya seri port açılınca otomatik açılır.
- Geçerli her örnek tek CSV satırıdır. Satır yazıldıktan sonra sayaç artar.
- Her satırdan sonra dosya flush edilir; bu elektrik kesintisine karşı disk garantisi değildir.
- Dosya adı benzersizdir, eski kayıtların üzerine yazılmaz.
- `sim_` ve `serial_` önekleri demo ve gerçek kayıtları ayırır.
- Son geçerli veri 2 saniyeden eskiyse değerler soluk gösterilir ve güncel değil uyarısı çıkar.
- Seri portun açık olması, fiziksel Bluetooth bağlantısının hâlâ çalıştığını tek başına kanıtlamaz.
- Seri modda 30 saniye geçerli veri yoksa program portu/CSV'yi kapatıp hata koduyla çıkar.
  Bu durum bağlantı, yanlış baud, eksik satır sonu veya hatalı biçimden kaynaklanabilir.
- Okuma hatası, Ctrl+C veya süre sonu geldiğinde dosya ve port kapatılır.
- Otomatik yeniden bağlantı yoktur; yeniden çalıştırma yeni dosya açar.
- Son uyarı, önceki hatanın açıklaması olarak görünmeye devam eder; güncel veri ayrı gösterilir.

CSV başlığı:

```csv
received_at_utc,source,T,L,F,R,ACT
```

`received_at_utc`: bilgisayarın UTC alınma zamanı. `T`: robotun kendi çalışma süresi.
`source`: sim veya serial. Simülasyon gerçek robot performansını temsil etmez.

Ayarlar:

```powershell
.\.venv\Scripts\python.exe main.py --help
.\.venv\Scripts\python.exe main.py --hz 10 --duration 20 --output logs
```

`--hz` yalnızca simülasyonu değiştirir. `--stale-after` ve `--idle-timeout` saniye cinsindedir.
Ekran saniyede 4 kez yenilenir; gelen her geçerli veri ayrıca kaydedilir.

## 6. Testler ve teslim planı

```powershell
.\.venv\Scripts\python.exe -m unittest -v
```

Otomatik kontroller: sözleşme örneği, hatalı alanlar, parçalı/birleşik/uzun satırlar,
400 özel değerinin CSV'de korunması, eski veri uyarısı, port hatası/zaman aşımı sonrası kapanış.
Geliştirme ortamında ayrıca Linux sanal seri portu üzerinden gerçek pyserial okuması ve Ctrl+C denenmiştir.
Windows'a özel kurulum ve gerçek HC-06 testi kullanıcının bilgisayarında henüz yapılmadı.

| Aşama | Kabul kontrolü |
|---|---|
| 1 — Simülasyon | Değerler değişir, T ve kayıt sayısı artar, CSV kapanır |
| 2 — Firmware uyumu | Enes'in örnek satırı hatasız ayrıştırılır; baud ve satır sonu doğrulanır |
| 3 — HC-06 | Gerçek porttan veri alınır; yanlış port/bağlantı kaybı denenir |
| 4 — Linux teslimi | Araç hareket ederken değerler canlı güncellenir ve CSV kaydedilir |
| 5 — Dokümantasyon | Çalıştırma komutu, baud ve örnek CSV dokümantasyon sorumlusuna aktarılır |

Şu an eksik: robotun seri portu, doğrulanmış baud hızı ve gerçek donanımla kabul testi.

## Kaynaklar

- https://pyserial.readthedocs.io/en/latest/shortintro.html
- https://pyserial.readthedocs.io/en/latest/pyserial_api.html
- https://rich.readthedocs.io/en/latest/live.html
