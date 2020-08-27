#include "network.h"
#include <QMessageBox>
#include <QTcpServer>
#include <QTcpSocket>

Network::Network(): QObject(0), m_pTcpServer(nullptr),
                                m_pTcpSocket(nullptr),
                                m_pClientTcpSocket(nullptr),
                                m_nNextBlockSize(0),
                                m_state(NetworkSate::OFF)
{
    if(initTCPServer())
        initTCPSocket();
}

Network::~Network()
{
    if(m_pTcpServer)
        delete m_pTcpServer;
    if(m_pTcpSocket)
        delete m_pTcpSocket;
    if(m_pClientTcpSocket)
        delete m_pClientTcpSocket;

}

bool Network::isServerAvailable()
{
    return m_state == NetworkSate::Server ? false : true;
}

void Network::send(const QByteArray data)
{
    if(m_state == NetworkSate::OFF)
        return;
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_2);
    out << quint16(0) << data;

    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    if(m_state == NetworkSate::Server)
    {
        if(m_pClientTcpSocket)
            m_pClientTcpSocket->write(arrBlock);
    }
    else
    {

        if(!m_pTcpSocket->isOpen())
            m_pTcpSocket->connectToHost(QHostAddress::Any, 2324);
        m_pTcpSocket->write(arrBlock);
    }
}

/*virtual*/
void Network::server_newConnection()
{
    if(m_pClientTcpSocket)
        return;
    m_pClientTcpSocket = m_pTcpServer->nextPendingConnection();
    connect(m_pClientTcpSocket, &QTcpSocket::disconnected,[=](){
        m_pClientTcpSocket->deleteLater();
        m_pClientTcpSocket=nullptr;
        emit(changeConnectionType("SERVER. \n"));
    });
    connect(m_pClientTcpSocket, &QTcpSocket::readyRead,
            this,          &Network::server_readClient
           );
    emit(changeConnectionType("SERVER. \nConnected with client"));
}

void Network::server_readClient()
{
    qDebug() << "slotReadClient";
    QDataStream in(m_pClientTcpSocket);
    in.setVersion(QDataStream::Qt_4_2);
    for (;;) {
        if (!m_nNextBlockSize) {
            if (m_pClientTcpSocket->bytesAvailable() < sizeof(quint16)) {
                break;
            }
            in >> m_nNextBlockSize;
        }

        if (m_pClientTcpSocket->bytesAvailable() < m_nNextBlockSize) {
            break;
        }

        m_nNextBlockSize = 0;
        QByteArray arr;
        in >> arr;
        emit(dataRecived(arr));
    }

}

int Network::initTCPServer()
{
    m_pTcpServer = new QTcpServer(this);
    if (!m_pTcpServer->listen(QHostAddress::Any, 2324)) {

        m_pTcpServer->close();
        m_pTcpServer = nullptr;
        m_state = NetworkSate::OFF;
        return 1;
    }
    connect(m_pTcpServer, &QTcpServer::newConnection,
            this,         &Network::server_newConnection
           );
    m_state = NetworkSate::Server;
    emit(changeConnectionType("SERVER. \n"));
    return 0;
}

int Network::initTCPSocket()
{
    if(!m_pTcpSocket)
        m_pTcpSocket = new QTcpSocket(this);

        m_pTcpSocket->connectToHost(QHostAddress::Any, 2324);
        connect(m_pTcpSocket, &QTcpSocket::connected,
                this, &Network::client_ConnectedToServer
                );
        connect(m_pTcpSocket,  &QTcpSocket::readyRead,
                this, &Network::client_readyReadFromServer
                );
        connect(m_pTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
                this,         SLOT(client_getError(QAbstractSocket::SocketError))
               );
    m_state = NetworkSate::Client;
    emit(changeConnectionType("CLIENT. \n"));

    return 0;
}



void Network::client_readyReadFromServer()
{
    QDataStream in(m_pTcpSocket);
    in.setVersion(QDataStream::Qt_4_2);
    for (;;) {
        if (!m_nNextBlockSize) {
            if (m_pTcpSocket->bytesAvailable() < sizeof(quint16)) {
                break;
            }
            in >> m_nNextBlockSize;
        }

        if (m_pTcpSocket->bytesAvailable() < m_nNextBlockSize) {
            break;
        }
        QByteArray str;
        in >> str;
        m_nNextBlockSize = 0;
        emit(dataRecived(str));
    }

}

void Network::client_getError(QAbstractSocket::SocketError err)
{
    m_pTcpSocket->close();
    emit(changeConnectionType("CLIENT. \n"));

}



void Network::client_ConnectedToServer()
{
    emit(changeConnectionType("CLIENT. \nConnected with server"));
}

