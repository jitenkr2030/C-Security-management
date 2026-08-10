#!/usr/bin/env python3
"""
SGMS License Key Generator
Run this on YOUR machine to generate license keys for customers.

Usage:
    python3 LicenseGenerator.py                       # 1 universal key
    python3 LicenseGenerator.py batch 10              # 10 universal keys
    python3 LicenseGenerator.py machine <machine-id>  # machine-specific key
"""

import hashlib
import random
import sys

SECRET_KEY = "SGMS2025SECURITY@KEY"

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

def generate_license(machine_id):
    key_part = generate_key_part()
    checksum = make_checksum(key_part, machine_id)
    return format_key(key_part, checksum)

def main():
    print()
    print("=" * 48)
    print("    SGMS License Key Generator v1.0")
    print("=" * 48)
    print()

    if len(sys.argv) >= 3 and sys.argv[1] == "batch":
        count = int(sys.argv[2])
        print(f"Generating {count} UNIVERSAL license keys:")
        print("(Works on any machine)")
        print()
        for i in range(count):
            key = generate_license("UNIVERSAL")
            print(f"  {i+1:3d}. {key}")
    elif len(sys.argv) >= 3 and sys.argv[1] == "machine":
        machine_id = sys.argv[2]
        print(f"Machine-specific license for:")
        print(f"  Machine ID: {machine_id}")
        print()
        key = generate_license(machine_id)
        print(f"  License Key: {key}")
        print()
        print("  NOTE: This key only works on this specific machine.")
    else:
        print("Generating UNIVERSAL license key:")
        print("(Works on any machine)")
        print()
        key = generate_license("UNIVERSAL")
        print(f"  License Key: {key}")

    print()
    print("=" * 48)
    print()
    print("USAGE:")
    print("  python3 LicenseGenerator.py                       # 1 universal key")
    print("  python3 LicenseGenerator.py batch 10              # 10 universal keys")
    print("  python3 LicenseGenerator.py machine <machine-id>  # machine-specific key")
    print()

if __name__ == "__main__":
    main()
