-- Migration 003: Add image_url to products, add wishlist table, add unique review constraint

-- Add image_url column to products if not exists
ALTER TABLE products ADD COLUMN IF NOT EXISTS image_url TEXT DEFAULT '';

-- Create wishlist table
CREATE TABLE IF NOT EXISTS wishlist_items (
    id BIGSERIAL PRIMARY KEY,
    user_id BIGINT NOT NULL,
    product_id BIGINT NOT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT fk_wishlist_user
        FOREIGN KEY (user_id)
        REFERENCES users(id)
        ON DELETE CASCADE,

    CONSTRAINT fk_wishlist_product
        FOREIGN KEY (product_id)
        REFERENCES products(id)
        ON DELETE CASCADE,

    CONSTRAINT uq_wishlist_user_product
        UNIQUE (user_id, product_id)
);

CREATE INDEX IF NOT EXISTS idx_wishlist_user_id
    ON wishlist_items(user_id);

-- Add unique constraint on reviews to prevent duplicate reviews
-- (user can only review a product once)
DO $$
BEGIN
    IF NOT EXISTS (
        SELECT 1 FROM pg_constraint WHERE conname = 'uq_review_user_product'
    ) THEN
        ALTER TABLE reviews ADD CONSTRAINT uq_review_user_product
            UNIQUE (user_id, product_id);
    END IF;
END $$;
