# 🛒 HariMart — Multi-Seller E-Commerce Marketplace

A full-stack multi-seller e-commerce marketplace built with **C++20**, **Drogon Framework**, and **PostgreSQL**.

## ✨ Features

### Authentication & Security
- Argon2id password hashing via libsodium
- Server-side session management (Drogon sessions)
- Role-based access control: **Buyer**, **Seller**, **Admin**
- Session validation endpoint (`/api/v1/auth/session`)

### Buyer Experience
- Browse 30+ products across 8 categories
- Real-time search and category filtering
- Add to cart with quantity management
- Wishlist (save for later)
- Full checkout flow with mock payment
- Order history with status tracking
- Product reviews and ratings (post-delivery)

### Seller Dashboard
- Add new products with name, description, price, stock, category, and image URL
- View and delete own products
- Incoming orders with fulfillment (Ship → Deliver)
- Product statistics

### Admin Dashboard
- View all registered users with roles
- View all orders with buyer information
- View all products with seller details
- Delete any product

### AI Chatbot
- Built-in shopping assistant (HariBot)
- Knowledge-based Q&A covering 19 topics
- No external API key required

## 🛠️ Tech Stack

| Layer        | Technology                    |
|-------------|-------------------------------|
| Backend     | C++20 + Drogon Framework      |
| Database    | PostgreSQL                    |
| Frontend    | HTML + CSS + Vanilla JS       |
| Auth        | libsodium (Argon2id)          |
| Build       | CMake + vcpkg                 |
| Font        | Inter (Google Fonts)          |

## 📁 Project Structure

```
HariMart/
├── CMakeLists.txt           # Build configuration
├── config.json              # Drogon server config
├── vcpkg.json               # vcpkg dependencies
├── db/
│   └── migrations/
│       ├── 001_initial_schema.sql
│       ├── 003_add_category_image_wishlist.sql
│       └── 004_seed_products.sql
├── frontend/
│   └── index.html           # Complete SPA frontend
└── src/
    ├── main.cpp
    ├── controller/
    │   ├── AuthController.cpp/h
    │   ├── ProductController.cpp/h
    │   ├── CartController.cpp/h
    │   ├── OrderController.cpp/h
    │   ├── ReviewController.cpp/h
    │   ├── WishlistController.cpp/h
    │   ├── AdminController.cpp/h
    │   └── ChatbotController.cpp/h
    ├── service/
    │   ├── AuthService.cpp/h
    │   ├── ProductService.cpp/h
    │   ├── CartService.cpp/h
    │   ├── OrderService.cpp/h
    │   ├── ReviewService.cpp/h
    │   └── WishlistService.cpp/h
    ├── repository/
    │   ├── Database.cpp/h
    │   ├── UserRepository.cpp/h
    │   ├── ProductRepository.cpp/h
    │   ├── CartRepository.cpp/h
    │   ├── OrderRepository.cpp/h
    │   ├── ReviewRepository.cpp/h
    │   └── WishlistRepository.cpp/h
    ├── model/
    │   ├── Product.h
    │   ├── CartItem.h
    │   ├── Order.h
    │   ├── OrderItem.h
    │   ├── Review.h
    │   └── WishlistItem.h
    └── util/
        ├── PasswordUtil.cpp/h
        └── AdminAuthUtil.cpp/h
```

## 🚀 Setup & Build

### Prerequisites
- **Visual Studio 2022** with C++20 support
- **PostgreSQL** (running on localhost:5432)
- **vcpkg** package manager
- **CMake** 3.25+

### 1. Install Dependencies
```bash
vcpkg install drogon nlohmann-json spdlog libsodium --triplet x64-windows
```

### 2. Database Setup
```bash
# Create database
psql -U postgres -c "CREATE DATABASE harimart;"

# Run migrations
psql -U postgres -d harimart -f db/migrations/001_initial_schema.sql
psql -U postgres -d harimart -f db/migrations/003_add_category_image_wishlist.sql
psql -U postgres -d harimart -f db/migrations/004_seed_products.sql
```

### 3. Configure
Edit `config.json` with your PostgreSQL credentials:
```json
{
    "db_clients": [{
        "user": "postgres",
        "passwd": "your_password",
        "dbname": "harimart"
    }]
}
```

### 4. Build
```bash
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
```

### 5. Run
```bash
./build/Release/HariMart.exe
```

Open **http://127.0.0.1:8080** in your browser.

## 🔑 Demo Accounts

| Role   | Email                        | Password  |
|--------|------------------------------|-----------|
| Buyer  | buyer@harimart.com           | admin123  |
| Seller | techworld@harimart.com       | admin123  |
| Seller | fashionhub@harimart.com      | admin123  |
| Admin  | *(create via SQL)*           | *(set)*   |

## 📡 API Endpoints

| Method | Endpoint                         | Description              |
|--------|----------------------------------|--------------------------|
| POST   | `/api/v1/auth/register`          | Register new user        |
| POST   | `/api/v1/auth/login`             | Login                    |
| POST   | `/api/v1/auth/logout`            | Logout                   |
| GET    | `/api/v1/auth/session`           | Check session            |
| GET    | `/api/v1/products`               | List all products        |
| GET    | `/api/v1/products/{id}`          | Get product by ID        |
| POST   | `/api/v1/products`               | Create product (seller)  |
| PUT    | `/api/v1/products/{id}`          | Update product (seller)  |
| DELETE | `/api/v1/products/{id}`          | Delete product (seller)  |
| GET    | `/api/v1/cart`                   | Get cart                 |
| POST   | `/api/v1/cart`                   | Add to cart              |
| PUT    | `/api/v1/cart/{id}`              | Update cart item         |
| DELETE | `/api/v1/cart/{id}`              | Remove from cart         |
| POST   | `/api/v1/orders`                 | Create order             |
| GET    | `/api/v1/orders`                 | Get buyer orders         |
| GET    | `/api/v1/orders/seller`          | Get seller orders        |
| POST   | `/api/v1/orders/{id}/confirm`    | Confirm order            |
| PUT    | `/api/v1/orders/{id}/status`     | Update order status      |
| GET    | `/api/v1/reviews/{productId}`    | Get product reviews      |
| POST   | `/api/v1/reviews`                | Submit review            |
| GET    | `/api/v1/wishlist`               | Get wishlist             |
| POST   | `/api/v1/wishlist`               | Add to wishlist          |
| DELETE | `/api/v1/wishlist/{productId}`   | Remove from wishlist     |
| POST   | `/api/v1/chatbot`                | Chatbot message          |
| GET    | `/api/v1/admin/users`            | Admin: list users        |
| GET    | `/api/v1/admin/orders`           | Admin: list orders       |
| GET    | `/api/v1/admin/products`         | Admin: list products     |
| DELETE | `/api/v1/admin/products/{id}`    | Admin: delete product    |

## 📋 Architecture

```
Frontend (SPA)
    │
    ▼
Controller Layer    ← HTTP handling, JSON parsing, session auth
    │
    ▼
Service Layer       ← Business logic, validation
    │
    ▼
Repository Layer    ← Parameterized SQL, PostgreSQL via Drogon ORM
    │
    ▼
PostgreSQL Database
```

---

Built with ❤️ by Hari Keerthana
