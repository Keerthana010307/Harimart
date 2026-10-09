# Design Document

## HariMart Frontend–Backend Integration Fix

---

## Overview

The fix resolves a cross-origin session cookie mismatch caused by a hardcoded absolute API base URL (`http://127.0.0.1:8080`). Because the Drogon backend serves the frontend directly on port 8080 under the `localhost` origin, the browser treats `127.0.0.1` and `localhost` as distinct origins and silently discards the session cookie on every subsequent authenticated request. Every call after login therefore fails with HTTP 401.

The solution is confined entirely to `frontend/index.html` — no C++ backend changes are needed. It has three parts:

1. Set `const API = ""` so all fetch URLs are relative (same-origin).
2. Centralise all `fetch()` calls behind `apiFetch()` and `apiErrorMessage()` helpers so credentials are always attached, errors are always logged, and users always receive human-readable feedback.
3. Fix the seller dashboard's strict-equality type-coercion bug where `sellerId` (API returns a number) was compared with `===` against `currentUser.userId` (stored as whatever JSON.parse gives, which can be a number or string depending on context), causing zero seller products to ever be visible.

---

## Architecture

### Component Diagram

```
┌─────────────────────────────────────────────────────────┐
│  Browser (same origin: localhost:8080)                  │
│                                                         │
│  ┌─────────────────────────────────────────────────┐   │
│  │  frontend/index.html                            │   │
│  │                                                 │   │
│  │  ┌────────────────┐   ┌──────────────────────┐  │   │
│  │  │  UI Layer      │   │  API Layer           │  │   │
│  │  │                │   │                      │  │   │
│  │  │  Auth Forms    │──▶│  apiFetch(url, opts) │  │   │
│  │  │  Product Grid  │   │  apiErrorMessage()   │  │   │
│  │  │  Cart View     │   └──────────┬───────────┘  │   │
│  │  │  Orders View   │              │               │   │
│  │  │  Wishlist View │              │               │   │
│  │  │  Seller Dash   │              │               │   │
│  │  │  Admin Dash    │              │               │   │
│  │  │  Chatbot Panel │              │               │   │
│  │  └────────────────┘              │               │   │
│  └──────────────────────────────────┼───────────────┘   │
└─────────────────────────────────────┼───────────────────┘
                                      │ relative URL
                                      │ credentials: 'include'
                          ┌───────────▼─────────────────┐
                          │  Drogon C++ Backend          │
                          │  localhost:8080              │
                          │                              │
                          │  /api/v1/auth/*              │
                          │  /api/v1/products/*          │
                          │  /api/v1/cart/*              │
                          │  /api/v1/orders/*            │
                          │  /api/v1/wishlist/*          │
                          │  /api/v1/reviews/*           │
                          │  /api/v1/admin/*             │
                          │  /api/v1/chatbot             │
                          └─────────────────────────────┘
```

### Data Flow for an Authenticated Request (After Fix)

```
User Action
    │
    ▼
UI Handler (e.g., addToCart)
    │
    ▼
apiFetch("/api/v1/cart", { method: "POST", credentials: "include", body: ... })
    │
    │  const fullUrl = "" + "/api/v1/cart"  → "/api/v1/cart"  (relative)
    │
    ▼
fetch("/api/v1/cart", { credentials: "include", ... })
    │
    │  Browser sends request to SAME origin (localhost:8080)
    │  Session cookie IS included ✓
    │
    ▼
Drogon Backend validates session cookie
    │
    ▼
JSON Response { success: true, data: {...}, error: null }
    │
    ▼
apiFetch returns { ok: true, status: 201, data: {...} }
    │
    ▼
UI Handler renders success state / toast
```

---

## Components

### 1. API Constants

**Location:** JavaScript globals at the top of the `<script>` block.

```javascript
const API = "";   // Empty string → relative URLs → same-origin requests
```

The previous value was `"http://127.0.0.1:8080"`, which broke session cookies when the page was loaded from `localhost:8080`.

---

### 2. `apiFetch(url, options)` — Centralised Fetch Helper

**Signature:**
```javascript
async function apiFetch(url, options = {})
  → Promise<{ ok: boolean, status: number, data: object|null, networkError?: true }>
```

**Responsibilities:**
- Prepend `API` to the URL: `const fullUrl = API + url`
- Delegate to native `fetch(fullUrl, options)`
- On network failure (catch block): return `{ ok: false, status: 0, data: null, networkError: true }`
- On any HTTP response: attempt `res.json()`, attach result as `data` (null if parse fails)
- On non-2xx response: call `console.error(method, fullUrl, status, errorMessage, data)`
- Return `{ ok: res.ok, status: res.status, data }`

**Return shape:**
```javascript
// Success
{ ok: true,  status: 200, data: { success: true, data: {...}, error: null } }

// HTTP error
{ ok: false, status: 401, data: { success: false, error: { message: "Login required" } } }

// Network error
{ ok: false, status: 0,   data: null, networkError: true }
```

---

### 3. `apiErrorMessage(result, fallback)` — Error Message Mapper

**Signature:**
```javascript
function apiErrorMessage(result, fallback = "An error occurred") → string
```

**Mapping table (pure function, no side effects):**

| Condition | Returned string |
|---|---|
| `result.networkError === true` | `"Cannot reach the server — make sure HariMart is running on port 8080"` |
| `result.status === 401` | `result.data?.error?.message` or `"Please log in to continue"` |
| `result.status === 403` | `result.data?.error?.message` or `"You do not have permission for this action"` |
| `result.status === 404` | `result.data?.error?.message` or `"Resource not found"` |
| `result.status === 400` | `result.data?.error?.message` or `"Invalid request"` |
| `result.status >= 500` | `"Server error — please try again later"` |
| all other cases | `result.data?.error?.message` or `fallback` |

---

### 4. UI Modules and Their API Calls

Each UI module calls `apiFetch()` with `credentials: 'include'` for all authenticated endpoints.

#### 4.1 Auth Module

| Action | Method | Endpoint | Auth required |
|---|---|---|---|
| Login | POST | `/api/v1/auth/login` | No |
| Register | POST | `/api/v1/auth/register` | No |
| Logout | POST | `/api/v1/auth/logout` | Session |

On successful login, the backend sets an HTTP-only session cookie at the `localhost:8080` origin. All subsequent requests use the same origin, so the cookie is automatically included.

User object stored in `currentUser`:
```javascript
{
  email: string,
  role: "BUYER" | "SELLER" | "ADMIN",
  name: string,
  userId: number   // backend returns Int64 → JS number
}
```

#### 4.2 Product Module

| Action | Method | Endpoint | Auth required |
|---|---|---|---|
| List all | GET | `/api/v1/products` | No |
| Add (seller) | POST | `/api/v1/products` | SELLER session |
| Delete (seller) | DELETE | `/api/v1/products/{id}` | SELLER session |

Client-side filtering is performed in memory against the `products` array. No additional API call is made for category filters or search.

Price encoding: prices are stored as integer cents (`priceCents`). Display conversion: `Number(priceCents) / 100`. Submission conversion: `Math.round(priceRupees * 100)`.

#### 4.3 Cart Module

| Action | Method | Endpoint | Auth required |
|---|---|---|---|
| Add item | POST | `/api/v1/cart` | Session |
| Get cart | GET | `/api/v1/cart` | Session |
| Remove item | DELETE | `/api/v1/cart/{productId}` | Session |

The `totalCents` field from `GET /api/v1/cart` is converted to rupees for display: `Number(totalCents) / 100`.

#### 4.4 Order Module

| Action | Method | Endpoint | Auth required |
|---|---|---|---|
| Create order | POST | `/api/v1/orders` | Session |
| Confirm order | POST | `/api/v1/orders/{id}/confirm` | Session |
| List buyer orders | GET | `/api/v1/orders` | Session |
| List seller orders | GET | `/api/v1/orders/seller` | SELLER session |
| Update status | PUT | `/api/v1/orders/{id}/status` | SELLER session |

Checkout is a two-step sequential flow: create → confirm. Both steps must succeed; failure at either step closes the modal and shows an error toast.

#### 4.5 Wishlist Module

| Action | Method | Endpoint | Auth required |
|---|---|---|---|
| Toggle add | POST | `/api/v1/wishlist` | Session |
| List items | GET | `/api/v1/wishlist` | Session |
| Remove item | DELETE | `/api/v1/wishlist/{productId}` | Session |

A 401 response on GET `/api/v1/wishlist` is surfaced as a "Please log in" empty state rather than a generic error.

#### 4.6 Review Module

| Action | Method | Endpoint | Auth required |
|---|---|---|---|
| List reviews | GET | `/api/v1/reviews/{productId}` | No |
| Submit review | POST | `/api/v1/reviews` | Session |

Client-side guard: if `selectedRating === 0`, submission is blocked with an error toast before any API call.

#### 4.7 Seller Dashboard Module

| Action | Method | Endpoint | Auth required |
|---|---|---|---|
| Get products | GET | `/api/v1/products` | No |
| Add product | POST | `/api/v1/products` | SELLER session |
| Delete product | DELETE | `/api/v1/products/{id}` | SELLER session |
| Get seller orders | GET | `/api/v1/orders/seller` | SELLER session |
| Update order status | PUT | `/api/v1/orders/{id}/status` | SELLER session |

**Type-coercion fix:** The seller filter changed from strict equality to numeric coercion:

```javascript
// Before (broken): API returns sellerId as number, currentUser.userId may be stored as number or string
const myProducts = allProducts.filter(p => p.sellerId === currentUser.userId);

// After (fixed): coerce both sides to Number before comparing
const myProducts = allProducts.filter(p => Number(p.sellerId) === Number(currentUser.userId));
```

#### 4.8 Admin Dashboard Module

| Action | Method | Endpoint | Auth required |
|---|---|---|---|
| List users | GET | `/api/v1/admin/users` | ADMIN session |
| List products | GET | `/api/v1/admin/products` | ADMIN session |
| Delete product | DELETE | `/api/v1/admin/products/{id}` | ADMIN session |
| List orders | GET | `/api/v1/admin/orders` | ADMIN session |

#### 4.9 Chatbot Module

| Action | Method | Endpoint | Auth required |
|---|---|---|---|
| Send message | POST | `/api/v1/chatbot` | Session |

Network errors display a hardcoded "Connection error. Make sure the backend is running." bot bubble rather than using `apiErrorMessage`, because the chatbot UX expects a conversational response rather than a structured error.

---

### 5. Toast Notification System

```javascript
function showToast(msg, type = '')
// type: '' | 'success' | 'error'
// Auto-dismisses after 3000ms with fade-out
```

All success and error feedback routes through `showToast`. Error text always comes from `apiErrorMessage(result, fallbackString)`.

---

## Data Models

### API Response Envelope (all endpoints)

```javascript
{
  success: boolean,
  data: object | null,
  error: { message: string } | null
}
```

### Product

```javascript
{
  id: number,
  name: string,
  description: string,
  priceCents: number,   // integer, e.g. 149900 = ₹1499.00
  stock: number,
  category: string,
  imageUrl: string | null,
  sellerId: number
}
```

### Cart Item

```javascript
{
  id: number,
  userId: number,
  productId: number,
  productName: string,
  quantity: number,
  priceCents: number
}
```

### Order

```javascript
{
  id: number,
  buyerId: number,
  status: "PENDING" | "CONFIRMED" | "SHIPPED" | "DELIVERED" | "CANCELLED",
  totalAmountCents: number
}
```

### User (session)

```javascript
{
  email: string,
  role: "BUYER" | "SELLER" | "ADMIN",
  name: string,
  userId: number
}
```

---

## Error Handling

### Error Propagation Strategy

```
API call site
    │
    ├─ result.networkError → showToast(apiErrorMessage(result), 'error')
    │                      OR render empty state with "Backend not reachable"
    │
    ├─ !result.ok          → showToast(apiErrorMessage(result, fallback), 'error')
    │                      OR render inline error message
    │
    └─ result.ok           → proceed with result.data
```

### Specific Error Handling Patterns

| Scenario | User-facing output |
|---|---|
| Network error on product load | Empty state: "Backend not reachable — make sure HariMart.exe is running on port 8080" |
| 401 on any auth-required endpoint | Toast: "Please log in to continue" (or server message) |
| 403 on admin/seller endpoint | Toast: "You do not have permission for this action" |
| 400 on form submission | Toast: server's `error.message` or "Invalid request" |
| 500 from backend | Toast: "Server error — please try again later" |
| Network error on chatbot | Bot bubble: "Connection error. Make sure the backend is running." |
| Review submit with no rating | Toast: "Please select a star rating" (client-side, no API call) |
| Checkout with empty cart | Toast: "Cart is empty" (client-side, no API call) |

---

## Correctness Properties

*A property is a characteristic or behavior that should hold true across all valid executions of a system — essentially, a formal statement about what the system should do. Properties serve as the bridge between human-readable specifications and machine-verifiable correctness guarantees.*

**Property Reflection:** Before listing properties, redundant ones were eliminated. The error-toast properties from Requirements 5.3, 6.5, 7.3, 11.5, and 13.3 all reduce to the same claim about `apiErrorMessage` output appearing in the UI, so they are consolidated under a single property. Similarly, the render-all-items properties for cart (5.4), orders (7.1), wishlist (8.2), admin users (11.1), admin products (11.2), and admin orders (11.4) share the same structural claim (N items in → N rendered items out) and are each kept distinct only because they test different data shapes.

---

### Property 1: URL construction never produces an absolute URL

*For any* path string `p` passed to `apiFetch`, the URL forwarded to the native `fetch` call equals `"" + p` (i.e., `p` itself), which contains no scheme (`http://` or `https://`), no hostname, and no port number.

**Validates: Requirements 1.2, 2.2**

---

### Property 2: JSON response body is always attached as `data`

*For any* JSON-serializable object `O`, when the mocked `fetch` resolves with a response whose body is `JSON.stringify(O)`, `apiFetch` returns a result object where `result.data` deep-equals `O`.

**Validates: Requirements 2.4**

---

### Property 3: Non-2xx responses always trigger a console error

*For any* HTTP status code `s` where `s < 200` or `s > 299`, and *for any* request path and method, when `fetch` resolves with status `s`, `apiFetch` calls `console.error` with an argument list containing the method, the full URL, `s`, and an error description string.

**Validates: Requirements 2.5**

---

### Property 4: `apiErrorMessage` is a total pure function with correct status mapping

*For any* result object `r` and *for any* fallback string `f`, `apiErrorMessage(r, f)` returns a non-empty string according to the mapping table defined in Requirement 2.6. Specifically:
- If `r.networkError === true`, the returned string contains `"port 8080"`.
- If `r.status === 401`, the returned string contains either the server's `error.message` or `"Please log in to continue"`.
- If `r.status === 403`, the returned string contains either the server's `error.message` or `"You do not have permission"`.
- If `r.status === 404`, the returned string contains either the server's `error.message` or `"Resource not found"`.
- If `r.status === 400`, the returned string contains either the server's `error.message` or `"Invalid request"`.
- If `r.status >= 500`, the returned string contains `"Server error"`.
- In all other cases, the returned string is the server's `error.message` if present, otherwise `f`.
- The function never returns `null`, `undefined`, or an empty string.

**Validates: Requirements 2.6, 13.1**

---

### Property 5: Successful login stores any valid user object and enters the app

*For any* user object `u` with fields `{ email, role, name, userId }` returned from `POST /api/v1/auth/login` with `success: true`, calling the login handler stores `u` in `currentUser`, hides `#authScreen`, and shows `#mainApp`.

**Validates: Requirements 3.2**

---

### Property 6: Login failure always surfaces `apiErrorMessage` output

*For any* error result `r` returned from the login API (i.e., `r.ok === false` or `r.data.success === false`), the text content of `#loginMessage` equals `apiErrorMessage(r, 'Login failed.')` and the element has CSS class `error`.

**Validates: Requirements 3.3**

---

### Property 7: Product list renders all products returned by the API

*For any* non-empty array of `N` product objects returned by `GET /api/v1/products`, the `#productGrid` DOM element contains exactly `N` elements with class `product-card`.

**Validates: Requirements 4.1**

---

### Property 8: Category filter is complete and sound

*For any* category string `c` and *for any* products array `P`, clicking the filter for `c` renders exactly the subset of `P` where `product.category.toLowerCase() === c.toLowerCase()` — no more, no fewer.

**Validates: Requirements 4.3**

---

### Property 9: Search filter is complete and sound across all three fields

*For any* search term `t` and *for any* products array `P`, the rendered product grid contains exactly those products in `P` where at least one of `name`, `description`, or `category` contains `t` as a case-insensitive substring.

**Validates: Requirements 4.4**

---

### Property 10: Product card contains all required display fields

*For any* product object `p` with fields `{ name, description, priceCents, stock, category }`, `createProductCard(p)` returns an HTML string that contains:
- the product name,
- the description,
- the price formatted as `₹X.XX` where `X.XX = (priceCents / 100).toFixed(2)`,
- a stock status indicator,
- the category badge text,
- a wishlist toggle button,
- action buttons for Add to Cart, Reviews, and Buy.

**Validates: Requirements 4.5**

---

### Property 11: Error results always produce an error toast via `apiErrorMessage`

*For any* failed API result `r` (i.e., `!r.ok`) in the cart add, checkout create, checkout confirm, order list, and admin panel flows, the toast or inline error displayed to the user equals `apiErrorMessage(r, <contextual fallback>)`.

**Validates: Requirements 5.3, 6.5, 7.3, 11.5, 13.3**

---

### Property 12: Cart total display correctly converts cents to rupees

*For any* integer `totalCents` value `n` returned by `GET /api/v1/cart`, the text displayed in `#cartTotal` and `#cartSubtotal` equals `"₹" + (n / 100).toFixed(2)`.

**Validates: Requirements 5.6**

---

### Property 13: Cart renders all items with correct per-line totals

*For any* cart items array of `N` items, each with `priceCents` and `quantity`, the `#cartItems` DOM element contains `N` cart item rows, and each row displays a line total equal to `(item.priceCents / 100 * item.quantity).toFixed(2)` formatted with `₹` prefix.

**Validates: Requirements 5.4**

---

### Property 14: Order list renders all orders with correct total conversion

*For any* array of `N` order objects, each with `id`, `status`, and `totalAmountCents`, the `#ordersList` DOM element contains `N` order cards each showing the order ID, a status badge, and the amount as `"₹" + (totalAmountCents / 100).toFixed(2)`.

**Validates: Requirements 7.1**

---

### Property 15: Wishlist renders all saved items

*For any* wishlist items array of `N` items returned by `GET /api/v1/wishlist`, the `#wishlistGrid` DOM element contains exactly `N` product card elements.

**Validates: Requirements 8.2**

---

### Property 16: Review summary accurately reflects all submitted reviews

*For any* array of `N` review objects each with an integer `rating` field (1–5), the `#reviewSummary` element displays an average equal to `(sum of ratings / N).toFixed(1)` and a count of `N`.

**Validates: Requirements 9.1**

---

### Property 17: Seller product filter is numerically coercion-safe

*For any* products array `P` (where `sellerId` may be stored as a number or as a numeric string) and *for any* `currentUser.userId` value (number or numeric string), the seller dashboard displays exactly those products where `Number(product.sellerId) === Number(currentUser.userId)`.

**Validates: Requirements 10.1**

---

### Property 18: Price entry is correctly converted to cents on product submission

*For any* valid price value `p` (a positive number) entered in the seller add-product form, the `priceCents` field in the submitted JSON body equals `Math.round(p * 100)`.

**Validates: Requirements 10.2**

---

### Property 19: Chatbot reply is always appended as a bot message bubble

*For any* non-empty reply string `s` in `result.data.data.reply`, calling `sendChat()` appends a DOM element with class `chat-msg bot` whose text content equals `escapeHtml(s)`.

**Validates: Requirements 12.2**

---

## Testing Strategy

### Dual Approach

- **Unit / example-based tests** cover specific interactions: form submissions, button clicks, empty states, and single-outcome edge cases (no-rating guard, empty cart guard, 401 wishlist handling, toast auto-dismiss timing).
- **Property-based tests** cover universal invariants: URL construction, `apiErrorMessage` mapping completeness, filter correctness, render completeness, cents-to-rupee conversion, and the seller type-coercion fix.

### Property Test Configuration

All property tests must run a minimum of 100 iterations. Each test must be tagged:

```
Feature: harimart-frontend-backend-integration, Property {N}: {property_title}
```

### Recommended Framework

Since HariMart's frontend is Vanilla JavaScript with no build system, property-based tests can be written using [fast-check](https://github.com/dubzzz/fast-check) in a lightweight Node.js test harness with JSDOM for DOM-dependent properties. Pure function properties (Properties 1, 4, 12, 17, 18) do not require JSDOM at all.

```javascript
// Example: Property 4 — apiErrorMessage mapping
import fc from "fast-check";
import { apiErrorMessage } from "./index-helpers.js"; // extracted pure functions

fc.assert(
  fc.property(fc.string(), (fallback) => {
    const result = { ok: false, status: 401, data: { error: { message: "Custom" } } };
    return apiErrorMessage(result, fallback) === "Custom";
  }),
  { numRuns: 100 }
);
```

### Unit Test Focus Areas

- Login/logout/register form submission paths (credential attachment, state transitions)
- Checkout two-step flow sequencing (create then confirm, failure at each step)
- Review submission guard (zero-rating block)
- Empty cart guard
- Toast auto-dismiss timing (3000 ms + fade)
- Admin tab switching (correct API endpoint called per tab)
- Network error empty states (product grid, admin panel)
