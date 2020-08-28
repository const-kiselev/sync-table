#ifndef VIEW_H
#define VIEW_H


#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QAbstractItemModel;
class QTableView;
class QLabel;
QT_END_NAMESPACE

class View : public QMainWindow
{
    Q_OBJECT
private:
    QTableView* m_pTableView;
    QLabel*     m_pState;
private slots:
    void        customMenuRequested(QPoint pos);
public:
                View(QAbstractItemModel* pitemModel, QWidget *parent = nullptr);
                ~View();
public slots:
    void        changeState(QString state);
signals:
    void        needToAddRow();
    void        needToDeleteRow(int row);
    void        sendClicked();
    void        saveClicked();
};

#endif // VIEW_H
