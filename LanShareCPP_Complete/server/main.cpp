#include "ServerCore.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <memory>
#include <string>

std::unique_ptr<LanShare::ServerCore> g_server;

void signalHandler(int signum) {
    std::cout << "\nInterrupt signal (" << signum << ") received. Shutting down..." << std::endl;
    if (g_server) {
        g_server->stop();
    }
    exit(signum);
}

void printBanner() {
    std::cout << R"(
╔═══════════════════════════════════════════════════════════╗
║                                                           ║
║          LanShareCPP - Secure LAN Messaging Server        ║
║                     Version 1.0.0                         ║
║                                                           ║
║          Secure • Fast • Encrypted • Professional         ║
║                                                           ║
╚═══════════════════════════════════════════════════════════╝
)" << std::endl;
}

void printHelp() {
    std::cout << "Usage: lanshare_server [OPTIONS]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --port PORT       Set server port (default: 5555)\n";
    std::cout << "  --help            Show this help message\n";
    std::cout << "  --version         Show version information\n\n";
    std::cout << "Examples:\n";
    std::cout << "  lanshare_server                 # Start on default port 5555\n";
    std::cout << "  lanshare_server --port 6000     # Start on custom port\n";
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    // Default settings
    unsigned short port = 5555;
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            printBanner();
            printHelp();
            return 0;
        }
        else if (arg == "--version" || arg == "-v") {
            std::cout << "LanShareCPP Server v1.0.0" << std::endl;
            return 0;
        }
        else if (arg == "--port" || arg == "-p") {
            if (i + 1 < argc) {
                port = static_cast<unsigned short>(std::stoi(argv[++i]));
            } else {
                std::cerr << "Error: --port requires a port number" << std::endl;
                return 1;
            }
        }
        else {
            std::cerr << "Unknown option: " << arg << std::endl;
            std::cerr << "Use --help for usage information" << std::endl;
            return 1;
        }
    }
    
    // Print banner
    printBanner();
    
    // Register signal handlers
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    try {
        // Create and start server
        std::cout << "Initializing server..." << std::endl;
        g_server = std::make_unique<LanShare::ServerCore>(port);
        
        std::cout << "Starting LanShareCPP Server on port " << port << "..." << std::endl;
        std::cout << "Press Ctrl+C to stop the server" << std::endl;
        std::cout << std::string(60, '=') << std::endl;
        
        g_server->start();
        
        std::cout << "\n✓ Server started successfully!" << std::endl;
        std::cout << "✓ Listening on port: " << port << std::endl;
        std::cout << "✓ Encryption: AES-256-GCM Enabled" << std::endl;
        std::cout << "✓ Ready to accept client connections" << std::endl;
        std::cout << std::string(60, '=') << std::endl << std::endl;
        
        // Keep the main thread alive
        while (g_server && g_server->isRunning()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Server error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\nServer shutdown complete." << std::endl;
    return 0;
}
