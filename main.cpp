#include <iostream>
#include <sstream>
#include "Engine.cpp" 

int main() {
    ProfessionalEngine db;
    
    std::cout << "========================================================\n";
    std::cout << "  ?? ENTERPRISE IN-MEMORY DATA ENGINE v2.0 (STABLE) \n";
    std::cout << "========================================================\n";
    db.loadFromWAL();
    std::cout << "Commands:\n"
              << "  SET <key> <val> [ttl_seconds]\n"
              << "  GET <key>\n"
              << "  DEL <key>\n"
              << "  STATS\n"
              << "  EXIT\n";

    std::string line, command, key, value;
    int ttl;

    while (true) {
        std::cout << "\nadvanced-db> ";
        if (!std::getline(std::cin, line)) break;

        std::stringstream ss(line);
        ss >> command;

        if (command == "SET" || command == "set") {
            ss >> key >> value;
            ttl = 0;
            ss >> ttl; 

            if (key.empty() || value.empty()) {
                std::cout << "Error: Invalid Syntax.\n";
            } else {
                db.put(key, value, ttl);
                std::cout << "OK " << (ttl > 0 ? "(Temporary key allocated)" : "") << "\n";
            }
        } 
        else if (command == "GET" || command == "get") {
            ss >> key;
            if (db.get(key, value)) {
                std::cout << "\"" << value << "\"\n";
            } else {
                std::cout << "(nil) - Key not found or expired.\n";
            }
        } 
        else if (command == "DEL" || command == "del") {
            ss >> key;
            if (db.remove(key)) std::cout << "(integer) 1\n";
            else std::cout << "(integer) 0\n";
        } 
        else if (command == "STATS" || command == "stats") {
            db.showStats();
        } 
        else if (command == "EXIT" || command == "exit") {
            std::cout << "Engine shutdown cleanly.\n";
            break;
        } 
        else {
            std::cout << "Error: Command syntax unsupported.\n";
        }
    }
    return 0;
}
