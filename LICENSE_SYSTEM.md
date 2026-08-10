# SGMS License Protection System

## Security Guard Management Platform

---

## 1. Overview

The Security Guard Management System (SGMS) is protected by a mandatory license activation system.
No user can access the application without a valid license key issued by the developer/owner.
The system enforces this check at every application startup.

- **Owner:** Jitendra Kumar
- **Version:** 1.0.0
- **Total Modules:** 30

---

## 2. How It Works

### Startup Flow:

```
User double-clicks .exe
    |
    v
Database initialized
    |
    v
License check in Settings table
    |
    +-- License EXISTS and ACTIVE --> Login Screen --> Application
    |
    +-- License NOT FOUND --> License Activation Dialog shown
                                  |
                                  +-- Valid key entered --> Saved --> Login --> App
                                  |
                                  +-- Invalid key --> Error --> Retry or Exit
```

### Protection Layers:

| Layer | Protection |
|-------|------------|
| 1. Format Check | Keys must be XXXX-XXXX-XXXX-XXXX (hex characters) |
| 2. SHA-256 Hash | Keys validated using cryptographic hash with secret key |
| 3. Machine Binding | Keys can be universal or machine-specific |
| 4. Database Storage | License stored in local SQLite database |
| 5. Startup Enforcement | Every launch checks license before login |
| 6. No Bypass | Without valid license, no module is accessible |

### Key Types:

| Type | Works On | Use Case |
|------|----------|----------|
| Universal Key | Any machine | Standard customers, demos |
| Machine Key | One specific PC | Enterprise, high-security |

---

## 3. License Plans

| Plan | Monthly | Annual | Guards | Users |
|------|---------|--------|--------|-------|
| Starter | Rs. 2,999 | Rs. 29,990 | Up to 50 | 1 |
| Professional | Rs. 7,999 | Rs. 79,990 | Up to 500 | 5 |
| Enterprise | Rs. 19,999 | Rs. 1,99,990 | Unlimited | Unlimited |

---

## 4. Distribution Process

### For Owner:

```
1. Generate keys:    python3 LicenseManager.py generate 100
2. Assign to client: python3 LicenseManager.py assign KEY name email phone
3. Set plan:         python3 LicenseManager.py plan KEY professional
4. Send key via email/WhatsApp to customer
```

### For Customer:

```
1. Download .zip from GitHub Actions (or receive from owner)
2. Extract .zip
3. Double-click SecurityGuardManager.exe
4. License Activation screen appears
5. Enter license key from owner
6. Click Activate
7. Login: admin / admin123
```

---

## 5. License Manager Commands

| Command | Description |
|---------|-------------|
| `generate 100` | Generate 100 new license keys |
| `assign KEY name email phone` | Assign license to customer |
| `plan KEY professional` | Set pricing plan |
| `list` | List all licenses |
| `list assigned` | List sold licenses |
| `list unassigned` | List stock licenses |
| `search rajesh` | Search by name/email/key |
| `info KEY` | Show full license details |
| `deactivate KEY` | Block a license |
| `activate KEY` | Re-activate a license |
| `stats` | Show revenue and statistics |
| `export` | Export all records to CSV |

---

## 6. Security

### What is Public:
- Source code on GitHub (for collaboration)
- Anyone can VIEW and BUILD the code

### What is Protected:
- License keys are NOT in source code
- Secret key is compiled into .exe binary
- Only the owner can generate valid keys
- License database is local to each PC

### Cannot Be Bypassed By:
- Deleting database file (app recreates it, asks for license again)
- Copying .exe to another PC (new PC has no license)
- Entering random keys (SHA-256 validation rejects invalid keys)

### Can Be Bypassed By:
- Someone who reads source code + compiles own version (same as all software)
- Solution: Make GitHub repo PRIVATE so source code is hidden

---

## 7. Making Repo Private (Recommended)

```
1. Go to: https://github.com/jitenkr2030/C-Security-management/settings
2. Scroll to 'Danger Zone'
3. Click 'Change visibility'
4. Select 'Make private'
5. Confirm

Now nobody can see your source code or secret key.
GitHub Actions still builds .exe even for private repos.
Only you can download the .exe from Actions tab.
```

---

## 8. Customer Support

| Issue | Solution |
|-------|----------|
| Key not working | `info KEY` to verify |
| Changed PC | Generate new key, assign to customer |
| Want upgrade | `plan KEY enterprise` |
| Stop paying | `deactivate KEY` |
| Refund | `deactivate KEY` |

---

## 9. Workflow for 100 Customers

```
Step 1: python3 LicenseManager.py generate 100
Step 2: Customer purchases -> assign KEY name email phone
Step 3: Set plan -> plan KEY professional
Step 4: Email/WhatsApp key to customer
Step 5: Track -> python3 LicenseManager.py stats
Step 6: If stops paying -> deactivate KEY
Step 7: Export for accounting -> export
```

---

All rights reserved. Unauthorized use is prohibited.
