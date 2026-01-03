#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QProcess>
#include <QTextStream>
#include <QDebug>
#include <QRegularExpression>
#include <QDir>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushUbuntu16_clicked() {
    checkUbuntuChroot();  // Assuming you have a function to handle changing the Ubuntu version
    switchToVersion("16.04");  // Для активации Ubuntu 16.04    
    currentUbuntuVersion = "16.04";
    ui->textEdit->append("Выбрана Ubuntu 16.04");
}

void MainWindow::on_pushUbuntu20_clicked() {
    checkUbuntuChroot();  // Assuming you have a function to handle changing the Ubuntu version
    switchToVersion("20.04");  // Для активации Ubuntu 20.04
    currentUbuntuVersion = "20.04";
    ui->textEdit->append("Выбрана Ubuntu 20.04");
}

void MainWindow::on_pushUbuntu24_clicked() {
    checkUbuntuChroot();  // Assuming you have a function to handle changing the Ubuntu version
    switchToVersion("24.04");  // Для активации Ubuntu 16.04
    currentUbuntuVersion = "24.04";
    ui->textEdit->append("Выбрана Ubuntu 24.04");
}

#include <QRegularExpression>   // Убедитесь, что эта строка есть в начале файла!

void MainWindow::on_pushCheck_clicked()
{
    QProcess process;
    process.start("bash", QStringList() << "-c"
                  << "cat /compat/ubuntu/etc/os-release 2>/dev/null | "
                     "grep -E '^(PRETTY_NAME|VERSION)=' | head -1");
    process.waitForFinished(-1);

    QByteArray output = process.readAllStandardOutput().trimmed();

    QString result;

    if (output.isEmpty()) {
        result = "Ubuntu не обнаружена или файл os-release отсутствует";
    } else {
        QString line = QString(output);

        QString version;

        // Извлекаем номер версии (например, "16.04.6 LTS" или "Ubuntu 20.04.6 LTS")
        if (line.startsWith("PRETTY_NAME=")) {
            version = line.mid(13);                 // после PRETTY_NAME="
            version = version.remove('"');           // убираем кавычки
            version = version.section(' ', 1, 1);    // берём второе слово: 16.04.6 / 20.04.6 / 24.04.1
        } else if (line.startsWith("VERSION=")) {
            version = line.mid(9);                  // после VERSION="
            version = version.section(' ', 0, 0);    // берём первое слово до пробела или (
            version = version.remove('"');
        }

        // Основная магия: обрезаем до двух частей (XX.XX), убираем третью цифру после точки
        QRegularExpression re(R"(\d+\.\d+)");        // ищем XX.XX
        QRegularExpressionMatch match = re.match(version);
        if (match.hasMatch()) {
            version = match.captured(0);            // берём только "16.04", "20.04", "24.04"
        }

        result = "Версия Ubuntu: <b>" + version + "</b>";
    }

    ui->textEdit->setHtml(result);
}

void MainWindow::switchToVersion(const QString &versionInput)
{
    const QString PATH_CHROOT = "/ntfs-2TB/compat";

    // Step 1: Get the Ubuntu version
    QString version = checkVersion();
    if (version.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Не удалось определить версию Ubuntu.");
        return;
    }

    // Step 2: Rename the existing ubuntu directory to ubuntu_<version>
    QString currentUbuntuPath = PATH_CHROOT + "/ubuntu";
    QString versionedUbuntuPath = PATH_CHROOT + "/ubuntu_" + version;

    // Rename ubuntu to ubuntu_<version>
    QDir dir(PATH_CHROOT);
    if (dir.exists(currentUbuntuPath)) {
        if (!dir.rename(currentUbuntuPath, versionedUbuntuPath)) {
            QMessageBox::critical(this, "Ошибка", "Не удалось переименовать ubuntu в " + versionedUbuntuPath);
            return;
        }
        ui->textEdit->append("<font color='green'>Переименовано: ubuntu → " + versionedUbuntuPath + "</font>");
    } else {
        QMessageBox::warning(this, "Ошибка", "Не найден каталог " + currentUbuntuPath);
        return;
    }

    // Step 3: Rename the ubuntu_<versionInput> directory to ubuntu
    QString versionUbuntuPath = PATH_CHROOT + "/ubuntu_" + versionInput;  // Use versionInput instead of hardcoding 20.04
    if (dir.exists(versionUbuntuPath)) {
        if (!dir.rename(versionUbuntuPath, currentUbuntuPath)) {
            QMessageBox::critical(this, "Ошибка", "Не удалось активировать " + versionUbuntuPath);
            return;
        }
        ui->textEdit->append("<font color='green'>Активирована версия: " + versionUbuntuPath + " → ubuntu</font>");
    } else {
        QMessageBox::warning(this, "Ошибка", "Не найден каталог " + versionUbuntuPath);
        return;
    }

    // Final output
    ui->textEdit->append("<font color='green'><b>Готово! Активная версия: ubuntu_" + versionInput + "</b></font>");
}



void MainWindow::checkUbuntuChroot()
{      
    // Run the mount command to check if Ubuntu is mounted
    QProcess process;
    process.start("bash", QStringList() << "-c" << "mount | grep ubuntu | head -n1");
    process.waitForFinished();

    QByteArray output = process.readAllStandardOutput().trimmed();

    if (output.isEmpty()) {
        // The Ubuntu chroot is not mounted
        qDebug() << "$var Пустая";

    } else {
        // The Ubuntu chroot is mounted
        qDebug() << "$var не пустая";
        
        // Show a Qt message box instead of Zenity
        QMessageBox::information(this, "Info", "First, umount a Ubuntu chroot.", QMessageBox::Ok);
    }
}

QString MainWindow::checkVersion()
{
    QProcess process;
    process.start("bash", QStringList() << "-c"
                                        << "cat /compat/ubuntu/etc/os-release 2>/dev/null | "
                                           "grep -E '^(PRETTY_NAME|VERSION)=' | head -1");
    process.waitForFinished(-1);

    QByteArray output = process.readAllStandardOutput().trimmed();
    QString result;

    if (output.isEmpty()) {
        result = "";
    } else {
        QString line = QString(output);
        QString version;

        // Extract the version info
        if (line.startsWith("PRETTY_NAME=")) {
            version = line.mid(13);  // After "PRETTY_NAME="
            version = version.remove('"');  // Remove quotes
            version = version.section(' ', 1, 1);  // Get second word: 16.04.6 / 20.04.6 / 24.04.1
        } else if (line.startsWith("VERSION=")) {
            version = line.mid(9);  // After "VERSION="
            version = version.section(' ', 0, 0);  // Get first word before space or parentheses
            version = version.remove('"');
        }

        // Regex to capture only the main version number (XX.XX)
        QRegularExpression re(R"(\d+\.\d+)");
        QRegularExpressionMatch match = re.match(version);
        if (match.hasMatch()) {
            version = match.captured(0);  // Capture only "16.04", "20.04", "24.04"
        }

        result = version;  // Return the version without extra info
    }

    return result;
}