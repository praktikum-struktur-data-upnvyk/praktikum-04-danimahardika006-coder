// =============================================================================
// report.h — Perekam hasil test + penulis hasil yang dapat dibaca mesin
// =============================================================================
// File ini milik instruktur. TIDAK boleh diubah mahasiswa.
//
// Tujuannya: selain output berwarna untuk manusia, checker juga menulis
// result.json dengan format yang stabil. File JSON itulah yang diambil oleh
// aplikasi web (lewat artifact GitHub Actions), sehingga penilaian tidak
// pernah bergantung pada hasil parsing teks terminal.
//
// ISOLASI CRASH
// -------------
// Kode mahasiswa bisa MEMBUNUH proses, bukan sekadar menghasilkan jawaban
// salah. Contoh yang sering terjadi:
//
//   - fungsi non-void selesai tanpa `return`  -> SIGILL  (exit 132)
//   - indeks array di luar batas / pointer null -> SIGSEGV (exit 139)
//   - pembagian dengan nol saat n == 0        -> SIGFPE  (exit 136)
//   - loop yang tidak pernah berhenti         -> menggantung tanpa batas
//
// Tanpa penanganan, satu kejadian di atas membunuh seluruh checker: test yang
// sudah lulus sebelumnya ikut hangus dan result.json tidak pernah ditulis.
//
// Karena itu setiap test suite dijalankan di PROSES ANAK tersendiri lewat
// jalankan_terisolasi(). Anak mengirim hasil tiap test ke induk lewat pipe
// begitu test selesai, jadi hasil yang sudah sempat direkam tetap selamat
// walaupun anak mati di tengah jalan. Induk tidak pernah ikut mati, sehingga
// suite berikutnya tetap dinilai dan result.json tetap ditulis.
//
// Bentuk result.json:
// {
//   "schema_version": 1,
//   "score": 100,
//   "passed": 10,
//   "total": 10,
//   "status": "PASS",
//   "commit_sha": "....",
//   "timestamp": "2026-08-12T10:00:00Z",
//   "tests": [
//     { "name": "tambah(2,3) == 5", "status": "PASS", "points": 10.0, "message": "" }
//   ]
// }
// =============================================================================

#ifndef REPORT_H
#define REPORT_H

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <vector>

// fork(), pipe(), alarm(), dan waitpid() adalah POSIX. Di Windows fasilitas ini
// tidak ada, jadi checker jatuh ke mode tanpa isolasi (tetap bisa dikompilasi,
// hanya tidak tahan crash). GitHub Actions memakai ubuntu-latest, jadi jalur
// yang dipakai untuk penilaian selalu jalur POSIX.
#if defined(_WIN32)
#  define REPORT_ISOLASI 0
#else
#  define REPORT_ISOLASI 1
#  include <cerrno>
#  include <csignal>
#  include <sys/wait.h>
#  include <unistd.h>
#endif

using namespace std;

struct TestRecord {
    string name;
    string status;   // "PASS" | "FAIL" | "SKIP"
    string message;
};

inline vector<TestRecord>& test_records() {
    static vector<TestRecord> records;
    return records;
}

// =============================================================================
// Saluran hasil dari proses anak ke proses induk
// =============================================================================
// Di dalam proses anak, record tidak disimpan di vector (vector itu ikut mati
// bersama anaknya) melainkan langsung dikirim ke induk lewat pipe.
//
// Format per record, biner dan berprefiks panjang supaya aman terhadap isi
// pesan apa pun sekaligus terdeteksi bila terpotong di tengah:
//
//   [1 byte status: 1 = PASS, 0 = FAIL]
//   [4 byte panjang nama][isi nama]
//   [4 byte panjang pesan][isi pesan]

inline int& pipa_anak() {
    static int fd = -1;   // -1 berarti "kita sedang di proses induk"
    return fd;
}

#if REPORT_ISOLASI

inline bool tulis_penuh(int fd, const void* data, size_t n) {
    const char* p = static_cast<const char*>(data);
    while (n > 0) {
        ssize_t w = write(fd, p, n);
        if (w < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        p += w;
        n -= static_cast<size_t>(w);
    }
    return true;
}

inline bool tulis_string(int fd, const string& s) {
    uint32_t len = static_cast<uint32_t>(s.size());
    return tulis_penuh(fd, &len, sizeof(len)) && tulis_penuh(fd, s.data(), len);
}

// Mengembalikan false bila pipe sudah tertutup atau datanya terpotong
// (mis. anak mati tepat di tengah penulisan record).
inline bool baca_penuh(int fd, void* data, size_t n) {
    char* p = static_cast<char*>(data);
    while (n > 0) {
        ssize_t r = read(fd, p, n);
        if (r < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        if (r == 0) return false;   // EOF
        p += r;
        n -= static_cast<size_t>(r);
    }
    return true;
}

inline bool baca_string(int fd, string& keluar) {
    uint32_t len = 0;
    if (!baca_penuh(fd, &len, sizeof(len))) return false;
    keluar.assign(len, '\0');
    if (len > 0 && !baca_penuh(fd, &keluar[0], len)) return false;
    return true;
}

#endif  // REPORT_ISOLASI

inline void record_test(const string& name,
                        bool passed,
                        const string& message) {
#if REPORT_ISOLASI
    if (pipa_anak() >= 0) {
        unsigned char status = passed ? 1 : 0;
        tulis_penuh(pipa_anak(), &status, 1);
        tulis_string(pipa_anak(), name);
        tulis_string(pipa_anak(), passed ? string() : message);
        return;
    }
#endif
    test_records().push_back(
        TestRecord{name, passed ? "PASS" : "FAIL", passed ? "" : message});
}

// Dipakai induk untuk mencatat test yang TIDAK SEMPAT dijalankan karena
// suite-nya mati duluan.
inline void record_skip(const string& name, const string& message) {
    test_records().push_back(TestRecord{name, "SKIP", message});
}

// =============================================================================
// Menerjemahkan sebab kematian proses menjadi kalimat yang berguna
// =============================================================================
// Inti dari isolasi: mahasiswa tidak lagi cuma melihat "Illegal instruction",
// melainkan penjelasan tentang kesalahan pemrograman yang menyebabkannya.

inline string alasan_sinyal(int sinyal, int batas_detik) {
#if REPORT_ISOLASI
    switch (sinyal) {
        case SIGILL:
            return "Program berhenti mendadak (SIGILL). Penyebab paling umum: "
                   "ada fungsi bertipe kembalian non-void yang selesai tanpa "
                   "menjalankan `return`. Periksa setiap cabang if/else dan "
                   "ujung setiap fungsi.";
        case SIGSEGV:
            return "Program mengakses memori yang bukan haknya (SIGSEGV). "
                   "Penyebab paling umum: indeks array di luar batas "
                   "(mis. data[n] atau data[-1]), atau pointer bernilai "
                   "nullptr yang tetap di-dereference.";
        case SIGFPE:
            return "Terjadi pembagian dengan nol (SIGFPE). Periksa kasus "
                   "n == 0: lakukan pembagian hanya setelah memastikan "
                   "pembaginya bukan nol.";
        case SIGABRT:
            return "Program dihentikan paksa (SIGABRT), mis. karena assert "
                   "gagal, exception yang tidak tertangkap, atau kerusakan "
                   "memori yang terdeteksi pustaka standar.";
        case SIGBUS:
            return "Akses memori tidak selaras / tidak sah (SIGBUS). Periksa "
                   "penggunaan pointer dan indeks array.";
        case SIGALRM:
            return "Melebihi batas waktu " + to_string(batas_detik) +
                   " detik. Penyebab paling umum: loop yang syarat berhentinya "
                   "tidak pernah tercapai (variabel penghitung lupa dinaikkan, "
                   "atau pointer `next` tidak pernah maju).";
        default:
            return "Program dihentikan oleh sinyal " + to_string(sinyal) + ".";
    }
#else
    (void)sinyal;
    (void)batas_detik;
    return "Program berhenti tidak wajar.";
#endif
}

// =============================================================================
// Menjalankan satu test suite di proses terpisah
// =============================================================================
/**
 * Menjalankan `suite` di proses anak dan menyalin setiap record yang sempat
 * dikirimnya ke test_records() milik induk.
 *
 * @param suite        fungsi test suite yang akan dijalankan
 * @param batas_detik  batas waktu; suite yang melewatinya dibunuh SIGALRM
 * @return             string kosong bila suite selesai normal, atau kalimat
 *                     penjelas bila suite mati / kehabisan waktu
 */
inline string jalankan_terisolasi(void (*suite)(), int batas_detik) {
#if !REPORT_ISOLASI
    (void)batas_detik;
    suite();
    return string();
#else
    // Kosongkan buffer keluaran SEBELUM fork, supaya isi buffer induk tidak
    // ikut tersalin ke anak lalu tercetak dua kali.
    cout.flush();
    fflush(nullptr);

    int pipa[2];
    if (pipe(pipa) != 0) {
        suite();          // fasilitas OS tidak tersedia: jalankan apa adanya
        return string();
    }

    pid_t pid = fork();
    if (pid < 0) {
        close(pipa[0]);
        close(pipa[1]);
        suite();
        return string();
    }

    if (pid == 0) {
        // ---- proses anak ----
        close(pipa[0]);
        pipa_anak() = pipa[1];
        alarm(static_cast<unsigned>(batas_detik));
        suite();
        cout.flush();
        fflush(nullptr);
        _exit(0);
    }

    // ---- proses induk ----
    close(pipa[1]);

    // Dibaca sampai EOF, yaitu sampai anak menutup pipe atau mati. Record yang
    // terlanjur terkirim tetap tercatat walaupun anak mati sesudahnya.
    for (;;) {
        unsigned char status = 0;
        if (!baca_penuh(pipa[0], &status, 1)) break;
        string nama, pesan;
        if (!baca_string(pipa[0], nama)) break;
        if (!baca_string(pipa[0], pesan)) break;
        test_records().push_back(
            TestRecord{nama, status ? "PASS" : "FAIL", pesan});
    }
    close(pipa[0]);

    int status_anak = 0;
    while (waitpid(pid, &status_anak, 0) < 0 && errno == EINTR) {
        // waitpid diinterupsi sinyal: ulangi.
    }

    if (WIFSIGNALED(status_anak)) {
        return alasan_sinyal(WTERMSIG(status_anak), batas_detik);
    }
    if (WIFEXITED(status_anak) && WEXITSTATUS(status_anak) != 0) {
        return "Program keluar lebih awal dengan exit code " +
               to_string(WEXITSTATUS(status_anak)) +
               ". Kode mahasiswa tidak boleh memanggil exit().";
    }
    return string();
#endif
}

// =============================================================================
// Penulisan result.json
// =============================================================================

// Meng-escape string agar valid sebagai JSON string literal.
inline string json_escape(const string& input) {
    string out;
    out.reserve(input.size() + 8);
    for (unsigned char c : input) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:
                if (c < 0x20) {
                    char buf[7];
                    snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else {
                    out += static_cast<char>(c);
                }
        }
    }
    return out;
}

// Waktu UTC dalam format ISO-8601.
inline string iso_timestamp() {
    time_t now = time(nullptr);
    tm utc{};
#if defined(_WIN32)
    gmtime_s(&utc, &now);
#else
    gmtime_r(&now, &utc);
#endif
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &utc);
    return string(buf);
}

// commit SHA diambil dari environment GitHub Actions; kosong saat dijalankan
// secara lokal.
inline string commit_sha() {
    const char* sha = getenv("GITHUB_SHA");
    return sha ? string(sha) : string("");
}

/**
 * Menulis result.json.
 *
 * @param path        lokasi berkas keluaran
 * @param score       nilai akhir 0-100
 * @param total_soal  jumlah test yang SEHARUSNYA dijalankan. Dipakai sebagai
 *                    penyebut supaya suite yang mati tidak mengecilkan
 *                    penyebut dan menaikkan score secara keliru. 0 berarti
 *                    "pakai jumlah record apa adanya".
 * @param error_code  bila tidak kosong, ditulis sebagai field "error"
 *                    (mis. "RUNTIME_CRASH") supaya aplikasi web tahu bahwa
 *                    ada suite yang tidak selesai wajar.
 */
inline bool write_result_json(const char* path,
                              int score,
                              int total_soal = 0,
                              const string& error_code = string()) {
    const vector<TestRecord>& records = test_records();

    int total = (total_soal > 0) ? total_soal : static_cast<int>(records.size());
    int passed = 0;
    for (const TestRecord& r : records) {
        if (r.status == "PASS") passed++;
    }

    // Bobot rata untuk setiap test; hanya test lulus yang mendapat poin.
    double per_test = (total > 0) ? (100.0 / total) : 0.0;

    FILE* file = fopen(path, "w");
    if (!file) return false;

    fprintf(file, "{\n");
    fprintf(file, "  \"schema_version\": 1,\n");
    fprintf(file, "  \"score\": %d,\n", score);
    fprintf(file, "  \"passed\": %d,\n", passed);
    fprintf(file, "  \"total\": %d,\n", total);
    fprintf(file, "  \"status\": \"%s\",\n",
                 (total > 0 && passed == total) ? "PASS" : "FAIL");
    if (!error_code.empty()) {
        fprintf(file, "  \"error\": \"%s\",\n",
                     json_escape(error_code).c_str());
    }
    fprintf(file, "  \"commit_sha\": \"%s\",\n",
                 json_escape(commit_sha()).c_str());
    fprintf(file, "  \"timestamp\": \"%s\",\n", iso_timestamp().c_str());
    fprintf(file, "  \"tests\": [\n");

    for (size_t i = 0; i < records.size(); ++i) {
        const TestRecord& r = records[i];
        fprintf(file,
                     "    { \"name\": \"%s\", \"status\": \"%s\", "
                     "\"points\": %.2f, \"message\": \"%s\" }%s\n",
                     json_escape(r.name).c_str(),
                     r.status.c_str(),
                     r.status == "PASS" ? per_test : 0.0,
                     json_escape(r.message).c_str(),
                     (i + 1 < records.size()) ? "," : "");
    }

    fprintf(file, "  ]\n");
    fprintf(file, "}\n");
    fclose(file);
    return true;
}

#endif  // REPORT_H
