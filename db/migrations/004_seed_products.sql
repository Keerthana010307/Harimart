-- ========================================================
-- HariMart: Seed Data — 10 Featured Products with Images
-- Run AFTER 001_initial_schema.sql and 003_add_...sql
-- ========================================================

-- Create seller accounts (Argon2id hash of 'admin123')
INSERT INTO users (name, email, password_hash, role)
VALUES
    ('TechWorld Store', 'techworld@harimart.com',
     '$argon2id$v=19$m=65536,t=3,p=1$c2FsdHNhbHRzYWx0$RdescudvJCsgt3ube/oB7MWb2rEaxB8/YR7OIIL0sLY',
     'SELLER'),
    ('StyleHub', 'stylehub@harimart.com',
     '$argon2id$v=19$m=65536,t=3,p=1$c2FsdHNhbHRzYWx0$RdescudvJCsgt3ube/oB7MWb2rEaxB8/YR7OIIL0sLY',
     'SELLER')
ON CONFLICT (email) DO NOTHING;

-- Create demo buyer account
INSERT INTO users (name, email, password_hash, role)
VALUES
    ('Demo Buyer', 'buyer@harimart.com',
     '$argon2id$v=19$m=65536,t=3,p=1$c2FsdHNhbHRzYWx0$RdescudvJCsgt3ube/oB7MWb2rEaxB8/YR7OIIL0sLY',
     'BUYER')
ON CONFLICT (email) DO NOTHING;

-- Create admin account
INSERT INTO users (name, email, password_hash, role)
VALUES
    ('Admin', 'admin@harimart.com',
     '$argon2id$v=19$m=65536,t=3,p=1$c2FsdHNhbHRzYWx0$RdescudvJCsgt3ube/oB7MWb2rEaxB8/YR7OIIL0sLY',
     'ADMIN')
ON CONFLICT (email) DO NOTHING;

-- Delete any existing seeded products to avoid duplicates on re-run
DELETE FROM products WHERE name IN (
    'Aether Pro Wireless Headphones',
    'Pulse Fit Smartwatch',
    'Aura Running Shoes',
    'Heritage Leather Backpack',
    'Artisan Ceramic Mug Set',
    'Nova 75 Mechanical Keyboard',
    'Luxe Aviator Sunglasses',
    'Lumina LED Desk Lamp',
    'Zen Cork Yoga Mat',
    'The Silent Echo — A Novel'
);

-- Insert 10 featured products with local images
-- (seller_id references are resolved via subquery)

INSERT INTO products (seller_id, name, description, price_cents, stock_qty, category, image_url)
VALUES
    -- 1. Headphones (Electronics)
    (
        (SELECT id FROM users WHERE email = 'techworld@harimart.com'),
        'Aether Pro Wireless Headphones',
        'Premium noise-cancelling over-ear headphones with 40-hour battery, Hi-Res Audio, adaptive ANC, and buttery-soft protein leather cushions. Rose gold accents on matte black.',
        1299900,
        25,
        'Electronics',
        '/images/headphones.jpg'
    ),

    -- 2. Smartwatch (Electronics)
    (
        (SELECT id FROM users WHERE email = 'techworld@harimart.com'),
        'Pulse Fit Smartwatch',
        'Advanced fitness smartwatch with AMOLED always-on display, heart rate monitoring, GPS tracking, sleep analysis, and 7-day battery life. Water resistant to 50m.',
        2499900,
        18,
        'Electronics',
        '/images/smartwatch.jpg'
    ),

    -- 3. Running Shoes (Sports)
    (
        (SELECT id FROM users WHERE email = 'stylehub@harimart.com'),
        'Aura Running Shoes',
        'Lightweight performance running shoes with responsive foam cushioning, breathable knit upper, and high-traction rubber outsole. Vibrant coral and white colorway.',
        899900,
        40,
        'Sports',
        '/images/running_shoes.jpg'
    ),

    -- 4. Leather Backpack (Fashion)
    (
        (SELECT id FROM users WHERE email = 'stylehub@harimart.com'),
        'Heritage Leather Backpack',
        'Handcrafted full-grain cognac leather backpack with brass hardware, padded 15" laptop compartment, and organizer pockets. Ages beautifully with use.',
        1199900,
        12,
        'Fashion',
        '/images/backpack.jpg'
    ),

    -- 5. Ceramic Mug Set (Home)
    (
        (SELECT id FROM users WHERE email = 'stylehub@harimart.com'),
        'Artisan Ceramic Mug Set',
        'Set of 4 handmade speckled ceramic mugs in earth tones — sage, terracotta, cream, and charcoal. Microwave and dishwasher safe. 350ml capacity each.',
        349900,
        30,
        'Home',
        '/images/mugs.jpg'
    ),

    -- 6. Mechanical Keyboard (Electronics)
    (
        (SELECT id FROM users WHERE email = 'techworld@harimart.com'),
        'Nova 75 Mechanical Keyboard',
        'Compact 75% layout mechanical keyboard with hot-swappable switches, per-key RGB, PBT keycaps, and USB-C with Bluetooth 5.0. Satisfying tactile feedback.',
        799900,
        22,
        'Electronics',
        '/images/keyboard.jpg'
    ),

    -- 7. Sunglasses (Accessories)
    (
        (SELECT id FROM users WHERE email = 'stylehub@harimart.com'),
        'Luxe Aviator Sunglasses',
        'Premium gold-frame aviator sunglasses with gradient brown CR-39 lenses, UV400 protection, and tortoiseshell acetate temple tips. Italian craftsmanship.',
        599900,
        35,
        'Accessories',
        '/images/sunglasses.jpg'
    ),

    -- 8. Desk Lamp (Home)
    (
        (SELECT id FROM users WHERE email = 'techworld@harimart.com'),
        'Lumina LED Desk Lamp',
        'Scandinavian-design adjustable LED desk lamp with 5 brightness levels, 3 color temperatures, touch controls, and eye-care flicker-free technology. Matte white finish.',
        449900,
        28,
        'Home',
        '/images/desk_lamp.jpg'
    ),

    -- 9. Yoga Mat (Sports)
    (
        (SELECT id FROM users WHERE email = 'stylehub@harimart.com'),
        'Zen Cork Yoga Mat',
        'Eco-friendly premium yoga mat with natural cork surface and TPE base. Non-slip grip improves with moisture. 6mm thickness for joint protection. Includes carry straps.',
        299900,
        45,
        'Sports',
        '/images/yoga_mat.jpg'
    ),

    -- 10. Book (Books)
    (
        (SELECT id FROM users WHERE email = 'stylehub@harimart.com'),
        'The Silent Echo — A Novel',
        'A mesmerizing literary fiction debut exploring memory, identity, and the echoes of choices we make. Gold-foil embossed hardcover, 384 pages. A must-read.',
        149900,
        60,
        'Books',
        '/images/book.jpg'
    );
