#include "FileTransferWidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileInfo>

// ── Sending constructor ────────────────────────────────────────────────────
FileTransferWidget::FileTransferWidget(const QString& filepath, bool isSending,
                                       QWidget* parent)
    : QWidget(parent)
    , filename_(filepath)
    , isSending_(isSending)
{
    QFileInfo fi(filepath);
    setupUI(fi.fileName(), isSending);
}

// ── Receiving constructor (filename only, no path) ─────────────────────────
FileTransferWidget::FileTransferWidget(const QString& filename, QWidget* parent)
    : QWidget(parent)
    , filename_(filename)
    , isSending_(false)
{
    setupUI(filename, false);
}

void FileTransferWidget::setupUI(const QString& displayName, bool isSending)
{
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(15, 12, 15, 12);
    mainLayout->setSpacing(12);

    // Icon
    iconLabel_ = new QLabel(isSending ? "📤" : "📥");
    iconLabel_->setStyleSheet("font-size: 28px;");
    iconLabel_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mainLayout->addWidget(iconLabel_);

    // Info column
    QVBoxLayout* infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(4);

    filenameLabel_ = new QLabel(displayName);
    filenameLabel_->setStyleSheet("color: white; font-size: 13px; font-weight: bold;");
    filenameLabel_->setWordWrap(true);
    infoLayout->addWidget(filenameLabel_);

    statusLabel_ = new QLabel(isSending ? "⏳ Preparing to send..." : "⏳ Waiting for data...");
    statusLabel_->setStyleSheet("color: #B9BBBE; font-size: 11px;");
    infoLayout->addWidget(statusLabel_);

    progressBar_ = new QProgressBar();
    progressBar_->setFixedHeight(6);
    progressBar_->setTextVisible(false);
    progressBar_->setValue(0);
    progressBar_->setStyleSheet(R"(
        QProgressBar {
            background-color: #202225;
            border-radius: 3px;
            border: none;
        }
        QProgressBar::chunk {
            background-color: #57F287;
            border-radius: 3px;
        }
    )");
    infoLayout->addWidget(progressBar_);

    mainLayout->addLayout(infoLayout, 1);

    // Cancel button (only for sending)
    if (isSending) {
        actionButton_ = new QPushButton("✕");
        actionButton_->setFixedSize(28, 28);
        actionButton_->setToolTip("Cancel");
        actionButton_->setStyleSheet(R"(
            QPushButton {
                background-color: #ED4245; color: white;
                border: none; border-radius: 14px;
                font-size: 12px; font-weight: bold;
            }
            QPushButton:hover { background-color: #c0392b; }
        )");
        connect(actionButton_, &QPushButton::clicked,
                this, &FileTransferWidget::cancelClicked);
        mainLayout->addWidget(actionButton_);
    } else {
        actionButton_ = nullptr;
    }

    setStyleSheet("background-color: #2F3136; border-radius: 8px;");
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setFixedHeight(85);
}

void FileTransferWidget::setProgress(int percentage)
{
    progressBar_->setValue(percentage);
    statusLabel_->setText(
        QString("%1 %2%").arg(isSending_ ? "📤 Sending..." : "📥 Receiving...",
                              QString::number(percentage)));
}

void FileTransferWidget::setStatusText(const QString& text)
{
    statusLabel_->setText(text);
}

void FileTransferWidget::setComplete()
{
    progressBar_->setValue(100);
    iconLabel_->setText("✅");
    statusLabel_->setText(isSending_ ? "✅ Sent successfully!" : "✅ Saved to Downloads!");
    statusLabel_->setStyleSheet("color: #57F287; font-size: 11px; font-weight: bold;");
    if (actionButton_) actionButton_->hide();
}

void FileTransferWidget::setFailed(const QString& reason)
{
    iconLabel_->setText("❌");
    statusLabel_->setText("❌ Failed: " + reason);
    statusLabel_->setStyleSheet("color: #ED4245; font-size: 11px; font-weight: bold;");
    if (actionButton_) actionButton_->hide();
}