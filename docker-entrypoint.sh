#!/bin/sh
set -e

PORT=${PORT:-8080}

# Parse DATABASE_URL if provided (Render sets this automatically)
# Format: postgresql://user:password@host:port/dbname
if [ -n "$DATABASE_URL" ]; then
    DB_REST="${DATABASE_URL#postgresql://}"
    DB_REST="${DB_REST#postgres://}"

    DB_USERPASS="${DB_REST%%@*}"
    DB_USER="${DB_USERPASS%%:*}"
    DB_PASS="${DB_USERPASS#*:}"

    DB_HOSTPORTDB="${DB_REST#*@}"
    DB_HOSTPORT="${DB_HOSTPORTDB%%/*}"
    DB_NAME="${DB_HOSTPORTDB#*/}"
    DB_NAME="${DB_NAME%%\?*}"

    DB_HOST="${DB_HOSTPORT%%:*}"
    DB_PORT="${DB_HOSTPORT##*:}"
    DB_PORT="${DB_PORT:-5432}"
else
    DB_HOST="${DB_HOST:-127.0.0.1}"
    DB_PORT="${DB_PORT:-5432}"
    DB_NAME="${DB_NAME:-harimart}"
    DB_USER="${DB_USER:-postgres}"
    DB_PASS="${DB_PASS:-}"
fi

echo "HariMart: Starting on port $PORT, DB: $DB_HOST:$DB_PORT/$DB_NAME"

# Run database migrations (IF NOT EXISTS guards make this safe to re-run)
echo "HariMart: Running database migrations..."
export PGPASSWORD="$DB_PASS"

psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" \
    -f db/migrations/001_initial_schema.sql 2>&1 || echo "Migration 001 skipped or already applied"

psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" \
    -f db/migrations/002_admin_seed.sql 2>&1 || echo "Migration 002 skipped or already applied"

psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" \
    -f db/migrations/003_add_category_image_wishlist.sql 2>&1 || echo "Migration 003 skipped or already applied"

psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" \
    -f db/migrations/004_seed_products.sql 2>&1 || echo "Migration 004 skipped or already applied"

echo "HariMart: Migrations complete"

# Generate config.json
cat > config.json << EOF
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

echo "HariMart: Config generated, launching server..."
exec ./HariMart
