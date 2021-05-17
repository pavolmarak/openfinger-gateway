#include <QCoreApplication>
#include <QObject>
#include "openfingergateway.h"

void cleanUp(){
    google::protobuf::ShutdownProtobufLibrary();
}

int main(int argc, char *argv[])
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    QCoreApplication a(argc, argv);
    QObject::connect(&a,&QCoreApplication::aboutToQuit,cleanUp);

    if(argc != 3){
        exit(EXIT_FAILURE);
    }

    QString ip = argv[1];
    quint16 port = QString(argv[2]).toUInt();
    OpenFingerGateway gate(ip,port);

    return a.exec();
}
