# Requirements Document

## Introduction

This feature fixes the broken frontend-to-backend integration in HariMart, a multi-seller e-commerce marketplace. The root cause is that the frontend used the hardcoded API base URL `http://127.0.0.1:8080`, while the browser served the page from the `localhost:8080` origin. Because `127.0.0.1` and `localhost` are treated as distinct origins by modern browsers, the session cookie established at login was silently dropped on every subsequent authenticated API call, causing all protected endpoints to return 401. The fix switches all API requests to relative URLs (same-origin), centralises the `fetch` logic in a single helper, maps HTTP status codes to human-readable messages, and resolves a type-coercion bug in the seller dashboard product filter.

## Glossary

- **Frontend**: The single-page application served from `frontend/index.html`, implemented in HTML, CSS, and Vanilla JavaScript.
- **Backend**: The C++20 Drogon Framework application listening on port 8080, which serves both the static frontend and the REST API.
- **API Base URL**: The string prepended to every API path in `apiFetch()`. Setting it to `""` (empty string) forces all requests to use relative URLs, ensuring the same origin as the page.
- **Session Cookie**: An HTTP-only cookie set by the Backend on login and sent by the browser only to the same origin that issued it.
- **apiFetch()**: The centralised JavaScript `fetch` wrapper that prepends the API Base URL, attaches credentials, logs failures, and maps status codes to messages.
- **apiErrorMessage()**: A JavaScript helper that inspects an `apiFetch` result and returns a human-readable error string.
- **Seller Dashboard**: The `/seller` page section visible only to users with the `SELLER` role, where sellers manage their products and view incoming orders.
- **sellerId type coercion**: The bug where a strict equality comparison (`===`) between a numeric `sellerId` from the API and a string stored in `currentUser.userId` caused all seller products to be hidden.
- **BUYER**: A registered user role that can browse products, manage a cart, place orders, manage a wishlist, and submit reviews.
- **SELLER**: A registered user role that additionally can list products and view and update incoming orders.
- **ADMIN**: A registered user role that has full visibility and deletion rights over users, products, and orders.

---

## Requirements

### Requirement 1 — Same-Origin API Requests

**User Story:** As a registered user, I want every API request to go to the same origin that served the page, so that my session cookie is included and I remain authenticated across all actions.

#### Acceptance Criteria

1. THE Frontend SHALL define the API Base URL as an empty string (`""`), so that all API paths resolve relative to the current page origin.
2. WHEN the Frontend constructs an API request URL, THE Frontend SHALL prepend the API Base URL to the path, producing a relative URL with no scheme, host, or port.
3. WHILE a user session is active, THE Frontend SHALL include `credentials: 'include'` on every `fetch` call that accesses an authenticated endpoint.
4. IF the API Base URL is a non-empty absolute URL, THEN THE Frontend SHALL NOT proceed with that configuration in the production build.

---

### Requirement 2 — Centralised Fetch Helper

**User Story:** As a developer, I want all API calls routed through a single `apiFetch()` helper, so that error handling, logging, and credential attachment are consistent across the entire application.

#### Acceptance Criteria

1. THE Frontend SHALL expose exactly one function, `apiFetch(url, options)`, that all API call sites invoke instead of calling `fetch()` directly.
2. WHEN `apiFetch()` is called, THE Frontend SHALL prepend the API Base URL to the provided path before invoking `fetch()`.
3. WHEN a network-level error occurs (no HTTP response received), THE Frontend SHALL return a result object with `ok: false`, `status: 0`, and `networkError: true`.
4. WHEN an HTTP response is received, THE Frontend SHALL attempt to parse the response body as JSON and attach the parsed object to the result as `data`.
5. WHEN an HTTP response with a status code outside the 200–299 range is received, THE Frontend SHALL log the method, URL, status code, and error message to the browser console.
6. THE Frontend SHALL expose an `apiErrorMessage(result, fallback)` function that maps `result.status` values to human-readable strings according to the following rules:
   - `networkError === true` → `"Cannot reach the server — make sure HariMart is running on port 8080"`
   - `status === 401` → the `error.message` field from `result.data`, or `"Please log in to continue"`
   - `status === 403` → the `error.message` field from `result.data`, or `"You do not have permission for this action"`
   - `status === 404` → the `error.message` field from `result.data`, or `"Resource not found"`
   - `status === 400` → the `error.message` field from `result.data`, or `"Invalid request"`
   - `status >= 500` → `"Server error — please try again later"`
   - all other cases → the `error.message` field from `result.data`, or the provided `fallback` string

---

### Requirement 3 — Authentication Flow

**User Story:** As a visitor, I want to register and log in, so that I can access my personalised shopping experience.

#### Acceptance Criteria

1. WHEN the user submits the login form, THE Frontend SHALL call `POST /api/v1/auth/login` via `apiFetch()` with `credentials: 'include'` and a JSON body containing `email` and `password`.
2. WHEN the login response has `success: true`, THE Frontend SHALL store the returned user object (containing `email`, `role`, `name`, `userId`) in the `currentUser` variable and transition to the main application view.
3. WHEN the login response does not have `success: true`, THE Frontend SHALL display the result of `apiErrorMessage()` in the login message element with the CSS class `error`.
4. WHEN the user submits the registration form, THE Frontend SHALL call `POST /api/v1/auth/register` via `apiFetch()` with a JSON body containing `name`, `email`, `password`, and `role`.
5. WHEN the registration response has `success: true`, THE Frontend SHALL display a success message and navigate to the login tab after 1200 ms.
6. WHEN the user clicks Logout, THE Frontend SHALL call `POST /api/v1/auth/logout` via `apiFetch()`, clear `currentUser`, and return to the authentication screen.

---

### Requirement 4 — Product Listing and Filtering

**User Story:** As a buyer, I want to browse, search, and filter products, so that I can find items I want to purchase.

#### Acceptance Criteria

1. WHEN the products page is shown, THE Frontend SHALL call `GET /api/v1/products` via `apiFetch()` and render all returned products in the product grid.
2. WHEN the backend is unreachable (networkError), THE Frontend SHALL display a human-readable "Backend not reachable" empty state in the product grid instead of a raw error.
3. WHEN a category filter button is clicked, THE Frontend SHALL filter the local `products` array client-side and re-render only products whose `category` field matches the selected category (case-insensitive).
4. WHEN the global search input value changes, THE Frontend SHALL filter the local `products` array client-side and re-render products whose `name`, `description`, or `category` field contains the search term (case-insensitive).
5. THE Frontend SHALL render a product card with `name`, `description`, price (converted from cents to rupees), stock status, category badge, wishlist toggle, and action buttons for each product.

---

### Requirement 5 — Cart Management

**User Story:** As a buyer, I want to add, view, and remove products from my cart, so that I can prepare an order.

#### Acceptance Criteria

1. WHEN the user clicks "Add to Cart", THE Frontend SHALL call `POST /api/v1/cart` via `apiFetch()` with `credentials: 'include'` and a JSON body `{ productId, quantity: 1 }`.
2. WHEN the cart add call returns `success: true`, THE Frontend SHALL display a success toast and refresh the cart badge count.
3. WHEN the cart add call does not return `success: true`, THE Frontend SHALL display the result of `apiErrorMessage()` as an error toast.
4. WHEN the cart page is shown, THE Frontend SHALL call `GET /api/v1/cart` via `apiFetch()` with `credentials: 'include'` and render all items with their names, per-unit prices, quantities, and line totals.
5. WHEN the user clicks the remove button on a cart item, THE Frontend SHALL call `DELETE /api/v1/cart/{productId}` via `apiFetch()` with `credentials: 'include'` and re-render the cart on success.
6. THE Frontend SHALL display the cart subtotal and total by converting `totalCents` from the API response from cents to rupees.

---

### Requirement 6 — Checkout and Order Placement

**User Story:** As a buyer, I want to confirm my cart and place an order, so that I can purchase my selected items.

#### Acceptance Criteria

1. WHEN the user clicks "Proceed to Checkout", THE Frontend SHALL open the checkout confirmation modal displaying the current cart total.
2. WHEN the user confirms the checkout, THE Frontend SHALL first call `POST /api/v1/orders` via `apiFetch()` with `credentials: 'include'` to create the order.
3. WHEN the order creation returns `success: true`, THE Frontend SHALL call `POST /api/v1/orders/{orderId}/confirm` via `apiFetch()` with `credentials: 'include'` to process the mock payment.
4. WHEN the order confirmation returns `success: true`, THE Frontend SHALL close the checkout modal, display a success toast, refresh the cart, and navigate to the orders page.
5. IF the order creation or confirmation call does not return `success: true`, THEN THE Frontend SHALL display the result of `apiErrorMessage()` as an error toast and close the checkout modal.

---

### Requirement 7 — Order History

**User Story:** As a buyer, I want to view my past orders, so that I can track my purchase history.

#### Acceptance Criteria

1. WHEN the orders page is shown, THE Frontend SHALL call `GET /api/v1/orders` via `apiFetch()` with `credentials: 'include'` and render each order with its ID, status badge, and total amount in rupees.
2. WHEN the orders response is empty or returns no orders, THE Frontend SHALL display an empty-state message prompting the user to start shopping.
3. WHEN the orders call does not return `success: true`, THE Frontend SHALL display an empty-state element with the result of `apiErrorMessage()`.

---

### Requirement 8 — Wishlist

**User Story:** As a buyer, I want to save products to a wishlist, so that I can revisit and purchase them later.

#### Acceptance Criteria

1. WHEN the user clicks the wishlist toggle on a product card, THE Frontend SHALL call `POST /api/v1/wishlist` via `apiFetch()` with `credentials: 'include'` and a JSON body `{ productId }`.
2. WHEN the wishlist page is shown, THE Frontend SHALL call `GET /api/v1/wishlist` via `apiFetch()` with `credentials: 'include'` and render saved products.
3. WHEN the user removes a wishlist item, THE Frontend SHALL call `DELETE /api/v1/wishlist/{productId}` via `apiFetch()` with `credentials: 'include'` and re-render the wishlist.
4. IF the wishlist call returns status 401, THEN THE Frontend SHALL display a "Please log in" empty state.

---

### Requirement 9 — Product Reviews

**User Story:** As a buyer, I want to read and write product reviews, so that I can make informed purchasing decisions and share my experience.

#### Acceptance Criteria

1. WHEN the review modal is opened for a product, THE Frontend SHALL call `GET /api/v1/reviews/{productId}` via `apiFetch()` and render the average rating, review count, and individual review entries.
2. WHEN the user submits a review, THE Frontend SHALL call `POST /api/v1/reviews` via `apiFetch()` with `credentials: 'include'` and a JSON body containing `productId`, `rating`, and `comment`.
3. WHEN a review is submitted without a star rating selected (`selectedRating === 0`), THE Frontend SHALL display an error toast and NOT call the API.
4. WHEN the review submission returns `success: true`, THE Frontend SHALL clear the form fields, reset the star selection, and reload the reviews list for the product.

---

### Requirement 10 — Seller Dashboard

**User Story:** As a seller, I want to manage my product listings and view incoming orders, so that I can run my shop on HariMart.

#### Acceptance Criteria

1. WHEN the seller dashboard is loaded, THE Frontend SHALL call `GET /api/v1/products` via `apiFetch()` and filter the result to only products where `Number(product.sellerId) === Number(currentUser.userId)`, using numeric coercion on both sides of the comparison.
2. WHEN a seller submits the add-product form, THE Frontend SHALL call `POST /api/v1/products` via `apiFetch()` with `credentials: 'include'` and a JSON body containing `name`, `description`, `priceCents` (price in rupees multiplied by 100, rounded to the nearest integer), `stock`, `category`, and `imageUrl`.
3. WHEN a seller clicks Delete on one of their products, THE Frontend SHALL call `DELETE /api/v1/products/{productId}` via `apiFetch()` with `credentials: 'include'` and reload both the seller product list and the global product list on success.
4. WHEN the seller dashboard is loaded, THE Frontend SHALL call `GET /api/v1/orders/seller` via `apiFetch()` with `credentials: 'include'` and render each incoming order with its ID, status, and total amount.
5. WHEN a seller clicks "Ship" on a CONFIRMED order, THE Frontend SHALL call `PUT /api/v1/orders/{orderId}/status` via `apiFetch()` with `credentials: 'include'` and a JSON body `{ status: "SHIPPED" }`.
6. WHEN a seller clicks "Deliver" on a SHIPPED order, THE Frontend SHALL call `PUT /api/v1/orders/{orderId}/status` via `apiFetch()` with `credentials: 'include'` and a JSON body `{ status: "DELIVERED" }`.

---

### Requirement 11 — Admin Dashboard

**User Story:** As an admin, I want to view and manage all users, products, and orders, so that I can maintain the marketplace.

#### Acceptance Criteria

1. WHEN the admin users tab is selected, THE Frontend SHALL call `GET /api/v1/admin/users` via `apiFetch()` with `credentials: 'include'` and render a table with each user's ID, name, email, and role badge.
2. WHEN the admin products tab is selected, THE Frontend SHALL call `GET /api/v1/admin/products` via `apiFetch()` with `credentials: 'include'` and render a table with each product's ID, name, seller name, price, stock, category, and a Delete action.
3. WHEN an admin deletes a product, THE Frontend SHALL call `DELETE /api/v1/admin/products/{id}` via `apiFetch()` with `credentials: 'include'` and reload both the admin product list and the global product list on success.
4. WHEN the admin orders tab is selected, THE Frontend SHALL call `GET /api/v1/admin/orders` via `apiFetch()` with `credentials: 'include'` and render a table with each order's ID, buyer name, buyer email, status badge, and total amount.
5. IF any admin API call returns a non-success result, THEN THE Frontend SHALL display the result of `apiErrorMessage()` as the content of the admin panel.

---

### Requirement 12 — AI Chatbot

**User Story:** As a user, I want to interact with HariBot, so that I can get shopping assistance without leaving the page.

#### Acceptance Criteria

1. WHEN the user sends a message in the chatbot panel, THE Frontend SHALL call `POST /api/v1/chatbot` via `apiFetch()` with `credentials: 'include'` and a JSON body `{ message }`.
2. WHEN the chatbot response is received, THE Frontend SHALL append the `data.reply` value as a bot message bubble in the chat panel.
3. IF the chatbot call results in `networkError`, THEN THE Frontend SHALL display `"Connection error. Make sure the backend is running."` as a bot message bubble.

---

### Requirement 13 — Error Communication and Toast Notifications

**User Story:** As a user, I want clear feedback when an action fails, so that I understand what went wrong and how to resolve it.

#### Acceptance Criteria

1. WHEN any API call fails, THE Frontend SHALL display a human-readable message derived from `apiErrorMessage()` rather than a raw HTTP status code or a generic "Backend not connected" string.
2. WHEN a success action completes (cart add, remove, order confirm, review submit, product add/delete), THE Frontend SHALL display a success toast with a descriptive message.
3. WHEN an error action occurs (failed add-to-cart, failed checkout, etc.), THE Frontend SHALL display an error toast with the result of `apiErrorMessage()`.
4. THE Frontend SHALL auto-dismiss toast notifications after 3000 ms with a fade-out transition.
