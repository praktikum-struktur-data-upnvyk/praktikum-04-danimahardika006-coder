# Praktikum Struktur Data C++ — Pertemuan 4

## Study Case: Peramban Web "Jelajah"

Repository ini dibuat otomatis oleh aplikasi praktikum. Setiap kali Anda
melakukan **push**, GitHub Actions akan mengompilasi kode Anda, menjalankan
test, dan mengirim nilainya ke aplikasi.

---

# Study Case

Pertemuan ini hanya punya **satu soal**, yaitu study case di bawah ini. Seluruh
fungsi yang Anda kerjakan berasal dari cerita yang sama — tidak ada cerita baru
lagi sesudahnya.

## Latar

Anda bergabung dengan tim kecil yang sedang membuat peramban web bernama
**Jelajah**. Tampilannya sudah jadi dan tombol-tombolnya sudah terpasang, tetapi
bagian dalamnya masih kosong — belum ada satu pun yang benar-benar menyimpan
data. Menekan tombol Back sekarang tidak melakukan apa-apa, karena memang belum
ada yang mencatat halaman mana saja yang sudah dikunjungi.

Ada dua fitur yang harus Anda hidupkan, dan keduanya membutuhkan bentuk
penyimpanan yang berbeda.

## Fitur pertama — Riwayat Halaman

Setiap halaman yang dikunjungi harus tercatat berurutan. Dari halaman yang
sedang dibuka, pengguna bisa menekan **Back** untuk mundur ke halaman
sebelumnya, atau **Forward** untuk maju lagi ke halaman sesudahnya.

Perhatikan kata "mundur" di situ. Kalau tiap catatan hanya menyimpan alamat
catatan **sesudahnya** — seperti linked list yang Anda buat di Pertemuan 3 —
maka tombol Back mustahil dikerjakan: dari sebuah catatan, tidak ada jalan untuk
tahu siapa yang ada di belakangnya. Satu-satunya cara adalah menelusuri ulang
dari awal setiap kali, dan itu jelas tidak masuk akal untuk sebuah tombol yang
ditekan setiap detik.

Karena itu riwayat disimpan sebagai **linked list ganda**: tiap catatan menyimpan
**dua** alamat sekaligus.

```
      head                                              tail
       |                                                 |
      [101] <-> [102] <-> [103] <-> [104]
       ^                                                 ^
       |                                                 |
  catatan paling lama                    halaman yang sedang dibuka

  prev = arah tombol Back          next = arah tombol Forward
```

`prev` milik `head` bernilai `nullptr`, dan `next` milik `tail` bernilai
`nullptr`. Keduanya menandai ujung riwayat.

## Fitur kedua — Putaran Tab

Jelajah bisa membuka beberapa tab sekaligus. Menekan **Ctrl+Tab** memindahkan
pengguna ke tab berikutnya — dan ketika ia sudah sampai di tab terakhir, tekanan
berikutnya membawanya **kembali ke tab pertama**. Tidak ada pesan "sudah mentok",
tidak ada tab yang buntu. Perpindahannya berputar terus.

Kalau tab disimpan seperti riwayat di atas, tab terakhir akan punya `next`
bernilai `nullptr`, dan Ctrl+Tab di situ berhenti. Supaya berputar, tab terakhir
harus menyambung **kembali** ke tab pertama.

Karena itu tab disimpan sebagai **linked list circular**:

```
       .--> [201] --> [202] --> [203] --.
       '--------------------------------'
```

Tidak ada ujung sama sekali. `head` di sini bukan "yang pertama", melainkan
**titik mulai putaran** — yaitu tab yang sedang aktif. Satu putaran penuh berarti
berjalan dari `head` sampai kembali lagi ke `head`.

> **Peringatan.** Karena tidak ada ujung, penelusuran yang lupa berhenti akan
> berputar **selamanya** dan program Anda menggantung. Setiap pekerjaan di
> Soal 4 harus berhenti tepat sesudah satu putaran.

## Satu sesi pemakaian, dari awal sampai akhir

Berikut satu sesi pemakaian Jelajah oleh seorang pengguna bernama Rani. **Empat
langkah bertanda SOAL adalah pekerjaan yang harus Anda kerjakan.**

| # | Yang terjadi | Pekerjaan |
|---:|---|---|
| 1 | Rani mengunjungi halaman 101, lalu 102, lalu 103. Setiap kunjungan tercatat sebagai yang **paling baru** di ujung riwayat. | **Soal 1** `bukaHalaman` |
| 2 | Ia membuka menu Riwayat dan melihat seluruh kunjungannya dari yang paling lama. | *sudah disediakan* (`riwayatMaju`) |
| 3 | Ia menekan Back berulang kali sampai riwayatnya habis, mencatat halaman yang dilewatinya. Arahnya kebalikan langkah 2 — di sinilah linked list ganda benar-benar terpakai. | **Soal 2** `riwayatMundur` |
| 4 | Ada satu halaman yang tidak ingin ia simpan. Ia menghapusnya, dan sesudahnya Back maupun Forward harus tetap mulus. | **Soal 3** `hapusHalaman` |
| 5 | Siangnya ia membuka tab 201, 202, 203. Tab baru selalu diletakkan sebagai yang terakhir dalam satu putaran. | **Soal 4** `bukaTab` |
| 6 | Pekerjaannya selesai. Ia menutup jendela Jelajah. | *sudah disediakan* (`tutupSemuaTab`) |

Keempat pekerjaan bertanda **Soal** itulah seluruh isi pertemuan ini — **tiga
tentang linked list ganda, satu tentang linked list circular**. Di bawah nanti
Anda tidak akan menemukan cerita baru, hanya **rincian teknis** tiap pekerjaan.

### Yang sudah disediakan (tidak dinilai)

| Fungsi | Gunanya |
|---|---|
| `riwayatMaju` | melihat isi riwayat dari depan — pembanding untuk Soal 2 |
| `putaranTab` | melihat isi satu putaran tab |
| `hapusSeluruhRiwayat` | membereskan memori riwayat |
| `tutupSemuaTab` | membereskan memori tab |

Keempatnya sudah ditulis lengkap di `src/student.cpp`. Pakai dua yang pertama
sesering mungkin untuk memeriksa hasil kerja Anda sendiri.

---

## Tujuan Praktikum

Setelah pertemuan ini Anda diharapkan mampu:

- menjelaskan mengapa sebuah fitur bisa **menuntut** bentuk penyimpanan tertentu,
  dan bukan sekadar memilihnya secara acak;
- membangun linked list ganda dan menjaga `head` serta `tail` tetap konsisten;
- menelusuri data ke **dua arah** dan memastikan kedua arah saling terbalik;
- menyisipkan dan menghapus node sambil merapikan `prev` maupun `next`;
- membangun linked list circular dan menjaga lingkarannya tetap tertutup;
- menghentikan penelusuran circular tepat sesudah satu putaran; dan
- membebaskan seluruh node pada kedua bentuk penyimpanan dengan benar.

---

## Materi

| Konsep | Yang perlu Anda kuasai |
|---|---|
| `DNode` | node dua arah: `data`, `prev`, `next` |
| `head` & `tail` | dua penanda yang harus **sama-sama** benar setiap saat |
| Penelusuran maju | dari `head` lewat `next`, berhenti di `nullptr` |
| Penelusuran mundur | dari `tail` lewat `prev` — inilah gunanya linked list ganda |
| Sisip & hapus dua arah | satu operasi mengurus **dua** sambungan sekaligus |
| `CNode` | node satu arah, tetapi melingkar |
| Lingkaran tertutup | `next` milik node terakhir menunjuk kembali ke `head` |
| Titik mulai putaran | `head` pada circular bukan "yang pertama", melainkan titik mulai |
| Syarat berhenti | pada circular **bukan** `p != nullptr`, melainkan "kembali ke titik mulai" |
| Pembebasan | node yang dilepas tidak boleh ditinggalkan di memori |

---

## Untuk Mahasiswa

### File yang Harus Dikerjakan

**Satu-satunya file yang dinilai adalah:**

```
src/student.cpp
```

Di bagian paling bawah `src/student.cpp` ada `main()`. Bagian itu memeragakan
seluruh sesi Rani di atas, sehingga Anda bisa langsung melihat hasil kerja Anda
berjalan sebagai satu cerita utuh. `main()` tersebut **tidak ikut dinilai** dan
**bebas Anda ubah** sesukanya.

Jangan mengubah file lain. Perubahan pada `src/student.h`, `tests/checker.cpp`,
`tests/report.h`, atau `.github/workflows/` tidak akan membuat nilai Anda naik
dan dapat menyebabkan penilaian gagal.

### Contract / API

Checker memanggil fungsi Anda secara langsung, jadi baris-baris berikut wajib
persis seperti ini (sudah dideklarasikan di `src/student.h`):

```cpp
// Linked list ganda — dipakai Soal 1, 2, dan 3
struct DNode {
    int data;
    DNode* prev;
    DNode* next;
};

bool bukaHalaman(DNode*& head, DNode*& tail, int nomor);     // Soal 1
int  riwayatMundur(DNode* tail, int* keluaran);              // Soal 2
bool hapusHalaman(DNode*& head, DNode*& tail, int nomor);    // Soal 3

// Linked list circular — dipakai Soal 4
struct CNode {
    int data;
    CNode* next;
};

bool bukaTab(CNode*& head, int nomor);                       // Soal 4

// Sudah disediakan, TIDAK dinilai
int  riwayatMaju(DNode* head, int* keluaran);
int  putaranTab(CNode* head, int* keluaran);
void hapusSeluruhRiwayat(DNode*& head, DNode*& tail);
void tutupSemuaTab(CNode*& head);
```

Yang **wajib sama**: nama kedua struct beserta field-nya, nama fungsi, tipe
parameter, dan tipe kembalian.

Yang **bebas Anda tentukan**: seluruh isi fungsi, nama variabel, jenis loop, mau
menambah fungsi bantu atau tidak.

### Pemetaan cerita ke operasi struktur data

| Soal | Fungsi | Operasi sebenarnya | Struktur |
|---|---|---|---|
| 1 | `bukaHalaman` | tambah di **akhir** | ganda |
| 2 | `riwayatMundur` | telusuri dari `tail` lewat `prev` | ganda |
| 3 | `hapusHalaman` | hapus satu node, sambung ulang dua arah | ganda |
| 4 | `bukaTab` | tambah di akhir putaran, lingkaran tetap tertutup | circular |

### Aturan yang berlaku untuk seluruh pekerjaan

- Setiap halaman dan setiap tab dikenali dari sebuah **nomor** bertipe `int`.
  Nomor boleh negatif, boleh nol, dan boleh muncul lebih dari sekali.
- `head` bernilai `nullptr` berarti daftarnya sedang **kosong**. Itu keadaan yang
  sah, bukan kesalahan.
- Parameter bertanda `&` berarti penanda milik pemanggil ikut berubah.
- Kecuali fungsi yang memang menambah atau menghapus, tidak ada fungsi yang boleh
  mengubah isi daftar.
- Tidak ada satu pun fungsi yang mencetak ke layar.
- `DNode` dan `CNode` adalah **dua struktur berbeda** dan tidak boleh dicampur.
- Penampung `keluaran` pada `riwayatMaju`, `riwayatMundur`, dan `putaranTab`
  ukurannya dijamin cukup. Bagian di luar yang terpakai **tidak boleh disentuh**.
- Anda boleh menambahkan fungsi bantu sendiri.

---

# Soal 1 — `bukaHalaman` (25 poin)

> Langkah 1 pada cerita · **linked list ganda** · tambah di **akhir**

Catat sebuah halaman baru sebagai kunjungan **paling baru**, yaitu di ujung
belakang riwayat. Seluruh catatan lama tetap berada di depannya dengan urutan
yang sama persis.

| Parameter | Artinya |
|---|---|
| `head` | Penanda catatan paling lama, milik pemanggil. Bertanda `&` |
| `tail` | Penanda catatan paling baru, milik pemanggil. Bertanda `&` |
| `nomor` | Nomor halaman yang baru dikunjungi |
| *kembalian* | `true` bila catatan baru berhasil masuk |

| Sebelum | Operasi | Sesudah |
|---|---|---|
| `101 <-> 102 <-> 103` | `bukaHalaman(head, tail, 104)` | `101 <-> 102 <-> 103 <-> 104` |
| `(kosong)` | `bukaHalaman(head, tail, 101)` | `101` |

**Yang perlu diingat.**
- `tail` milik pemanggil harus berpindah ke catatan yang baru.
- Pada riwayat yang tadinya **kosong**, `head` ikut berubah — dan `head` serta
  `tail` sama-sama menunjuk node yang sama.
- Pada riwayat yang **sudah berisi**, `head` **tidak boleh** ikut berpindah.
  Ini kesalahan yang paling sering terjadi.
- Catatan baru harus tersambung dari **kedua arah**: `prev` miliknya menunjuk
  catatan terakhir yang lama, dan `next` milik catatan lama itu menunjuk catatan
  baru.
- `prev` milik `head` dan `next` milik `tail` harus tetap `nullptr` sesudahnya.
- Setiap pemanggilan menyediakan **tepat satu** node baru.
- Periksa cepat dengan `riwayatMaju()` yang sudah disediakan — hasil maju dan
  mundur harus saling terbalik.

---

# Soal 2 — `riwayatMundur` (20 poin)

> Langkah 3 pada cerita · **linked list ganda** · telusuri dari `tail`

Salin seluruh nomor halaman ke penampung milik pemanggil, berurutan dari
kunjungan **paling baru** menuju kunjungan **paling lama**. Kembalikan banyaknya
nomor yang disalin.

Inilah pekerjaan yang menjadi **alasan** riwayat disimpan sebagai linked list
ganda. Penelusurannya dimulai dari `tail` dan berjalan lewat `prev`.

> **Petunjuk.** Bandingkan dengan `riwayatMaju()` yang sudah disediakan di
> `src/student.cpp`. Bedanya cuma **dua kata**: `head` menjadi `tail`, dan
> `next` menjadi `prev`.

| Parameter | Artinya |
|---|---|
| `tail` | Penanda catatan paling baru. Perhatikan: yang diberikan `tail`, **bukan** `head`. **Tanpa** tanda `&`. Boleh `nullptr` |
| `keluaran` | Penampung milik pemanggil. Ukurannya dijamin cukup |
| *kembalian* | Banyaknya nomor yang disalin |

```cpp
// Riwayat: 101 <-> 102 <-> 103
int n = riwayatMundur(tail, keluaran);
// n bernilai 3, keluaran berisi {103, 102, 101}

// Riwayat kosong
int n = riwayatMundur(nullptr, keluaran);
// n bernilai 0, keluaran tidak disentuh sama sekali
```

**Yang perlu diingat.**
- Penulisan dimulai dari `keluaran[0]` dan rapat tanpa lubang.
- Riwayat kosong menghasilkan 0, dan **tidak ada** satu pun penulisan.
- Bagian `keluaran` di luar yang terpakai **tidak boleh disentuh** — ini ikut
  diperiksa.
- Hasil fungsi ini dan `riwayatMaju()` pada riwayat yang sama harus merupakan
  **kebalikan** satu sama lain, dengan banyaknya nomor yang sama.
- Membaca tidak boleh mengubah riwayat. Dipanggil dua kali harus sama persis.

---

# Soal 3 — `hapusHalaman` (30 poin)

> Langkah 4 pada cerita · **linked list ganda** · hapus + sambung ulang dua arah

Keluarkan catatan yang nomornya sama dengan `nomor`, lalu buang dari memori.
Sesudahnya riwayat harus tetap utuh dan tetap dapat ditelusuri dari **kedua
arah**.

Inilah pekerjaan dengan bobot terbesar, karena catatan yang dihapus punya **dua**
sambungan yang harus diurus sekaligus — dan letaknya menentukan penanda mana yang
ikut berpindah.

| Parameter | Artinya |
|---|---|
| `head` | Penanda catatan paling lama, milik pemanggil. Bertanda `&` |
| `tail` | Penanda catatan paling baru, milik pemanggil. Bertanda `&` |
| `nomor` | Nomor halaman yang mau dihapus |
| *kembalian* | `true` bila ada catatan yang benar-benar dihapus |

| Sebelum | Operasi | Sesudah | Yang berpindah |
|---|---|---|---|
| `101 <-> 102 <-> 103` | hapus `102` | `101 <-> 103` | tidak ada |
| `101 <-> 102 <-> 103` | hapus `101` | `102 <-> 103` | `head` |
| `101 <-> 102 <-> 103` | hapus `103` | `101 <-> 102` | `tail` |
| `101` | hapus `101` | `(kosong)` | `head` dan `tail` |

**Yang perlu diingat.**
- Catatan sebelum dan sesudahnya harus tersambung kembali lewat `prev` **maupun**
  `next`. Menyambung satu arah saja adalah kesalahan yang paling sering terjadi:
  riwayat masih terbaca benar dari depan, tetapi rusak dari belakang.
- Bila yang dihapus catatan **pertama**, `head` berpindah dan `prev` milik
  catatan barunya menjadi `nullptr`.
- Bila yang dihapus catatan **terakhir**, `tail` berpindah dan `next` milik
  catatan barunya menjadi `nullptr`.
- Bila yang dihapus **satu-satunya** catatan, `head` dan `tail` sama-sama
  `nullptr`.
- Bila nomornya muncul lebih dari sekali, yang dihapus **hanya** kemunculan
  pertama.
- Bila nomornya tidak ada, kembaliannya `false` dan riwayat **tidak boleh berubah
  sedikit pun** — termasuk tidak boleh ada node yang terbuang.
- Catatan yang dihapus harus dibuang dengan `delete`, **tepat satu** node per
  penghapusan yang berhasil.

---

# Soal 4 — `bukaTab` (25 poin)

> Langkah 5 pada cerita · **linked list circular** · tambah ke lingkaran

Mulai di sini strukturnya **berganti**. Tab memakai `CNode`, yang hanya punya
`next` — tidak ada `prev`. Yang paling penting: `next` milik tab **terakhir**
menunjuk kembali ke tab pertama, sehingga lingkaran ini **tidak punya ujung**.

Tambahkan sebuah tab baru sebagai tab **terakhir** dalam satu putaran, yaitu
tepat sebelum putaran kembali ke `head`.

| Parameter | Artinya |
|---|---|
| `head` | Penanda titik mulai putaran, milik pemanggil. Bertanda `&`. Boleh `nullptr` |
| `nomor` | Nomor tab yang baru dibuka |
| *kembalian* | `true` bila tab baru berhasil dibuka |

| Sebelum | Operasi | Sesudah |
|---|---|---|
| `201 -> 202 -> (kembali ke 201)` | `bukaTab(head, 203)` | `201 -> 202 -> 203 -> (kembali ke 201)` |
| `(kosong)` | `bukaTab(head, 201)` | `201 -> (kembali ke 201)` |

**Yang perlu diingat.**
- Bila sudah ada tab lain, `head` milik pemanggil **tidak boleh** berpindah.
- Bila belum ada tab sama sekali, tab baru menjadi satu-satunya tab dan `next`-nya
  menunjuk **dirinya sendiri**. Dalam keadaan itu `head` ikut berubah.
- Sesudahnya lingkaran harus tetap **tertutup**: berjalan dari `head` lewat
  `next` harus kembali ke `head`, tanpa pernah menemui `nullptr`.
- Untuk menaruh tab baru di posisi terakhir, Anda perlu menemukan dulu tab yang
  `next`-nya menunjuk `head`.
- **Penting:** pencarian itu **tidak boleh** memakai syarat berhenti
  `p != nullptr` seperti Soal 1–3, karena pada lingkaran syarat itu tidak akan
  pernah terpenuhi. Program Anda akan berputar selamanya dan checker menandainya
  **WAKTU HABIS**. Syarat berhentinya adalah "sudah kembali ke titik mulai".
- Setiap pemanggilan menyediakan **tepat satu** node baru.
- Periksa hasilnya dengan `putaranTab()` yang sudah disediakan.

---

## Batasan

- Hanya **empat** fungsi di `src/student.cpp` yang dinilai: `bukaHalaman`,
  `riwayatMundur`, `hapusHalaman`, dan `bukaTab`. Empat fungsi lain sudah
  disediakan dan tidak dinilai. `main()` di bagian paling bawah file itu bebas
  Anda ubah dan tidak ikut dinilai.
- `cin` hanya boleh dipakai di dalam `main()` tersebut.
- Jangan mengubah `struct DNode`, `struct CNode`, maupun signature fungsi di
  `src/student.h`.
- Jangan mencampur kedua struktur: `DNode` untuk Soal 1–3, `CNode` untuk Soal 4.
- Materi pertemuan ini terbatas pada **linked list ganda** dan **linked list
  circular**. Tidak diperlukan stack, queue, pohon, maupun graf.
- Tidak perlu memakai container pustaka standar (`list`, `vector`, dan
  sejenisnya) — node harus benar-benar dibuat sendiri di memori dinamis dengan
  `new` dan dibebaskan dengan `delete`.

---

## Penilaian Otomatis

Penilaian sepenuhnya berdasarkan **perilaku** program: checker memanggil fungsi
Anda, lalu memeriksa isi dan urutan struktur sesudahnya, arah maju dan arah
mundur, tertutup atau tidaknya lingkaran, node yang ditunjuk, nilai yang
dikembalikan, dan apakah node yang dilepas benar-benar dibebaskan.

Struktur untuk pengujian dibangun sendiri oleh checker, bukan lewat fungsi Anda.
Karena itu satu pekerjaan yang belum benar **tidak** ikut menjatuhkan nilai
pekerjaan yang lain.

Total **20 test case**, masing-masing bernilai **5 poin**:

| Soal | Fungsi | Struktur | Test | Bobot |
|---|---|---|---:|---:|
| 1 | `bukaHalaman` | ganda | 5 | 25 |
| 2 | `riwayatMundur` | ganda | 4 | 20 |
| 3 | `hapusHalaman` | ganda | 6 | 30 |
| 4 | `bukaTab` | circular | 5 | 25 |
| | **Total** | | **20** | **100** |

Tiga soal memakai linked list **ganda** dan satu memakai **circular**: doubly
adalah materi utama pertemuan ini.

| Kondisi | Score |
|---|---|
| Gagal compile | 0 |
| Sebagian test lolos | jumlah test lolos × 5 |
| Semua test lolos | 100 |

Mengerjakan sebagian pekerjaan tetap mendapat nilai.

### Membaca Hasil

| Status | Artinya |
|---|---|
| ✅ hijau | Semua test berhasil |
| ❌ merah | Ada test yang gagal, compile error, atau program berhenti tidak wajar |

Klik run tersebut, lalu baca **Summary** — ada tabel nilai dan rincian setiap
test beserta keterangan `Expected` vs `Got`. Contohnya:

```
Expected: maju=[101 102 103] mundur=[103 102 101] headPrev=nullptr tailNext=nullptr
Got     : maju=[101 102 103] mundur=[103] headPrev=nullptr tailNext=nullptr
```

Contoh ini berarti arah maju sudah benar, tetapi sambungan `prev` terputus
sehingga penelusuran mundur berhenti terlalu awal.

| Step yang gagal | Penyebab |
|---|---|
| `Periksa penggunaan cin` | Ada `cin`/`scanf` di dalam fungsi yang dinilai |
| `Compile student.cpp` | Ada syntax/compile error di `student.cpp` |
| `Compile checker` | Nama atau signature fungsi tidak sesuai `student.h`, atau `main()` Anda keluar dari blok `#ifndef ADA_MAIN_LAIN` |
| `Jalankan test & hitung score` | Kode berhasil dikompilasi tetapi perilakunya belum sesuai |

---

## Cara Menjalankan Program Anda

`src/student.cpp` adalah program C++ utuh. Ada `main()` di bagian paling
bawahnya, jadi Anda bisa menjalankannya seperti tugas C++ biasa.

**Lewat VS Code (paling gampang).** Buka `src/student.cpp`, lalu tekan **F5**
(Run and Debug) atau **Ctrl+F5** (jalankan tanpa debug). Repo ini sudah membawa
setelannya.

**Lewat terminal.**

```bash
g++ -std=c++17 src/student.cpp -o latihan
./latihan
```

`main()` bawaan menjalankan **sesi Rani secara berurutan**,
dan menampilkan hasil tiap langkah berdampingan dengan jawaban yang benar —
sehingga Anda bisa langsung membandingkan.

> **Saran urutan pengerjaan.** Kerjakan A1 lebih dulu, karena hampir seluruh
> percobaan di `main()` memerlukan riwayat yang sudah terisi.

> **Peringatan.** Bila pencarian Anda di Soal 4 belum pernah berhenti,
> program ini akan ikut menggantung. Hentikan dengan `Ctrl+C`, lalu perbaiki
> fungsinya. Checker tidak ikut menggantung — ia mendeteksi keadaan itu dan
> menandainya gagal.

> **Penting: `cin` hanya di dalam `main()`.**
> Jangan pernah menaruh `cin` di dalam keempat fungsi yang dinilai. Saat
> menilai, checker memanggil fungsi-fungsi itu tanpa memberi masukan apa pun,
> sehingga `cin` di sana membaca sampah — dan nilai Anda berubah-ubah setiap kali
> dinilai, dari kode yang sama persis.

> **Kalau program berhenti mendadak** dengan pesan seperti `Segmentation fault`,
> artinya kode Anda menyentuh memori yang bukan haknya. Perintah ini menunjukkan
> baris persisnya:
>
> ```bash
> g++ -std=c++17 -g -fsanitize=address src/student.cpp -o latihan && ./latihan
> ```

### Memeriksa Kebocoran Memori (opsional, sangat disarankan)

```bash
g++ -std=c++17 -fsanitize=address,leak -g src/student.cpp -o latihan_periksa
./latihan_periksa
```

Bila ada node yang bocor, program melaporkannya di akhir dengan keterangan
`LeakSanitizer: detected memory leaks`.

---

## Cara Menjalankan Test di Komputer Sendiri

Butuh `g++` yang mendukung C++17, dan sistem berbasis Linux/macOS (atau WSL di
Windows) karena checker menjalankan setiap test sebagai proses terpisah:

```bash
chmod +x scripts/run_tests.sh
./scripts/run_tests.sh
```

Alurnya sama persis dengan yang dijalankan GitHub Actions.

Pada starter code yang belum diisi, compiler memunculkan peringatan
*unused parameter*. Itu wajar dan **tidak** mengurangi nilai.

---

## Cara Mengumpulkan

Tidak ada tombol "submit". **Push adalah pengumpulan.**

```bash
git clone https://github.com/<ORG>/praktikum-04-<username>.git
cd praktikum-04-<username>
# edit src/student.cpp
git add src/student.cpp
git commit -m "Kerjakan pertemuan 4"
git push
```

Lalu buka tab **Actions** di GitHub, atau lihat nilainya di aplikasi praktikum.

Anda boleh push berkali-kali. **Setiap percobaan tersimpan**, misalnya
45 → 75 → 100.

---

## Struktur Repository

```
.
├── .github/workflows/test.yml     ← workflow penilaian (jangan diubah)
├── .vscode/                       ← setelan tombol Run (jangan diubah)
├── src/
│   ├── student.h                  ← kontrak/interface (jangan diubah)
│   └── student.cpp                ← KERJAKAN DI SINI ← (main() ada di bawahnya)
├── tests/
│   ├── checker.cpp                ← test instruktur (jangan diubah)
│   └── report.h                   ← penulis result.json (jangan diubah)
├── scripts/
│   ├── run_tests.sh               ← uji lokal
│   ├── periksa_masukan.py         ← menolak cin di dalam fungsi yang dinilai
│   ├── anotasi_gcc.py             ← terjemahan error compiler ke bahasa Indonesia
│   ├── job_summary.py             ← Job Summary dari result.json
│   └── write_error_result.sh      ← result.json saat compile error
└── README.md
```
