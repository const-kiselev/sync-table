#ifndef DATAMODEL_H
#define DATAMODEL_H

#include <QSqlRelationalTableModel>

//! Класс, отвечающий за обработку и хранение данных
/*!
    Хранение данных зависит от типа инициализации объекта.
    Для клиента создается базаданных SQLite в памяти.
    При инициализации как сервер пользователю предоставляется выбор:
    создать новый файл БД или использовать существующий. Далее, к этому
    подключаемся как к БД SQLite.

    Работа с данными происходит через класс QSqlRelationalTableModel/QSqlTableModel,
    который предоставлет возможность свзяать данную модель данных с представлением
    и отслеживать все события, инициированные пользователем, для их последующей
    обраотки (пересылки клиенту/серверу; сохранению).
*/
class DataModel: public QObject
{
    Q_OBJECT
private:
    //! Тип отправляемого действия
    enum ActionType {
        getAllData, /*!< Запрос на получение всех данных (верни мне все данные)*/
        setAllData, /*!< Установить все элементы из массива data*/
        setElement, /*!< Установить отдельный элемент*/
        syncData    /*!< Синхронизация (на будущее, не реализовано)*/
    };

    QSqlDatabase*   m_pDB;
    QSqlRelationalTableModel* m_pTableModel;

private:
    //! Метод, для получения массива со всеми данными из БД
    QByteArray      getDataSQLToJson();
    void            init();
private slots:
    //! Данный метод-слот срабатывает, когда в модели данных изменились данные
    //! (не относится к удалению и добавлению строк!!!)
    void            changeData(int row, QSqlRecord & record);
public:
                    DataModel();
                    ~DataModel();
    //! Инициализация модели данных, для серверного решения
    void            initAsServer();
    //! Инициализация модели данных, для клиентского решения
    void            initAsClient();
    QSqlRelationalTableModel* getPTableModel() const;

public slots:
    //! отвечает за добавление новой строки в конец таблицы
    void            addRow();
    void            deleteRow(int row);
    //! отвечает за чтение полученных данных и их обработку
    void            readData(QByteArray data);
    //! отправляет запрос на получение всех данных
    void            sendRequestForAllData();
    //! отправляет все данные
    void            sendAllData();
    //! сохраняет БД в JSON формате в текстовый файл
    void            saveFile();
signals:
    //! сигнал, отправляющий данные для пересылки клиенту/серверу
    void            writeData(QByteArray data);
};

#endif // DATAMODEL_H
