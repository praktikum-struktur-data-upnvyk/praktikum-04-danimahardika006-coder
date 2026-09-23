#!/usr/bin/env python3
"""Menolak pembacaan masukan di dalam fungsi yang dinilai.

KENAPA PEMERIKSAAN INI ADA
--------------------------
Sejak main() boleh ditulis di src/student.cpp, cin menjadi hal yang wajar
terlihat di file itu. Cepat atau lambat ada mahasiswa yang menaruhnya di dalam
salah satu fungsi yang dinilai.

Saat menilai, checker memanggil fungsi-fungsi itu TANPA memberi masukan apa
pun. Yang terjadi kemudian tidak konsisten dan sangat sulit didiagnosis:

  - `cin >> n` yang gagal TIDAK mengubah n pada libstdc++, jadi kadang tidak
    berefek apa-apa dan semua test tetap lulus;
  - `cin >> x` ke variabel lokal yang belum diinisialisasi meninggalkan sampah
    tumpukan, sehingga hasilnya berubah-ubah setiap eksekusi;
  - hasilnya bahkan bergantung pada apa yang kebetulan ada di stdin runner CI,
    yaitu hal yang sama sekali di luar kendali kode mahasiswa.

Artinya nilai bisa berbeda antar-push dari kode yang sama persis. Kegagalan
yang jelas jauh lebih baik daripada nilai yang acak, jadi kasus ini ditolak
lebih awal dengan pesan yang menyebutkan baris dan cara memperbaikinya.

Yang TIDAK diperiksa: isi blok #ifndef ADA_MAIN_LAIN ... #endif, yaitu main()
milik mahasiswa. Di sana cin memang dipersilakan.
"""

import re
import sys

BERKAS = "src/student.cpp"

# Nama fungsi/objek pembaca masukan. Dicocokkan sebagai kata utuh supaya
# `mencintai` atau `scanfoo` tidak ikut tertangkap.
TERLARANG = ("cin", "scanf", "getline", "getchar", "gets", "fgets", "fscanf")

POLA = re.compile(r"\b(" + "|".join(TERLARANG) + r")\b")


def buang_komentar_dan_string(baris_asli):
    """Mengosongkan isi komentar dan string, tanpa mengubah jumlah baris.

    Perlu karena src/student.cpp sendiri menyebut kata "cin" berkali-kali di
    dalam komentar peringatannya. Tanpa ini, peringatan itu justru memicu
    pemeriksaan ini menolak berkas yang sebenarnya benar.
    """
    hasil = []
    dalam_blok = False
    for baris in baris_asli:
        keluar = []
        i = 0
        dalam_string = None
        while i < len(baris):
            dua = baris[i:i + 2]
            if dalam_blok:
                if dua == "*/":
                    dalam_blok = False
                    i += 2
                    continue
                i += 1
                continue
            if dalam_string:
                if baris[i] == "\\":
                    i += 2
                    continue
                if baris[i] == dalam_string:
                    dalam_string = None
                i += 1
                continue
            if dua == "//":
                break
            if dua == "/*":
                dalam_blok = True
                i += 2
                continue
            if baris[i] in "\"'":
                dalam_string = baris[i]
                i += 1
                continue
            keluar.append(baris[i])
            i += 1
        hasil.append("".join(keluar))
    return hasil


def batas_blok_main(baris):
    """Mengembalikan (awal, akhir) indeks blok #ifndef ADA_MAIN_LAIN.

    Mengembalikan (None, None) bila blok itu tidak ada, mis. karena mahasiswa
    menghapusnya. Kasus tersebut bukan urusan pemeriksaan ini — linker yang
    akan menolaknya, dengan anotasinya sendiri.
    """
    awal = None
    for i, b in enumerate(baris):
        if b.strip().startswith("#ifndef") and "ADA_MAIN_LAIN" in b:
            awal = i
            break
    if awal is None:
        return None, None

    dalam = 0
    for i in range(awal, len(baris)):
        t = baris[i].strip()
        if t.startswith(("#ifndef", "#ifdef", "#if")):
            dalam += 1
        elif t.startswith("#endif"):
            dalam -= 1
            if dalam == 0:
                return awal, i
    return awal, len(baris) - 1


def main() -> int:
    try:
        with open(BERKAS, encoding="utf-8", errors="replace") as handle:
            baris_asli = handle.read().splitlines()
    except OSError:
        # Tidak ada berkas berarti ada masalah lain yang lebih dulu dilaporkan.
        return 0

    bersih = buang_komentar_dan_string(baris_asli)
    awal, akhir = batas_blok_main(bersih)

    temuan = []
    for i, b in enumerate(bersih):
        if awal is not None and awal <= i <= akhir:
            continue
        cocok = POLA.search(b)
        if cocok:
            temuan.append((i + 1, cocok.group(1), baris_asli[i].strip()))

    if not temuan:
        return 0

    for nomor, nama, isi in temuan:
        pesan = (
            f"`{nama}` dipakai di luar main(). Fungsi-fungsi yang dinilai "
            f"tidak boleh membaca masukan: saat menilai, checker memanggilnya "
            f"tanpa memberi masukan apa pun, sehingga hasilnya menjadi sampah "
            f"yang berubah-ubah setiap kali dinilai.%0A%0A"
            f"Perbaikannya: hapus `{nama}` dari fungsi ini. Fungsi menerima "
            f"datanya lewat PARAMETER. Kalau Anda ingin mengetik data sendiri, "
            f"lakukan di dalam main() di bagian bawah file, lalu kirimkan "
            f"hasilnya ke fungsi ini sebagai argumen."
        )
        print(
            f"::error file={BERKAS},line={nomor},"
            f"title=Masukan (cin) di dalam fungsi yang dinilai::{pesan}"
        )

    sys.stderr.write(
        f"\nDITOLAK: ditemukan {len(temuan)} pembacaan masukan di luar main() "
        f"pada {BERKAS}.\n"
    )
    for nomor, _, isi in temuan:
        sys.stderr.write(f"  baris {nomor}: {isi}\n")
    return 1


if __name__ == "__main__":
    sys.exit(main())
