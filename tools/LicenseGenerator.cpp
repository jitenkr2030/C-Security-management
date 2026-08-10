/*
 * SGMS License Key Generator
 * Run on YOUR machine to generate keys for customers.
 *
 * Compile: g++ -std=c++17 -o LicenseGenerator LicenseGenerator.cpp
 *
 * Usage:
 *   ./LicenseGenerator                       # Generate 1 universal key
 *   ./LicenseGenerator batch 10              # Generate 10 universal keys
 *   ./LicenseGenerator machine <machine-id>  # Generate machine-specific key
 */

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <random>
#include <algorithm>
#include <cstdlib>

const std::string SECRET_KEY = "SGMS2025SECURITY@KEY";

std::string simpleHash(const std::string& input) {
    std::size_t h1 = std::hash<std::string>{}(input);
    std::size_t h2 = std::hash<std::string>{}(input + SECRET_KEY);
    std::size_t h3 = std::hash<std::string>{}(SECRET_KEY + input);
    std::size_t h4 = std::hash<std::string>{}(input + "SALT" + SECRET_KEY);
    h1 ^= (h2 << 7) ^ (h3 >> 11);
    h3 ^= (h4 << 13) ^ (h1 >> 5);
    std::ostringstream oss;
    oss << std::hex << std::setfill('0') << std::setw(16) << h1;
    oss << std::hex << std::setfill('0') << std::setw(16) << h3;
    std::string result = oss.str();
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

std::string generateKeyPart() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::string key;
    for (int i = 0; i < 8; ++i) {
        key += "0123456789ABCDEF"[dis(gen)];
    }
    return key;
}

std::string makeChecksum(const std::string& keyPart, const std::string& machineId) {
    std::string combined = keyPart + machineId + SECRET_KEY;
    std::string hash = simpleHash(combined);
    return hash.substr(0, 8);
}

std::string formatKey(const std::string& keyPart, const std::string& checksum) {
    std::string full = keyPart + checksum;
    return full.substr(0,4) + "-" + full.substr(4,4) + "-" + full.substr(8,4) + "-" + full.substr(12,4);
}

std::string generateLicense(const std::string& machineId) {
    std::string keyPart = generateKeyPart();
    std::string checksum = makeChecksum(keyPart, machineId);
    return formatKey(keyPart, checksum);
}

int main(int argc, char* argv[]) {
    std::cout << "\n========================================\n";
    std::cout << "    SGMS License Key Generator v1.0\n";
    std::cout << "========================================\n\n";

    if (argc >= 3 && std::string(argv[1]) == "batch") {
        int count = std::atoi(argv[2]);
        std::cout << "Generating " << count << " UNIVERSAL license keys:\n";
        std::cout << "(Works on any machine)\n\n";
        for (int i = 0; i < count; ++i) {
            std::string key = generateLicense("UNIVERSAL");
            std::cout << "  " << std::setw(3) << (i+1) << ". " << key << "\n";
        }
    } else if (argc >= 3 && std::string(argv[1]) == "machine") {
        std::string machineId = argv[2];
        std::cout << "Machine-specific license for:\n";
        std::cout << "  Machine ID: " << machineId << "\n\n";
        std::string key = generateLicense(machineId);
        std::cout << "  License Key: " << key << "\n";
        std::cout << "\n  NOTE: This key only works on this specific machine.\n";
    } else {
        std::cout << "Generating UNIVERSAL license key:\n";
        std::cout << "(Works on any machine)\n\n";
        std::string key = generateLicense("UNIVERSAL");
        std::cout << "  License Key: " << key << "\n";
    }

    std::cout << "\n========================================\n\n";
    std::cout << "USAGE:\n";
    std::cout << "  ./LicenseGenerator                       # 1 universal key\n";
    std::cout << "  ./LicenseGenerator batch 10              # 10 universal keys\n";
    std::cout << "  ./LicenseGenerator machine <machine-id>  # machine-specific key\n\n";

    return 0;
}
