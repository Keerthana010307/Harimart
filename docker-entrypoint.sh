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

echo "HariMart: Parsed config:"
echo "HariMart:   HOST = $DB_HOST"
echo "HariMart:   PORT = $DB_PORT"
echo "HariMart:   NAME = $DB_NAME"
echo "HariMart:   USER = $DB_USER"
echo "HariMart:   App PORT = $PORT"

# ── Set password for libpq ──────────────────────────────────
export PGPASSWORD="$DB_PASS"

# ── Try connecting: internal first, then external ───────────
# On Render, Docker containers sometimes can't resolve the
# internal hostname (dpg-xxx-a).  If internal fails, we
# automatically try the external hostname which adds the
# region suffix (e.g. dpg-xxx-a.oregon-postgres.render.com).
echo "HariMart: Testing DB connectivity..."

DB_CONNECTED=false

# Attempt 1: Internal hostname (no SSL)
echo "HariMart: Trying internal host: $DB_HOST:$DB_PORT ..."
if psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" -c "SELECT 1" > /dev/null 2>&1; then
    echo "HariMart: ✓ Internal connection OK"
    DB_CONNECTED=true
    SSLMODE=""
else
    echo "HariMart: ✗ Internal connection failed"

    # Attempt 2: Internal hostname + SSL
    echo "HariMart: Trying internal host with SSL..."
    export PGSSLMODE=require
    if psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" -c "SELECT 1" > /dev/null 2>&1; then
        echo "HariMart: ✓ Internal + SSL connection OK"
        DB_CONNECTED=true
        SSLMODE="require"
    else
        echo "HariMart: ✗ Internal + SSL failed"

        # Attempt 3: External hostname (try common Render regions)
        for REGION in oregon ohio singapore frankfurt; do
            EXT_HOST="${DB_HOST}.${REGION}-postgres.render.com"
            echo "HariMart: Trying external host: $EXT_HOST ..."
            if psql -h "$EXT_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" -c "SELECT 1" > /dev/null 2>&1; then
                echo "HariMart: ✓ External connection OK ($REGION)"
                DB_HOST="$EXT_HOST"
                DB_CONNECTED=true
                SSLMODE="require"
                break
            fi
        done
    fi
fi

if [ "$DB_CONNECTED" = "false" ]; then
    echo "HariMart: ✗ ALL connection attempts failed!"
    echo "HariMart: Trying DNS resolution..."
    getent hosts "$DB_HOST" 2>/dev/null || echo "HariMart: DNS failed for $DB_HOST"
    echo "HariMart: Will proceed anyway — Drogon will retry..."
fi

echo "HariMart: Using DB_HOST=$DB_HOST SSLMODE=${SSLMODE:-prefer}"

# ── Run migrations ──────────────────────────────────────────
echo "HariMart: Running migrations..."

PSQL_ARGS="-h $DB_HOST -p $DB_PORT -U $DB_USER -d $DB_NAME"

psql $PSQL_ARGS -f db/migrations/001_initial_schema.sql 2>&1 || echo "Migration 001 done/skipped"
psql $PSQL_ARGS -f db/migrations/002_admin_seed.sql 2>&1 || echo "Migration 002 done/skipped"
psql $PSQL_ARGS -f db/migrations/003_add_category_image_wishlist.sql 2>&1 || echo "Migration 003 done/skipped"
psql $PSQL_ARGS -f db/migrations/004_seed_products.sql 2>&1 || echo "Migration 004 done/skipped"

echo "HariMart: Migrations complete"

# ── Build Drogon config ─────────────────────────────────────
# Set PGSSLMODE for Drogon's libpq connections
if [ -n "$SSLMODE" ]; then
    export PGSSLMODE="$SSLMODE"
fi

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
            "number_of_connections": 3,
            "connect_timeout": 10,
            "client_encoding": "utf8"
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

echo "HariMart: Config written (host=$DB_HOST port=$DB_PORT db=$DB_NAME)"

echo "HariMart: ============================================"
echo "HariMart: Launching Drogon server on port ${PORT}"
echo "HariMart: ============================================"
exec ./HariMart
