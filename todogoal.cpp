#include "todogoal.h"

ToDoGoal::ToDoGoal(QWidget *parent)
    : QMainWindow(parent)
{

    m_pMainTasksWidget = new MainTasksWidget(this);
    setCentralWidget(m_pMainTasksWidget);

    m_pAddAction = new QAction(this);
    m_pAddAction->setIcon(QIcon(":/icons/add.png"));

    m_pRemoveAction = new QAction(this);
    m_pRemoveAction->setIcon(QIcon(":/icons/remove.png"));

    m_pEditAction = new QAction(this);
    m_pEditAction->setIcon(QIcon(":/icons/edit.png"));

    m_pMarkAsDoneAction = new QAction(this);
    m_pMarkAsDoneAction->setIcon(QIcon(":/icons/done.png"));

    m_pToolBar = new QToolBar("title", this);
    m_pToolBar->addAction(m_pAddAction);
    m_pToolBar->addAction(m_pRemoveAction);
    m_pToolBar->addAction(m_pEditAction);
    m_pToolBar->addAction(m_pMarkAsDoneAction);
    addToolBar(m_pToolBar);

    QMenuBar* menuBar = new QMenuBar(this);
    m_pMenu = new QMenu(this);
    m_pMenu->setTitle(tr("Меню"));
    m_pMenu->addAction(QIcon(":/icons/en.png"), "Сохранить", m_pMainTasksWidget, &MainTasksWidget::saveDatabaseToFile);
    //m_pMenu->addAction(QIcon(":/icons/de.png"), "Открыть",  m_pMainTasksWidget, &MainTasksWidget::openDatabaseFromFile);
    menuBar->addMenu(m_pMenu);
    setMenuBar(menuBar);

    m_pSaveAction = new QAction(this);
    //m_pLoadAction = new QAction(this);

    // Конекты сигналов и слотов
    connect(m_pAddAction, &QAction::triggered, m_pMainTasksWidget, &MainTasksWidget::addTaskSlot);
    connect(m_pRemoveAction, &QAction::triggered, m_pMainTasksWidget, &MainTasksWidget::removeTaskSlot);
    connect(m_pEditAction, &QAction::triggered, m_pMainTasksWidget, &MainTasksWidget::editTaskSlot);
    connect(m_pMarkAsDoneAction, &QAction::triggered, m_pMainTasksWidget, &MainTasksWidget::markAsDoneSlot);

    connect(m_pSaveAction, &QAction::triggered, m_pMainTasksWidget, &MainTasksWidget::saveDatabaseToFile);
    //connect(m_pLoadAction, &QAction::triggered, m_pMainTasksWidget, &MainTasksWidget::openDatabaseFromFile);
}



ToDoGoal::~ToDoGoal()
{
}
