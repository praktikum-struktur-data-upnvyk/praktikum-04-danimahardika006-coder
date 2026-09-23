#!/bin/bash
# =============================================================================
# run_tests.sh — Script Lokal untuk Menjalankan Test
# =============================================================================
# Untuk instruktur: jalankan script ini sebelum push untuk verifikasi lokal.
# Alur sama persis dengan yang ada di GitHub Actions.
#
# Cara pakai:
#   chmod +x scripts/run_tests.sh
#   ./scripts/run_tests.sh
#
# Exit code:
#   0 = semua test berhasil
#   1 = ada test yang gagal
#   2 = compile error (Score: 0/100)
# =============================================================================

# Warna output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
BOLD='\033[1m'
RESET='\033[0m'

# Pindah ke root project (parent dari scripts/)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
cd "$PROJECT_ROOT"

echo -e "${BOLD}${CYAN}============================================${RESET}"
echo -e "${BOLD}  Script Test Lokal — Praktikum C++${RESET}"
echo -e "${BOLD}${CYAN}============================================${RESET}"

# --- Step 0: Tolak pembacaan masukan di dalam fungsi yang dinilai ---
# Kasus ini tidak menimbulkan error compile, tetapi membuat nilai berubah-ubah
# antar-penilaian. Lihat scripts/periksa_masukan.py.
echo -e "\n${YELLOW}[0/3] Memeriksa penggunaan cin...${RESET}"
if ! python3 scripts/periksa_masukan.py; then
    echo -e "\n${RED}${BOLD}[DITOLAK] Ada cin/scanf di dalam fungsi yang dinilai.${RESET}"
    echo -e "${RED}Fungsi-fungsi itu menerima datanya lewat parameter."
    echo -e "Pindahkan cin ke dalam main() di bagian bawah src/student.cpp.${RESET}"
    exit 2
fi
echo -e "${GREEN}[OK] Tidak ada pembacaan masukan di luar main()${RESET}"

# --- Step 1: Compile student.cpp menjadi object file ---
echo -e "\n${YELLOW}[1/3] Compile student.cpp...${RESET}"
COMPILE_STUDENT_ERR=$(g++ -std=c++17 -Wall -Wextra -DADA_MAIN_LAIN -c src/student.cpp -o student.o 2>&1)
if [ $? -ne 0 ]; then
    echo -e "\n${RED}${BOLD}[ERROR] Compile GAGAL pada student.cpp${RESET}"
    echo -e "${RED}${COMPILE_STUDENT_ERR}${RESET}"
    echo ""
    echo -e "${BOLD}============================================${RESET}"
    echo -e "${BOLD} SCORING SUMMARY${RESET}"
    echo -e "${BOLD}============================================${RESET}"
    echo -e " Compile Error  : ${RED}${BOLD}YA${RESET}"
    echo -e " Score          : ${RED}${BOLD}0 / 100${RESET}"
    echo -e "${BOLD}============================================${RESET}"
    echo -e "${RED}${BOLD} STATUS: COMPILE ERROR ✗${RESET}"
    rm -f student.o
    exit 2
fi
echo -e "${GREEN}[OK] student.cpp berhasil dikompilasi${RESET}"

# --- Step 2: Compile checker.cpp dan link dengan student.o ---
echo -e "\n${YELLOW}[2/3] Compile checker.cpp dan link...${RESET}"
COMPILE_CHECKER_ERR=$(g++ -std=c++17 -Wall -Wextra tests/checker.cpp student.o -o checker_bin 2>&1)
if [ $? -ne 0 ]; then
    echo -e "\n${RED}${BOLD}[ERROR] Compile/link GAGAL pada checker.cpp${RESET}"
    echo -e "${RED}${COMPILE_CHECKER_ERR}${RESET}"
    echo ""
    echo -e "${BOLD}============================================${RESET}"
    echo -e "${BOLD} SCORING SUMMARY${RESET}"
    echo -e "${BOLD}============================================${RESET}"
    echo -e " Compile Error  : ${RED}${BOLD}YA (checker)${RESET}"
    echo -e " Score          : ${RED}${BOLD}0 / 100${RESET}"
    echo -e "${BOLD}============================================${RESET}"
    echo -e "${RED}${BOLD} STATUS: COMPILE ERROR ✗${RESET}"
    rm -f student.o checker_bin
    exit 2
fi
echo -e "${GREEN}[OK] Checker berhasil dikompilasi${RESET}"

# --- Step 3: Jalankan checker (output score sudah ada di dalamnya) ---
echo -e "\n${YELLOW}[3/3] Menjalankan test...${RESET}"
echo ""

EXIT_CODE=0
./checker_bin || EXIT_CODE=$?

# Bersihkan binary
rm -f student.o checker_bin

echo ""
if [ $EXIT_CODE -eq 0 ]; then
    echo -e "${GREEN}${BOLD}============================================${RESET}"
    echo -e "${GREEN}${BOLD}  LOKAL: SEMUA TEST BERHASIL ✓${RESET}"
    echo -e "${GREEN}${BOLD}============================================${RESET}"
else
    echo -e "${RED}${BOLD}============================================${RESET}"
    echo -e "${RED}${BOLD}  LOKAL: ADA TEST YANG GAGAL ✗${RESET}"
    echo -e "${RED}${BOLD}============================================${RESET}"
fi

exit $EXIT_CODE
