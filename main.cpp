#include "todogoal.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ToDoGoal w;
    w.resize(425, 600);
    w.show();

    return a.exec();
}
