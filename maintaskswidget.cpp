#include "maintaskswidget.h"
#include "todogoal.h"
#include <QSqlRecord>
#include <QSqlError>

#include <QMessageBox>
#include <QTimer>

MainTasksWidget::MainTasksWidget(QWidget* parent)
    : QTabWidget(parent)
{
    m_dataBase = QSqlDatabase::addDatabase("QSQLITE");
    m_dataBase.setDatabaseName("todogoal.db");

    if (!m_dataBase.open()) {
        qDebug() << "Can't open database";
    } else {
        qDebug() << "Database connected!";
        QSqlQuery query;
        query.exec("CREATE TABLE IF NOT EXISTS tasks (id INT PRIMARY KEY AUTOINCREMENT, task VARCHAR(255), deadline VARCHAR(255), status INT);");
        //m_pMainModel->select();
    }

    m_pMainModel = new QSqlTableModel(this);
    m_pMainModel->setTable("tasks");
    m_pMainModel->removeColumn(0);

    m_pMainModel->select();

    readTasks();

    m_pPendingTasksPart = new CTasksPart(m_pMainModel);
    m_pPendingTasksPart->setFilter("2", 2);

    m_pDoneTasksPart = new CTasksPart(m_pMainModel);
    m_pDoneTasksPart->setFilter("1", 2);

    m_pFailedTasksPart = new CTasksPart(m_pMainModel);
    m_pFailedTasksPart->setFilter("0", 2);

    m_pAllTasksPart = new CTasksPart(m_pMainModel);

    addTab(m_pAllTasksPart, tr("Все цели"));
    addTab(m_pPendingTasksPart, tr("В процессе"));
    addTab(m_pDoneTasksPart, tr("Достижения"));
    addTab(m_pFailedTasksPart, tr("Провалено"));
}


void MainTasksWidget::readTasks()
{
    QSqlQuery selectQuery;
    QString selectQueryString = "SELECT * FROM tasks;";
    selectQuery.exec(selectQueryString);

    QSqlRecord rec = selectQuery.record();
    QString task;
    QString deadline;
    int status;
    while (selectQuery.next()) {
        task = selectQuery.value(rec.indexOf("task")).toString();
        deadline = selectQuery.value(rec.indexOf("deadline")).toString();

        Task oneTask(task, deadline);
        // Генерация нового статуса
        status = Task::generateStatus(oneTask);
        oneTask.status = status;
        m_lslTasks.append(oneTask);

        int statusInDataBase = selectQuery.value(rec.indexOf("status")).toInt();
        if (statusInDataBase != Task::DONE && statusInDataBase != status) {
            QSqlQuery updateQuery;
            QString updateQueryString = "UPDATE tasks SET status = :status WHERE "
                                        "task = :task AND "
                                        "deadline = :deadline;";
            updateQuery.prepare(updateQueryString);
            updateQuery.bindValue(":status", status);
            updateQuery.bindValue(":task", task);
            updateQuery.bindValue(":deadline", deadline);
            bool updateResult = updateQuery.exec();
            if (!updateResult) {
                QMessageBox::critical(this, "Error", "Can't read tasks!");
                m_lslTasks.clear();
                return;
            }
            m_pMainModel->select();
        }
    }
}


bool MainTasksWidget::isInDataBase(QString task, QString deadline) {
    auto res = std::find_if(m_lslTasks.begin(), m_lslTasks.end(), [&](const Task& oneTask){
        return task == oneTask.task && deadline == oneTask.deadline;
    });
    if (res != m_lslTasks.end())
        return true;
    return false;
}


void MainTasksWidget::addTaskSlot() {
    NewTaskDialog newTaskDialog;
    if (newTaskDialog.exec() == QDialog::Accepted) {
        if (isInDataBase(newTaskDialog.getDescription(), newTaskDialog.getDeadline())) {
            QMessageBox::information(this, tr("Duplicate"), tr("The task already exists"));
            return;
        }
        QSqlQuery insertQuery;
        QString insertQueryString = "INSERT INTO tasks(task, deadline, status) VALUES('%1', '%2', '%3');";
        bool insertResult = insertQuery.exec(insertQueryString.arg(newTaskDialog.getDescription())
                                                              .arg(newTaskDialog.getDeadline())
                                                              .arg(Task::generateStatus(Task(newTaskDialog.getDescription(), newTaskDialog.getDeadline()))));
        if (!insertResult) {
            qWarning() << "Failed to execute query:" << insertQuery.lastError().text();
            QMessageBox::critical(this, tr("Error"), tr("Can't add new task!"));
            return;
        }
        m_pMainModel->select();
        m_lslTasks.append(Task(newTaskDialog.getDescription(), newTaskDialog.getDeadline()));

        QMessageBox* successMessageBox = new QMessageBox(this);
        successMessageBox->setWindowTitle(tr("Удача!"));
        successMessageBox->setText(tr("Цель добавлена!"));
        successMessageBox->show();

        QTimer::singleShot(3000, successMessageBox, &QMessageBox::close);
    }
}


void MainTasksWidget::removeTaskSlot(){
    QMessageBox::StandardButton confirmDelete;
    confirmDelete = QMessageBox::question(this, tr("Confirmation"), tr("Вы уверены, что хотите удалить цель?"), QMessageBox::Yes | QMessageBox::No);
    if (confirmDelete == QMessageBox::No) {
        return;
    }
    CTasksPart* currentPart = static_cast<CTasksPart*>(currentWidget());
    QTableView* curentView = currentPart->getView();
    QSortFilterProxyModel* currentSortModel = currentPart->getSortModel();

    QItemSelectionModel *selectionModel = curentView->selectionModel();

    const QModelIndexList indexes = selectionModel->selectedRows();

    for(QModelIndex index : indexes){
        QString task = currentSortModel->data(index).toString();
        QString deadline = currentSortModel->data(index.sibling(index.row(), index.column() + 1)).toString();
        QSqlQuery deleteQuery;
        QString deleteQueryString = "DELETE FROM tasks WHERE task = \'" +
                                    task  + "\' AND deadline = \'" +
                                    deadline + "\';";
        bool deleteResult = deleteQuery.exec(deleteQueryString);
        if(!deleteResult){
            QMessageBox::critical(this, tr("Error"), tr("Can't delete a task!"));
            return;
        }
        m_pMainModel->select();

        auto res = std::find_if(m_lslTasks.begin(), m_lslTasks.end(), [&](const Task& second){
             return task == second.task  &&
                    deadline == second.deadline;
        });
        if(res != m_lslTasks.end())
            m_lslTasks.erase(res);
        else{
            QMessageBox::critical(this, tr("Error"), tr("No such task!"));
            qWarning() << "Failed to execute query:" << deleteQuery.lastError().text();
            return;
        }
    }
}


void MainTasksWidget::editTaskSlot(){
    QMessageBox::StandardButton confirmDelete;
    confirmDelete = QMessageBox::question(this, tr("Confirmation"), tr("Вы уверены, что хотите редактировать цель?"), QMessageBox::Yes | QMessageBox::No);
    if (confirmDelete == QMessageBox::No) {
        return;
    }
    CTasksPart* currentPart = static_cast<CTasksPart*>(currentWidget());
    QTableView* curentView = currentPart->getView();
    QSortFilterProxyModel* currentSortModel = currentPart->getSortModel();

    QItemSelectionModel *selectionModel = curentView->selectionModel();

    const QModelIndexList indexes = selectionModel->selectedRows();
    for(QModelIndex index : indexes){
        QString task = currentSortModel->data(index).toString();
        QString deadline = currentSortModel->data(index.sibling(index.row(), index.column() + 1)).toString();

        NewTaskDialog newTaskDialog;
        newTaskDialog.setDialog(task, deadline);
        if(newTaskDialog.exec() == QDialog::Accepted){
            if(!isInDataBase(newTaskDialog.getDescription(), newTaskDialog.getDeadline())){
                int row = index.row();
                m_lslTasks.replace(row, {newTaskDialog.getDescription(), newTaskDialog.getDeadline()});

                int year = deadline.mid(0, deadline.indexOf('-')).toInt();
                int month = deadline.mid(deadline.indexOf('-'), 2).toInt();
                int day = deadline.mid(deadline.lastIndexOf('-') + 1, 2).toInt();
                int newStatus;
                if(QDate(year, month, day) < QDate::currentDate()){
                    newStatus = Task::FAILED;
                }else{
                    newStatus = Task::PENDING;
                }

                QSqlQuery updateQuery;
                QString upfateQueryString = "update tasks set "
                                            "status = \'%1\', "
                                            "task = \'%2\', "
                                            "deadline = \'%3\' where "
                                            "task = \'%4\' and "
                                            "deadline = \'%5\';";
                bool updateResult = updateQuery.exec(upfateQueryString.arg(QString().number(newStatus))
                                                                      .arg(newTaskDialog.getDescription())
                                                                      .arg(newTaskDialog.getDeadline())
                                                                      .arg(task)
                                                                      .arg(deadline));
                if(!updateResult){
                    QMessageBox::critical(this, tr("Error"), tr("Can't update database!"));
                    return;
                }
                m_pMainModel->select();
            }else{
                QMessageBox::information(this, tr("Duplicate"), tr("The task already exists"));
            }
        }
    }
}


void MainTasksWidget::markAsDoneSlot(){
    CTasksPart* currentPart = static_cast<CTasksPart*>(currentWidget());
    QTableView* curentView = currentPart->getView();
    QSortFilterProxyModel* currentSortModel = currentPart->getSortModel();

    QItemSelectionModel *selectionModel = curentView->selectionModel();

    const QModelIndexList indexes = selectionModel->selectedRows();

    for(QModelIndex index : indexes){
        QString task = currentSortModel->data(index).toString();
        QString deadline = currentSortModel->data(index.sibling(index.row(), index.column() + 1)).toString();
        QSqlQuery updateQuery;
        QString updateQueryString = "update tasks set status = " +
                                    QString().number(Task::DONE) + " where " +
                                    "task = '%1' and "
                                    "deadline = '%2';";
        bool updateResult = updateQuery.exec(updateQueryString.arg(task).arg(deadline));
        if(!updateResult){
            QMessageBox::critical(this, tr("Error"), tr("Can't update a task!"));
            return;
        }
        int row = index.row();
        Task updatedTask = m_lslTasks.at(row);
        updatedTask.status = Task::DONE;
        m_lslTasks.replace(row, updatedTask);
        m_pMainModel->select();
    }
}

void MainTasksWidget::saveDatabaseToFile()
{
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save Database"), "", tr("Text Files (*.txt)"));
    if (filePath.isEmpty())
        return;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Can't open file for writing";
        return;
    }

    QTextStream out(&file);

    QString tableName = m_pMainModel->tableName();
    out << "table: " << tableName << "\n";

    int rowCount = m_pMainModel->rowCount();
    //out << rowCount << "\n";

    for (int row = 0; row < rowCount; ++row) {
        QString task = m_pMainModel->index(row, 0).data().toString();
        QString deadline = m_pMainModel->index(row, 1).data().toString();
        int status = m_pMainModel->index(row, 2).data().toInt();

        out << "task: " << task;
        out << "    deadline: " << deadline;
        out << "    status: " << status << "\n";
    }

    file.close();
    qDebug() << "Database saved to file";
}

