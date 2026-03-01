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
    explicit FileTransferWidget(const QString& filename, bool isSending, QWidget* parent = nullptr);
    
    void setProgress(int percentage);
    void setComplete();
    void setFailed(const QString& reason);

signals:
    void downloadClicked();
    void cancelClicked();

private:
    void setupUI(const QString& filename, bool isSending);
    
    QString filename_;
    bool isSending_;
    
    QLabel* iconLabel_;
    QLabel* filenameLabel_;
    QLabel* statusLabel_;
    QProgressBar* progressBar_;
    QPushButton* actionButton_;
};

#endif // FILETRANSFERWIDGET_H
