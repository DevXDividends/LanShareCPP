#include "LoginWindow.h"
#include <QApplication>
#include <QFont>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application info
    app.setApplicationName("LanShare");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("LanShareCPP");
    
    // Set default font
    QFont font("Segoe UI", 10);
    app.setFont(font);
    
    // Load global stylesheet
    QFile styleFile(":/styles/main.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        app.setStyleSheet(styleSheet);
    }
    
    // Create and show login window
    LoginWindow loginWindow;
    loginWindow.show();
    
    return app.exec();
}
