#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QProcess>
#include <QTextStream>
#include <QDebug>
#include <QRegularExpression>
#include <QDir>
#include <QMessageBox>
#include <QCoreApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)

{
    ui->setupUi(this);
    checkUbuntuChroot2();
}

MainWindow::~MainWindow()
{
 delete ui;
}

void MainWindow::on_pushUbuntu16_clicked() {

    switchToVersion("16.04");  // Для активации Ubuntu 16.04    
    currentUbuntuVersion = "16.04";
    ui->textEdit->append("Выбрана Ubuntu 16.04");
}

void MainWindow::on_pushUbuntu20_clicked() {

    switchToVersion("20.04");  // Для активации Ubuntu 20.04
    currentUbuntuVersion = "20.04";
    ui->textEdit->append("Выбрана Ubuntu 20.04");
}

void MainWindow::on_pushUbuntu24_clicked() {

    switchToVersion("24.04");  // Для активации Ubuntu 24.04
    currentUbuntuVersion = "24.04";
    ui->textEdit->append("Выбрана Ubuntu 24.04");
}

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
    checkUbuntuChroot2();
}

void MainWindow::switchToVersion(const QString &versionInput)
{

     if (checkUbuntuChroot()) {
        // Если checkUbuntuChroot вернул true (chroot смонтирован), прекращаем выполнение
        return;
    }

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

bool MainWindow::checkUbuntuChroot()
{
    // Запускаем команду для проверки, смонтирован ли Ubuntu chroot
    QProcess process;
    process.start("bash", QStringList() << "-c" << "mount | grep ubuntu | head -n1");
    process.waitForFinished();

    QByteArray output = process.readAllStandardOutput().trimmed();

    if (output.isEmpty()) {
        // Если Ubuntu chroot не смонтирован
        qDebug() << "$var Пустая";
        return false;  // Возвращаем false, если chroot не смонтирован
    } else {
        // Если Ubuntu chroot смонтирован
        qDebug() << "$var не пустая";

        // Показываем сообщение о том, что chroot смонтирован
        QMessageBox::information(this, "Info", "First, umount a Ubuntu chroot.", QMessageBox::Ok);

        return true;  // Возвращаем true, если chroot смонтирован
    }
}


bool MainWindow::checkUbuntuChroot2()
{
    // Запускаем команду для проверки, смонтирован ли Ubuntu chroot
    QProcess process;
    process.start("bash", QStringList() << "-c" << "mount | grep ubuntu | head -n1");
    process.waitForFinished();

    QByteArray output = process.readAllStandardOutput().trimmed();

    QString version = checkVersion();  // Get Ubuntu version from checkVersion()

    if (output.isEmpty()) {
        // Если Ubuntu chroot не смонтирован
        if (!version.isEmpty()) {
            ui->textEdit->append("Ubuntu " + version + " не смонтирована");
        } else {
            ui->textEdit->append("Ubuntu не смонтирована");
        }
        return false;  // Возвращаем false, если chroot не смонтирован
    } else {
        // Если Ubuntu chroot смонтирован
        if (!version.isEmpty()) {
            ui->textEdit->append("Ubuntu " + version + " смонтирована");
        } else {
            ui->textEdit->append("Ubuntu " + version + " не смонтирована");
        }
        return true;  // Возвращаем true, если chroot смонтирован
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

void MainWindow::on_pushStart_clicked()
{
    // Создаём процесс
    QProcess *process = new QProcess(this);

    // Подготавливаем команду
    QStringList arguments;
    arguments << "service" << "ubuntu" << "onestart";  // Замените на вашу команду

    // Запускаем процесс
    process->start("doas", arguments);

    // Проверка на успешный старт процесса
    if (!process->waitForStarted()) {
        qDebug() << "Ошибка при запуске команды!";
        QMessageBox::critical(this, "Ошибка", "Не удалось запустить команду.");
        return;
    }

    // Подключаем обработку ошибок
    connect(process, &QProcess::readyReadStandardError, [process]() {
        QByteArray errorOutput = process->readAllStandardError();
        qDebug() << "Ошибка:" << errorOutput;
    });

    // Проверка на успешное завершение процесса
    if (!process->waitForFinished()) {
        qDebug() << "Ошибка при завершении команды!";
                QMessageBox::critical(this, "Ошибка", "Не удалось запустить команду.");
    } else {
        qDebug() << "Команда успешно запущена!";
        checkUbuntuChroot2();
    }
}

void MainWindow::on_pushStop_clicked()
{
    // Создаём процесс
    QProcess *process = new QProcess(this);

    // Подготавливаем команду
    QStringList arguments;
    arguments << "service" << "ubuntu" << "onestop";  // Замените на вашу команду

    // Запускаем процесс
    process->start("doas", arguments);

    // Проверка на успешный старт процесса
    if (!process->waitForStarted()) {
        qDebug() << "Ошибка при запуске команды!";
        QMessageBox::critical(this, "Ошибка", "Не удалось запустить команду.");
        return;
    }

    // Подключаем обработку ошибок
    connect(process, &QProcess::readyReadStandardError, [process]() {
        QByteArray errorOutput = process->readAllStandardError();
        qDebug() << "Ошибка:" << errorOutput;
    });

    // Проверка на успешное завершение процесса
    if (!process->waitForFinished()) {
        qDebug() << "Ошибка при завершении команды!";
        QMessageBox::critical(this, "Ошибка", "Команда не завершена корректно.");
    } else {
        qDebug() << "Команда успешно остановлена!";
        checkUbuntuChroot2();
    }
}
