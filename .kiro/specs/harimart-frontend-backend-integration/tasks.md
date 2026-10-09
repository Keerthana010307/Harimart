# Implementation Plan: HariMart Frontend–Backend Integration Fix

## Overview

All changes are confined to `frontend/index.html`. The fix has three logical parts: (1) set the API base URL to an empty string so all fetch calls use same-origin relative URLs, (2) introduce `apiFetch()` and `apiErrorMessage()` as the centralised API layer, and (3) fix the seller dashboard type-coercion bug. A Node.js + JSDOM + fast-check test harness validates the pure-function and DOM-dependent properties.

---

## Tasks

- [ ] 1. Set up the test harness for property-based and unit tests
  - Install `fast-check`, `jsdom`, and a minimal test runner (`node:test` or `jest`) as dev dependencies in a new `tests/` directory alongside `frontend/index.html`
  - Create `tests/helpers.js` that extracts the pure JS functions (`apiFetch`, `apiErrorMessage`, `escapeHtml`, `createProductCard`) from `index.html` into an importable module so tests can `import` them without a browser
  - Configure a `package.json` (or extend an existing one) with a `test` script that runs all files under `tests/` once (`--run` or equivalent) and exits
  - _Requirements: 2.1, 2.6, 4.5_

- [ ] 2. Implement and verify the API constants and `apiFetch` / `apiErrorMessage` helpers
  - [ ] 2.1 Set `const API = ""` in `frontend/index.html`
    - Remove (or replace) the old hardcoded `"http://127.0.0.1:8080"` value
    - Add the JSDoc comment explaining why the empty string forces same-origin requests
    - _Requirements: 1.1, 1.2_

  - [ ] 2.2 Implement `apiFetch(url, options)` in `frontend/index.html`
    - Build `fullUrl = API + url`, delegate to native `fetch(fullUrl, options)`
    - On network failure (catch): return `{ ok: false, status: 0, data: null, networkError: true }`
    - On any HTTP response: attempt `res.json()`, attach as `data` (null on parse failure)
    - On non-2xx: call `console.error(method, fullUrl, status, errMsg, data)`
    - Return `{ ok: res.ok, status: res.status, data }`
    - _Requirements: 2.1, 2.2, 2.3, 2.4, 2.5_

  - [ ]* 2.3 Write property test for URL construction (Property 1)
    - **Property 1: URL construction never produces an absolute URL**
    - Mock `fetch` to capture the URL argument; assert it equals `"" + path` with no scheme/host/port
    - Run 100+ iterations with arbitrary path strings
    - **Validates: Requirements 1.2, 2.2**

  - [ ]* 2.4 Write property test for JSON body attachment (Property 2)
    - **Property 2: JSON response body is always attached as `data`**
    - For any JSON-serializable object `O`, mock fetch to return `JSON.stringify(O)`; assert `result.data` deep-equals `O`
    - **Validates: Requirements 2.4**

  - [ ]* 2.5 Write property test for non-2xx console error (Property 3)
    - **Property 3: Non-2xx responses always trigger a console error**
    - For any status `s` outside 200–299, spy on `console.error`; assert it is called with method, URL, `s`, and a description string
    - **Validates: Requirements 2.5**

  - [ ] 2.6 Implement `apiErrorMessage(result, fallback)` in `frontend/index.html`
    - Implement the full status-code mapping table as specified in Requirement 2.6
    - Function must be pure (no side effects) and always return a non-empty string
    - _Requirements: 2.6, 13.1_

  - [ ]* 2.7 Write property test for `apiErrorMessage` mapping completeness (Property 4)
    - **Property 4: `apiErrorMessage` is a total pure function with correct status mapping**
    - Use fast-check to generate arbitrary `result` objects and `fallback` strings; assert the returned string matches the mapping table and is never empty/null/undefined
    - Verify each status-code branch: networkError → "port 8080"; 401 → login message; 403 → permission message; 404 → not found; 400 → invalid request; ≥500 → server error
    - **Validates: Requirements 2.6, 13.1**

- [ ] 3. Checkpoint — Ensure API helpers pass all tests
  - Ensure all tests pass, ask the user if questions arise.

- [ ] 4. Wire `apiFetch` into authentication and navigation flows
  - [ ] 4.1 Update the login form handler to use `apiFetch`
    - Call `apiFetch('/api/v1/auth/login', { method: 'POST', credentials: 'include', ... })`
    - On `success: true`: store `result.data.data` in `currentUser`, call `enterApp()`
    - On failure: set `#loginMessage` text to `apiErrorMessage(result, 'Login failed.')` and add class `error`
    - _Requirements: 3.1, 3.2, 3.3_

  - [ ]* 4.2 Write property test for login success → `currentUser` stored (Property 5)
    - **Property 5: Successful login stores any valid user object and enters the app**
    - For any user object `{ email, role, name, userId }`, mock the login API to return `success: true`; assert `currentUser` equals the user object and `#mainApp` is visible
    - **Validates: Requirements 3.2**

  - [ ]* 4.3 Write property test for login failure → `apiErrorMessage` in DOM (Property 6)
    - **Property 6: Login failure always surfaces `apiErrorMessage` output**
    - For any error result `r`, assert `#loginMessage.textContent === apiErrorMessage(r, 'Login failed.')` and the element has class `error`
    - **Validates: Requirements 3.3**

  - [ ] 4.4 Update the registration form handler to use `apiFetch`
    - Call `apiFetch('/api/v1/auth/register', { method: 'POST', ... })`
    - On `success: true`: show success message and `setTimeout(() => showAuth('login'), 1200)`
    - On failure: show `apiErrorMessage(result, 'Registration failed.')` with class `error`
    - _Requirements: 3.4, 3.5_

  - [ ] 4.5 Update the logout handler to use `apiFetch`
    - Call `apiFetch('/api/v1/auth/logout', { method: 'POST', credentials: 'include' })`
    - Clear `currentUser`, remove from `localStorage`, return to auth screen
    - _Requirements: 3.6_

- [ ] 5. Wire `apiFetch` into the product listing and filtering flows
  - [ ] 5.1 Update `loadProducts()` to use `apiFetch`
    - Call `apiFetch('/api/v1/products')`, handle `networkError` with "Backend not reachable" empty state in `#productGrid`
    - On non-success: render `apiErrorMessage(result)` in empty state
    - On success: populate `products` array, call `buildCategoryFilters()`, `renderProducts()`, `updateStats()`
    - _Requirements: 4.1, 4.2_

  - [ ]* 5.2 Write property test for product list renders all products (Property 7)
    - **Property 7: Product list renders all products returned by the API**
    - For any array of `N` product objects, mock `GET /api/v1/products`; assert `#productGrid` contains exactly `N` elements with class `product-card`
    - **Validates: Requirements 4.1**

  - [ ]* 5.3 Write property test for category filter completeness (Property 8)
    - **Property 8: Category filter is complete and sound**
    - For any category string `c` and products array `P`, assert the rendered grid shows exactly those products where `category.toLowerCase() === c.toLowerCase()`
    - **Validates: Requirements 4.3**

  - [ ]* 5.4 Write property test for search filter completeness (Property 9)
    - **Property 9: Search filter is complete and sound across all three fields**
    - For any search term `t` and products array `P`, assert exactly those products with `t` in `name`, `description`, or `category` (case-insensitive) are rendered
    - **Validates: Requirements 4.4**

  - [ ]* 5.5 Write property test for product card required fields (Property 10)
    - **Property 10: Product card contains all required display fields**
    - For any product object `p`, call `createProductCard(p)`; assert the HTML contains name, description, `₹X.XX` price, stock indicator, category badge, wishlist button, and action buttons
    - **Validates: Requirements 4.5**

- [ ] 6. Wire `apiFetch` into the cart and checkout flows
  - [ ] 6.1 Update `addToCart()` to use `apiFetch`
    - Call `apiFetch('/api/v1/cart', { method: 'POST', credentials: 'include', body: JSON.stringify({productId, quantity: 1}) })`
    - On success: `showToast('Added to cart! 🛒', 'success')` and call `loadCartBadge()`
    - On failure: `showToast(apiErrorMessage(result, 'Unable to add to cart'), 'error')`
    - _Requirements: 5.1, 5.2, 5.3_

  - [ ] 6.2 Update `loadCart()` and `loadCartBadge()` to use `apiFetch`
    - Call `apiFetch('/api/v1/cart', { credentials: 'include' })`
    - Render cart items with per-unit price, quantity, and line total (`priceCents / 100 * quantity`)
    - Display `totalCents / 100` in `#cartSubtotal` and `#cartTotal`
    - _Requirements: 5.4, 5.6_

  - [ ]* 6.3 Write property test for cart total cents-to-rupees conversion (Property 12)
    - **Property 12: Cart total display correctly converts cents to rupees**
    - For any integer `n`, assert `#cartTotal` and `#cartSubtotal` text equals `"₹" + (n / 100).toFixed(2)`
    - **Validates: Requirements 5.6**

  - [ ]* 6.4 Write property test for cart renders all items with correct line totals (Property 13)
    - **Property 13: Cart renders all items with correct per-line totals**
    - For any array of `N` cart items, assert `#cartItems` contains `N` rows each showing `(priceCents / 100 * quantity).toFixed(2)` with `₹` prefix
    - **Validates: Requirements 5.4**

  - [ ] 6.5 Update `removeFromCart()` to use `apiFetch`
    - Call `apiFetch('/api/v1/cart/' + productId, { method: 'DELETE', credentials: 'include' })`
    - On success: toast and re-render cart; on failure: error toast via `apiErrorMessage`
    - _Requirements: 5.5_

  - [ ] 6.6 Update `confirmCheckout()` to use `apiFetch` for both order steps
    - Step 1: `apiFetch('/api/v1/orders', { method: 'POST', credentials: 'include', body: JSON.stringify({}) })`
    - On step 1 failure: `showToast(apiErrorMessage(orderResult, 'Order creation failed'), 'error')` and close modal
    - Step 2: `apiFetch('/api/v1/orders/${orderId}/confirm', { method: 'POST', credentials: 'include' })`
    - On step 2 success: close modal, success toast, reload cart, navigate to orders
    - On step 2 failure: `showToast(apiErrorMessage(confirmResult, 'Payment failed'), 'error')` and close modal
    - _Requirements: 6.1, 6.2, 6.3, 6.4, 6.5_

- [ ] 7. Checkpoint — Ensure cart and checkout tests pass
  - Ensure all tests pass, ask the user if questions arise.

- [ ] 8. Wire `apiFetch` into orders, wishlist, and reviews flows
  - [ ] 8.1 Update `loadOrders()` to use `apiFetch`
    - Call `apiFetch('/api/v1/orders', { credentials: 'include' })`
    - On non-success or empty array: render empty-state with `apiErrorMessage(result)` or "No orders yet" message
    - Render each order with ID, status badge, and `₹(totalAmountCents/100).toFixed(2)`
    - _Requirements: 7.1, 7.2, 7.3_

  - [ ]* 8.2 Write property test for order list rendering (Property 14)
    - **Property 14: Order list renders all orders with correct total conversion**
    - For any array of `N` order objects, assert `#ordersList` contains `N` order cards with correct ID, status badge, and `₹(totalAmountCents/100).toFixed(2)` amount
    - **Validates: Requirements 7.1**

  - [ ] 8.3 Update `toggleWishlist()`, `loadWishlist()`, and `removeWishlist()` to use `apiFetch`
    - `toggleWishlist`: `apiFetch('/api/v1/wishlist', { method: 'POST', credentials: 'include', body: JSON.stringify({productId}) })`
    - `loadWishlist`: handle 401 with "Please log in" empty state; handle empty array; render `N` product cards
    - `removeWishlist`: `apiFetch('/api/v1/wishlist/' + productId, { method: 'DELETE', credentials: 'include' })`
    - _Requirements: 8.1, 8.2, 8.3, 8.4_

  - [ ]* 8.4 Write property test for wishlist renders all saved items (Property 15)
    - **Property 15: Wishlist renders all saved items**
    - For any array of `N` wishlist items, assert `#wishlistGrid` contains exactly `N` product card elements
    - **Validates: Requirements 8.2**

  - [ ] 8.5 Update `loadReviews()` and `submitReview()` to use `apiFetch`
    - `loadReviews`: `apiFetch('/api/v1/reviews/' + productId)`, compute `avg = (sum of ratings / N).toFixed(1)`, render in `#reviewSummary`
    - `submitReview`: guard `selectedRating === 0` with error toast; call `apiFetch('/api/v1/reviews', { method: 'POST', credentials: 'include', ... })`; on success clear form, reset stars, reload reviews
    - _Requirements: 9.1, 9.2, 9.3, 9.4_

  - [ ]* 8.6 Write property test for review summary accuracy (Property 16)
    - **Property 16: Review summary accurately reflects all submitted reviews**
    - For any array of `N` reviews with integer ratings 1–5, assert `#reviewSummary` shows `(sum/N).toFixed(1)` and the count `N`
    - **Validates: Requirements 9.1**

- [ ] 9. Wire `apiFetch` into the seller dashboard and fix the type-coercion bug
  - [ ] 9.1 Update `loadSellerData()` to use `apiFetch` and fix the seller filter
    - Fetch products: `apiFetch('/api/v1/products')`, then filter with `Number(p.sellerId) === Number(currentUser.userId)`
    - Fetch seller orders: `apiFetch('/api/v1/orders/seller', { credentials: 'include' })`
    - Render seller stats, product list, and order list with ship/deliver action buttons per order status
    - On orders failure: render `apiErrorMessage(orderResult, 'Could not load orders')` in `#sellerOrdersList`
    - _Requirements: 10.1, 10.4, 10.5, 10.6_

  - [ ]* 9.2 Write property test for seller product filter type-coercion safety (Property 17)
    - **Property 17: Seller product filter is numerically coercion-safe**
    - For any products array `P` with `sellerId` as number or numeric string, and any `currentUser.userId` as number or numeric string, assert the filter shows exactly those products where `Number(sellerId) === Number(userId)`
    - **Validates: Requirements 10.1**

  - [ ] 9.3 Update `deleteSellerProduct()` to use `apiFetch`
    - Call `apiFetch('/api/v1/products/' + productId, { method: 'DELETE', credentials: 'include' })`
    - On success: toast, reload seller data and global products; on failure: error toast via `apiErrorMessage`
    - _Requirements: 10.3_

  - [ ] 9.4 Update the add-product form handler to use `apiFetch`
    - Call `apiFetch('/api/v1/products', { method: 'POST', credentials: 'include', body: JSON.stringify({...}) })`
    - Convert price rupees → cents: `priceCents: Math.round(Number(priceInput) * 100)`
    - On success: success message, reset form, reload products and seller data; on failure: error message via `apiErrorMessage`
    - _Requirements: 10.2_

  - [ ]* 9.5 Write property test for price rupees-to-cents conversion (Property 18)
    - **Property 18: Price entry is correctly converted to cents on product submission**
    - For any positive number `p`, assert the `priceCents` in the submitted JSON body equals `Math.round(p * 100)`
    - **Validates: Requirements 10.2**

- [ ] 10. Wire `apiFetch` into the admin dashboard and chatbot
  - [ ] 10.1 Update `loadAdminUsers()`, `loadAdminProducts()`, `loadAdminOrders()`, and `adminDeleteProduct()` to use `apiFetch`
    - Each loader: `apiFetch('/api/v1/admin/...', { credentials: 'include' })`
    - On non-success: render `apiErrorMessage(result, 'Failed to load ...')` as content of `#adminContent`
    - `adminDeleteProduct`: on success reload admin products and global products; on failure error toast
    - _Requirements: 11.1, 11.2, 11.3, 11.4, 11.5_

  - [ ] 10.2 Update `sendChat()` to use `apiFetch`
    - Call `apiFetch('/api/v1/chatbot', { method: 'POST', credentials: 'include', body: JSON.stringify({message}) })`
    - Append `result.data?.data?.reply` as bot bubble; on `networkError` append "Connection error. Make sure the backend is running." bot bubble
    - _Requirements: 12.1, 12.2, 12.3_

  - [ ]* 10.3 Write property test for chatbot bot message bubble (Property 19)
    - **Property 19: Chatbot reply is always appended as a bot message bubble**
    - For any non-empty reply string `s`, mock the chatbot API; assert a `.chat-msg.bot` element is appended whose text equals `escapeHtml(s)`
    - **Validates: Requirements 12.2**

- [ ] 11. Implement toast auto-dismiss and wire error toasts everywhere
  - [ ] 11.1 Verify/implement `showToast()` with 3000 ms auto-dismiss and fade-out
    - Confirm the existing `showToast` implementation fades out after 3000 ms with a CSS opacity transition
    - All error call sites must pass `apiErrorMessage(result, fallback)` as the message — do a final sweep of every `showToast(... 'error')` call to ensure none use raw status codes or the old "Backend not connected" string
    - _Requirements: 13.1, 13.2, 13.3, 13.4_

  - [ ]* 11.2 Write property test for error results producing error toasts (Property 11)
    - **Property 11: Error results always produce an error toast via `apiErrorMessage`**
    - For any failed result `r` (`!r.ok`) in cart add, checkout create/confirm, order list, and admin panel flows, assert the toast or inline error shown equals `apiErrorMessage(r, <contextual fallback>)`
    - **Validates: Requirements 5.3, 6.5, 7.3, 11.5, 13.3**

- [ ] 12. Final checkpoint — Ensure all tests pass
  - Ensure all tests pass, ask the user if questions arise.

---

## Notes

- Tasks marked with `*` are optional and can be skipped for faster MVP
- All changes are in `frontend/index.html` — no C++ backend files are touched
- Test helpers must extract pure functions from `index.html` into `tests/helpers.js` (or similar) before tests can `import` them
- Property tests use [fast-check](https://github.com/dubzzz/fast-check) with JSDOM for DOM-dependent properties; pure function properties (1, 4, 12, 17, 18) do not need JSDOM
- Each property test must run a minimum of 100 iterations and be tagged: `Feature: harimart-frontend-backend-integration, Property {N}: {title}`
- Checkpoints ensure incremental validation after logical groups of tasks

---

## Task Dependency Graph

```json
{
  "waves": [
    { "id": 0, "tasks": ["1.1"] },
    { "id": 1, "tasks": ["2.1", "2.6"] },
    { "id": 2, "tasks": ["2.2"] },
    { "id": 3, "tasks": ["2.3", "2.4", "2.5", "2.7"] },
    { "id": 4, "tasks": ["4.1", "4.4", "4.5", "5.1"] },
    { "id": 5, "tasks": ["4.2", "4.3", "5.2", "5.3", "5.4", "5.5"] },
    { "id": 6, "tasks": ["6.1", "6.2", "6.5", "6.6", "8.1", "8.3", "8.5"] },
    { "id": 7, "tasks": ["6.3", "6.4", "8.2", "8.4", "8.6"] },
    { "id": 8, "tasks": ["9.1", "9.3", "9.4", "10.1", "10.2"] },
    { "id": 9, "tasks": ["9.2", "9.5", "10.3", "11.1"] },
    { "id": 10, "tasks": ["11.2"] }
  ]
}
```
