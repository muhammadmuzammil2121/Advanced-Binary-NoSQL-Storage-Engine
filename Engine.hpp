#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <string>
#include <memory>
#include <vector>
#include <ctime>

// Node jo data aur uski expiry time store karega
class HashNode {
public:
    std::string key;
    std::string value;
    std::time_t expiryTime; // TTL feature ke liye
    bool hasTTL;
    std::unique_ptr<HashNode> next;

    HashNode(const std::string& k, const std::string& v, int ttlSeconds = 0) 
        : key(k), value(v), hasTTL(ttlSeconds > 0), next(nullptr) {
        if (hasTTL) {
            this->expiryTime = std::time(nullptr) + ttlSeconds;
        } else {
            this->expiryTime = 0;
        }
    }
};

class ProfessionalEngine {
private:
    static const size_t BUCKET_SIZE = 512;
    std::vector<std::unique_ptr<HashNode>> buckets;
    std::string logFileName;

    size_t getHash(const std::string& key) const;
    void appendToWAL(const std::string& command, const std::string& key, const std::string& value, int ttl = 0);

public:
    ProfessionalEngine(std::string logFile = "wal_storage.db");
    
    void put(const std::string& key, const std::string& value, int ttlSeconds = 0, bool triggerWAL = true);
    bool get(const std::string& key, std::string& value);
    bool remove(const std::string& key, bool triggerWAL = true);
    void loadFromWAL();
    void showStats() const;
};

#endif
