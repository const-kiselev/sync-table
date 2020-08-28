#include "datamodel.h"

#include <QMessageBox>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlRecord>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QFileDialog>
#include <QThread>
#include <QSqlRelation>

DataModel::DataModel()
{
    m_pDB = new QSqlDatabase();
    *m_pDB = QSqlDatabase::addDatabase("QSQLITE");
}

DataModel::~DataModel()
{
    m_pDB->close();
    delete m_pTableModel;
    delete m_pDB;
}

void DataModel::initAsServer()
{
    int n = QMessageBox::information(nullptr,"W arning",
    "Привет! Для начала необходимо определиться, будем ли мы создавать новую БД или будем работать с уже существующей."
    "\n Создадим новую БД?",
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    QString dbName;
    if(n == QMessageBox::Yes)
    {
        while(dbName.isNull())
            dbName = QFileDialog::getSaveFileName(nullptr,
                        tr("SQlite database file with person information"), QDir::homePath(),
                        tr("SQlite database file (*.sqlite)"));
        m_pDB->setDatabaseName(dbName);
        m_pDB->open();
        QSqlQuery query(*m_pDB);
        query.exec("create table person (ID integer primary key autoincrement, "
                   "Name varchar(20), Telefon varchar(20), Country int)");
        query.exec("create table country (id integer primary key autoincrement, "
                   "name varchar(20))");
        query.exec("insert into country values(8, 'Russia')");
        query.exec("insert into country values(1, 'USA')");
        query.exec("insert into country values(38, 'Ukraine')");
    }
    else
    {
        while(dbName.isNull())
            dbName = QFileDialog::getOpenFileName(nullptr,
                        tr("SQlite database file with person information"), QDir::homePath(),
                        tr("SQlite database file (*.sqlite)"));
        m_pDB->setDatabaseName(dbName);
        m_pDB->open();
    }



    init();
}

void DataModel::initAsClient()
{

    m_pDB->setDatabaseName(":memory:");
    m_pDB->open();
    QSqlQuery query(*m_pDB);
    query.exec("create table person (ID integer primary key autoincrement, "
               "Name varchar(20), Telefon varchar(20), Country int)");
    query.exec("create table country (id integer primary key autoincrement, "
               "name varchar(20))");
    query.exec("insert into country values(8, 'Russia')");
    query.exec("insert into country values(1, 'USA')");
    query.exec("insert into country values(38, 'Ukraine')");
    init();
}

void DataModel::addRow()
{
    QSqlQuery qu(*m_pDB);
    qu.prepare("INSERT INTO person ( Name, Telefon, Country) "
                  "VALUES ('', '', 1)");
    qu.exec();


    m_pTableModel->select();

    qu.exec("SELECT TOP 1 * FROM Table ORDER BY ID DESC");
    qu.first();

    QJsonObject textObject;
    textObject["action"] = DataModel::ActionType::setElement;
    QJsonArray dataArray;
    QJsonObject jsonRow;
    jsonRow["ID"] = qu.value("ID").toInt();
    jsonRow["Name"] = qu.value("Name").toString();
    jsonRow["Telefon"] = qu.value("Telefon").toString();

    jsonRow["Country"] = qu.value("Country").toInt();

    dataArray.append(jsonRow);

    textObject["data"] = dataArray;
    emit(writeData(QJsonDocument(textObject).toJson(QJsonDocument::Compact)));





//        sendAllData();
}

void DataModel::changeData(int row, QSqlRecord &record)
{
    QJsonObject textObject;
    textObject["action"] = DataModel::ActionType::setElement;
    QJsonArray dataArray;
    QJsonObject jsonRow;
    jsonRow["ID"] = record.value("ID").toInt();
    jsonRow["Name"] = record.value("Name").toString();
    jsonRow["Telefon"] = record.value("Telefon").toString();
    if(record.value("Country").type() == QVariant::Type::String){
        QSqlQuery qu;
        qu.prepare("SELECT id FROM country WHERE name = ?");
        qu.bindValue(0, record.value("Country"));
        qu.exec();
        qu.first();
        jsonRow["Country"] = qu.value(0).toInt();
    }
    else
        jsonRow["Country"] = record.value("Country").toInt();
    dataArray.append(jsonRow);

    textObject["data"] = dataArray;
    emit(writeData(QJsonDocument(textObject).toJson(QJsonDocument::Compact)));
}

void DataModel::deleteRow(int row)
{
    QJsonObject textObject;
    textObject["action"] = DataModel::ActionType::setElement;
    QJsonArray dataArray;
        QJsonObject jsonRow;
        jsonRow["deleteID"] = m_pTableModel->record(row).value("ID").toInt();
        dataArray.append(jsonRow);

    textObject["data"] = dataArray;
    emit(writeData(QJsonDocument(textObject).toJson(QJsonDocument::Compact)));
    m_pTableModel->removeRow(row);
    m_pTableModel->submitAll();
    m_pTableModel->select();
}

void DataModel::readData(QByteArray data)
{
    QJsonDocument jsonDocument(QJsonDocument::fromJson(data));
    QJsonObject jsonObj = jsonDocument.object();
    if(!jsonObj.contains("action"))
        return;

    switch (jsonObj["action"].toInt()) {
    case DataModel::ActionType::getAllData:
        emit(writeData(getDataSQLToJson()));
        break;
    case DataModel::ActionType::setAllData:{
        if(!jsonObj.contains("data"))
            return;
        QSqlQuery qu(*m_pDB);
        qu.exec("DELETE FROM person");

        m_pTableModel->select();



        QJsonArray dataArray = jsonObj["data"].toArray();
        for(int i=0; i<dataArray.count(); i++)
        {
            QJsonObject arrElem = dataArray.at(i).toObject();
            qu.prepare("INSERT INTO person (ID, Name, Telefon, Country) "
                          "VALUES (:i, :nm, :tl, :cntry)");
            qu.bindValue(":i", arrElem["ID"].toInt());
            qu.bindValue(":nm", arrElem["Name"].toString());
            qu.bindValue(":tl", arrElem["Telefon"].toString());
            qu.bindValue(":cntry", arrElem["Country"].toInt());

            qu.exec();

        }
        m_pTableModel->select();

    }
        break;
    case DataModel::ActionType::syncData:
        break;
    case DataModel::ActionType::setElement: {
        if(!jsonObj.contains("data"))
            return;

        //Три варианта: либо это новая строка, либо это изменение, либо удаление
        // 1: ID = нет в базе еще
        // 2: ID = уже существует
        // 3: deleteID:

        QJsonArray dataArray = jsonObj["data"].toArray();
        for(int i=0; i<dataArray.count(); i++)
        {
            QSqlRecord rec;
            QJsonObject arrElem = dataArray.at(i).toObject();
            if(arrElem.contains("deleteID"))
            {
                QSqlQuery query(*m_pDB);
                query.prepare("DELETE FROM person WHERE ID = ?");
                query.bindValue(0,arrElem["deleteID"].toInt());
                query.exec();
            }
            else {
                    QSqlQuery query(*m_pDB);
                    query.prepare("SELECT NAME FROM person WHERE ID = ?");
                    query.bindValue(0,arrElem["ID"].toInt());
                    query.exec();
                    if(query.next())
                    {
                        query.prepare("UPDATE person SET Name = :nm, Telefon=:tl, Country=:cntry WHERE ID = :i");
                        query.bindValue(":nm", arrElem["Name"].toString());
                        query.bindValue(":tl", arrElem["Telefon"].toString());
                        query.bindValue(":cntry", arrElem["Country"].toInt());
                        query.bindValue(":i", arrElem["ID"].toInt());
                        query.exec();
                    }
                    else
                    {
                        query.prepare("INSERT INTO person (ID, Name, Telefon, Country) "
                                      "VALUES (:i, :nm, :tl, :cntry)");
                        query.bindValue(":nm", arrElem["Name"].toString());
                        query.bindValue(":tl", arrElem["Telefon"].toString());
                        query.bindValue(":cntry", arrElem["Country"].toInt());
                        query.bindValue(":i", arrElem["ID"].toInt());
                        query.exec();
                    }

            }
        m_pTableModel->select();
        }
    }
        break;

    }
}

void DataModel::sendRequestForAllData()
{
    QJsonObject textObject;
    textObject["action"] = DataModel::ActionType::getAllData;
    emit writeData(QJsonDocument(textObject).toJson(QJsonDocument::Compact));
}

void DataModel::sendAllData()
{
    emit(writeData(getDataSQLToJson()));
}


void DataModel::saveFile()
{
    QString fileName = QFileDialog::getSaveFileName(nullptr,
            tr("Save Person Base"), QDir::homePath(),
            tr("Persons JSON file (*.json)"));
    QFile saveFile(fileName);

       if (!saveFile.open(QIODevice::WriteOnly)) {
           qWarning("Couldn't open save file.");
           return;
       }
       saveFile.write(DataModel::getDataSQLToJson());
}




QByteArray DataModel::getDataSQLToJson()
{
    QJsonObject textObject;
    textObject["action"] = DataModel::ActionType::setAllData;
    QJsonArray dataArray;
    for (int i = 0; i < m_pTableModel->rowCount(); ++i) {
        QJsonObject row;

        row["ID"] = m_pTableModel->record(i).value("ID").toInt();
        row["Name"] = m_pTableModel->record(i).value("Name").toString();
        row["Telefon"] = m_pTableModel->record(i).value("Telefon").toString();
        QSqlQuery q(*m_pDB);
        q.prepare("SELECT Country FROM person WHERE ID = ?");
        q.bindValue(0, m_pTableModel->record(i).value("ID").toInt());
        q.exec();
        q.next();
        row["Country"] = q.value(0).toInt();
        dataArray.append(row);
    }
    textObject["data"] = dataArray;
    return QJsonDocument(textObject).toJson(QJsonDocument::Compact);
}

void DataModel::init()
{
    m_pTableModel = new QSqlRelationalTableModel(nullptr, *m_pDB);
    m_pTableModel->setTable("person");

    m_pTableModel->setEditStrategy(QSqlTableModel::OnFieldChange);
    m_pTableModel->select();

    m_pTableModel->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    m_pTableModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Name"));
    m_pTableModel->setHeaderData(2, Qt::Horizontal, QObject::tr("Telefon"));
    m_pTableModel->setHeaderData(3, Qt::Horizontal, QObject::tr("Country"));
    ((QSqlRelationalTableModel*)m_pTableModel)->setRelation(3, QSqlRelation("country", "id", "name"));

    connect(m_pTableModel, SIGNAL(beforeUpdate(int, QSqlRecord&)), this, SLOT(changeData(int, QSqlRecord&)));
}

QSqlTableModel *DataModel::getPTableModel() const
{
    return m_pTableModel;
}
