#ifndef DATAMODEL_H
#define DATAMODEL_H

#include <QSqlTableModel>

class DataModel: public QObject
{
    Q_OBJECT
private:
    enum ActionType {
        getAllData,
        setAllData,
        setElement,
        syncData
    };

    QSqlDatabase*   m_pDB;
    QSqlTableModel* m_pTableModel;

private:
    QByteArray      getDataSQLToJson();
    void            init();
private slots:
    void            changeData(int row, QSqlRecord & record);
public:
                    DataModel();
                    ~DataModel();
    void            initAsServer();
    void            initAsClient();
    QSqlTableModel* getPTableModel() const;

public slots:
    void            addRow();
    void            deleteRow(int row);
    void            readData(QByteArray data);
    void            sendRequestForAllData();
    void            sendAllData();

    void            saveFile();
signals:
    void            writeData(QByteArray data);
};

#endif // DATAMODEL_H
