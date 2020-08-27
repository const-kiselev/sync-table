#include <QApplication>
#include "view.h"
#include "network.h"
#include "datamodel.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Network network;
    DataModel dataModel;

    {
        QObject::connect(&dataModel, &DataModel::writeData, &network, &Network::send);
        QObject::connect(&network, &Network::dataRecived, &dataModel, &DataModel::readData);

        if(network.isServerAvailable())
        {
            dataModel.initAsClient();
            dataModel.sendRequestForAllData();
        }

        else
            dataModel.initAsServer();
    }
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
