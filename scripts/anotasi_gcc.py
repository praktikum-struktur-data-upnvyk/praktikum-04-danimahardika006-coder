#!/usr/bin/env python3
"""Mengubah keluaran g++ menjadi anotasi GitHub Actions.

Anotasi membuat pesan compiler muncul MENEMPEL pada baris kode yang
bersangkutan di tab "Files changed" dan di halaman commit, bukan terkubur di
dalam log. Untuk mahasiswa yang baru belajar, perbedaannya besar: mereka
tidak perlu lagi membaca log mentah untuk tahu baris mana yang salah.

Selain meneruskan pesan asli g++, script ini menambahkan petunjuk berbahasa
Indonesia untuk kesalahan yang paling sering muncul di praktikum.

Pemakaian:
    python3 scripts/anotasi_gcc.py compile_student.log
"""

import re
import sys

# Format diagnostik g++:  berkas:baris:kolom: error|warning: pesan
POLA = re.compile(
    r"^(?P<berkas>[^:]+\.(?:cpp|h|hpp|cc)):"
    r"(?P<baris>\d+):(?P<kolom>\d+):\s+"
    r"(?P<jenis>error|warning|catatan|note):\s+"
    r"(?P<pesan>.*)$"
)

# Petunjuk tambahan untuk kesalahan yang paling sering ditemui praktikan.
# Kuncinya dicocokkan sebagai substring pada pesan asli g++.
PETUNJUK = [
    ("was not declared in this scope",
     "Nama ini belum dikenal compiler. Periksa ejaannya — kesalahan yang "
     "paling sering: huruf 'o' tertukar dengan angka '0', atau huruf besar "
     "dan kecil tidak sama persis. Bisa juga variabelnya memang belum "
     "dideklarasikan."),
    ("expected ';' before",
     "Ada tanda baca yang salah atau kurang. Ingat: tiga bagian di dalam "
     "for(...) dipisah TITIK KOMA, bukan koma. Bentuk benarnya "
     "for (int i = 0; i < n; i++)."),
    ("expected ';' at end of",
     "Sebuah pernyataan belum ditutup titik koma."),
    ("expected '(' before",
     "Kurung buka kurang. Syarat if dan while harus dibungkus kurung, "
     "misalnya if (n == 0) { ... }."),
    ("expected ')' before",
     "Kurung tutup kurang. Hitung ulang pasangan buka-tutupnya."),
    ("expected '}' at end of input",
     "Ada kurung kurawal yang belum ditutup. Periksa apakah setiap { punya "
     "pasangan }."),
    ("no return statement in function returning non-void",
     "Fungsi ini bisa selesai tanpa menjalankan 'return'. Saat dijalankan, "
     "program akan mati mendadak (Illegal instruction) dan soal ini tidak "
     "bisa dinilai. Pastikan SETIAP jalur keluar fungsi mengembalikan nilai, "
     "termasuk cabang if/else dan baris terakhir fungsi."),
    ("does not guard",
     "Indentasi menipu. Tanpa kurung kurawal, if/for/while hanya mencakup "
     "SATU baris berikutnya — baris setelahnya selalu ikut dijalankan walau "
     "terlihat menjorok ke dalam. Bungkus dengan { } bila maksudnya lebih "
     "dari satu baris."),
    ("redefinition of",
     "Nama ini dideklarasikan dua kali. Hapus salah satunya."),
    ("assignment of read-only",
     "Parameter ber-const tidak boleh diubah isinya. Fungsi ini memang "
     "dirancang hanya membaca array, bukan mengubahnya."),
    ("invalid conversion",
     "Tipe data tidak cocok. Periksa apakah yang dikembalikan sudah sesuai "
     "tipe kembalian fungsi."),
    ("'main' must return 'int'",
     "main() harus bertipe int, jadi tulis `int main()` -- bukan `void main()` "
     "-- dan akhiri dengan `return 0;`."),
    ("multiple definition of `main'",
     "Ada DUA fungsi main() di src/student.cpp. Sebuah program C++ hanya boleh "
     "punya satu. Hapus salah satunya, dan pastikan main() Anda tetap berada "
     "di dalam blok #ifndef ADA_MAIN_LAIN ... #endif di bagian bawah file."),
    ("multiple definition of",
     "Ada fungsi yang didefinisikan lebih dari sekali di src/student.cpp. "
     "Hapus salah satu definisinya."),
]


# =============================================================================
# Kesalahan LINKER
# =============================================================================
# Linker tidak menyebut baris dan kolom. Bentuk pesannya:
#
#   /usr/bin/ld: student.o: in function `main':
#   student.cpp:(.text+0x3d5): multiple definition of `main'; ...
#
# POLA di atas mensyaratkan "berkas:baris:kolom: error:", sehingga pesan
# linker TIDAK PERNAH cocok. Tanpa penanganan khusus, kegagalan link berakhir
# tanpa satu pun anotasi -- mahasiswa hanya melihat score 0 tanpa keterangan,
# padahal inilah kegagalan yang paling sering terjadi.
#
# Anotasinya dipasang di tingkat berkas (tanpa nomor baris) karena memang
# tidak ada baris tunggal yang bisa ditunjuk.
PETUNJUK_LINKER = [
    ("multiple definition of `main'",
     "Ada DUA fungsi main() yang ikut dinilai. Sebuah program C++ hanya boleh "
     "punya satu, dan saat menilai, main() sudah disediakan oleh checker. "
     "Pastikan main() Anda tetap berada di dalam blok "
     "#ifndef ADA_MAIN_LAIN ... #endif di bagian bawah src/student.cpp, dan "
     "jangan menulis main() kedua di luar blok itu."),
    ("undefined reference to",
     "Ada fungsi yang dipakai checker tetapi tidak ditemukan di "
     "src/student.cpp. Penyebab paling umum: nama atau signature fungsi "
     "diubah sehingga tidak lagi cocok dengan src/student.h. Kembalikan "
     "persis seperti semula -- termasuk tipe kembalian, jumlah parameter, "
     "dan kata const."),
    ("multiple definition of",
     "Ada fungsi yang didefinisikan lebih dari sekali di src/student.cpp. "
     "Hapus salah satu definisinya."),
]

# Peringatan yang TIDAK layak dianotasi.
#
# "unused parameter" muncul di hampir setiap fungsi yang belum diisi, dan
# src/student.cpp sendiri sudah menyatakan peringatan itu wajar. Membiarkannya
# lewat berarti satu skeleton kosong menghasilkan 18 anotasi sampah yang
# mengubur error yang sesungguhnya -- apalagi GitHub hanya menampilkan sekitar
# 10 anotasi per jenis per step.
PERINGATAN_DIABAIKAN = (
    "[-Wunused-parameter]",
    "[-Wunused-variable]",
    "[-Wunused-but-set-variable]",
    "[-Wunused-function]",
    "[-Wunused-but-set-parameter]",
)

# Satu kesalahan kecil bisa memicu berderet error susulan. Yang paling berguna
# selalu error PERTAMA, jadi anotasi dibatasi supaya tidak melampaui batas
# tampilan GitHub dan mengubur yang awal.
BATAS_ANOTASI = 10


def escape(teks: str) -> str:
    """Escape untuk isi pesan anotasi GitHub Actions."""
    return (
        teks.replace("%", "%25")
        .replace("\r", "%0D")
        .replace("\n", "%0A")
    )


def escape_properti(teks: str) -> str:
    """Escape untuk nilai properti (file, line, col); koma ikut di-escape."""
    return escape(teks).replace(":", "%3A").replace(",", "%2C")


def petunjuk_untuk(pesan: str) -> str:
    # g++ mengapit token dengan kutip TIPOGRAFIS (‘ ’), bukan apostrof ASCII.
    # Tanpa penyeragaman ini, kunci seperti "expected \';\' before" tidak akan
    # pernah cocok dan petunjuknya diam-diam tidak muncul.
    rata = pesan.replace("‘", "'").replace("’", "'").replace("“", '"').replace("”", '"')
    for kunci, saran in PETUNJUK:
        if kunci in rata:
            return saran
    return ""


def main() -> int:
    path = sys.argv[1] if len(sys.argv) > 1 else "compile_student.log"

    try:
        with open(path, encoding="utf-8", errors="replace") as handle:
            baris_log = handle.readlines()
    except OSError:
        # Tidak ada log berarti tidak ada yang perlu dianotasi.
        return 0

    # Kesalahan yang sama bisa dilaporkan berkali-kali (mis. lewat header yang
    # di-include berulang). Anotasi kembar hanya menambah kebisingan.
    sudah = set()
    jumlah = {"error": 0, "warning": 0}

    for baris in baris_log:
        cocok = POLA.match(baris.strip())
        if not cocok:
            continue

        jenis = cocok.group("jenis")
        if jenis in ("catatan", "note"):
            continue

        pesan_asli = cocok.group("pesan")
        if jenis == "warning" and any(
            tanda in pesan_asli for tanda in PERINGATAN_DIABAIKAN
        ):
            continue

        berkas = cocok.group("berkas")
        nomor = cocok.group("baris")
        kolom = cocok.group("kolom")
        pesan = cocok.group("pesan")

        kunci = (berkas, nomor, kolom, pesan)
        if kunci in sudah:
            continue
        sudah.add(kunci)

        if jumlah[jenis] >= BATAS_ANOTASI:
            continue

        saran = petunjuk_untuk(pesan)
        isi = pesan if not saran else f"{pesan}\n\nArtinya: {saran}"

        print(
            f"::{jenis} file={escape_properti(berkas)},"
            f"line={nomor},col={kolom},"
            f"title=Periksa baris {nomor}::{escape(isi)}"
        )
        jumlah[jenis] += 1

    # Kegagalan link tidak menghasilkan satu pun baris ber-"baris:kolom",
    # jadi ditangani terpisah -- lihat catatan di PETUNJUK_LINKER.
    seluruh_log = "".join(baris_log).replace("\u2018", "'").replace("\u2019", "'")
    for kunci, saran in PETUNJUK_LINKER:
        if kunci in seluruh_log:
            print(
                f"::error file=src/student.cpp,"
                f"title=Gagal menggabungkan dengan checker::{escape(saran)}"
            )
            # Sengaja TIDAK menaikkan jumlah["error"]: penghitung itu
            # dipasangkan dengan set `sudah` untuk menghitung sisa pesan yang
            # tidak dianotasi, sedangkan anotasi linker ini tidak berasal dari
            # sana. Menaikkannya membuat hitungan sisa kurang satu.
            break

    # Beri tahu bila masih ada sisa yang tidak dianotasi, supaya mahasiswa
    # tidak mengira daftar di atas sudah lengkap.
    sisa = len(sudah) - (jumlah["error"] + jumlah["warning"])
    if sisa > 0:
        print(
            f"::notice::Masih ada {sisa} pesan compiler lain yang tidak "
            f"ditampilkan sebagai anotasi. Perbaiki dulu yang di atas, lalu "
            f"push lagi -- kesalahan susulan biasanya ikut hilang sendiri."
        )

    return 0


if __name__ == "__main__":
    sys.exit(main())
