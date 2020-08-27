#include "view.h"

#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QSqlRelationalDelegate>
#include <QTableView>
#include <QVBoxLayout>

View::View(QAbstractItemModel *pitemModel, QWidget *parent)
    : QMainWindow(parent)
{
    m_pTableView = new QTableView();
    m_pTableView->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(m_pTableView, SIGNAL(customContextMenuRequested(QPoint)),
                SLOT(customMenuRequested(QPoint)));
    m_pTableView->setModel(pitemModel);
    m_pTableView->setItemDelegate(new QSqlRelationalDelegate(m_pTableView));
    m_pTableView->setWindowTitle("STC-test-app");

    setCentralWidget(new QWidget(this));
    QVBoxLayout* pvbxLayout = new QVBoxLayout();
    m_pState = new QLabel("STC-test-app");
    pvbxLayout->addWidget(m_pState);
    pvbxLayout->addWidget(m_pTableView);

    QDialogButtonBox * buttonBox = new QDialogButtonBox(Qt::Horizontal);




    QPushButton* ppb = new QPushButton("Добавить строку");
    connect(ppb, &QPushButton::released, this, &View::needToAddRow);
    buttonBox->addButton(ppb, QDialogButtonBox::ApplyRole);
    ppb = new QPushButton("Отправить");
    connect(ppb, &QPushButton::released, this, &View::sendClicked);
    buttonBox->addButton(ppb, QDialogButtonBox::ResetRole);
    ppb = new QPushButton("Сохранить");
    connect(ppb, &QPushButton::released, this, &View::saveClicked);
    buttonBox->addButton(ppb, QDialogButtonBox::ActionRole);
    pvbxLayout->addWidget(buttonBox);
    centralWidget()->setLayout(pvbxLayout);



}

View::~View()
{
    delete m_pTableView;
}

void View::customMenuRequested(QPoint pos)
{
    QModelIndex index=m_pTableView->indexAt(pos);

    QMenu *menu=new QMenu(this);
    QAction *paction = new QAction("Удалить строку", this);

    connect(paction, &QAction::triggered, [=](){

        emit(needToDeleteRow(index.row()));
    });
    menu->addAction(paction);
    paction = new QAction("Добавить строку", this);

    connect(paction, &QAction::triggered, [=](){
        emit(this->needToAddRow());
    });
    menu->addAction(paction);
    menu->exec(m_pTableView->viewport()->mapToGlobal(pos));
}

void View::changeState(QString state)
{
    m_pState->setText(QString("<H3>%1</H3>").arg(state));
}
