INSERT INTO users (name, email, password_hash, role)
VALUES (
    'Admin',
    'admin@harimart.com',
    '$argon2id$v=19$m=65536,t=2,p=1$VJkp6zN1ji6O4vO9h/+yqw$xdsG2qCMWv/BUQJD2asWVdmy3hzUZDwV2dNb61qMGC0',
    'ADMIN'
)
ON CONFLICT (email) DO NOTHING;
