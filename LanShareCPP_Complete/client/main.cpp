#include "ClientCore.h"
#include <iostream>
#include <string>
#include <thread>

void printMenu() {
    std::cout << "\n=== LanShare Client ===\n";
    std::cout << "1. Register\n";
    std::cout << "2. Login\n";
    std::cout << "3. Send message\n";
    std::cout << "4. List users\n";
    std::cout << "5. Create group\n";
    std::cout << "6. Join group\n";
    std::cout << "7. Send group message\n";
    std::cout << "8. Quit\n";
    std::cout << "Choice: ";
}

int main() {
    std::cout << "LanShare C++ Client\n";
    std::cout << "==================\n\n";
    
    LanShare::ClientCore client;
    
    // Set up callbacks
    client.setAuthCallback([](bool success, const std::string& userID, const std::string& msg) {
        if (success) {
            std::cout << "\n✓ " << msg << " (UserID: " << userID << ")\n";
        } else {
            std::cout << "\n✗ " << msg << "\n";
        }
    });
    
    client.setMessageCallback([&client](const std::string& from, const std::vector<uint8_t>& encrypted, uint64_t) {
        std::cout << "\n[Message from " << from << "]\n";
        std::cout << "Encrypted (" << encrypted.size() << " bytes)\n";
        
        // Try to decrypt
        try {
            auto decrypted = LanShare::AESGCM::EncryptedData::deserialize(encrypted);
            std::string plaintext = client.getCrypto().decryptToString(decrypted, client.getEncryptionKey());
            std::cout << "Decrypted: " << plaintext << "\n";
        } catch (const std::exception& e) {
            std::cout << "Decryption failed: " << e.what() << "\n";
        }
    });
    
    client.setGroupMessageCallback([&client](const std::string& group, const std::string& from, 
                                              const std::vector<uint8_t>& encrypted, uint64_t) {
        std::cout << "\n[Group message in " << group << " from " << from << "]\n";
        try {
            auto decrypted = LanShare::AESGCM::EncryptedData::deserialize(encrypted);
            std::string plaintext = client.getCrypto().decryptToString(decrypted, client.getEncryptionKey());
            std::cout << "Decrypted: " << plaintext << "\n";
        } catch (const std::exception& e) {
            std::cout << "Decryption failed: " << e.what() << "\n";
        }
    });
    
    client.setUserListCallback([](const std::vector<std::string>& users) {
        std::cout << "\n=== Online Users ===\n";
        for (const auto& user : users) {
            std::cout << "  • " << user << "\n";
        }
    });
    
    // Get server IP
    std::string serverIP;
    std::cout << "Server IP (default: 127.0.0.1): ";
    std::getline(std::cin, serverIP);
    if (serverIP.empty()) {
        serverIP = "127.0.0.1";
    }
    
    // Connect
    std::cout << "Connecting to " << serverIP << ":5555...\n";
    client.connect(serverIP, 5555);
    
    // Start async I/O
    client.startAsync();
    
    // Wait a bit for connection
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    if (!client.isConnected()) {
        std::cerr << "Failed to connect to server\n";
        return 1;
    }
    
    // Main loop
    bool running = true;
    while (running) {
        printMenu();
        
        int choice;
        std::cin >> choice;
        std::cin.ignore();
        
        switch (choice) {
            case 1: {
                std::string username, password;
                std::cout << "Username: ";
                std::getline(std::cin, username);
                std::cout << "Password: ";
                std::getline(std::cin, password);
                client.registerUser(username, password);
                break;
            }
            
            case 2: {
                std::string username, password;
                std::cout << "Username: ";
                std::getline(std::cin, username);
                std::cout << "Password: ";
                std::getline(std::cin, password);
                client.login(username, password);
                break;
            }
            
            case 3: {
                if (!client.isAuthenticated()) {
                    std::cout << "Please login first\n";
                    break;
                }
                
                std::string toUser, message;
                std::cout << "To (UserID): ";
                std::getline(std::cin, toUser);
                std::cout << "Message: ";
                std::getline(std::cin, message);
                
                // Encrypt message
                auto encrypted = client.getCrypto().encrypt(message, client.getEncryptionKey());
                auto blob = encrypted.serialize();
                
                client.sendPrivateMessage(toUser, blob);
                std::cout << "Message sent (encrypted)\n";
                break;
            }
            
            case 4: {
                if (!client.isAuthenticated()) {
                    std::cout << "Please login first\n";
                    break;
                }
                client.requestUserList();
                break;
            }
            
            case 5: {
                if (!client.isAuthenticated()) {
                    std::cout << "Please login first\n";
                    break;
                }
                
                std::string groupName;
                std::cout << "Group name: ";
                std::getline(std::cin, groupName);
                client.createGroup(groupName);
                break;
            }
            
            case 6: {
                if (!client.isAuthenticated()) {
                    std::cout << "Please login first\n";
                    break;
                }
                
                std::string groupName;
                std::cout << "Group name: ";
                std::getline(std::cin, groupName);
                std::string joinCode;
std::cout << "Enter join code: ";
std::getline(std::cin, joinCode);
client.joinGroup(groupName, joinCode);
                break;
            }
            
            case 7: {
                if (!client.isAuthenticated()) {
                    std::cout << "Please login first\n";
                    break;
                }
                
                std::string groupName, message;
                std::cout << "Group name: ";
                std::getline(std::cin, groupName);
                std::cout << "Message: ";
                std::getline(std::cin, message);
                
                // Encrypt message
                auto encrypted = client.getCrypto().encrypt(message, client.getEncryptionKey());
                auto blob = encrypted.serialize();
                
                client.sendGroupMessage(groupName, blob);
                std::cout << "Group message sent (encrypted)\n";
                break;
            }
            
            case 8:
                running = false;
                break;
                
            default:
                std::cout << "Invalid choice\n";
        }
        
        // Small delay to see responses
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "Disconnecting...\n";
    client.disconnect();
    
    return 0;
}
