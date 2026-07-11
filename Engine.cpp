#include "Engine.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

ProfessionalEngine::ProfessionalEngine(std::string logFile) : buckets(BUCKET_SIZE), logFileName(logFile) {}

size_t ProfessionalEngine::getHash(const std::string& key) const {
    size_t hash = 0;
    for (char ch : key) {
        hash = (hash * 31 + ch) % BUCKET_SIZE;
    }
    return hash;
}

// Hard Drive par data permanently likhne ke liye (Write-Ahead Log)
void ProfessionalEngine::appendToWAL(const std::string& command, const std::string& key, const std::string& value, int ttl) {
    std::ofstream file(logFileName, std::ios::app);
    if (file.is_open()) {
        file << command << " " << key << " " << (value.empty() ? "_" : value) << " " << ttl << "\n";
    }
}

// SET Command ki coding
void ProfessionalEngine::put(const std::string& key, const std::string& value, int ttlSeconds, bool triggerWAL) {
    size_t index = getHash(key);
    HashNode* entry = buckets[index].get();

    while (entry != nullptr) {
        if (entry->key == key) {
            entry->value = value;
            entry->hasTTL = (ttlSeconds > 0);
            entry->expiryTime = entry->hasTTL ? (std::time(nullptr) + ttlSeconds) : 0;
            if (triggerWAL) appendToWAL("SET", key, value, ttlSeconds);
            return;
        }
        entry = entry->next.get();
    }

    std::unique_ptr<HashNode> newNode(new HashNode(key, value, ttlSeconds));
    if (buckets[index] != nullptr) {
        newNode->next = std::move(buckets[index]);
    }
    buckets[index] = std::move(newNode);
    
    if (triggerWAL) appendToWAL("SET", key, value, ttlSeconds);
}

// GET Command ki coding (With Auto-Expiry Check)
bool ProfessionalEngine::get(const std::string& key, std::string& value) {
    size_t index = getHash(key);
    HashNode* entry = buckets[index].get();

    while (entry != nullptr) {
        if (entry->key == key) {
            // Check agar key expire ho chuki hai
            if (entry->hasTTL && std::time(nullptr) >= entry->expiryTime) {
                remove(key, true); // Memory aur file dono se delete kardo
                return false;
            }
            value = entry->value;
            return true;
        }
        entry = entry->next.get();
    }
    return false;
}

// DEL Command ki coding
bool ProfessionalEngine::remove(const std::string& key, bool triggerWAL) {
    size_t index = getHash(key);
    HashNode* entry = buckets[index].get();
    HashNode* prev = nullptr;

    while (entry != nullptr) {
        if (entry->key == key) {
            if (prev == nullptr) {
                buckets[index] = std::move(entry->next);
            } else {
                prev->next = std::move(entry->next);
            }
            if (triggerWAL) appendToWAL("DEL", key, "", 0);
            return true;
        }
        prev = entry;
        entry = entry->next.get();
    }
    return false;
}

// Hard Drive se data wapis RAM me load karne ka function
void ProfessionalEngine::loadFromWAL() {
    std::ifstream file(logFileName);
    if (!file.is_open()) return;

    std::string line, command, key, value;
    int ttl;
    int loadedCount = 0;

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        ss >> command >> key >> value >> ttl;
        if (value == "_") value = "";

        if (command == "SET") {
            put(key, value, ttl, false); // false taake file me dobara double entry na ho
            loadedCount++;
        } else if (command == "DEL") {
            remove(key, false);
        }
    }
    if(loadedCount > 0) {
        std::cout << "[? Storage Engine] Successfully recovered " << loadedCount << " active keys from disk.\n";
    }
}

// Database Diagnostics Panel
void ProfessionalEngine::showStats() const {
    size_t totalKeys = 0;
    size_t collisions = 0;
    
    for (size_t i = 0; i < BUCKET_SIZE; ++i) {
        HashNode* entry = buckets[i].get();
        if (entry != nullptr) {
            totalKeys++;
            entry = entry->next.get();
            while (entry != nullptr) {
                totalKeys++;
                collisions++;
                entry = entry->next.get();
            }
        }
    }
    std::cout << "--- ?? DATABASE INFRASTRUCTURE METRICS ---\n"
              << "  Total Allocated Buckets : " << BUCKET_SIZE << "\n"
              << "  Active Keys in RAM      : " << totalKeys << "\n"
              << "  Hash Collisions Detected: " << collisions << "\n"
              << "----------------------------------------\n";
}
