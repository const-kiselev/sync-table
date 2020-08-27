#ifndef NETWORK_H
#define NETWORK_H

#include <QObject>
#include <QTcpSocket>

QT_BEGIN_NAMESPACE
class QTcpServer;
QT_END_NAMESPACE

//! Класс, обеспечивающий взаимодействие приложений по протоколу TCP
class Network : public QObject {
    Q_OBJECT
private:
    //! Состояние объекта
    enum NetworkSate {
        OFF,    /*!< Выключен*/
        Server, /*!< Работает как сервер*/
        Client  /*!< Работает как клиент*/
    }           m_state;
    QTcpServer* m_pTcpServer;
    QTcpSocket* m_pTcpSocket;
    QTcpSocket* m_pClientTcpSocket;
    quint16     m_nNextBlockSize;

private:

    int         initTCPServer();
    int         initTCPSocket();
private slots:
    //! Данный метод-слот срабатывает, когда клиент готов получить пакеты от сервера
    void        client_readyReadFromServer   ();
    //! Данный метод-слот срабатывает, когда сокет инициализировал ошибку соединения
    void        client_getError       (QAbstractSocket::SocketError);
    //! Данный метод-слот срабатывает, когда клиент подключился к серверу
    void        client_ConnectedToServer   ();
    //! Данный метод-слот срабатывает, когда к серверу подключился клиент
virtual
    void        server_newConnection();
    //! Данный метод-слот срабатывает, когда сервер готов получить пакеты от клиента
    void        server_readClient   ();
public:
                Network();
                ~Network();
    //! Возвращает true, если есть возможность подключиться к серверу
    bool        isServerAvailable();
public slots:
    //! Метод-слот для отправки сообщения
    void        send(const QByteArray data);
signals:
    //! Сигнал о полученных данных
    void        dataRecived(QByteArray data);
    void        changeConnectionType(const QString);
};

#endif // NETWORK_H
