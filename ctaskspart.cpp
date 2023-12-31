#include "ctaskspart.h"

CTasksPart::CTasksPart(QAbstractItemModel* mainModel ,QWidget *parent)
    : QWidget(parent)
{
    m_pSortModel = new QSortFilterProxyModel(this);
    m_pSortModel->setSourceModel(mainModel);

    m_pView = new QTableView(this);
    m_pView->setModel(m_pSortModel);
    m_pView->hideColumn(3);
    m_pView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_pView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_pView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_pView);
    setLayout(mainLayout);
}


void CTasksPart::setFilter(const QString pattern, int column){
    m_pSortModel->setFilterFixedString(pattern);
    m_pSortModel->setFilterKeyColumn(column);
}


void CTasksPart::deleteRow(const int row){
    m_pSortModel->removeRow(row);
}


void CTasksPart::setHeaderData(){
    m_pSortModel->setHeaderData(0, Qt::Horizontal, tr("Task"));
    m_pSortModel->setHeaderData(1, Qt::Horizontal, tr("Deadline"));
}
