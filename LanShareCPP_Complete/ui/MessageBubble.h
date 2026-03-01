#ifndef MESSAGEBUBBLE_H
#define MESSAGEBUBBLE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QString>
#include <vector>

class MessageBubble : public QWidget
{
    Q_OBJECT

public:
    // Regular message
    explicit MessageBubble(const QString& sender, const QString& message, 
                          bool isOwn, bool isEncrypted, QWidget* parent = nullptr);
    
    // Encrypted message
    explicit MessageBubble(const QString& sender, const std::vector<uint8_t>& encryptedData,
                          bool isOwn, QWidget* parent = nullptr);
    
    void setDecryptedText(const QString& text);
    std::vector<uint8_t> getEncryptedData() const;

signals:
    void decryptClicked(MessageBubble* bubble);

private:
    void setupUI(const QString& sender, const QString& message, bool isOwn, bool showDecrypt);
    void applyStyles(bool isOwn);

    QString sender_;
    QString message_;
    std::vector<uint8_t> encryptedData_;
    bool isOwn_;
    bool isEncrypted_;
    bool isDecrypted_;
    
    QLabel* messageLabel_;
    QPushButton* decryptButton_;
    QLabel* lockIcon_;
};

#endif // MESSAGEBUBBLE_H
