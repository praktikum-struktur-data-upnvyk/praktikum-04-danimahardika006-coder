#!/usr/bin/env python3
"""Membuat GitHub Job Summary dari result.json.

Ringkasan untuk manusia dibangun dari hasil yang dapat dibaca mesin, bukan
dari parsing teks terminal, sehingga keduanya dijamin konsisten.

Pemakaian:
    python3 scripts/job_summary.py result.json >> "$GITHUB_STEP_SUMMARY"
"""

import json
import sys

ICON = {"PASS": "✅", "FAIL": "❌", "SKIP": "➖", "ERROR": "⚠️"}


def main() -> int:
    path = sys.argv[1] if len(sys.argv) > 1 else "result.json"

    try:
        with open(path, encoding="utf-8") as handle:
            result = json.load(handle)
    except (OSError, json.JSONDecodeError) as error:
        print("# Praktikum Struktur Data C++\n")
        print(f"⚠️ Hasil penilaian tidak dapat dibaca: `{error}`")
        return 0

    status = result.get("status", "ERROR")
    score = result.get("score", 0)
    passed = result.get("passed", 0)
    total = result.get("total", 0)

    print("# Praktikum Struktur Data C++\n")
    print("| Parameter | Hasil |")
    print("|---|---:|")
    print(f"| Tests Berhasil | {passed} / {total} |")
    print(f"| Tests Gagal | {max(total - passed, 0)} / {total} |")
    print(f"| Score | **{score} / 100** |")
    print(f"| Status | **{ICON.get(status, '')} {status}** |")

    error = result.get("error")
    if error:
        print(f"\n> Penyebab: `{error}`")

    tests = result.get("tests", [])

    # total == 0 berarti tidak ada satu pun soal yang sempat dinilai: compile
    # gagal, atau checker dibunuh sebelum sempat bekerja. Dalam keadaan itu
    # satu-satunya informasi yang berguna adalah keluaran compiler SELENGKAPNYA.
    #
    # Sebelumnya pesan itu dijejalkan ke dalam satu sel tabel dan dipotong di
    # 200 karakter, sehingga mahasiswa hanya melihat error pertama, terpotong
    # di tengah kata, dengan semua baris tergencet jadi satu. Blok kode di
    # bawah menampilkannya utuh beserta indikator baris/kolom dari g++.
    if total == 0 and tests:
        print("\n## Pesan Compiler\n")
        print(str(tests[0].get("name", "Penilaian gagal dijalankan.")))
        detail = str(tests[0].get("message", "")).strip()

        if detail:
            # Keluaran g++ mencampur error dengan peringatan dan seni ASCII
            # penunjuk kolom. Untuk mahasiswa baru, error yang sesungguhnya
            # tenggelam di antaranya. Barisan error disaring keluar dan
            # ditampilkan lebih dulu; log lengkapnya tetap disediakan, tapi
            # dilipat supaya tidak menutupi yang penting.
            baris_error = [
                b for b in detail.splitlines() if ": error:" in b
            ]
            if baris_error:
                jumlah = len(baris_error)
                kata = "error" if jumlah == 1 else "error"
                print(f"\n### {jumlah} {kata} yang harus diperbaiki\n")
                for b in baris_error:
                    print(f"- `{b.strip()}`")
                print(
                    "\nPerbaiki yang **paling atas** lebih dulu, lalu push "
                    "lagi. Error di bawahnya sering hanya akibat beruntun "
                    "dari error pertama dan ikut hilang sendiri."
                )

            print("\n<details><summary>Keluaran compiler selengkapnya"
                  "</summary>\n")
            print("```text")
            print(detail)
            print("```")
            print("\n</details>")

        print(
            "\nSetiap error juga ditandai langsung pada barisnya di tab "
            "**Files changed**, beserta penjelasan artinya."
        )
        return 0

    if tests:
        print("\n## Rincian Test\n")
        print("| Test | Status | Keterangan |")
        print("|---|---|---|")
        for test in tests:
            name = str(test.get("name", "")).replace("|", "\\|")
            message = str(test.get("message", "")).replace("|", "\\|")
            # Batas dinaikkan dari 200: penjelasan sebab crash (mis. SIGSEGV)
            # panjangnya sekitar 200 karakter dan dulu terpotong tepat di ujung.
            message = message.replace("\n", " ")[:500]
            test_status = test.get("status", "")
            print(
                f"| {name} | {ICON.get(test_status, '')} {test_status} "
                f"| {message} |"
            )

    if error == "RUNTIME_CRASH":
        print(
            "\n> ⚠️ Ada bagian yang membuat program **berhenti mendadak saat "
            "dijalankan**, bukan sekadar menjawab salah. Test pada bagian itu "
            "ditandai ➖ SKIP karena tidak sempat dijalankan. Soal lain tetap "
            "dinilai seperti biasa — lihat kolom Keterangan untuk penyebabnya."
        )

    return 0


if __name__ == "__main__":
    sys.exit(main())
