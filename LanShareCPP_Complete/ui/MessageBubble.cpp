#include "MessageBubble.h"
#include <QHBoxLayout>
#include <QDateTime>

MessageBubble::MessageBubble(const QString& sender, const QString& message, 
                             bool isOwn, bool isEncrypted, QWidget* parent)
    : QWidget(parent)
    , sender_(sender)
    , message_(message)
    , isOwn_(isOwn)
    , isEncrypted_(isEncrypted)
    , isDecrypted_(!isEncrypted)
    , decryptButton_(nullptr)
    , lockIcon_(nullptr)
{
    setupUI(sender, message, isOwn, isEncrypted);
    applyStyles(isOwn);
}

MessageBubble::MessageBubble(const QString& sender, const std::vector<uint8_t>& encryptedData,
                             bool isOwn, QWidget* parent)
    : QWidget(parent)
    , sender_(sender)
    , message_("[Encrypted Message]")
    , encryptedData_(encryptedData)
    , isOwn_(isOwn)
    , isEncrypted_(true)
    , isDecrypted_(false)
    , decryptButton_(nullptr)
    , lockIcon_(nullptr)
{
    setupUI(sender, "[Encrypted Message]", isOwn, true);
    applyStyles(isOwn);
}

void MessageBubble::setupUI(const QString& sender, const QString& message, bool isOwn, bool showDecrypt)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(isOwn ? 100 : 10, 5, isOwn ? 10 : 100, 5);
    
    // Create bubble container
    QWidget* bubble = new QWidget();
    QVBoxLayout* bubbleLayout = new QVBoxLayout(bubble);
    bubbleLayout->setSpacing(5);
    bubbleLayout->setContentsMargins(15, 10, 15, 10);
    
    // Sender label (only for others' messages)
    if (!isOwn) {
        QLabel* senderLabel = new QLabel(sender);
        senderLabel->setStyleSheet("color: #B9BBBE; font-size: 12px; font-weight: bold;");
        bubbleLayout->addWidget(senderLabel);
    }
    
    // Message content with lock icon
    QHBoxLayout* contentLayout = new QHBoxLayout();
    
    if (showDecrypt) {
        lockIcon_ = new QLabel("🔒");
        lockIcon_->setStyleSheet("font-size: 16px;");
        contentLayout->addWidget(lockIcon_);
    }
    
    messageLabel_ = new QLabel(message);
    messageLabel_->setWordWrap(true);
    messageLabel_->setStyleSheet("color: white; font-size: 14px;");
    contentLayout->addWidget(messageLabel_, 1);
    
    bubbleLayout->addLayout(contentLayout);
    
    // Decrypt button
    if (showDecrypt) {
        decryptButton_ = new QPushButton("🔓 Decrypt");
        decryptButton_->setMaximumWidth(100);
        decryptButton_->setStyleSheet(R"(
            QPushButton {
                background-color: #FEE75C;
                color: #202225;
                border: none;
                border-radius: 4px;
                padding: 5px 10px;
                font-size: 12px;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #FDD74C;
            }
        )");
        
        connect(decryptButton_, &QPushButton::clicked, this, [this]() {
            emit decryptClicked(this);
        });
        
        bubbleLayout->addWidget(decryptButton_, 0, isOwn ? Qt::AlignRight : Qt::AlignLeft);
    }
    
    // Timestamp
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm");
    QLabel* timeLabel = new QLabel(timestamp);
    timeLabel->setStyleSheet("color: #72767D; font-size: 11px;");
    bubbleLayout->addWidget(timeLabel, 0, Qt::AlignRight);
    
    mainLayout->addWidget(bubble, 0, isOwn ? Qt::AlignRight : Qt::AlignLeft);
    
    // Set bubble background
    QString bgColor = isOwn ? "#5865F2" : "#40444B";
    bubble->setStyleSheet(QString("background-color: %1; border-radius: 10px;").arg(bgColor));
    
    setMaximumWidth(800);
}

void MessageBubble::applyStyles(bool isOwn)
{
    // Styles are applied in setupUI
}

void MessageBubble::setDecryptedText(const QString& text)
{
    message_ = text;
    messageLabel_->setText(text);
    
    if (lockIcon_) {
        lockIcon_->setText("🔓");
    }
    
    if (decryptButton_) {
        decryptButton_->hide();
    }
    
    isDecrypted_ = true;
}

std::vector<uint8_t> MessageBubble::getEncryptedData() const
{
    return encryptedData_;
}
