#!/usr/bin/env bash
# =============================================================================
# write_error_result.sh — Menulis result.json saat penilaian tidak bisa berjalan
# =============================================================================
# Dipakai oleh GitHub Actions ketika compile gagal atau checker crash, sehingga
# aplikasi web tetap menerima hasil dengan bentuk yang sama (score 0).
#
# Pemakaian:
#   ./scripts/write_error_result.sh <STATUS> <PESAN> [BERKAS_LOG]
# =============================================================================
set -euo pipefail

STATUS="${1:-ERROR}"
MESSAGE="${2:-Penilaian gagal dijalankan.}"
LOG_FILE="${3:-}"

DETAIL=""
if [ -n "$LOG_FILE" ] && [ -f "$LOG_FILE" ]; then
    # Ambil 40 baris pertama saja agar payload tetap kecil.
    DETAIL="$(head -c 4000 "$LOG_FILE")"
fi

# Escape untuk JSON string (backslash, kutip, dan newline).
json_escape() {
    python3 -c 'import json,sys; print(json.dumps(sys.stdin.read())[1:-1])'
}

ESCAPED_MESSAGE="$(printf '%s' "$MESSAGE" | json_escape)"
ESCAPED_DETAIL="$(printf '%s' "$DETAIL" | json_escape)"
TIMESTAMP="$(date -u +%Y-%m-%dT%H:%M:%SZ)"

cat > result.json <<JSON
{
  "schema_version": 1,
  "score": 0,
  "passed": 0,
  "total": 0,
  "status": "ERROR",
  "error": "${STATUS}",
  "commit_sha": "${GITHUB_SHA:-}",
  "timestamp": "${TIMESTAMP}",
  "tests": [
    { "name": "${ESCAPED_MESSAGE}", "status": "FAIL", "points": 0, "message": "${ESCAPED_DETAIL}" }
  ]
}
JSON

echo "result.json ditulis dengan status ${STATUS}."
