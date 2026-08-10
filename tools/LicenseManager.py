#!/usr/bin/env python3
"""
SGMS License Manager
Manage all license keys from your Termux server.

Usage:
    python3 LicenseManager.py generate [count]     # Generate new licenses
    python3 LicenseManager.py assign <key> <name> <email> <phone>  # Assign to customer
    python3 LicenseManager.py list                  # List all licenses
    python3 LicenseManager.py list assigned         # List assigned licenses
    python3 LicenseManager.py list unassigned       # List unassigned licenses
    python3 LicenseManager.py list active           # List active licenses
    python3 LicenseManager.py search <query>        # Search by name, email, or key
    python3 LicenseManager.py deactivate <key>      # Deactivate a license
    python3 LicenseManager.py activate <key>        # Re-activate a license
    python3 LicenseManager.py info <key>            # Show license details
    python3 LicenseManager.py stats                 # Show summary statistics
    python3 LicenseManager.py export                # Export to CSV
    python3 LicenseManager.py plan <key> <plan>     # Set plan (starter/pro/enterprise)
"""

import hashlib
import random
import sqlite3
import sys
import os
from datetime import datetime, timedelta

SECRET_KEY = "SGMS2025SECURITY@KEY"
DB_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), "licenses.db")

PLANS = {
    "starter":    {"price": 2999,  "desc": "Up to 50 guards"},
    "professional": {"price": 7999,  "desc": "Up to 500 guards"},
    "enterprise": {"price": 19999, "desc": "Unlimited guards"},
}

# ─── Database ─────────────────────────────────────────────────

def init_db():
    conn = sqlite3.connect(DB_PATH)
    c = conn.cursor()
    c.execute("""CREATE TABLE IF NOT EXISTS licenses (
        id          INTEGER PRIMARY KEY AUTOINCREMENT,
        license_key TEXT UNIQUE NOT NULL,
        plan        TEXT DEFAULT 'starter',
        customer_name   TEXT DEFAULT '',
        customer_email  TEXT DEFAULT '',
        customer_phone  TEXT DEFAULT '',
        company     TEXT DEFAULT '',
        status      TEXT DEFAULT 'unassigned',
        machine_id  TEXT DEFAULT '',
        created_at  TEXT NOT NULL,
        assigned_at TEXT DEFAULT '',
        activated_at TEXT DEFAULT '',
        expiry_date TEXT DEFAULT '',
        notes       TEXT DEFAULT ''
    )""")
    conn.commit()
    return conn

# ─── License Generation ───────────────────────────────────────

def make_checksum(key_part, machine_id):
    combined = (key_part.upper() + machine_id + SECRET_KEY).encode("utf-8")
    h = hashlib.sha256(combined).hexdigest().upper()
    return h[:8]

def generate_key_part():
    chars = "0123456789ABCDEF"
    return "".join(random.choice(chars) for _ in range(8))

def format_key(key_part, checksum):
    full = key_part + checksum
    return f"{full[0:4]}-{full[4:8]}-{full[8:12]}-{full[12:16]}"

def generate_license():
    key_part = generate_key_part()
    checksum = make_checksum(key_part, "UNIVERSAL")
    return format_key(key_part, checksum)

# ─── Commands ─────────────────────────────────────────────────

def cmd_generate(conn, count=1):
    c = conn.cursor()
    now = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    generated = []
    for i in range(count):
        while True:
            key = generate_license()
            try:
                c.execute("INSERT INTO licenses (license_key, created_at) VALUES (?, ?)",
                          (key, now))
                generated.append(key)
                break
            except sqlite3.IntegrityError:
                continue  # duplicate key, regenerate
    conn.commit()
    print(f"\n  Generated {count} license key(s):\n")
    for i, key in enumerate(generated, 1):
        print(f"    {i:3d}. {key}")
    print()

def cmd_assign(conn, key, name, email, phone):
    c = conn.cursor()
    c.execute("SELECT id, status FROM licenses WHERE license_key = ?", (key.upper(),))
    row = c.fetchone()
    if not row:
        print(f"\n  ERROR: License key '{key}' not found.\n")
        return
    if row[1] == "deactivated":
        print(f"\n  ERROR: License '{key}' is deactivated. Re-activate first.\n")
        return

    now = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    expiry = (datetime.now() + timedelta(days=365)).strftime("%Y-%m-%d")

    company = input("  Company name (optional): ").strip()

    c.execute("""UPDATE licenses SET customer_name=?, customer_email=?, customer_phone=?,
                 company=?, status='assigned', assigned_at=?, expiry_date=?
                 WHERE license_key=?""",
              (name, email, phone, company, now, expiry, key.upper()))
    conn.commit()
    print(f"\n  License {key.upper()} assigned to {name} ({email})")
    print(f"  Plan: {get_plan(c, key.upper())} | Expires: {expiry}\n")

def cmd_list(conn, filter_type="all"):
    c = conn.cursor()
    if filter_type == "assigned":
        c.execute("SELECT * FROM licenses WHERE status = 'assigned' ORDER BY assigned_at DESC")
    elif filter_type == "unassigned":
        c.execute("SELECT * FROM licenses WHERE status = 'unassigned' ORDER BY created_at DESC")
    elif filter_type == "active":
        c.execute("SELECT * FROM licenses WHERE status IN ('assigned','activated') ORDER BY assigned_at DESC")
    elif filter_type == "deactivated":
        c.execute("SELECT * FROM licenses WHERE status = 'deactivated' ORDER BY created_at DESC")
    else:
        c.execute("SELECT * FROM licenses ORDER BY id DESC")

    rows = c.fetchall()
    if not rows:
        print(f"\n  No licenses found ({filter_type}).\n")
        return

    print(f"\n  {'KEY':<20} {'STATUS':<14} {'PLAN':<14} {'CUSTOMER':<25} {'EMAIL':<30} {'EXPIRY':<12}")
    print(f"  {'─'*20} {'─'*14} {'─'*14} {'─'*25} {'─'*30} {'─'*12}")
    for row in rows:
        key = row[1]
        plan = row[2]
        name = row[3] or "—"
        email = row[4] or "—"
        status = row[7]
        expiry = row[11] or "—"
        status_icon = {"unassigned": "⬜", "assigned": "🟨", "activated": "🟩", "deactivated": "🟥"}.get(status, "❓")
        print(f"  {key:<20} {status_icon} {status:<11} {plan:<14} {name:<25} {email:<30} {expiry:<12}")

    print(f"\n  Total: {len(rows)} license(s)\n")

def cmd_search(conn, query):
    c = conn.cursor()
    q = f"%{query}%"
    c.execute("""SELECT * FROM licenses WHERE
                 license_key LIKE ? OR customer_name LIKE ? OR
                 customer_email LIKE ? OR company LIKE ? ORDER BY id DESC""",
              (q, q, q, q))
    rows = c.fetchall()
    if not rows:
        print(f"\n  No results for '{query}'.\n")
        return

    print(f"\n  Search results for '{query}':\n")
    for row in rows:
        print(f"    Key:      {row[1]}")
        print(f"    Plan:     {row[2]}")
        print(f"    Customer: {row[3] or '—'}")
        print(f"    Email:    {row[4] or '—'}")
        print(f"    Phone:    {row[5] or '—'}")
        print(f"    Company:  {row[6] or '—'}")
        print(f"    Status:   {row[7]}")
        print(f"    Expiry:   {row[11] or '—'}")
        print(f"    Notes:    {row[13] or '—'}")
        print()

def cmd_info(conn, key):
    c = conn.cursor()
    c.execute("SELECT * FROM licenses WHERE license_key = ?", (key.upper(),))
    row = c.fetchone()
    if not row:
        print(f"\n  License '{key}' not found.\n")
        return

    print(f"\n  ╔══════════════════════════════════════════════╗")
    print(f"  ║  LICENSE DETAILS                             ║")
    print(f"  ╠══════════════════════════════════════════════╣")
    print(f"  ║  Key:        {row[1]:<33}║")
    print(f"  ║  Plan:       {row[2]:<33}║")
    print(f"  ║  Status:     {row[7]:<33}║")
    print(f"  ║  Customer:   {(row[3] or '—'):<33}║")
    print(f"  ║  Email:      {(row[4] or '—'):<33}║")
    print(f"  ║  Phone:      {(row[5] or '—'):<33}║")
    print(f"  ║  Company:    {(row[6] or '—'):<33}║")
    print(f"  ║  Machine ID: {(row[8] or '—'):<33}║")
    print(f"  ║  Created:    {row[9]:<33}║")
    print(f"  ║  Assigned:   {(row[10] or '—'):<33}║")
    print(f"  ║  Activated:  {(row[12] or '—'):<33}║")
    print(f"  ║  Expires:    {(row[11] or '—'):<33}║")
    print(f"  ║  Notes:      {(row[13] or '—'):<33}║")
    print(f"  ╚══════════════════════════════════════════════╝\n")

def cmd_deactivate(conn, key):
    c = conn.cursor()
    c.execute("UPDATE licenses SET status='deactivated' WHERE license_key=?", (key.upper(),))
    if c.rowcount > 0:
        conn.commit()
        print(f"\n  License {key.upper()} DEACTIVATED.\n")
    else:
        print(f"\n  License '{key}' not found.\n")

def cmd_activate(conn, key):
    c = conn.cursor()
    c.execute("SELECT status, customer_name FROM licenses WHERE license_key=?", (key.upper(),))
    row = c.fetchone()
    if not row:
        print(f"\n  License '{key}' not found.\n")
        return
    new_status = "assigned" if row[1] else "unassigned"
    c.execute("UPDATE licenses SET status=? WHERE license_key=?", (new_status, key.upper()))
    conn.commit()
    print(f"\n  License {key.upper()} RE-ACTIVATED (status: {new_status}).\n")

def cmd_plan(conn, key, plan):
    plan = plan.lower()
    if plan not in PLANS:
        print(f"\n  Invalid plan. Choose: starter, professional, enterprise\n")
        return
    c = conn.cursor()
    c.execute("UPDATE licenses SET plan=? WHERE license_key=?", (plan, key.upper()))
    if c.rowcount > 0:
        conn.commit()
        p = PLANS[plan]
        print(f"\n  License {key.upper()} set to {plan.upper()} (Rs. {p['price']}/month - {p['desc']}).\n")
    else:
        print(f"\n  License '{key}' not found.\n")

def cmd_stats(conn):
    c = conn.cursor()
    total = c.execute("SELECT COUNT(*) FROM licenses").fetchone()[0]
    unassigned = c.execute("SELECT COUNT(*) FROM licenses WHERE status='unassigned'").fetchone()[0]
    assigned = c.execute("SELECT COUNT(*) FROM licenses WHERE status='assigned'").fetchone()[0]
    activated = c.execute("SELECT COUNT(*) FROM licenses WHERE status='activated'").fetchone()[0]
    deactivated = c.execute("SELECT COUNT(*) FROM licenses WHERE status='deactivated'").fetchone()[0]

    starter = c.execute("SELECT COUNT(*) FROM licenses WHERE plan='starter' AND status != 'deactivated'").fetchone()[0]
    pro = c.execute("SELECT COUNT(*) FROM licenses WHERE plan='professional' AND status != 'deactivated'").fetchone()[0]
    enterprise = c.execute("SELECT COUNT(*) FROM licenses WHERE plan='enterprise' AND status != 'deactivated'").fetchone()[0]

    # Revenue calculation
    revenue = starter * PLANS["starter"]["price"] + pro * PLANS["professional"]["price"] + enterprise * PLANS["enterprise"]["price"]

    print(f"""
  ╔══════════════════════════════════════════════╗
  ║           LICENSE STATISTICS                 ║
  ╠══════════════════════════════════════════════╣
  ║  Total Licenses:     {total:<27}║
  ║  ─────────────────────────────────────────── ║
  ║  Unassigned (stock): {unassigned:<27}║
  ║  Assigned (sold):    {assigned:<27}║
  ║  Activated (in use): {activated:<27}║
  ║  Deactivated:        {deactivated:<27}║
  ║  ─────────────────────────────────────────── ║
  ║  Starter Plan:       {starter:<27}║
  ║  Professional Plan:  {pro:<27}║
  ║  Enterprise Plan:    {enterprise:<27}║
  ║  ─────────────────────────────────────────── ║
  ║  Monthly Revenue:    Rs. {revenue:,}/month{"":<17}║
  ║  Annual Revenue:     Rs. {revenue*12:,}/year{"":<15}║
  ╚══════════════════════════════════════════════╝
""")

def cmd_export(conn):
    c = conn.cursor()
    c.execute("SELECT * FROM licenses ORDER BY id")
    rows = c.fetchall()
    export_path = os.path.join(os.path.dirname(DB_PATH), "license_export.csv")
    with open(export_path, "w") as f:
        f.write("ID,Key,Plan,Customer,Email,Phone,Company,Status,MachineID,Created,Assigned,Expiry,Notes\n")
        for row in rows:
            fields = [str(row[i]).replace(",", ";") if row[i] else "" for i in range(len(row))]
            f.write(",".join(fields) + "\n")
    print(f"\n  Exported {len(rows)} licenses to: {export_path}\n")

def get_plan(c, key):
    c.execute("SELECT plan FROM licenses WHERE license_key=?", (key,))
    row = c.fetchone()
    return row[0] if row else "unknown"

# ─── Main ─────────────────────────────────────────────────────

def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return

    conn = init_db()
    cmd = sys.argv[1].lower()

    try:
        if cmd == "generate":
            count = int(sys.argv[2]) if len(sys.argv) > 2 else 1
            cmd_generate(conn, count)

        elif cmd == "assign":
            if len(sys.argv) < 5:
                print("  Usage: assign <key> <name> <email> <phone>")
                return
            cmd_assign(conn, sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5] if len(sys.argv) > 5 else "")

        elif cmd == "list":
            filter_type = sys.argv[2] if len(sys.argv) > 2 else "all"
            cmd_list(conn, filter_type)

        elif cmd == "search":
            if len(sys.argv) < 3:
                print("  Usage: search <query>")
                return
            cmd_search(conn, sys.argv[2])

        elif cmd == "info":
            if len(sys.argv) < 3:
                print("  Usage: info <key>")
                return
            cmd_info(conn, sys.argv[2])

        elif cmd == "deactivate":
            if len(sys.argv) < 3:
                print("  Usage: deactivate <key>")
                return
            cmd_deactivate(conn, sys.argv[2])

        elif cmd == "activate":
            if len(sys.argv) < 3:
                print("  Usage: activate <key>")
                return
            cmd_activate(conn, sys.argv[2])

        elif cmd == "plan":
            if len(sys.argv) < 4:
                print("  Usage: plan <key> <starter|professional|enterprise>")
                return
            cmd_plan(conn, sys.argv[2], sys.argv[3])

        elif cmd == "stats":
            cmd_stats(conn)

        elif cmd == "export":
            cmd_export(conn)

        else:
            print(f"  Unknown command: {cmd}")
            print(__doc__)

    finally:
        conn.close()

if __name__ == "__main__":
    main()
