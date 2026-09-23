// =============================================================================
// checker.cpp — Test Harness Instruktur
// Pertemuan 4: Linked List Ganda (Doubly) & Linked List Circular
// =============================================================================
// File ini adalah milik instruktur dan TIDAK boleh diubah mahasiswa.
//
// PRINSIP PENILAIAN (behavior-based):
//   Checker hanya memanggil fungsi mahasiswa lalu memeriksa akibatnya: isi dan
//   urutan struktur sesudah pemanggilan, arah maju dan arah mundur, tertutup
//   atau tidaknya lingkaran, node yang ditunjuk, nilai yang dikembalikan, dan
//   apakah memori node yang dilepas benar-benar dibebaskan. Nama variabel, gaya
//   penulisan pointer, jenis loop, urutan kode, komentar, dan formatting tidak
//   pernah diperiksa. Implementasi apa pun yang memenuhi kontrak di student.h
//   akan PASS.
//
// BOBOT:
//   20 test case, bobot rata 100 / 20 = 5 poin per test (lihat report.h).
//
//   Pertemuan ini hanya punya SATU soal, yaitu study case peramban "Jelajah".
//   Keempat fungsi di bawah adalah pekerjaan di dalam study case yang sama,
//   bukan soal yang berdiri sendiri-sendiri. Masing-masing tetap diberi suite
//   terpisah supaya nilai parsial tetap dapat.
//
//     Soal 1  bukaHalaman     5 test = 25   doubly, tambah di akhir
//     Soal 2  riwayatMundur   4 test = 20   doubly, telusuri dari tail
//     Soal 3  hapusHalaman    6 test = 30   doubly, sambung ulang dua arah
//     Soal 4  bukaTab         5 test = 25   circular, tambah ke lingkaran
//
//   Tiga soal memakai linked list ganda dan satu memakai circular: doubly
//   adalah materi utama pertemuan ini.
//
//   Empat fungsi lain (riwayatMaju, putaranTab, hapusSeluruhRiwayat,
//   tutupSemuaTab) SUDAH DISEDIAKAN lengkap di src/student.cpp dan TIDAK
//   dinilai. Checker tidak mengujinya, dan tidak memakainya untuk membangun
//   daftar uji.
//
// SOAL SALING BEBAS:
//   Struktur untuk pengujian dibangun oleh checker sendiri (lihat buatGanda dan
//   buatCircular), bukan lewat fungsi mahasiswa. Dengan begitu satu soal yang
//   salah tidak ikut menjatuhkan nilai soal yang lain.
//
//   Pengecualiannya adalah test "berturut-turut dari daftar kosong" pada Soal 1,
//   2, dan 9, yang memang bertujuan menguji pembangunan struktur dari nol dan
//   karena itu wajar memakai fungsi mahasiswa berulang kali.
//
// ISOLASI PROSES (bagian terpenting pertemuan ini):
//   Linked list circular membuat satu kelas kesalahan baru yang tidak ada di
//   pertemuan sebelumnya: penelusuran yang TIDAK PERNAH BERHENTI. Bila fungsi
//   seperti itu dipanggil langsung, checker akan menggantung dan job CI habis
//   waktunya tanpa nilai apa pun.
//
//   Karena itu setiap test dijalankan di dalam PROSES ANAK hasil fork() dengan
//   batas waktu. Anak membangun strukturnya sendiri, memanggil fungsi
//   mahasiswa, lalu menuliskan RINGKASAN KEADAAN AKHIR sebagai teks; induk
//   hanya membandingkan teks itu dengan teks yang diharapkan. Akibatnya:
//     - traversal yang tidak berhenti  -> terdeteksi sebagai "WAKTU HABIS"
//     - program berhenti tidak wajar   -> terdeteksi sebagai FAIL biasa
//     - kedua-duanya TIDAK menghentikan test-test berikutnya
//   Anak juga dibatasi pemakaian memorinya, supaya loop yang terus menumpuk
//   teks tidak menghabiskan memori runner.
//
//   Isolasi ini murni soal ketahanan checker. Yang dinilai tetap perilaku
//   fungsi mahasiswa, dan mekanisme grading global (report.h, scripts/,
//   workflow, skema result.json) tidak diubah sama sekali.
//
// PEMERIKSAAN PEMBEBASAN MEMORI:
//   Checker mengganti operator new/delete global agar dapat menghitung berapa
//   blok memori dinamis yang masih hidup. Angka itu hanya dibaca sebagai
//   SELISIH pada potongan kode yang sangat pendek (persis sebelum dan sesudah
//   satu pemanggilan fungsi mahasiswa), sehingga alokasi milik checker sendiri
//   tidak ikut terhitung. Ini tetap pengujian perilaku: yang diuji adalah "node
//   yang dilepas benar-benar dibebaskan", bukan cara penulisan kodenya.
//
// KETAHANAN TERHADAP CRASH:
//   Checker menyimpan snapshot result.json setiap kali satu test selesai,
//   dengan test yang belum sempat berjalan dicatat sebagai FAIL. Nilai parsial
//   yang sudah diperoleh tetap tercatat dan tidak berubah menjadi 0.
//
// CATATAN KEAMANAN:
//   Mahasiswa dapat membaca file ini. Mitigasi:
//   - Setiap kontrak diuji dengan beberapa bentuk struktur (kosong, satu node,
//     beberapa node) dan beberapa posisi (depan, tengah, belakang)
//   - Repository mahasiswa bersifat privat
// =============================================================================

#include <csignal>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <new>
#include <sstream>
#include <string>
#include <vector>

#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

#include "report.h"
#include "../src/student.h"

using namespace std;

// =============================================================================
// Penghitung blok memori dinamis yang masih hidup
// =============================================================================
// Mengganti operator new/delete global adalah cara yang sah dalam C++ dan
// berlaku untuk seluruh program, termasuk `new DNode` / `new CNode` di dalam
// student.cpp.

static long long g_blokHidup = 0;

void* operator new(size_t ukuran) {
    if (ukuran == 0) ukuran = 1;
    void* blok = malloc(ukuran);
    if (blok == nullptr) throw bad_alloc();
    ++g_blokHidup;
    return blok;
}

void* operator new[](size_t ukuran) {
    return ::operator new(ukuran);
}

void operator delete(void* blok) noexcept {
    if (blok != nullptr) {
        --g_blokHidup;
        free(blok);
    }
}

void operator delete[](void* blok) noexcept {
    ::operator delete(blok);
}

void operator delete(void* blok, size_t) noexcept {
    ::operator delete(blok);
}

void operator delete[](void* blok, size_t) noexcept {
    ::operator delete(blok);
}

static long long blokHidup() {
    return g_blokHidup;
}

// =============================================================================
// Test Framework (sederhana, tanpa dependency eksternal)
// =============================================================================

// ANSI color codes untuk output terminal yang jelas
#define COLOR_GREEN "\033[32m"
#define COLOR_RED   "\033[31m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_CYAN  "\033[36m"
#define COLOR_RESET "\033[0m"
#define COLOR_BOLD  "\033[1m"

static int total_tests = 0;
static int passed_tests = 0;
static int failed_tests = 0;

// Banyaknya test yang direncanakan pada pertemuan ini. Dipakai untuk menghitung
// skor snapshot supaya test yang belum berjalan tetap terhitung sebagai gagal.
static const int TOTAL_TEST_DIRENCANAKAN = 20;

// Menulis result.json versi sementara setelah setiap test selesai.
// Test yang belum dijalankan ditambahkan sebagai FAIL, lalu dilepas kembali,
// sehingga isi rekaman sebenarnya tidak terpengaruh.
static void simpanHasilSementara() {
    vector<TestRecord>& rekaman = test_records();
    const size_t jumlahAsli = rekaman.size();

    for (size_t i = jumlahAsli;
         i < static_cast<size_t>(TOTAL_TEST_DIRENCANAKAN); ++i) {
        rekaman.push_back(TestRecord{
            "(test #" + to_string(i + 1) + " belum dijalankan)",
            "FAIL",
            "Checker berhenti sebelum test ini sempat dijalankan."});
    }

    const int skor = passed_tests * 100 / TOTAL_TEST_DIRENCANAKAN;
    write_result_json("result.json", skor);

    rekaman.resize(jumlahAsli);
}

// Mencatat satu hasil test. Dipakai oleh makro UJI di bawah.
static void catatHasil(const string& nama, bool lulus,
                       const string& keterangan) {
    total_tests++;
    if (lulus) {
        passed_tests++;
        cout << COLOR_GREEN << "  [PASS]" << COLOR_RESET << " " << nama
                  << endl;
    } else {
        failed_tests++;
        cout << COLOR_RED << "  [FAIL]" << COLOR_RESET << " " << nama
                  << endl;
        cout << "         Keterangan: " << keterangan << endl;
    }
    record_test(nama, lulus, lulus ? "" : keterangan);
    simpanHasilSementara();
}

// =============================================================================
// Menjalankan satu test di dalam proses anak dengan batas waktu
// =============================================================================

// Batas waktu satu test. Implementasi yang benar selesai dalam hitungan
// milidetik; batas ini semata-mata menangkap penelusuran yang tidak berhenti.
static const int BATAS_DETIK = 5;

// Batas pemakaian memori proses anak (512 MB), supaya loop yang terus menumpuk
// teks tidak menghabiskan memori runner sebelum batas waktu tercapai.
static const rlim_t BATAS_MEMORI = static_cast<rlim_t>(512) * 1024 * 1024;

// Batas panjang teks hasil, jauh di bawah kapasitas pipe (64 KB) sehingga anak
// tidak pernah terhalang saat menulis.
static const size_t BATAS_KELUARAN = 8000;

enum KeadaanAnak { ANAK_SELESAI, ANAK_WAKTU_HABIS, ANAK_BERHENTI };

struct HasilAnak {
    KeadaanAnak keadaan;
    string teks;
    int penyebab;   // nomor sinyal atau kode keluar, sesuai keadaan
};

typedef string (*FungsiUji)();

static HasilAnak jalankanTerisolasi(FungsiUji uji) {
    HasilAnak hasil;
    hasil.keadaan = ANAK_BERHENTI;
    hasil.penyebab = 0;

    int pipa[2];
    if (pipe(pipa) != 0) {
        hasil.teks = "(checker gagal menyiapkan pipe)";
        return hasil;
    }

    cout.flush();
    cerr.flush();

    pid_t anak = fork();
    if (anak < 0) {
        close(pipa[0]);
        close(pipa[1]);
        hasil.teks = "(checker gagal membuat proses anak)";
        return hasil;
    }

    if (anak == 0) {
        // ---------------------------------------------------------------
        // Proses anak: di sinilah fungsi mahasiswa benar-benar dipanggil.
        // ---------------------------------------------------------------
        close(pipa[0]);

        struct rlimit batas;
        batas.rlim_cur = BATAS_MEMORI;
        batas.rlim_max = BATAS_MEMORI;
        setrlimit(RLIMIT_AS, &batas);

        string keluaran;
        try {
            keluaran = uji();
        } catch (const exception& e) {
            keluaran = string("(program melempar exception: ") + e.what() + ")";
        } catch (...) {
            keluaran = "(program melempar exception)";
        }
        if (keluaran.size() > BATAS_KELUARAN) {
            keluaran.resize(BATAS_KELUARAN);
            keluaran += "...(dipotong)";
        }

        const char* data = keluaran.c_str();
        size_t sisa = keluaran.size();
        while (sisa > 0) {
            ssize_t ditulis = write(pipa[1], data, sisa);
            if (ditulis <= 0) break;
            data += ditulis;
            sisa -= static_cast<size_t>(ditulis);
        }
        close(pipa[1]);
        _exit(0);
    }

    // -------------------------------------------------------------------
    // Proses induk: menunggu anak, dengan batas waktu.
    // -------------------------------------------------------------------
    close(pipa[1]);

    int status = 0;
    bool berakhir = false;
    for (int i = 0; i < BATAS_DETIK * 100; ++i) {
        pid_t hasilTunggu = waitpid(anak, &status, WNOHANG);
        if (hasilTunggu == anak) { berakhir = true; break; }
        if (hasilTunggu < 0) { berakhir = true; break; }
        struct timespec jeda;
        jeda.tv_sec = 0;
        jeda.tv_nsec = 10L * 1000L * 1000L;   // 10 ms
        nanosleep(&jeda, nullptr);
    }

    if (!berakhir) {
        kill(anak, SIGKILL);
        waitpid(anak, &status, 0);
    }

    string teks;
    char penyangga[4096];
    ssize_t dibaca;
    while ((dibaca = read(pipa[0], penyangga, sizeof(penyangga))) > 0) {
        teks.append(penyangga, static_cast<size_t>(dibaca));
    }
    close(pipa[0]);

    if (!berakhir) {
        hasil.keadaan = ANAK_WAKTU_HABIS;
        hasil.penyebab = BATAS_DETIK;
    } else if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        hasil.keadaan = ANAK_SELESAI;
    } else if (WIFSIGNALED(status)) {
        hasil.keadaan = ANAK_BERHENTI;
        hasil.penyebab = WTERMSIG(status);
    } else {
        hasil.keadaan = ANAK_BERHENTI;
        hasil.penyebab = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    }
    hasil.teks = teks;
    return hasil;
}

// Menerjemahkan hasil proses anak menjadi teks "Got" yang dibaca manusia.
static string bacaHasil(const HasilAnak& hasil) {
    switch (hasil.keadaan) {
        case ANAK_SELESAI:
            return hasil.teks;
        case ANAK_WAKTU_HABIS: {
            ostringstream out;
            out << "(operasi tidak pernah berhenti — dihentikan setelah "
                << hasil.penyebab << " detik; kemungkinan penelusuran yang "
                   "tidak menemukan titik berhenti)";
            return out.str();
        }
        default: {
            ostringstream out;
            out << "(program berhenti tidak wajar";
            if (hasil.penyebab == SIGSEGV) {
                out << ": mengakses memori yang tidak sah (SIGSEGV)";
            } else if (hasil.penyebab == SIGABRT) {
                out << ": program dihentikan paksa (SIGABRT), mis. membebaskan "
                       "node yang sama dua kali";
            } else if (hasil.penyebab != 0) {
                out << ": sinyal/kode " << hasil.penyebab;
            }
            out << ")";
            if (!hasil.teks.empty()) out << " keluaran sebagian: " << hasil.teks;
            return out.str();
        }
    }
}

// Makro utama: jalankan `fungsi` secara terisolasi, bandingkan teks hasilnya
// dengan `harapan`.
#define UJI(nama, fungsi, harapan) do { \
    HasilAnak _h = jalankanTerisolasi(fungsi); \
    string _dapat = bacaHasil(_h); \
    string _harap = (harapan); \
    bool _ok = (_h.keadaan == ANAK_SELESAI) && (_dapat == _harap); \
    ostringstream _pesan; \
    _pesan << "Expected: " << _harap << " | Got: " << _dapat; \
    catatHasil((nama), _ok, _pesan.str()); \
} while (0)

// =============================================================================
// Utilitas struktur milik checker (dipakai di dalam proses anak)
// =============================================================================

// Batas langkah penelusuran saat checker MENGGAMBARKAN sebuah struktur.
// Struktur yang benar pada pertemuan ini tidak pernah melebihi belasan node,
// jadi batas ini hanya berlaku untuk struktur yang rusak.
static const int BATAS_TELUSUR = 200;

static const char* bo(bool nilai) { return nilai ? "true" : "false"; }

// --- Linked list ganda -------------------------------------------------------

// Membangun daftar ganda uji tanpa memakai fungsi mahasiswa.
static void buatGanda(const int* nilai, int n, DNode*& head, DNode*& tail) {
    head = nullptr;
    tail = nullptr;
    for (int i = 0; i < n; ++i) {
        DNode* baru = new DNode;
        baru->data = nilai[i];
        baru->prev = tail;
        baru->next = nullptr;
        if (tail == nullptr) head = baru; else tail->next = baru;
        tail = baru;
    }
}

// Isi daftar dibaca maju dari `head`, dipisahkan spasi.
static string telusurMaju(DNode* head) {
    ostringstream out;
    int n = 0;
    for (DNode* p = head; p != nullptr; p = p->next) {
        if (n > 0) out << " ";
        if (++n > BATAS_TELUSUR) { out << "...(tidak berujung)"; break; }
        out << p->data;
    }
    return out.str();
}

// Isi daftar dibaca mundur dari `tail`, dipisahkan spasi.
static string telusurMundur(DNode* tail) {
    ostringstream out;
    int n = 0;
    for (DNode* p = tail; p != nullptr; p = p->prev) {
        if (n > 0) out << " ";
        if (++n > BATAS_TELUSUR) { out << "...(tidak berujung)"; break; }
        out << p->data;
    }
    return out.str();
}

// Ringkasan keadaan sebuah daftar ganda: isi dari kedua arah, ditambah keadaan
// kedua ujungnya. Inilah teks yang dibandingkan dengan harapan.
static string gambarGanda(DNode* head, DNode* tail) {
    ostringstream out;
    out << "maju=[" << telusurMaju(head) << "]";
    out << " mundur=[" << telusurMundur(tail) << "]";
    if (head == nullptr && tail == nullptr) {
        out << " head=nullptr tail=nullptr";
    } else if (head == nullptr) {
        out << " head=nullptr tail=terisi";
    } else if (tail == nullptr) {
        out << " head=terisi tail=nullptr";
    } else {
        out << " headPrev=" << (head->prev == nullptr ? "nullptr" : "terisi");
        out << " tailNext=" << (tail->next == nullptr ? "nullptr" : "terisi");
    }
    return out.str();
}



// --- Linked list circular ----------------------------------------------------

// Membangun daftar circular uji tanpa memakai fungsi mahasiswa.
static CNode* buatCircular(const int* nilai, int n) {
    if (n <= 0) return nullptr;
    CNode* head = nullptr;
    CNode* ekor = nullptr;
    for (int i = 0; i < n; ++i) {
        CNode* baru = new CNode;
        baru->data = nilai[i];
        baru->next = nullptr;
        if (ekor == nullptr) head = baru; else ekor->next = baru;
        ekor = baru;
    }
    ekor->next = head;
    return head;
}

// Ringkasan keadaan sebuah daftar circular: isi satu putaran dari `head`,
// ditambah keterangan apakah lingkarannya benar-benar tertutup.
static string gambarCircular(CNode* head) {
    if (head == nullptr) return "putaran=[] head=nullptr";

    ostringstream isi;
    string tutup = "ya";
    CNode* p = head;
    int n = 0;
    while (true) {
        if (n > 0) isi << " ";
        isi << p->data;
        ++n;
        CNode* lanjut = p->next;
        if (lanjut == head) break;
        if (lanjut == nullptr) { tutup = "tidak(putus di nullptr)"; break; }
        if (n >= BATAS_TELUSUR) { tutup = "tidak(tidak kembali ke head)"; break; }
        p = lanjut;
    }
    return "putaran=[" + isi.str() + "] tutup=" + tutup;
}


// --- Penampung keluaran untuk riwayatMaju / riwayatMundur / putaranTab --------

// Nilai penanda yang diisikan ke seluruh penampung sebelum fungsi mahasiswa
// dipanggil. Bagian yang masih bernilai penanda ini berarti tidak tersentuh.
static const int SENTINEL = -999999;

static void isiSentinel(int* penampung, int kapasitas) {
    for (int i = 0; i < kapasitas; ++i) penampung[i] = SENTINEL;
}

// Ringkasan hasil sebuah operasi salin: banyaknya angka yang dilaporkan, isi
// bagian yang terpakai, dan apakah sisa penampung masih utuh.
static string gambarSalin(int banyak, const int* penampung, int kapasitas) {
    ostringstream out;
    out << "n=" << banyak << " isi=[";
    if (banyak >= 0 && banyak <= kapasitas) {
        for (int i = 0; i < banyak; ++i) {
            if (i > 0) out << " ";
            out << penampung[i];
        }
    } else {
        out << "?";
    }
    out << "]";

    bool sisaUtuh = (banyak >= 0 && banyak <= kapasitas);
    if (sisaUtuh) {
        for (int i = banyak; i < kapasitas; ++i) {
            if (penampung[i] != SENTINEL) { sisaUtuh = false; break; }
        }
    }
    out << " sisa=" << (sisaUtuh ? "utuh" : "tersentuh");
    return out.str();
}

// =============================================================================
// SOAL 1 — sisipkanRiwayatLama  (2 test = 8 poin)
// =============================================================================



// =============================================================================
// SOAL 2 — bukaHalaman  (2 test = 8 poin)
// =============================================================================

// Menambah di belakang, pada daftar kosong dan pada daftar berisi beberapa
// angka.
static string uji03() {
    ostringstream out;

    DNode* hKosong = nullptr;
    DNode* tKosong = nullptr;
    bool r1 = bukaHalaman(hKosong, tKosong, 10);
    out << "kosong: ret=" << bo(r1) << " " << gambarGanda(hKosong, tKosong);

    const int awal[] = {10, 20, 30};
    DNode* h = nullptr;
    DNode* t = nullptr;
    buatGanda(awal, 3, h, t);
    bool r2 = bukaHalaman(h, t, 40);
    out << " | isi: ret=" << bo(r2) << " " << gambarGanda(h, t);

    return out.str();
}

// Membangun daftar dari kosong dengan penambahan berturut-turut di belakang.
static string uji04() {
    long long sebelum = blokHidup();
    DNode* h = nullptr;
    DNode* t = nullptr;
    bukaHalaman(h, t, 10);
    bukaHalaman(h, t, -5);
    bukaHalaman(h, t, 10);
    bool terakhir = bukaHalaman(h, t, 0);
    long long sesudah = blokHidup();

    ostringstream out;
    out << "ret=" << bo(terakhir) << " " << gambarGanda(h, t)
        << " nodeBaru=" << ((sesudah - sebelum) >= 4 ? "ya" : "tidak");
    return out.str();
}

// =============================================================================
// SOAL 3 — riwayatMaju  (2 test = 8 poin)
// =============================================================================



// =============================================================================
// SOAL 4 — riwayatMundur  (2 test = 8 poin)
// =============================================================================

// Tiga bentuk daftar: beberapa node, satu node, dan kosong.
static string uji07() {
    ostringstream out;

    const int tiga[] = {10, -20, 30};
    DNode* h3 = nullptr;
    DNode* t3 = nullptr;
    buatGanda(tiga, 3, h3, t3);
    int k3[8];
    isiSentinel(k3, 8);
    int n3 = riwayatMundur(t3, k3);
    out << "tiga: " << gambarSalin(n3, k3, 8);

    const int satu[] = {7};
    DNode* h1 = nullptr;
    DNode* t1 = nullptr;
    buatGanda(satu, 1, h1, t1);
    int k1[8];
    isiSentinel(k1, 8);
    int n1 = riwayatMundur(t1, k1);
    out << " | satu: " << gambarSalin(n1, k1, 8);

    int k0[8];
    isiSentinel(k0, 8);
    int n0 = riwayatMundur(nullptr, k0);
    out << " | kosong: " << gambarSalin(n0, k0, 8);

    return out.str();
}

// Penampung jauh lebih besar daripada isi daftar, hasilnya harus merupakan
// kebalikan persis dari urutan aslinya, dan daftarnya tidak boleh berubah.
//
// Pembandingnya adalah urutan yang dibangun checker sendiri, BUKAN hasil
// riwayatMaju — supaya Soal 3 yang salah tidak ikut menjatuhkan nilai Soal 4.
static string uji08() {
    const int isi[] = {5, 5, 0, -3, 9};
    DNode* h = nullptr;
    DNode* t = nullptr;
    buatGanda(isi, 5, h, t);

    int k[12];
    isiSentinel(k, 12);
    int n = riwayatMundur(t, k);

    bool terbalik = (n == 5);
    for (int i = 0; i < n && terbalik; ++i) {
        if (k[i] != isi[5 - 1 - i]) terbalik = false;
    }

    ostringstream out;
    out << gambarSalin(n, k, 12) << " terbalik=" << bo(terbalik)
        << " | " << gambarGanda(h, t);
    return out.str();
}

// =============================================================================
// SOAL 5 — cariHalaman  (2 test = 8 poin)
// =============================================================================



// =============================================================================
// SOAL 6 — hapusHalaman  (3 test = 12 poin)
// =============================================================================

// Node pertama dan node terakhir: `head` dan `tail` milik pemanggil harus
// berpindah, dan ujung yang baru harus ditutup dengan nullptr.
static string uji11() {
    ostringstream out;

    const int isi[] = {10, 20, 30};
    DNode* hDepan = nullptr;
    DNode* tDepan = nullptr;
    buatGanda(isi, 3, hDepan, tDepan);
    bool rDepan = hapusHalaman(hDepan, tDepan, 10);
    out << "depan: ret=" << bo(rDepan) << " " << gambarGanda(hDepan, tDepan);

    DNode* hBelakang = nullptr;
    DNode* tBelakang = nullptr;
    buatGanda(isi, 3, hBelakang, tBelakang);
    bool rBelakang = hapusHalaman(hBelakang, tBelakang, 30);
    out << " | belakang: ret=" << bo(rBelakang) << " "
        << gambarGanda(hBelakang, tBelakang);

    return out.str();
}

// Node tengah: hubungan dua arah harus tersambung kembali. Data berulang: hanya
// kemunculan pertama yang keluar.
static string uji12() {
    ostringstream out;

    const int isi[] = {10, 20, 30, 40};
    DNode* h = nullptr;
    DNode* t = nullptr;
    buatGanda(isi, 4, h, t);
    long long sebelum = blokHidup();
    bool rTengah = hapusHalaman(h, t, 20);
    long long sesudah = blokHidup();
    out << "tengah: ret=" << bo(rTengah) << " " << gambarGanda(h, t)
        << " dibuang=" << ((sebelum - sesudah) >= 1 ? "ya" : "tidak");

    const int berulang[] = {7, 3, 7};
    DNode* hd = nullptr;
    DNode* td = nullptr;
    buatGanda(berulang, 3, hd, td);
    bool rDup = hapusHalaman(hd, td, 7);
    out << " | berulang: ret=" << bo(rDup) << " " << gambarGanda(hd, td);

    return out.str();
}

// Satu-satunya node, data tidak tersedia, daftar kosong, dan pemeriksaan bahwa
// node yang dikeluarkan benar-benar dibuang dari memori.
static string uji13() {
    ostringstream out;

    const int satu[] = {10};
    DNode* h1 = nullptr;
    DNode* t1 = nullptr;
    buatGanda(satu, 1, h1, t1);
    long long sebelum = blokHidup();
    bool rSatu = hapusHalaman(h1, t1, 10);
    long long sesudah = blokHidup();
    out << "satu: ret=" << bo(rSatu) << " " << gambarGanda(h1, t1)
        << " dibuang=" << ((sebelum - sesudah) >= 1 ? "ya" : "tidak");

    const int isi[] = {10, 20, 30};
    DNode* h = nullptr;
    DNode* t = nullptr;
    buatGanda(isi, 3, h, t);
    bool rTidakAda = hapusHalaman(h, t, 99);
    out << " | tidakAda: ret=" << bo(rTidakAda) << " " << gambarGanda(h, t);

    DNode* hKosong = nullptr;
    DNode* tKosong = nullptr;
    bool rKosong = hapusHalaman(hKosong, tKosong, 10);
    out << " | kosong: ret=" << bo(rKosong) << " "
        << gambarGanda(hKosong, tKosong);

    return out.str();
}

// =============================================================================
// SOAL 7 — hapusSeluruhRiwayat  (1 test = 4 poin)
// =============================================================================


// =============================================================================
// SOAL 8 — tambahAwalCircular  (2 test = 8 poin)
// =============================================================================



// =============================================================================
// SOAL 9 — bukaTab  (2 test = 8 poin)
// =============================================================================

// Daftar kosong dan lingkaran berisi. Pada lingkaran berisi, `head` TIDAK boleh
// berpindah.
static string uji17() {
    ostringstream out;

    CNode* hKosong = nullptr;
    bool r1 = bukaTab(hKosong, 10);
    string sendiri = "tidak";
    if (hKosong != nullptr && hKosong->next == hKosong) sendiri = "ya";
    out << "kosong: ret=" << bo(r1) << " " << gambarCircular(hKosong)
        << " menunjukDiriSendiri=" << sendiri;

    const int isi[] = {10, 20, 30};
    CNode* head = buatCircular(isi, 3);
    CNode* headLama = head;
    bool r2 = bukaTab(head, 40);
    out << " | isi: ret=" << bo(r2) << " " << gambarCircular(head)
        << " headTetap=" << bo(head == headLama);

    return out.str();
}

// Membangun lingkaran dari kosong dengan penambahan berturut-turut di belakang,
// memakai nilai negatif, nol, dan berulang.
static string uji18() {
    long long sebelum = blokHidup();
    CNode* head = nullptr;
    bukaTab(head, 10);
    bukaTab(head, -5);
    bukaTab(head, 10);
    bool terakhir = bukaTab(head, 0);
    long long sesudah = blokHidup();

    ostringstream out;
    out << "ret=" << bo(terakhir) << " " << gambarCircular(head)
        << " nodeBaru=" << ((sesudah - sebelum) >= 4 ? "ya" : "tidak");
    return out.str();
}

// =============================================================================
// SOAL 10 — putaranTab  (2 test = 8 poin)
// =============================================================================



// =============================================================================
// SOAL 11 — cariCircular  (2 test = 8 poin)
// =============================================================================



// =============================================================================
// SOAL 12 — tutupTab  (2 test = 8 poin)
// =============================================================================



// =============================================================================
// SOAL 13 — tutupSemuaTab  (1 test = 4 poin)
// =============================================================================


// =============================================================================
// Test tambahan — memperkuat Bagian A dan titik mulai putaran Bagian B
// =============================================================================

// A1: pada riwayat yang SUDAH berisi, hanya `tail` yang boleh berpindah. `head`
// harus tetap, dan catatan baru harus tersambung ke catatan lama lewat `prev`.
static string uji26() {
    const int isi[] = {101, 102, 103};
    DNode* h = nullptr;
    DNode* t = nullptr;
    buatGanda(isi, 3, h, t);

    DNode* headLama = h;
    DNode* tailLama = t;

    bool r = bukaHalaman(h, t, 104);

    ostringstream out;
    out << "ret=" << bo(r) << " " << gambarGanda(h, t)
        << " headTetap=" << bo(h == headLama)
        << " tailPindah=" << bo(t != tailLama)
        << " sambungPrev=" << bo(t != nullptr && t->prev == tailLama);
    return out.str();
}

// A6: penghapusan BERANTAI sampai riwayat habis. Tiap langkah menggeser head,
// tail, atau keduanya, sehingga penanda yang tidak konsisten pada satu langkah
// pasti ketahuan di langkah berikutnya.
static string uji27() {
    ostringstream out;

    const int isi[] = {10, 20, 30};
    DNode* h = nullptr;
    DNode* t = nullptr;
    buatGanda(isi, 3, h, t);

    bool a = hapusHalaman(h, t, 20);    // di tengah
    out << "sisa2: ret=" << bo(a) << " " << gambarGanda(h, t);

    bool b = hapusHalaman(h, t, 10);    // sekarang yang paling depan
    out << " | sisa1: ret=" << bo(b) << " " << gambarGanda(h, t);

    bool c = hapusHalaman(h, t, 30);    // satu-satunya yang tersisa
    out << " | habis: ret=" << bo(c) << " " << gambarGanda(h, t);

    return out.str();
}


// B1: titik mulai putaran tidak boleh bergeser saat tab baru dibuka, dan nomor
// yang sudah ada tetap boleh dibuka lagi sebagai tab terpisah.
static string uji29() {
    const int isi[] = {201, 202, 203};
    CNode* head = buatCircular(isi, 3);
    CNode* headLama = head;

    bool r1 = bukaTab(head, 204);
    bool r2 = bukaTab(head, 201);

    ostringstream out;
    out << "ret=" << bo(r1 && r2) << " " << gambarCircular(head)
        << " headTetap=" << bo(head == headLama)
        << " headData=" << (head != nullptr ? head->data : -1);
    return out.str();
}

// =============================================================================
// Test tambahan
// =============================================================================

// S1: riwayat panjang. Sambungan yang putus di tengah hanya kelihatan bila
// kedua arah dibaca dan panjangnya dibandingkan.
static string uji30() {
    DNode* h = nullptr;
    DNode* t = nullptr;
    const int isi[] = {1, 2, 3, 4, 5, 6, 7, 8};
    for (int i = 0; i < 8; ++i) bukaHalaman(h, t, isi[i]);

    string maju = telusurMaju(h);
    string mundur = telusurMundur(t);

    ostringstream out;
    out << "maju=[" << maju << "] mundur=[" << mundur << "]"
        << " headPrev=" << (h != nullptr && h->prev == nullptr ? "nullptr" : "terisi")
        << " tailNext=" << (t != nullptr && t->next == nullptr ? "nullptr" : "terisi");
    return out.str();
}

// S1: setiap pemanggilan menyediakan TEPAT satu node, tidak lebih.
static string uji31() {
    DNode* h = nullptr;
    DNode* t = nullptr;

    long long sebelum = blokHidup();
    bool semua = true;
    for (int i = 0; i < 5; ++i) {
        if (!bukaHalaman(h, t, 10 * i)) semua = false;
    }
    long long sesudah = blokHidup();

    ostringstream out;
    out << "semuaTrue=" << bo(semua)
        << " nodeBaru=" << (sesudah - sebelum) << "/5"
        << " " << gambarGanda(h, t);
    return out.str();
}

// S2: hasil mundur harus kebalikan PERSIS dari pembacaan maju milik checker,
// termasuk pada nomor negatif, nol, dan berulang.
static string uji32() {
    const int isi[] = {4, -1, 0, 4, 9, -1};
    DNode* h = nullptr;
    DNode* t = nullptr;
    buatGanda(isi, 6, h, t);

    int k[16];
    isiSentinel(k, 16);
    int n = riwayatMundur(t, k);

    bool cocok = (n == 6);
    for (int i = 0; i < n && cocok; ++i) {
        if (k[i] != isi[6 - 1 - i]) cocok = false;
    }

    ostringstream out;
    out << gambarSalin(n, k, 16) << " kebalikanMaju=" << bo(cocok);
    return out.str();
}

// S2: membaca tidak boleh mengubah riwayat, dan dua pemanggilan berturut-turut
// harus memberi hasil yang sama persis.
static string uji33() {
    const int isi[] = {11, 22, 33};
    DNode* h = nullptr;
    DNode* t = nullptr;
    buatGanda(isi, 3, h, t);

    string sebelum = gambarGanda(h, t);

    int a[8];
    isiSentinel(a, 8);
    int na = riwayatMundur(t, a);

    int b[8];
    isiSentinel(b, 8);
    int nb = riwayatMundur(t, b);

    bool sama = (na == nb);
    for (int i = 0; i < na && sama; ++i) {
        if (a[i] != b[i]) sama = false;
    }

    ostringstream out;
    out << gambarSalin(na, a, 8)
        << " samaDuaKali=" << bo(sama)
        << " riwayatUtuh=" << bo(sebelum == gambarGanda(h, t));
    return out.str();
}

// S3: tepat SATU node dibebaskan per penghapusan yang berhasil, dan TIDAK ADA
// node yang dibebaskan saat penghapusannya gagal.
static string uji34() {
    const int isi[] = {10, 20, 30};
    DNode* h = nullptr;
    DNode* t = nullptr;
    buatGanda(isi, 3, h, t);

    long long a1 = blokHidup();
    bool berhasil = hapusHalaman(h, t, 20);
    long long a2 = blokHidup();

    long long b1 = blokHidup();
    bool gagal = hapusHalaman(h, t, 999);
    long long b2 = blokHidup();

    ostringstream out;
    out << "berhasil: ret=" << bo(berhasil) << " dibuang=" << (a1 - a2) << "/1"
        << " | gagal: ret=" << bo(gagal) << " dibuang=" << (b1 - b2) << "/0"
        << " | " << gambarGanda(h, t);
    return out.str();
}

// S3: pada riwayat panjang, tiga penghapusan di posisi berbeda harus
// meninggalkan riwayat yang tetap utuh dari kedua arah.
static string uji35() {
    const int isi[] = {1, 2, 3, 4, 5, 6, 7};
    DNode* h = nullptr;
    DNode* t = nullptr;
    buatGanda(isi, 7, h, t);

    bool a = hapusHalaman(h, t, 1);    // paling depan
    bool b = hapusHalaman(h, t, 7);    // paling belakang
    bool c = hapusHalaman(h, t, 4);    // tengah

    ostringstream out;
    out << "ret=" << bo(a && b && c) << " " << gambarGanda(h, t);
    return out.str();
}

// S4: lingkaran harus tetap tertutup sesudah banyak tab dibuka, dan panjang
// putarannya tepat — tidak kurang, tidak berulang.
static string uji36() {
    CNode* head = nullptr;
    for (int i = 1; i <= 8; ++i) bukaTab(head, 200 + i);

    ostringstream out;
    out << gambarCircular(head)
        << " headData=" << (head != nullptr ? head->data : -1);
    return out.str();
}

// S4: setiap pemanggilan menyediakan tepat satu node, dan tab pertama menunjuk
// dirinya sendiri.
static string uji37() {
    CNode* head = nullptr;

    long long sebelum = blokHidup();
    bool r1 = bukaTab(head, 201);
    long long sesudah = blokHidup();

    bool menunjukDiriSendiri = (head != nullptr && head->next == head);

    long long a = blokHidup();
    bool r2 = bukaTab(head, 202);
    bool r3 = bukaTab(head, 203);
    long long b = blokHidup();

    ostringstream out;
    out << "pertama: ret=" << bo(r1) << " nodeBaru=" << (sesudah - sebelum) << "/1"
        << " menunjukDiriSendiri=" << bo(menunjukDiriSendiri)
        << " | lanjut: ret=" << bo(r2 && r3) << " nodeBaru=" << (b - a) << "/2"
        << " " << gambarCircular(head);
    return out.str();
}

// =============================================================================
// Pendaftaran test — satu suite per pekerjaan pada study case
// =============================================================================

static void judulSuite(const string& teks) {
    cout << COLOR_CYAN << COLOR_BOLD << "\n[TEST SUITE] " << teks
              << COLOR_RESET << endl;
}


static void suiteSoal1() {
    judulSuite("Soal 1 — bukaHalaman()   (doubly, tambah di akhir)");

    UJI("Riwayat: bukaHalaman pada riwayat kosong dan pada riwayat berisi",
        uji03,
        "kosong: ret=true maju=[10] mundur=[10] headPrev=nullptr tailNext=nullptr"
        " | isi: ret=true maju=[10 20 30 40] mundur=[40 30 20 10] "
        "headPrev=nullptr tailNext=nullptr");

    UJI("Riwayat: bukaHalaman berturut-turut dari kosong, dengan nomor negatif, "
        "nol, dan berulang",
        uji04,
        "ret=true maju=[10 -5 10 0] mundur=[0 10 -5 10] headPrev=nullptr "
        "tailNext=nullptr nodeBaru=ya");

    UJI("Riwayat: pada riwayat berisi hanya tail yang berpindah, dan catatan "
        "baru tersambung ke catatan lama lewat prev",
        uji26,
        "ret=true maju=[101 102 103 104] mundur=[104 103 102 101] "
        "headPrev=nullptr tailNext=nullptr headTetap=true tailPindah=true "
        "sambungPrev=true");

    UJI("Riwayat: riwayat panjang tetap utuh dan saling terbalik dari kedua arah",
        uji30,
        "maju=[1 2 3 4 5 6 7 8] mundur=[8 7 6 5 4 3 2 1] headPrev=nullptr "
        "tailNext=nullptr");

    UJI("Riwayat: setiap pemanggilan menyediakan tepat satu node baru",
        uji31,
        "semuaTrue=true nodeBaru=5/5 maju=[0 10 20 30 40] "
        "mundur=[40 30 20 10 0] headPrev=nullptr tailNext=nullptr");
}


static void suiteSoal2() {
    judulSuite("Soal 2 — riwayatMundur()   (doubly, telusuri dari tail)");

    UJI("Riwayat: riwayatMundur pada daftar tiga node, satu node, dan kosong",
        uji07,
        "tiga: n=3 isi=[30 -20 10] sisa=utuh"
        " | satu: n=1 isi=[7] sisa=utuh"
        " | kosong: n=0 isi=[] sisa=utuh");

    UJI("Riwayat: riwayatMundur menghasilkan kebalikan urutan aslinya tanpa "
        "menyentuh sisa penampung",
        uji08,
        "n=5 isi=[9 -3 0 5 5] sisa=utuh terbalik=true"
        " | maju=[5 5 0 -3 9] mundur=[9 -3 0 5 5] headPrev=nullptr "
        "tailNext=nullptr");

    UJI("Riwayat: riwayatMundur tetap kebalikan persis pada nomor negatif, nol, "
        "dan berulang",
        uji32,
        "n=6 isi=[-1 9 4 0 -1 4] sisa=utuh kebalikanMaju=true");

    UJI("Riwayat: riwayatMundur tidak mengubah riwayat dan sama saat dipanggil "
        "dua kali",
        uji33,
        "n=3 isi=[33 22 11] sisa=utuh samaDuaKali=true riwayatUtuh=true");
}


static void suiteSoal3() {
    judulSuite("Soal 3 — hapusHalaman()   (doubly, sambung ulang dua arah)");

    UJI("Riwayat: hapusHalaman pada catatan pertama dan pada catatan terakhir "
        "memindahkan head/tail",
        uji11,
        "depan: ret=true maju=[20 30] mundur=[30 20] headPrev=nullptr "
        "tailNext=nullptr"
        " | belakang: ret=true maju=[10 20] mundur=[20 10] headPrev=nullptr "
        "tailNext=nullptr");

    UJI("Riwayat: hapusHalaman di tengah menjaga hubungan dua arah, dan hanya "
        "kemunculan pertama yang keluar",
        uji12,
        "tengah: ret=true maju=[10 30 40] mundur=[40 30 10] headPrev=nullptr "
        "tailNext=nullptr dibuang=ya"
        " | berulang: ret=true maju=[3 7] mundur=[7 3] headPrev=nullptr "
        "tailNext=nullptr");

    UJI("Riwayat: hapusHalaman pada satu-satunya catatan, pada nomor yang tidak "
        "ada, dan pada riwayat kosong",
        uji13,
        "satu: ret=true maju=[] mundur=[] head=nullptr tail=nullptr dibuang=ya"
        " | tidakAda: ret=false maju=[10 20 30] mundur=[30 20 10] "
        "headPrev=nullptr tailNext=nullptr"
        " | kosong: ret=false maju=[] mundur=[] head=nullptr tail=nullptr");

    UJI("Riwayat: hapusHalaman berantai sampai riwayat habis menjaga head dan "
        "tail tetap konsisten di setiap langkah",
        uji27,
        "sisa2: ret=true maju=[10 30] mundur=[30 10] headPrev=nullptr "
        "tailNext=nullptr"
        " | sisa1: ret=true maju=[30] mundur=[30] headPrev=nullptr "
        "tailNext=nullptr"
        " | habis: ret=true maju=[] mundur=[] head=nullptr tail=nullptr");

    UJI("Riwayat: tepat satu node dibuang saat berhasil, dan tidak ada yang "
        "dibuang saat nomornya tidak ada",
        uji34,
        "berhasil: ret=true dibuang=1/1 | gagal: ret=false dibuang=0/0"
        " | maju=[10 30] mundur=[30 10] headPrev=nullptr tailNext=nullptr");

    UJI("Riwayat: tiga penghapusan di posisi berbeda pada riwayat panjang tetap "
        "meninggalkan riwayat yang utuh dari kedua arah",
        uji35,
        "ret=true maju=[2 3 5 6] mundur=[6 5 3 2] headPrev=nullptr "
        "tailNext=nullptr");
}



static void suiteSoal4() {
    judulSuite("Soal 4 — bukaTab()   (circular, tambah ke lingkaran)");

    UJI("Tab: bukaTab saat belum ada tab dan saat sudah ada tab, tanpa "
        "memindahkan titik mulai putaran",
        uji17,
        "kosong: ret=true putaran=[10] tutup=ya menunjukDiriSendiri=ya"
        " | isi: ret=true putaran=[10 20 30 40] tutup=ya headTetap=true");

    UJI("Tab: bukaTab berturut-turut dari kosong, dengan nomor negatif, nol, "
        "dan berulang",
        uji18,
        "ret=true putaran=[10 -5 10 0] tutup=ya nodeBaru=ya");

    UJI("Tab: bukaTab tidak menggeser titik mulai putaran, dan nomor yang sudah "
        "ada tetap boleh dibuka lagi sebagai tab terpisah",
        uji29,
        "ret=true putaran=[201 202 203 204 201] tutup=ya headTetap=true "
        "headData=201");

    UJI("Tab: lingkaran tetap tertutup dan panjang putarannya tepat sesudah "
        "delapan tab dibuka",
        uji36,
        "putaran=[201 202 203 204 205 206 207 208] tutup=ya headData=201");

    UJI("Tab: setiap pemanggilan menyediakan tepat satu node, dan tab pertama "
        "menunjuk dirinya sendiri",
        uji37,
        "pertama: ret=true nodeBaru=1/1 menunjukDiriSendiri=true"
        " | lanjut: ret=true nodeBaru=2/2 putaran=[201 202 203] tutup=ya");
}





// =============================================================================
// Main
// =============================================================================

int main() {
    cout << COLOR_BOLD
              << "============================================" << endl;
    cout << " Praktikum Struktur Data C++ — Auto Checker" << endl;
    cout << " Pertemuan 4: Study Case Peramban \"Jelajah\"" << endl;
    cout << "============================================"
              << COLOR_RESET << endl;

    suiteSoal1();   // bukaHalaman     5 test = 25 poin  (doubly)
    suiteSoal2();   // riwayatMundur   4 test = 20 poin  (doubly)
    suiteSoal3();   // hapusHalaman    6 test = 30 poin  (doubly)
    suiteSoal4();   // bukaTab         5 test = 25 poin  (circular)

    // Pengaman untuk instruktur: bobot per test dihitung dari angka rencana,
    // jadi jumlah test yang benar-benar berjalan harus sama dengan rencana.
    if (total_tests != TOTAL_TEST_DIRENCANAKAN) {
        cerr << "PERINGATAN (instruktur): jumlah test berjalan ("
                  << total_tests << ") tidak sama dengan rencana ("
                  << TOTAL_TEST_DIRENCANAKAN << ")." << endl;
    }

    // -----------------------------------------------------------------------
    // Scoring Summary
    // -----------------------------------------------------------------------
    int score = (total_tests > 0) ? (passed_tests * 100 / total_tests) : 0;

    cout << "\n" << COLOR_BOLD
              << "============================================\n"
              << " SCORING SUMMARY\n"
              << "============================================\n"
              << COLOR_RESET;

    cout << " Tests Berhasil : " << COLOR_GREEN << COLOR_BOLD
              << passed_tests << COLOR_RESET << " / " << total_tests << "\n";
    cout << " Tests Gagal    : " << COLOR_RED << COLOR_BOLD
              << failed_tests << COLOR_RESET << " / " << total_tests << "\n";

    // Score line — warna hijau jika sempurna, kuning jika sebagian, merah jika 0
    string score_color = (score == 100) ? COLOR_GREEN
                            : (score > 0)    ? COLOR_YELLOW
                                             : COLOR_RED;
    cout << " Score          : " << score_color << COLOR_BOLD
              << score << " / 100" << COLOR_RESET << "\n";

    cout << COLOR_BOLD
              << "============================================\n"
              << COLOR_RESET;

    // -----------------------------------------------------------------------
    // Hasil yang dapat dibaca mesin.
    // Berkas inilah yang diunggah sebagai artifact dan dibaca aplikasi web.
    // -----------------------------------------------------------------------
    if (!write_result_json("result.json", score)) {
        cerr << "PERINGATAN: gagal menulis result.json" << endl;
    }

    if (failed_tests == 0) {
        cout << COLOR_GREEN << COLOR_BOLD
                  << " STATUS: SEMUA TEST BERHASIL ✓\n"
                  << COLOR_RESET;
        return 0; // exit code 0 = GitHub Actions SUCCESS
    } else {
        cout << COLOR_RED << COLOR_BOLD
                  << " STATUS: " << failed_tests << " TEST GAGAL ✗\n"
                  << COLOR_RESET;
        return 1; // exit code non-zero = GitHub Actions FAIL
    }
}
