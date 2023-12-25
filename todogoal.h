#ifndef TODOGOAL_H
#define TODOGOAL_H

#include <QMainWindow>
#include <QIcon>

#include "maintaskswidget.h"

class ToDoGoal : public QMainWindow
{
    Q_OBJECT
private:
    MainTasksWidget* m_pMainTasksWidget;
    QToolBar* m_pToolBar;
    QAction* m_pAddAction;
    QAction* m_pRemoveAction;
    QAction* m_pEditAction;
    QAction* m_pMarkAsDoneAction;
    QAction* m_pSaveAction;
   // QAction* m_pLoadAction;

    QMenu* m_pMenu;

public:
    ToDoGoal(QWidget *parent = nullptr);
    ~ToDoGoal();

private slots:

};
#endif // TODOGOAL_H
