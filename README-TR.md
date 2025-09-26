# TCFS - Zaman Kapsülü Dosya Sistemi

[![Build Status](https://github.com/username/tcfs/workflows/CI/badge.svg)](https://github.com/code-alchemist01/tcfs/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)

**TCFS (Time Capsule File System)**, dosyalarınızı belirli bir gelecek tarih ve saate kadar kilitleyebileceğiniz güvenli, zaman tabanlı bir dosya şifreleme sistemidir. Önemli belgeleriniz, mesajlarınız veya belirli bir süre sonra erişmek istediğiniz herhangi bir dosya için dijital bir zaman kapsülü olarak düşünebilirsiniz.

## 🌟 Özellikler

- **⏰ Zaman Tabanlı Erişim Kontrolü**: Dosyaları belirli bir gelecek tarih ve saate kadar kilitleyin
- **🔐 Askeri Düzeyde Şifreleme**: PBKDF2 anahtar türetme ile AES-256-GCM şifreleme
- **🛡️ Güvenlik Odaklı Tasarım**: Orijinal dosyalar şifreleme sonrası güvenli şekilde silinir
- **📝 Zengin Metadata**: Şifrelenmiş dosyalarla birlikte etiket, not ve politika bilgileri saklayın
- **🔍 Durum İzleme**: Şifre çözmeden kalan süre ve dosya bilgilerini kontrol edin
- **💻 Çapraz Platform**: Windows, Linux ve macOS'ta çalışır
- **🎯 Basit CLI Arayüzü**: Kullanımı kolay komut satırı arayüzü

## 🚀 Hızlı Başlangıç

### Gereksinimler

- **C++17** uyumlu derleyici
- **CMake** 3.15 veya üzeri
- **OpenSSL** geliştirme kütüphaneleri
- **nlohmann/json** kütüphanesi

### Kaynak Koddan Derleme

```bash
# Depoyu klonlayın
git clone https://github.com/username/tcfs.git
cd tcfs

# Build dizini oluşturun
mkdir build && cd build

# Yapılandırın ve derleyin
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release

# Çalıştırılabilir dosya build/src/cli/Release/tcfs.exe (Windows)
# veya build/src/cli/tcfs (Linux/macOS) konumunda olacaktır
```

### Kurulum

Derleme sonrası, çalıştırılabilir dosyayı PATH'inize kopyalayabilir veya doğrudan build dizininden kullanabilirsiniz.

## 📖 Kullanım

### 1. TCFS Deposu Başlatma

Önce bir TCFS deposu (şifrelenmiş dosyaların saklanacağı dizin) oluşturun:

```bash
tcfs --store ./zaman_kapsullerim init --owner "Adınız Soyadınız"
```

### 2. Dosya Kilitleme (Zaman Kapsülü Oluşturma)

Bir dosyayı belirli bir tarihe kadar şifreleyin ve kilitleyin:

```bash
tcfs --store ./zaman_kapsullerim lock gizli_belge.txt \
  --unlock-at "2025-12-25T09:00:00Z" \
  --label "Noel Mesajı" \
  --notes "Noel sabahı için özel bir mesaj"
```

**Önemli**: Orijinal dosya şifreleme sonrası güvenli şekilde silinecektir!

### 3. Dosya Durumu Kontrolü

Kilitli bir dosya hakkında şifre çözmeden bilgi görüntüleyin:

```bash
tcfs --store ./zaman_kapsullerim status gizli_belge.txt.tcfs
```

Örnek çıktı:
```
Status for: gizli_belge.txt.tcfs
Store file: "./zaman_kapsullerim/gizli_belge.txt.tcfs"
Metadata file: ./zaman_kapsullerim/gizli_belge.txt.tcfs.meta
Policy: Policy{unlock_at=2025-12-25T09:00:00Z, owner=Adınız Soyadınız, label=Noel Mesajı, algorithm=AES-256-GCM, kdf=pbkdf2}
Unlock time: 2025-12-25T09:00:00Z
Time remaining: 8640000 seconds
Can unlock: No
Created at: 2024-01-01T12:00:00Z
Original filename: gizli_belge.txt
Tool version: 0.1.0
```

### 4. Dosya Kilidini Açma

Bir dosyanın şifresini çözmeye ve geri yüklemeye çalışın (sadece açılış zamanı geldiğinde çalışır):

```bash
tcfs --store ./zaman_kapsullerim unlock gizli_belge.txt.tcfs --output geri_yuklenen_belge.txt
```

Eğer zaman henüz gelmemişse:
```
Cannot unlock yet. Time remaining: 8639950 seconds
Unlock time: 2025-12-25T09:00:00Z
```

Eğer zaman gelmişse:
```
File unlocked successfully!
Decrypted file: geri_yuklenen_belge.txt
```

<img width="688" height="563" alt="Ekran görüntüsü 2025-09-26 174052" src="https://github.com/user-attachments/assets/696c77d9-2cec-4fc9-b685-306a2764de74" />

## 🏗️ Mimari

### Temel Bileşenler

1. **TCFS Deposu**: Şifrelenmiş dosyalar ve metadata içeren dizin
2. **Şifrelenmiş Dosyalar** (`.tcfs`): AES-256-GCM ile şifrelenmiş dosya içeriği
3. **Metadata Dosyaları** (`.tcfs.meta`): Politika ve dosya bilgilerini içeren JSON dosyaları
4. **Politika Motoru**: Zaman tabanlı erişim kontrol kurallarını uygular

### Güvenlik Özellikleri

- **AES-256-GCM Şifreleme**: Hem gizlilik hem de bütünlük sağlayan kimlik doğrulamalı şifreleme
- **PBKDF2 Anahtar Türetme**: Yapılandırılabilir iterasyonlarla güvenli anahtar türetme
- **Zaman Tabanlı Erişim Kontrolü**: Dosyalar belirtilen açılış zamanından önce şifresi çözülemez
- **Güvenli Dosya Silme**: Orijinal dosyalar şifreleme sonrası üzerine yazılır ve silinir
- **Metadata Koruması**: Kritik politika bilgileri ayrı olarak saklanır ve doğrulanır

### Dosya Yapısı

```
zaman_kapsullerim/              # TCFS Deposu
├── config.json                 # Depo yapılandırması
├── belge1.txt.tcfs             # Şifrelenmiş dosya
├── belge1.txt.tcfs.meta        # Metadata ve politika
├── foto.jpg.tcfs               # Başka bir şifrelenmiş dosya
└── foto.jpg.tcfs.meta          # Onun metadata'sı
```

## 🔧 Yapılandırma

### Depo Yapılandırması (`config.json`)

```json
{
  "version": "1.0",
  "owner": "Adınız Soyadınız",
  "kdf": "pbkdf2",
  "created_at": "2024-01-01T12:00:00Z"
}
```

### Metadata Yapısı (`.tcfs.meta`)

```json
{
  "version": "1.0",
  "policy": {
    "unlock_at": "2025-12-25T09:00:00Z",
    "owner": "Adınız Soyadınız",
    "label": "Noel Mesajı",
    "notes": "Noel sabahı için özel bir mesaj",
    "algorithm": "AES-256-GCM",
    "kdf": "pbkdf2"
  },
  "created_at": "2024-01-01T12:00:00Z",
  "original_filename": "gizli_belge.txt",
  "tool_version": "0.1.0"
}
```

<img width="580" height="483" alt="Ekran görüntüsü 2025-09-26 174108" src="https://github.com/user-attachments/assets/d93d0767-ccf1-4a9d-af55-559c66fd8a3c" />

## 🛠️ Geliştirme

### Proje Yapısı

```
tcfs/
├── src/
│   ├── libtcfs/           # Ana kütüphane
│   │   ├── crypto/        # Kriptografik fonksiyonlar
│   │   ├── policy/        # Politika yönetimi
│   │   ├── store/         # Depo işlemleri
│   │   └── utils/         # Yardımcı fonksiyonlar
│   └── cli/               # Komut satırı arayüzü
├── include/tcfs/          # Genel başlık dosyaları
├── tests/                 # Birim testleri
├── examples/              # Örnek programlar
└── cmake/                 # CMake modülleri
```

### Debug Bilgileriyle Derleme

```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --config Debug
```

### Testleri Çalıştırma

```bash
# Testleri derle
cmake --build . --target tcfs_tests

# Testleri çalıştır
./tests/tcfs_tests  # Linux/macOS
# veya
.\tests\Debug\tcfs_tests.exe  # Windows
```

## 🔒 Güvenlik Değerlendirmeleri

### TCFS'nin Koruduğu Durumlar

- **Erken Erişim**: Dosyalara belirtilen zamandan önce erişilemez
- **Veri Manipülasyonu**: AES-GCM kimlik doğrulama ve bütünlük kontrolü sağlar
- **Yetkisiz Şifre Çözme**: Güvenli anahtar türetme ile güçlü şifreleme

### TCFS'nin Korumadığı Durumlar

- **Sistem Saati Manipülasyonu**: Saldırgan sistem saatini değiştirebilirse zaman kısıtlamalarını atlayabilir
- **Fiziksel Erişim**: Saldırganın sisteme fiziksel erişimi varsa ve binary'yi değiştirebilirse
- **Yan Kanal Saldırıları**: Gelişmiş kriptografik saldırılar bu implementasyonun kapsamı dışındadır
- **Kuantum Bilgisayarlar**: Mevcut şifreleme yöntemleri gelecekteki kuantum bilgisayarlara karşı savunmasız olabilir

### En İyi Uygulamalar

1. **Şifrelenmiş Dosyaları Yedekleyin**: Kopyaları birden fazla güvenli konumda saklayın
2. **Açılış Zamanlarını Hatırlayın**: Dosyaların ne zaman açılabileceğinin kaydını tutun
3. **Sisteminizi Güvende Tutun**: Sistem saatinizin doğru ve güvenli olduğundan emin olun
4. **Düzenli Güncellemeler**: TCFS'yi en son sürümde tutun

## 🤝 Katkıda Bulunma

Katkılarınızı memnuniyetle karşılıyoruz! Detaylar için [Katkıda Bulunma Rehberi](CONTRIBUTING.md)'ne bakın.

### Geliştirme Kurulumu

1. Depoyu fork edin
2. Özellik dalı oluşturun: `git checkout -b feature/harika-ozellik`
3. Değişikliklerinizi yapın ve testler ekleyin
4. Tüm testlerin geçtiğinden emin olun: `cmake --build . --target test`
5. Değişikliklerinizi commit edin: `git commit -m 'Harika özellik ekle'`
6. Dala push edin: `git push origin feature/harika-ozellik`
7. Pull Request açın

## 📄 Lisans

Bu proje MIT Lisansı altında lisanslanmıştır - detaylar için [LICENSE](LICENSE) dosyasına bakın.

## 🙏 Teşekkürler

- Kriptografik fonksiyonlar için **OpenSSL**
- JSON ayrıştırma için **nlohmann/json**
- Komut satırı ayrıştırma için **CLI11**
- TCFS'nin tüm katkıda bulunanları ve kullanıcıları

## 📞 Destek

- **Sorunlar**: [GitHub Issues](https://github.com/code-alchemist01/tcfs/issues)
- **Tartışmalar**: [GitHub Discussions](https://github.com/code-alchemist01/tcfs/discussions)
- **Dokümantasyon**: [Wiki](https://github.com/code-alchemist01/tcfs/wiki)

## 🗺️ Güncelleme Fikirleri

- [ ] GUI Uygulaması
- [ ] Mobil Uygulamalar (iOS/Android)
- [ ] Bulut Depolama Entegrasyonu
- [ ] Çoklu Kullanıcı Desteği
- [ ] Gelişmiş Politika Seçenekleri
- [ ] Yedekleme ve Kurtarma Araçları

## 💡 Kullanım Senaryoları

### Kişisel Kullanım
- **Doğum günü sürprizleri**: Sevdikleriniz için gelecekteki doğum günlerinde açılacak mesajlar
- **Yıllık değerlendirmeler**: Bir yıl sonra kendinize açacağınız kişisel notlar
- **Hatıralar**: Belirli yıldönümlerinde açılacak fotoğraf ve video koleksiyonları

### Profesyonel Kullanım
- **Proje dokümantasyonu**: Proje tamamlanma tarihinde açılacak değerlendirme raporları
- **Yasal belgeler**: Belirli tarihlerde geçerli olacak sözleşme ve anlaşmalar
- **Yedekleme stratejisi**: Kritik verilerin zaman tabanlı arşivlenmesi

### Eğitim
- **Ödev sistemi**: Öğrencilerin belirli tarihte erişebileceği ders materyalleri
- **Sınav soruları**: Sınav tarihinde açılacak soru bankaları
- **Araştırma projeleri**: Uzun vadeli araştırmaların aşamalı dokümantasyonu

## 🔍 Teknik Detaylar

### Şifreleme Süreci

1. **Anahtar Türetme**: PBKDF2 ile güvenli anahtar oluşturma
2. **IV Üretimi**: Her dosya için benzersiz Initialization Vector
3. **Şifreleme**: AES-256-GCM ile dosya içeriğinin şifrelenmesi
4. **Kimlik Doğrulama**: GCM modu ile bütünlük kontrolü
5. **Metadata Oluşturma**: Politika ve dosya bilgilerinin JSON formatında saklanması

### Zaman Kontrolü Mekanizması

```cpp
// Pseudo-kod örneği
bool canUnlock(const Policy& policy) {
    auto now = std::chrono::system_clock::now();
    auto unlock_time = parseISO8601(policy.unlock_at);
    return now >= unlock_time;
}
```

### Dosya Güvenliği

- Orijinal dosyalar şifreleme sonrası güvenli şekilde silinir
- Şifrelenmiş dosyalar sadece TCFS ile açılabilir
- Metadata dosyaları politika bilgilerini korur
- Sistem saati manipülasyonuna karşı uyarılar

---

**⚠️ Sorumluluk Reddi**: TCFS herhangi bir garanti olmaksızın olduğu gibi sağlanmaktadır. Şifreleme öncesi önemli dosyalarınızı her zaman yedekleyin. Geliştiriciler herhangi bir veri kaybından sorumlu değildir.

---

*İngilizce dokümantasyon için [README.md](README.md) dosyasına bakın*
