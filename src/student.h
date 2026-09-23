// =============================================================================
// student.h — Interface Praktikum
// Pertemuan 4: Linked List Ganda (Doubly) & Linked List Circular
// =============================================================================
// INSTRUKSI UNTUK MAHASISWA:
//   - JANGAN mengubah file ini.
//   - Implementasikan seluruh fungsi di dalam src/student.cpp.
//   - Nama struct, nama field, nama fungsi, tipe parameter, dan tipe kembalian
//     adalah KONTRAK: checker memanggilnya langsung, jadi tidak boleh diubah.
//   - Cara Anda memenuhi kontrak sepenuhnya bebas. Penilaian hanya melihat
//     perilaku yang teramati oleh pemanggil.
//
// -----------------------------------------------------------------------------
// STUDY CASE: PERAMBAN WEB "JELAJAH"
// -----------------------------------------------------------------------------
// Pertemuan ini hanya punya SATU soal, yaitu study case peramban web Jelajah.
// Ceritanya lengkap ada di src/student.cpp. File ini hanya ringkasan kontraknya.
//
// Ada EMPAT pekerjaan yang dinilai:
//
//     Soal 1  bukaHalaman      tambah di akhir              doubly    25 poin
//     Soal 2  riwayatMundur    telusuri dari tail           doubly    20 poin
//     Soal 3  hapusHalaman     hapus, sambung ulang 2 arah  doubly    30 poin
//     Soal 4  bukaTab          tambah ke lingkaran          circular  25 poin
//
// Empat fungsi lain SUDAH DISEDIAKAN lengkap di src/student.cpp dan tidak
// dinilai: `riwayatMaju`, `putaranTab`, `hapusSeluruhRiwayat`, dan
// `tutupSemuaTab`. Keduanya yang pertama dipakai untuk melihat isi daftar, dan
// keduanya yang terakhir untuk membereskan memori.
//
// -----------------------------------------------------------------------------
// KONVENSI UMUM
// -----------------------------------------------------------------------------
//   - Setiap halaman dan setiap tab dikenali dari sebuah NOMOR bertipe `int`.
//     Nomor boleh negatif, boleh nol, dan boleh muncul lebih dari sekali.
//   - `head` bernilai `nullptr` berarti daftarnya sedang KOSONG. Itu keadaan
//     yang sah, bukan kesalahan.
//   - Parameter bertanda `&` berarti penanda milik pemanggil ikut berubah.
//   - Tidak ada fungsi yang mencetak apa pun ke layar.
//   - DNode dan CNode adalah DUA struktur berbeda dan tidak boleh dicampur.
// =============================================================================

#ifndef STUDENT_H
#define STUDENT_H

// =============================================================================
// BAGIAN A — RIWAYAT HALAMAN  (linked list ganda)
// =============================================================================

/**
 * Satu catatan riwayat: sebuah halaman yang pernah dikunjungi.
 *
 *   `data`    nomor halaman
 *   `prev`    catatan SEBELUMNYA — inilah yang dituju tombol Back
 *   `next`    catatan SESUDAHNYA — inilah yang dituju tombol Forward
 *
 * Huruf D pada DNode berarti Doubly. Riwayat dipegang oleh DUA penanda:
 * `head` (catatan paling lama) dan `tail` (catatan paling baru).
 *
 *       head                                  tail
 *        |                                     |
 *       [101] <-> [102] <-> [103] <-> [104]
 *
 * `prev` milik `head` dan `next` milik `tail` sama-sama bernilai nullptr.
 */
struct DNode {
    int data;
    DNode* prev;
    DNode* next;
};

/**
 * SOAL 1 — Pengguna membuka halaman baru, sehingga halaman itu tercatat sebagai
 * kunjungan PALING BARU di ujung belakang riwayat.
 *
 * Kontrak:
 *   - Setelah pemanggilan, catatan baru itulah catatan terakhir riwayat, dan
 *     `tail` milik pemanggil menunjuk node tersebut.
 *   - Seluruh catatan yang sudah ada tetap berada di depan catatan baru dengan
 *     urutan yang sama persis.
 *   - Riwayat yang semula kosong berubah menjadi riwayat berisi satu catatan;
 *     dalam keadaan itu `head` milik pemanggil ikut berubah. Pada riwayat yang
 *     sudah berisi, `head` TIDAK boleh berpindah.
 *   - Catatan baru tersambung dari kedua arah: `prev` miliknya menunjuk catatan
 *     terakhir yang lama, dan `next` milik catatan lama itu menunjuk catatan
 *     baru.
 *   - Sesudahnya riwayat tetap dapat ditelusuri utuh dari kedua arah.
 *   - Kembaliannya `true` bila catatan baru berhasil masuk.
 */
bool bukaHalaman(DNode*& head, DNode*& tail, int nomor);

/**
 * SOAL 2 — Pengguna menekan tombol Back berulang kali sampai riwayatnya habis.
 * Nomor halaman yang dilewatinya disalin ke penampung milik pemanggil, dari
 * kunjungan paling baru menuju kunjungan paling lama.
 *
 * Kontrak:
 *   - Penelusuran dimulai dari `tail` dan berjalan lewat `prev`. Inilah gunanya
 *     linked list GANDA.
 *   - Nomor disalin ke `keluaran` berurutan mulai dari `keluaran[0]`, rapat
 *     tanpa lubang.
 *   - Nilai yang dikembalikan adalah banyaknya nomor yang disalin.
 *   - Riwayat kosong menghasilkan 0 dan tidak ada satu pun penulisan.
 *   - Bagian `keluaran` di luar yang terpakai tidak boleh disentuh.
 *   - Riwayat tidak boleh berubah oleh pemanggilan ini.
 *
 * Contoh: riwayat 101 <-> 102 <-> 103 mengisi keluaran {103, 102, 101} dan
 * mengembalikan 3.
 */
int riwayatMundur(DNode* tail, int* keluaran);

/**
 * SOAL 3 — Pengguna menghapus satu halaman dari riwayatnya. Sesudah itu riwayat
 * harus tetap utuh dan tetap dapat ditelusuri dari KEDUA arah.
 *
 * Kontrak:
 *   - Yang dihapus adalah catatan PERTAMA yang nomornya sama dengan `nomor`.
 *     Kemunculan berikutnya tetap tinggal di riwayat.
 *   - Catatan sebelum dan sesudahnya harus tersambung kembali lewat `prev`
 *     maupun `next`.
 *   - Bila yang dihapus adalah catatan pertama, `head` milik pemanggil harus
 *     berpindah. Bila catatan terakhir, `tail` milik pemanggil harus berpindah.
 *   - Bila yang dihapus adalah satu-satunya catatan, `head` dan `tail`
 *     sama-sama menjadi `nullptr`.
 *   - Catatan yang dihapus memorinya harus dibuang dengan `delete`.
 *   - Kembaliannya `true` bila ada catatan yang benar-benar dihapus. Bila
 *     nomornya tidak tersedia, atau riwayat sedang kosong, kembaliannya `false`
 *     dan riwayat tidak boleh berubah sedikit pun.
 */
bool hapusHalaman(DNode*& head, DNode*& tail, int nomor);

// =============================================================================
// BAGIAN B — PUTARAN TAB  (linked list circular)
// =============================================================================

/**
 * Satu tab yang sedang terbuka.
 *
 *   `data`    nomor tab
 *   `next`    tab BERIKUTNYA — inilah yang dituju Ctrl+Tab
 *
 * Huruf C pada CNode berarti Circular. Tab tidak punya `prev`. Yang membuatnya
 * berbeda dari Bagian A: `next` milik tab TERAKHIR menunjuk kembali ke tab
 * pertama, sehingga lingkaran ini tidak punya ujung.
 *
 *        .--> [201] --> [202] --> [203] --.
 *        '--------------------------------'
 *
 * `head` adalah TITIK MULAI putaran, yaitu tab yang sedang aktif.
 */
struct CNode {
    int data;
    CNode* next;
};

/**
 * SOAL 4 — Pengguna membuka tab baru. Tab itu diletakkan sebagai tab TERAKHIR
 * dalam satu putaran, yaitu tepat sebelum putaran kembali ke `head`.
 *
 * Kontrak:
 *   - Bila sudah ada tab lain, `head` milik pemanggil TIDAK berubah; tab yang
 *     sedang aktif tetap sama.
 *   - Tab baru menjadi yang terakhir dikunjungi dalam satu putaran dari `head`.
 *   - Lingkarannya harus tetap TERTUTUP: berjalan dari `head` terus lewat
 *     `next` harus kembali ke `head`, tanpa pernah menemui `nullptr`.
 *   - Bila belum ada tab sama sekali, tab baru menjadi satu-satunya tab dan
 *     `next`-nya menunjuk DIRINYA SENDIRI; dalam keadaan itu `head` milik
 *     pemanggil ikut berubah.
 *   - Pencarian tab terakhir harus BERHENTI sesudah satu putaran. Penelusuran
 *     yang tidak pernah berhenti dianggap salah dan ikut diuji.
 *   - Kembaliannya `true` bila tab baru berhasil dibuka.
 */
bool bukaTab(CNode*& head, int nomor);

// =============================================================================
// SUDAH DISEDIAKAN — TIDAK DINILAI
// =============================================================================
// Keempat fungsi di bawah sudah ditulis lengkap di src/student.cpp. Anda tidak
// perlu mengerjakannya. Dua yang pertama dipakai untuk melihat isi daftar, dan
// dua yang terakhir untuk membereskan memori sebelum program berakhir.
// =============================================================================

/** Menyalin seluruh riwayat MAJU (dari `head`) ke `keluaran`, mengembalikan
 *  banyaknya nomor. Pasangan pembanding untuk Soal 2. */
int riwayatMaju(DNode* head, int* keluaran);

/** Menyalin satu putaran penuh tab (dari `head`) ke `keluaran`, mengembalikan
 *  banyaknya nomor. */
int putaranTab(CNode* head, int* keluaran);

/** Membuang seluruh catatan riwayat, lalu mengosongkan `head` dan `tail`. */
void hapusSeluruhRiwayat(DNode*& head, DNode*& tail);

/** Membuang seluruh tab, lalu mengosongkan `head`. */
void tutupSemuaTab(CNode*& head);

#endif // STUDENT_H
