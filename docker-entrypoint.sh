#!/bin/sh
set -e

PORT=${PORT:-8080}

echo "HariMart: ============================================"
echo "HariMart: Starting HariMart Server"
echo "HariMart: ============================================"

# ── Parse DATABASE_URL ──────────────────────────────────────
if [ -n "$DATABASE_URL" ]; then
    echo "HariMart: DATABASE_URL is set (length=${#DATABASE_URL})"

    # Strip protocol
    DB_REST="${DATABASE_URL#postgresql://}"
    DB_REST="${DB_REST#postgres://}"

    # user:pass@host:port/db  or  user:pass@host/db
    DB_USERPASS="${DB_REST%%@*}"
    DB_USER="${DB_USERPASS%%:*}"
    DB_PASS="${DB_USERPASS#*:}"

    # everything after @
    DB_AFTER_AT="${DB_REST#*@}"

    # host:port/db  or  host/db
    DB_HOSTPART="${DB_AFTER_AT%%/*}"
    DB_NAME="${DB_AFTER_AT#*/}"
    DB_NAME="${DB_NAME%%\?*}"

    # check if port is present
    case "$DB_HOSTPART" in
        *:*)
            DB_HOST="${DB_HOSTPART%%:*}"
            DB_PORT="${DB_HOSTPART##*:}"
            ;;
        *)
            DB_HOST="$DB_HOSTPART"
            DB_PORT="5432"
            ;;
    esac
else
    echo "HariMart: WARNING — No DATABASE_URL set!"
    DB_HOST="${DB_HOST:-127.0.0.1}"
    DB_PORT="${DB_PORT:-5432}"
    DB_NAME="${DB_NAME:-harimart}"
    DB_USER="${DB_USER:-postgres}"
    DB_PASS="${DB_PASS:-}"
fi

echo "HariMart: Parsed DB config:"
echo "HariMart:   HOST = $DB_HOST"
echo "HariMart:   PORT = $DB_PORT"
echo "HariMart:   NAME = $DB_NAME"
echo "HariMart:   USER = $DB_USER"
echo "HariMart:   App PORT = $PORT"

# ── Set SSL + password for libpq ────────────────────────────
export PGPASSWORD="$DB_PASS"
export PGSSLMODE=require

# ── Test raw TCP connectivity ───────────────────────────────
echo "HariMart: Testing TCP connectivity to $DB_HOST:$DB_PORT ..."
if command -v timeout > /dev/null 2>&1; then
    timeout 5 sh -c "echo > /dev/tcp/$DB_HOST/$DB_PORT" 2>/dev/null && echo "HariMart: TCP OK" || echo "HariMart: TCP FAILED (may be normal)"
fi

# ── DNS lookup ──────────────────────────────────────────────
echo "HariMart: Resolving $DB_HOST ..."
getent hosts "$DB_HOST" 2>/dev/null || nslookup "$DB_HOST" 2>/dev/null || echo "HariMart: DNS lookup tool not available"

# ── Wait for DB ─────────────────────────────────────────────
echo "HariMart: Waiting for database..."
RETRIES=15
until psql "postgresql://${DB_USER}:${DB_PASS}@${DB_HOST}:${DB_PORT}/${DB_NAME}?sslmode=require" -c "SELECT 1" > /dev/null 2>&1; do
    RETRIES=$((RETRIES - 1))
    if [ "$RETRIES" -le 0 ]; then
        echo "HariMart: WARNING — DB not ready after 30s"
        # Try without SSL as fallback
        echo "HariMart: Trying without sslmode=require..."
        psql "postgresql://${DB_USER}:${DB_PASS}@${DB_HOST}:${DB_PORT}/${DB_NAME}" -c "SELECT 1" 2>&1 || echo "HariMart: Fallback also failed"
        break
    fi
    echo "HariMart: DB not ready, retrying in 2s... ($RETRIES left)"
    sleep 2
done

# ── Run migrations ──────────────────────────────────────────
echo "HariMart: Running migrations..."
PSQL_URI="postgresql://${DB_USER}:${DB_PASS}@${DB_HOST}:${DB_PORT}/${DB_NAME}?sslmode=require"

psql "$PSQL_URI" -f db/migrations/001_initial_schema.sql 2>&1 || echo "Migration 001 done/skipped"
psql "$PSQL_URI" -f db/migrations/002_admin_seed.sql 2>&1 || echo "Migration 002 done/skipped"
psql "$PSQL_URI" -f db/migrations/003_add_category_image_wishlist.sql 2>&1 || echo "Migration 003 done/skipped"
psql "$PSQL_URI" -f db/migrations/004_seed_products.sql 2>&1 || echo "Migration 004 done/skipped"

echo "HariMart: Migrations complete"

# ── Build Drogon config ─────────────────────────────────────
# Use libpq key=value connection string — this is the most
# reliable format and Drogon passes it straight to libpq.
CONN_STR="host=${DB_HOST} port=${DB_PORT} dbname=${DB_NAME} user=${DB_USER} password=${DB_PASS} sslmode=require"

cat > config.json <<ENDOFCONFIG
{
    "listeners": [
        {
            "address": "0.0.0.0",
            "port": ${PORT},
            "https": false
        }
    ],
    "db_clients": [
        {
            "name": "default",
            "rdbms": "postgresql",
            "host": "${DB_HOST}",
            "port": ${DB_PORT},
            "dbname": "${DB_NAME}",
            "user": "${DB_USER}",
            "passwd": "${DB_PASS}",
            "is_fast": false,
            "number_of_connections": 5,
            "connection_string": "${CONN_STR}"
        }
    ],
    "app": {
        "document_root": "./frontend",
        "number_of_threads": 4,
        "enable_session": true,
        "session_timeout": 3600,
        "session_same_site": "Lax",
        "session_cookie_key": "JSESSIONID",
        "session_max_age": 3600
    }
}
ENDOFCONFIG

echo "HariMart: Generated config.json:"
echo "HariMart: ---"
# Show config without passwords
sed 's/"passwd":[^,]*/"passwd": "***"/g; s/"password":[^,]*/"password": "***"/g' config.json
echo "HariMart: ---"

# Final connectivity test right before launch
echo "HariMart: Final DB test before launch..."
psql "postgresql://${DB_USER}:${DB_PASS}@${DB_HOST}:${DB_PORT}/${DB_NAME}?sslmode=require" -c "SELECT 'DB_OK'" 2>&1 || echo "HariMart: FINAL DB TEST FAILED"

echo "HariMart: ============================================"
echo "HariMart: Launching Drogon server on port ${PORT}..."
echo "HariMart: ============================================"
exec ./HariMart
