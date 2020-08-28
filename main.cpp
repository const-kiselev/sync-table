#include <QApplication>
#include "view.h"
#include "network.h"
#include "datamodel.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // Создаем объект, отвечающий за коммуникацию. Происходит определение, будет ли он работать
    // как сервер, или как клиент.
    Network network;
    //Создаем объект, отвечающий за представление данных.
    DataModel dataModel;

    {
        QObject::connect(&dataModel, &DataModel::writeData, &network, &Network::send);
        QObject::connect(&network, &Network::dataRecived, &dataModel, &DataModel::readData);
        // На данном этапе, если нам доступно подключение к серверу,
        // следовательно данный экземпляр приложения — клиент. Необходимо
        // инициализировать объект, отвечающий за данные по правилам клиента
        // и отправить запрос к серверу, чтобы он вернул данные.
        if(network.isServerAvailable())
        {
            dataModel.initAsClient();
            dataModel.sendRequestForAllData();
        }

        else
            dataModel.initAsServer();
    }
    // Создаем объект, отвечающий за представление данных и взаимодействие с пользователеме.
    View view(dataModel.getPTableModel());

    {
        dataModel.getPTableModel()->select();
        QObject::connect(&view, &View::needToAddRow, &dataModel,&DataModel::addRow);
        QObject::connect(&view, &View::needToDeleteRow, &dataModel,&DataModel::deleteRow);
        QObject::connect(&view, &View::sendClicked, &dataModel, &DataModel::sendAllData);
        QObject::connect(&view, &View::saveClicked, &dataModel, &DataModel::saveFile);
        QObject::connect(&network, &Network::changeConnectionType, &view, &View::changeState);
    }
    view.show();

    return a.exec();
}
