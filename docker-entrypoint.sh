#!/bin/sh
set -e

PORT=${PORT:-8080}

# Parse DATABASE_URL (Render sets this automatically)
# Handles both: postgresql://user:pass@host/db
#           and: postgresql://user:pass@host:5432/db
if [ -n "$DATABASE_URL" ]; then
    echo "HariMart: DATABASE_URL is set, parsing..."

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

    # check if port is present (contains a colon)
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
    echo "HariMart: No DATABASE_URL, using fallback env vars..."
    DB_HOST="${DB_HOST:-127.0.0.1}"
    DB_PORT="${DB_PORT:-5432}"
    DB_NAME="${DB_NAME:-harimart}"
    DB_USER="${DB_USER:-postgres}"
    DB_PASS="${DB_PASS:-}"
fi

echo "HariMart: DB_HOST=$DB_HOST DB_PORT=$DB_PORT DB_NAME=$DB_NAME DB_USER=$DB_USER"
echo "HariMart: Starting on port $PORT"

# Wait for DB to be ready (max 30 seconds)
echo "HariMart: Waiting for database to be ready..."
export PGPASSWORD="$DB_PASS"
RETRIES=15
until psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" -c "SELECT 1" > /dev/null 2>&1; do
    RETRIES=$((RETRIES - 1))
    if [ "$RETRIES" -le 0 ]; then
        echo "HariMart: WARNING — DB not ready after 30s, proceeding anyway..."
        break
    fi
    echo "HariMart: DB not ready, retrying in 2s... ($RETRIES retries left)"
    sleep 2
done

# Run migrations
echo "HariMart: Running migrations..."

psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" \
    -f db/migrations/001_initial_schema.sql 2>&1 || echo "Migration 001 done/skipped"

psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" \
    -f db/migrations/002_admin_seed.sql 2>&1 || echo "Migration 002 done/skipped"

psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" \
    -f db/migrations/003_add_category_image_wishlist.sql 2>&1 || echo "Migration 003 done/skipped"

psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" \
    -f db/migrations/004_seed_products.sql 2>&1 || echo "Migration 004 done/skipped"

echo "HariMart: Migrations complete"

# Write config.json
cat > config.json <<EOF
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
            "number_of_connections": 5
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
EOF

echo "HariMart: Config written. Launching..."
exec ./HariMart
