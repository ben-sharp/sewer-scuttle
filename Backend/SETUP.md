# SewerScuttle Backend Setup

## Overview
This Laravel 12 backend provides a RESTful API for the SewerScuttle game with an admin panel powered by Filament.

## What's Been Implemented

### ✅ Core Features

1. **Authentication System**
   - Email/password registration and login
   - Social login (Google OAuth, Facebook OAuth)
   - Device-based anonymous authentication
   - Laravel Sanctum API tokens
   - Token refresh mechanism

2. **Player Management**
   - Player profiles (username, display name, stats)
   - Player statistics (total coins, distance, runs, best scores)
   - Player class selection
   - Currency management (multiple currency types)
   - Player customizations (JSON storage)

3. **Leaderboard System**
   - Score-based leaderboards
   - Filter by player class
   - Time-based leaderboards (daily, weekly, all-time)
   - Player rank lookup
   - Score submission with validation

4. **Store System**
   - Store items (cosmetics, class unlocks, etc.)
   - Item metadata (JSON)
   - Purchase system with currency validation
   - Purchase history
   - Active/inactive item management

5. **Run Statistics**
   - Track run start/end
   - Store run data (score, distance, time, coins, obstacles, powerups)
   - Run history per player
   - Run completion tracking

6. **Admin Panel (Filament)**
   - Dashboard with key metrics widget
   - Admin authentication configured
   - Resources can be created for all models

## API Endpoints

All API endpoints are prefixed with `/api`

### Authentication
- `POST /api/auth/register` - Register with email/password
- `POST /api/auth/login` - Login with email/password
- `POST /api/auth/logout` - Logout (revoke token)
- `POST /api/auth/refresh` - Refresh token
- `GET /api/auth/user` - Get current user
- `POST /api/auth/social/{provider}` - Social login (google, facebook)
- `POST /api/auth/device` - Device-based auth

### Player Data
- `GET /api/player/profile` - Get profile
- `PUT /api/player/profile` - Update profile
- `GET /api/player/currency` - Get currencies
- `POST /api/player/currency/add` - Add currency (admin only)
- `GET /api/player/customizations` - Get customizations
- `PUT /api/player/customizations` - Update customizations

### Leaderboard
- `GET /api/leaderboard` - Get leaderboard (query: top, class, timeframe)
- `POST /api/leaderboard/submit` - Submit score
- `GET /api/leaderboard/player/{id}` - Get player rank

### Store
- `GET /api/store/items` - List items
- `GET /api/store/items/{id}` - Get item details
- `POST /api/store/purchase` - Purchase item
- `GET /api/store/purchases` - Get purchase history

### Run Stats
- `POST /api/runs/start` - Start run
- `POST /api/runs/end` - End run and save stats
- `GET /api/runs/history` - Get run history
- `GET /api/runs/{id}` - Get run details

## Database Schema

### Tables Created
- `users` - Extended with device_id, google_id, facebook_id, banned_at, is_admin
- `players` - Player profiles and stats
- `player_currencies` - Multi-currency support
- `leaderboard_entries` - Leaderboard scores
- `store_items` - Store inventory
- `purchases` - Purchase history
- `runs` - Run statistics

## Setup Instructions

### 1. Environment Configuration

Add to your `.env` file:

```env
# Sanctum Configuration
SANCTUM_STATEFUL_DOMAINS=localhost,127.0.0.1,127.0.0.1:8000

# Social OAuth (optional)
GOOGLE_CLIENT_ID=your_google_client_id
GOOGLE_CLIENT_SECRET=your_google_client_secret
GOOGLE_REDIRECT_URI=/api/auth/social/google/callback

FACEBOOK_CLIENT_ID=your_facebook_client_id
FACEBOOK_CLIENT_SECRET=your_facebook_client_secret
FACEBOOK_REDIRECT_URI=/api/auth/social/facebook/callback
```

### 2. Create Admin User

To create an admin user, you can use tinker:

```bash
php artisan tinker
```

```php
$user = \App\Models\User::create([
    'name' => 'Admin',
    'email' => 'admin@example.com',
    'password' => \Illuminate\Support\Facades\Hash::make('password'),
    'is_admin' => true,
]);

$player = \App\Models\Player::create([
    'user_id' => $user->id,
    'username' => 'admin',
    'display_name' => 'Admin',
]);
```

### 3. Access Admin Panel

Navigate to `/admin` and login with your admin credentials.

### 4. Create Filament Resources (Optional)

To create Filament resources for managing models in the admin panel, run:

```bash
php artisan make:filament-resource User
php artisan make:filament-resource Player
php artisan make:filament-resource StoreItem
php artisan make:filament-resource Purchase
php artisan make:filament-resource LeaderboardEntry
php artisan make:filament-resource Run
```

When prompted for "title attribute", use:
- User: `name` or `email`
- Player: `username` or `display_name`
- StoreItem: `name`
- Purchase: leave blank or use `id`
- LeaderboardEntry: leave blank or use `id`
- Run: leave blank or use `id`

## API Authentication

All protected endpoints require a Bearer token in the Authorization header:

```
Authorization: Bearer {token}
```

Tokens are obtained from:
- `/api/auth/register` - Returns token on registration
- `/api/auth/login` - Returns token on login
- `/api/auth/social/{provider}` - Returns token on social login
- `/api/auth/device` - Returns token on device auth

## Security Features

- Rate limiting (configured via Laravel middleware)
- CORS support (configure in `config/cors.php` if needed)
- Input validation on all endpoints
- Token expiration (configure in `config/sanctum.php`)
- Admin-only endpoints protected by middleware

## Next Steps

1. Configure OAuth credentials in `.env` for social login
2. Set up CORS for your game clients (PC and Android)
3. Create Filament resources for admin management
4. Add rate limiting rules as needed
5. Configure token expiration settings
6. Set up production database (MySQL)

## Notes

- The admin panel requires the `intl` PHP extension. If you encounter issues, install it:
  - Ubuntu/Debian: `sudo apt-get install php-intl`
  - Or use `--ignore-platform-req=ext-intl` when installing Filament (already done)
- All migrations have been run
- Sanctum is configured and ready to use
- Socialite is configured for Google and Facebook OAuth



