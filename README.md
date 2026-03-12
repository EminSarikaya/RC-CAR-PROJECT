# 🏎️ RC Araç ve Özel Kumanda Sistemi

Bu proje, iki adet STM32F103C8T6 mikrodenetleyicisi ve NRF24L01 modülleri kullanılarak geliştirilmiş, kendi özel kumandasıyla haberleşen bir uzaktan kumandalı araç sistemidir. 

## 📝 Proje Amacı ve Özellikleri
* **Kesintisiz İletişim:** NRF24L01 modülleri ile araba ve kumanda arasında hızlı ve güvenilir RF haberleşmesi.
* **Özel Kumanda Tasarımı:** Standart kumandalar yerine, tamamen bu projeye özgü tasarlanmış ve programlanmış kontrolcü.
* **Modüler Yapı:** Araba ve kumanda kodları birbirinden bağımsız olarak geliştirilebilir.

## 🛠️ Kullanılan Donanımlar
**Araba (Alıcı) Tarafı:**
* STM32F103C8T6 (Blue Pill) Mikrodenetleyici
* NRF24L01 2.4GHz RF Modülü
* TB6612FNG Motor Sürücü
* 2 Adet ULN2003A Bipolar Transistör Entegresi
* LM2596 Voltaj Regülatörü
* 4 Adet DC Motor
* 4 Adet MG90S Servo Motor
* 2 Adet VTC6 Lİtyum İyon Pil

**Kumanda (Verici) Tarafı:**
* STM32F103C8T6 (Blue Pill) Mikrodenetleyici
* NRF24L01 2.4GHz RF Modülü
* 3 Adet Joystick Modülü
* 0.96 OLED Ekran
* 4x4 Keypad
* LM2596 Voltaj Regülatörü

## 🚀 Kurulum ve Kullanım
1. Bu repoyu bilgisayarınıza klonlayın:
   `git clone https://github.com/KullaniciAdin/RC-CAR-PROJECT.git`
2. `projeAraba` klasörünü STM32CubeIDE (veya kullandığın IDE) ile açın ve araçtaki karta yükleyin.
3. `ProjeKumanda` klasörünü açarak kumanda kartına yükleyin.
4. Her iki sisteme de güç verin, bağlantı otomatik kurulacaktır.
