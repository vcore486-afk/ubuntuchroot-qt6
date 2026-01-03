#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QMessageBox>
#include <QProcess>
#include <QString>
#include <QDebug>
#include <QRegularExpression>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // Declare the switchToVersion function
    void switchToVersion(const QString &versionInput);  // Function to handle version switching
    void checkUbuntuChroot();  // Assuming you have a function to handle changing the Ubuntu version;  // Function to check the current version of Ubuntu

private slots:
    void on_pushUbuntu16_clicked();
    void on_pushUbuntu20_clicked();
    void on_pushUbuntu24_clicked();
    void on_pushCheck_clicked();
   

private:
    Ui::MainWindow *ui;
    QString currentUbuntuVersion;  // To store the current Ubuntu version
    QString checkVersion();
};

#endif // MAINWINDOW_H
