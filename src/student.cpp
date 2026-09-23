// =============================================================================
// student.cpp — Implementasi Mahasiswa
// Pertemuan 4: Linked List Ganda (Doubly) & Linked List Circular
// =============================================================================
// FILE YANG BOLEH DIEDIT      : src/student.cpp  ← HANYA FILE INI
// FILE YANG TIDAK BOLEH DIEDIT: src/student.h, tests/checker.cpp, tests/report.h
//
// -----------------------------------------------------------------------------
// DAFTAR PEKERJAAN DAN BOBOTNYA
// -----------------------------------------------------------------------------
//   Soal 1  bukaHalaman     tambah di akhir               doubly     25 poin
//   Soal 2  riwayatMundur   telusuri dari tail            doubly     20 poin
//   Soal 3  hapusHalaman    hapus, sambung ulang 2 arah   doubly     30 poin
//   Soal 4  bukaTab         tambah ke lingkaran           circular   25 poin
//
//   Tiga soal pertama memakai linked list ganda. Soal 4 memakai linked list circular.
//
// MENCOBA SENDIRI:
//   File ini adalah program C++ utuh. Tekan tombol Run di VS Code, atau:
//     g++ -std=c++17 src/student.cpp -o latihan && ./latihan
//   Yang dijalankan adalah main() di bagian paling bawah file ini. main() itu
//   memeragakan seluruh sesi Rani, tidak ikut dinilai, dan bebas Anda ubah.
//
// Sebelum diisi, compiler memunculkan peringatan "unused parameter".
// Itu wajar dan tidak mengurangi nilai.
// =============================================================================

#include "student.h"

#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

// =============================================================================
// SUDAH DISEDIAKAN — TIDAK DINILAI, TIDAK PERLU DIUBAH
// =============================================================================
// Keempat fungsi di bawah sudah ditulis lengkap. Dua yang pertama dipakai untuk
// MELIHAT isi daftar, dan dua yang terakhir untuk membereskan memori.
//
// riwayatMaju() sengaja disediakan sebagai pembanding untuk Soal 2.

int riwayatMaju(DNode* head, int* keluaran) {
    int n = 0;
    for (DNode* p = head; p != nullptr; p = p->next) {
        keluaran[n] = p->data;
        ++n;
    }
    return n;
}

int putaranTab(CNode* head, int* keluaran) {
    if (head == nullptr) return 0;

    int n = 0;
    CNode* p = head;
    do {
        keluaran[n] = p->data;
        ++n;
        p = p->next;
    } while (p != head);
    return n;
}

void hapusSeluruhRiwayat(DNode*& head, DNode*& tail) {
    while (head != nullptr) {
        DNode* berikut = head->next;
        delete head;
        head = berikut;
    }
    tail = nullptr;
}

void tutupSemuaTab(CNode*& head) {
    if (head == nullptr) return;

    CNode* p = head->next;
    while (p != head) {
        CNode* berikut = p->next;
        delete p;
        p = berikut;
    }
    delete head;
    head = nullptr;
}

// =============================================================================

// SOAL 1
bool bukaHalaman(DNode*& head, DNode*& tail, int nomor) {
    DNode* baru = new DNode();
    baru->data = nomor;
    baru->next = nullptr;
    baru->prev = tail;

    if(tail != nullptr){
        tail->next = baru;
    }else{
        head = baru;
    }
    tail = baru;
    return true;
}

// SOAL 2
int riwayatMundur(DNode* tail, int* keluaran) {
    int n = 0;
    for(DNode* p = tail; p != nullptr; p = p->prev){
        keluaran[n] = p->data;
        ++n;
    }
    return n;
}

// SOAL 3
bool hapusHalaman(DNode*& head, DNode*& tail, int nomor) {
    // cocokin nomor
    DNode* p = head;
    while(p != nullptr && p->data != nomor){
        p = p->next;
    }

    if(p == nullptr){
        return false; // tidak ketemu
    }

    // bagian kiri
    if(p->prev != nullptr){
        p->prev->next = p->next;
    }else{
        head = p->next;
    }

    // bagian kanan
    if(p->next != nullptr){
        p->next->prev = p->prev;
    }else{
        tail = p->prev;
    }

    //delete
    delete p;
    return true;
}

// SOAL 4
bool bukaTab(CNode*& head, int nomor) {
    CNode* baru = new CNode();
    baru->data = nomor;

    if(head == nullptr){
        baru->next = baru;
        head = baru;
        return true;
    }

    // cari terakhir
    CNode* p = head;
    while(p->next != head){
        p = p->next;
    }

    p->next = baru;
    baru->next = head;

    return true;
}

// =============================================================================
// MAIN() — memeragakan pagi di loket. TIDAK dinilai, bebas diubah.
// =============================================================================
// Di bawah ini file ini menjadi program C++ biasa. Tekan Run di VS Code, atau
// jalankan lewat terminal:
//
//     g++ -std=c++17 src/student.cpp -o latihan
//     ./latihan
//
// Isinya menjalankan pagi di Klinik Sehat Selalu secara berurutan, dan
// menampilkan hasil tiap langkah berdampingan dengan jawaban yang benar —
// sehingga Anda bisa langsung membandingkan.
//
// SATU ATURAN YANG TIDAK BOLEH DILANGGAR
// --------------------------------------
// cin hanya boleh dipakai DI DALAM main() ini.
//
// (Baris #ifndef di bawah hanya urusan teknis: saat menilai, checker memakai
//  main() miliknya sendiri, jadi main() Anda dilewati supaya tidak bentrok.)
// =============================================================================

#ifndef ADA_MAIN_LAIN

static const int KAPASITAS = 32;

static const char* benarSalah(bool nilai) {
    return nilai ? "true" : "false";
}

static ostream& baris(const string& label) {
    return cout << "    " << left << setw(16) << label << ": ";
}

static void cetakPenampung(const int* penampung, int banyak) {
    cout << "{";
    for (int i = 0; i < banyak && i < KAPASITAS; ++i) {
        if (i > 0) cout << ", ";
        cout << penampung[i];
    }
    cout << "}";
}

static void laporkanSalin(const string& label, int banyak, const int* penampung) {
    baris(label);
    cout << "n=" << banyak << "  isi=";
    if (banyak < 0 || banyak > KAPASITAS) cout << "(nilai kembalian tidak masuk akal)";
    else                                  cetakPenampung(penampung, banyak);
    cout << "\n";
}

// Riwayat dibaca dari KEDUA arah, supaya sambungan yang putus langsung terlihat.
static void keadaanRiwayat(DNode* head, DNode* tail) {
    int penampung[KAPASITAS];
    laporkanSalin("riwayatMaju", riwayatMaju(head, penampung), penampung);
    laporkanSalin("riwayatMundur", riwayatMundur(tail, penampung), penampung);
}

static void keadaanTab(CNode* head) {
    int penampung[KAPASITAS];
    laporkanSalin("putaranTab", putaranTab(head, penampung), penampung);
}

static void langkah(const string& teks) {
    cout << "\n" << teks << "\n";
}

int main() {
    cout << "==================================================\n";
    cout << " Study Case — Peramban Web \"Jelajah\"\n";
    cout << " Memeragakan satu sesi pemakaian oleh Rani\n";
    cout << " (bagian ini tidak ikut dinilai)\n";
    cout << "==================================================\n";

    DNode* head = nullptr;
    DNode* tail = nullptr;

    langkah("[0] Jelajah baru dibuka, riwayat masih kosong");
    keadaanRiwayat(head, tail);

    langkah("[1] SOAL 1 — bukaHalaman: mengunjungi 101, lalu 102, lalu 103");
    bukaHalaman(head, tail, 101);
    bukaHalaman(head, tail, 102);
    bool hasil1 = bukaHalaman(head, tail, 103);
    baris("kembalian") << benarSalah(hasil1) << "\n";
    keadaanRiwayat(head, tail);
    cout << "\n    Yang benar: maju {101, 102, 103}, mundur {103, 102, 101}\n";

    langkah("[2] SOAL 2 — riwayatMundur: menekan Back sampai riwayat habis");
    bukaHalaman(head, tail, 104);
    keadaanRiwayat(head, tail);
    cout << "\n    Yang benar: kedua baris berisi 4 nomor, saling terbalik\n";

    langkah("[3] SOAL 3 — hapusHalaman: menghapus 102 (catatan di tengah)");
    baris("kembalian") << benarSalah(hapusHalaman(head, tail, 102)) << "\n";
    keadaanRiwayat(head, tail);
    cout << "\n    Yang benar: maju {101, 103, 104}, mundur {104, 103, 101}\n";

    langkah("[3b] SOAL 3 — menghapus 101 (paling depan) lalu 104 (paling belakang)");
    hapusHalaman(head, tail, 101);
    hapusHalaman(head, tail, 104);
    keadaanRiwayat(head, tail);
    cout << "\n    Yang benar: maju {103}, mundur {103}\n";

    langkah("[3c] SOAL 3 — menghapus 999 (tidak ada di riwayat)");
    baris("kembalian") << benarSalah(hapusHalaman(head, tail, 999)) << "\n";
    keadaanRiwayat(head, tail);
    cout << "\n    Yang benar: false, dan riwayat tidak berubah\n";

    CNode* tab = nullptr;

    langkah("[4] SOAL 4 — bukaTab: membuka tab 201, lalu 202, lalu 203");
    bukaTab(tab, 201);
    bukaTab(tab, 202);
    bool hasil4 = bukaTab(tab, 203);
    baris("kembalian") << benarSalah(hasil4) << "\n";
    keadaanTab(tab);
    cout << "\n    Yang benar: {201, 202, 203} — tepat 3 tab, tidak berulang\n";
    cout << "    Kalau program menggantung di sini, pencarian tab terakhir\n";
    cout << "    Anda belum punya syarat berhenti yang benar.\n";

    // -------------------------------------------------------------------------
    // Mau mencoba sendiri? Hapus tanda // di bawah ini, lalu jalankan lagi.
    // -------------------------------------------------------------------------
    // int nomor;
    // cout << "\nKetik satu nomor halaman: ";
    // cin >> nomor;
    // bukaHalaman(head, tail, nomor);
    // keadaanRiwayat(head, tail);

    cout << "\n==================================================\n";
    cout << " Sesi selesai. Silakan ubah bagian ini untuk\n";
    cout << " mencoba percobaan Anda sendiri.\n";
    cout << "==================================================\n";

    // Membereskan memori dengan fungsi yang sudah disediakan.
    hapusSeluruhRiwayat(head, tail);
    tutupSemuaTab(tab);
    return 0;
}
#endif
