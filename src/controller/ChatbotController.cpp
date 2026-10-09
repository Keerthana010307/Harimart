#include "ChatbotController.h"
#include <json/json.h>
#include <string>
#include <vector>
#include <algorithm>

namespace
{

struct QAPair
{
    std::vector<std::string> keywords;
    std::string answer;
};

const std::vector<QAPair> knowledgeBase = {
    {{"hello", "hi", "hey", "greet"},
     "Hello! Welcome to HariMart 🛒 I'm your shopping assistant. How can I help you today? You can ask me about products, orders, cart, reviews, or how to use the marketplace."},

    {{"add", "cart", "how to add"},
     "To add a product to your cart: 1) Browse or search for products on the Products page. 2) Click the '🛒 Add' button on any product card. 3) The item will be added to your cart automatically. You can view your cart by clicking the Cart button in the navigation bar."},

    {{"buy", "purchase", "buy now"},
     "To buy a product instantly: Click the '⚡ Buy' button on any product card. This adds the item to your cart and takes you directly to checkout. You can also add multiple items to your cart first and then proceed to checkout from the Cart page."},

    {{"checkout", "pay", "payment", "order", "place order"},
     "To checkout: 1) Go to your Cart page. 2) Review your items and quantities. 3) Click 'Proceed to Checkout'. 4) Confirm your order. The system uses mock payment for demonstration. After successful payment, your order is created and the cart is cleared."},

    {{"order", "track", "status", "history", "my orders"},
     "To view your orders: Click 'Orders' in the navigation bar. You'll see all your past orders with their status. Orders go through these stages: PENDING → CONFIRMED → SHIPPED → DELIVERED. Sellers can update the order status from their dashboard."},

    {{"review", "rate", "rating", "stars", "feedback"},
     "To leave a review: 1) Click the '⭐ Reviews' button on a product card. 2) Select your star rating (1-5). 3) Write an optional comment. 4) Click 'Submit Review'. Note: You can only review products from orders that have been DELIVERED to you, and you can only review each product once."},

    {{"seller", "sell", "add product", "create product", "listing"},
     "To sell on HariMart: 1) Register as a Seller. 2) Go to the Seller Dashboard. 3) Fill in product details (name, description, price, stock, category, image URL). 4) Click 'Add Product'. Your products will appear in the marketplace immediately. You can also edit or delete your products."},

    {{"wishlist", "wish list", "save", "favorite", "later"},
     "To use the Wishlist: Click the '♡' button on any product card to save it for later. View your saved items by clicking 'Wishlist' in the navigation. From your wishlist, you can move items to your cart or remove them."},

    {{"search", "find", "filter", "category"},
     "To find products: 1) Use the search bar at the top to search by name or description. 2) Click category filter buttons (All, Electronics, Fashion, Home, Books, etc.) on the Products page to filter by category. 3) Products are loaded live from the database."},

    {{"account", "register", "sign up", "create account"},
     "To create an account: 1) On the login page, click 'Register'. 2) Enter your name, email, and password. 3) Choose your role (Buyer or Seller). 4) Click 'Create Account'. You can then log in with your credentials."},

    {{"login", "sign in", "log in"},
     "To log in: Enter your email and password on the login page and click 'Login to HariMart'. Your session will be maintained until you log out. Demo accounts: buyer@harimart.com / admin@harimart.com (password: admin123)."},

    {{"logout", "sign out", "log out"},
     "To log out: Click the 'Logout' button (red) in the top-right corner of the navigation bar. This will clear your session and return you to the login page."},

    {{"admin", "manage", "moderate", "dashboard"},
     "The Admin Dashboard allows administrators to: View all users, View all orders with buyer information, View all product listings with seller details, and Delete/moderate products. Admin access is restricted to admin accounts only."},

    {{"price", "cost", "expensive", "cheap", "affordable"},
     "Product prices are displayed on each product card in Indian Rupees (₹). You can browse all products on the Products page and sort through different categories to find items in your price range."},

    {{"delivery", "shipping", "ship", "deliver"},
     "Order delivery status is tracked through: PENDING → CONFIRMED → SHIPPED → DELIVERED. Sellers update the status as they process orders. You can check your order status on the Orders page."},

    {{"help", "support", "assist", "what can you do"},
     "I can help you with: 🛒 Adding products to cart, 💳 Checkout process, 📦 Order tracking, ⭐ Writing reviews, 🏪 Seller product management, ❤️ Wishlist management, 🔍 Searching products, 👤 Account management. Just ask me anything!"},

    {{"tech", "stack", "built", "technology", "backend", "c++", "drogon"},
     "HariMart is built with: Backend: C++20 with Drogon framework, Database: PostgreSQL, Frontend: HTML + CSS + JavaScript, Authentication: Argon2id password hashing via libsodium, Build: CMake + vcpkg. It's a full-stack multi-seller e-commerce marketplace."},

    {{"safe", "secure", "security", "password"},
     "HariMart uses industry-standard security: Argon2id password hashing (via libsodium), server-side sessions, parameterized SQL queries to prevent injection, role-based access control (Buyer/Seller/Admin), and input validation on all endpoints."},

    {{"return", "refund", "cancel"},
     "Currently, HariMart supports order cancellation for orders in PENDING status. For returns and refunds, please contact the seller directly. The order status workflow is: PENDING → CONFIRMED → SHIPPED → DELIVERED."},
};

std::string findBestAnswer(const std::string& message)
{
    std::string lowerMessage = message;
    std::transform(lowerMessage.begin(), lowerMessage.end(),
                   lowerMessage.begin(), ::tolower);

    int bestScore = 0;
    std::string bestAnswer = "";

    for (const auto& qa : knowledgeBase)
    {
        int score = 0;
        for (const auto& keyword : qa.keywords)
        {
            if (lowerMessage.find(keyword) != std::string::npos)
            {
                score++;
            }
        }
        if (score > bestScore)
        {
            bestScore = score;
            bestAnswer = qa.answer;
        }
    }

    if (bestScore == 0)
    {
        return "I'm not sure about that. Here are some things I can help with:\n\n"
               "🛒 How to add products to cart\n"
               "💳 How checkout works\n"
               "📦 How to view orders\n"
               "⭐ How reviews work\n"
               "🏪 Seller product management\n"
               "❤️ Wishlist features\n"
               "🔍 Searching and filtering products\n"
               "👤 Account and login help\n\n"
               "Try asking about any of these topics!";
    }

    return bestAnswer;
}

} // anonymous namespace

void ChatbotController::chat(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto jsonBody = req->getJsonObject();
    Json::Value responseJson;

    if (!jsonBody || !jsonBody->isMember("message"))
    {
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"]["message"] = "message is required";
        auto response = drogon::HttpResponse::newHttpJsonResponse(responseJson);
        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    std::string userMessage = (*jsonBody)["message"].asString();

    if (userMessage.empty())
    {
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"]["message"] = "Message cannot be empty";
        auto response = drogon::HttpResponse::newHttpJsonResponse(responseJson);
        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    std::string botReply = findBestAnswer(userMessage);

    responseJson["success"] = true;
    responseJson["data"]["reply"] = botReply;
    responseJson["error"] = Json::nullValue;

    auto response = drogon::HttpResponse::newHttpJsonResponse(responseJson);
    response->setStatusCode(drogon::k200OK);
    callback(response);
}
