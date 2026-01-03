#include "mainwindow.h"

#include <QApplication>
#include <QScreen>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
      // Центрирование окна на экране
    QScreen* screen = QGuiApplication::primaryScreen();
    if (!screen) {
        qWarning("No primary screen found");
        return EXIT_FAILURE;
    }

    const QRect availableGeometry = screen->availableGeometry();
    const QPoint centerPosition = availableGeometry.center() -
                                  QPoint(w.width() / 2, w.height() / 2);

    w.move(centerPosition);  // перемещаем окно в центр экрана
    w.show();
    return a.exec();
}
