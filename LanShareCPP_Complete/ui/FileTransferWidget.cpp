#include "FileTransferWidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileInfo>

FileTransferWidget::FileTransferWidget(const QString& filename, bool isSending, QWidget* parent)
    : QWidget(parent)
    , filename_(filename)
    , isSending_(isSending)
{
    setupUI(filename, isSending);
}

void FileTransferWidget::setupUI(const QString& filename, bool isSending)
{
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(15, 10, 15, 10);
    
    // File icon
    iconLabel_ = new QLabel(isSending ? "📤" : "📥");
    iconLabel_->setStyleSheet("font-size: 32px;");
    mainLayout->addWidget(iconLabel_);
    
    // File info
    QVBoxLayout* infoLayout = new QVBoxLayout();
    
    QFileInfo fileInfo(filename);
    filenameLabel_ = new QLabel(fileInfo.fileName());
    filenameLabel_->setStyleSheet("color: white; font-size: 14px; font-weight: bold;");
    infoLayout->addWidget(filenameLabel_);
    
    QString sizeStr = QString::number(fileInfo.size() / 1024.0, 'f', 2) + " KB";
    statusLabel_ = new QLabel(isSending ? "Sending... " + sizeStr : "Ready to download • " + sizeStr);
    statusLabel_->setStyleSheet("color: #B9BBBE; font-size: 12px;");
    infoLayout->addWidget(statusLabel_);
    
    progressBar_ = new QProgressBar();
    progressBar_->setMaximumHeight(8);
    progressBar_->setTextVisible(false);
    progressBar_->setStyleSheet(R"(
        QProgressBar {
            background-color: #202225;
            border-radius: 4px;
            border: none;
        }
        QProgressBar::chunk {
            background-color: #57F287;
            border-radius: 4px;
        }
    )");
    
    if (isSending) {
        progressBar_->setValue(50); // Simulate progress
    } else {
        progressBar_->setValue(0);
    }
    
    infoLayout->addWidget(progressBar_);
    
    mainLayout->addLayout(infoLayout, 1);
    
    // Action button
    actionButton_ = new QPushButton(isSending ? "Cancel" : "Download");
    actionButton_->setMaximumWidth(100);
    actionButton_->setStyleSheet(R"(
        QPushButton {
            background-color: #3BA55D;
            color: white;
            border: none;
            border-radius: 5px;
            padding: 8px 15px;
            font-size: 12px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #2D7D46;
        }
    )");
    
    if (isSending) {
        connect(actionButton_, &QPushButton::clicked, this, &FileTransferWidget::cancelClicked);
    } else {
        connect(actionButton_, &QPushButton::clicked, this, &FileTransferWidget::downloadClicked);
    }
    
    mainLayout->addWidget(actionButton_);
    
    // Widget styling
    setStyleSheet("background-color: #2F3136; border-radius: 8px;");
    setMaximumWidth(600);
}

void FileTransferWidget::setProgress(int percentage)
{
    progressBar_->setValue(percentage);
    statusLabel_->setText(QString("%1%2").arg(
        isSending_ ? "Sending... " : "Downloading... ",
        QString::number(percentage) + "%"
    ));
}

void FileTransferWidget::setComplete()
{
    progressBar_->setValue(100);
    statusLabel_->setText(isSending_ ? "✅ Sent!" : "✅ Downloaded!");
    statusLabel_->setStyleSheet("color: #57F287; font-size: 12px; font-weight: bold;");
    
    actionButton_->hide();
    iconLabel_->setText("✅");
}

void FileTransferWidget::setFailed(const QString& reason)
{
    statusLabel_->setText("❌ Failed: " + reason);
    statusLabel_->setStyleSheet("color: #ED4245; font-size: 12px; font-weight: bold;");
    
    actionButton_->setText("Retry");
    iconLabel_->setText("❌");
}
