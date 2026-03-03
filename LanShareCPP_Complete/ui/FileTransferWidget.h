#ifndef FILETRANSFERWIDGET_H
#define FILETRANSFERWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QString>

class FileTransferWidget : public QWidget
{
    Q_OBJECT

public:
    // For sending — pass full filepath
    explicit FileTransferWidget(const QString& filepath, bool isSending,
                                QWidget* parent = nullptr);

    // For receiving — pass just filename (no path needed)
    explicit FileTransferWidget(const QString& filename,
                                QWidget* parent = nullptr);

    void setProgress(int percentage);
    void setComplete();
    void setFailed(const QString& reason);
    void setStatusText(const QString& text);   // ← NEW

signals:
    void cancelClicked();

private:
    void setupUI(const QString& displayName, bool isSending);

    QString  filename_;
    bool     isSending_;

    QLabel*       iconLabel_;
    QLabel*       filenameLabel_;
    QLabel*       statusLabel_;
    QProgressBar* progressBar_;
    QPushButton*  actionButton_;
};

#endif // FILETRANSFERWIDGET_H