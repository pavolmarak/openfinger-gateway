#include "sslserver.h"
#include <QDebug>

quint16 SslServer::getPort() const
{
    return port;
}

void SslServer::setPort(quint16 newPort)
{
    port = newPort;
}

const QString &SslServer::getIp() const
{
    return ip;
}

void SslServer::setIp(const QString &newIp)
{
    ip = newIp;
}

SslServer::SslServer(QObject *parent) : QTcpServer(parent)
{

}


void SslServer::startListening()
{
    if(!this->listen(QHostAddress(this->ip), this->port)){
        qDebug() << "Server failed to listen on IP:" << this->ip << "and port:" << this->port ;
        exit(EXIT_FAILURE);
    }
    else{
        qDebug() << "Server is listening on IP:" << this->ip << "and port:" << this->port;
    }
}

void SslServer::incomingConnection(qintptr socketDescriptor)
{
    QSslSocket* ssl_socket= new QSslSocket;
    QMap<QSslSocket*,QByteArray>::iterator i_ssl = this->server_ssl_sockets.insert(ssl_socket,{});

    connect(    i_ssl.key(),
                &QSslSocket::encrypted,
                this,
                &SslServer::encryptedSlot);

    connect(    i_ssl.key(),
                &QSslSocket::readyRead,
                this,
                &SslServer::readyReadSslSlot);

    connect(    i_ssl.key(),
                &QSslSocket::disconnected,
                this,
                &SslServer::disconnectedSslSlot);

    connect(    i_ssl.key(),
                &QSslSocket::errorOccurred,
                this,
                &SslServer::errorOccurredSslSlot);

    connect(    i_ssl.key(),
                qOverload<const QList<QSslError>&>(&QSslSocket::sslErrors),
                this,
                qOverload<const QList<QSslError>&>(&SslServer::sslErrorsSlot));

    if(this->ip == "147.175.106.8"){
        ssl_socket->setPrivateKey("cert/server_key.key");
        ssl_socket->setLocalCertificate("cert/server_cert.crt");
    }
    else{
        ssl_socket->setPrivateKey("cert/client_key.key");
        ssl_socket->setLocalCertificate("cert/client_cert.crt");
    }
    if (ssl_socket->setSocketDescriptor(socketDescriptor)) {
        addPendingConnection(ssl_socket);
        ssl_socket->startServerEncryption();
    } else {
        ssl_socket->close();
        this->server_ssl_sockets.remove(ssl_socket);
        delete ssl_socket;
    }

    qDebug() << "\n[SSL Server: NEW CONNECTION] from"
            << i_ssl.key()->peerAddress()
            << ", socket descriptor:" << i_ssl.key()->socketDescriptor();
    qDebug() << "SSL Server: number of connected SSL sockets is" << this->server_ssl_sockets.size();
}

void SslServer::newConnectionSlot()
{

}

void SslServer::encryptedSlot()
{
    QSslSocket * socket = qobject_cast<QSslSocket*>(sender());
    qDebug() << "Server SSL socket" << socket->socketDescriptor()
             << ": entered the encrypted state.";
}

void SslServer::sslErrorsSlot(const QList<QSslError> &errors)
{
    qDebug() << "SSL Server error:";
    QSslSocket *sender_socket = qobject_cast<QSslSocket*>(sender());
    sender_socket->ignoreSslErrors();
    foreach (const QSslError& e, errors) {
        qDebug() << e.errorString();
    }
}

void SslServer::disconnectedSslSlot()
{
    QSslSocket * socket = qobject_cast<QSslSocket*>(sender());
    qDebug() << "Server SSL socket" << socket->socketDescriptor()
             << ": disconnected.";
    this->server_ssl_sockets.remove(socket);
    qDebug() << "SSL Server: number of connected SSL sockets is" << this->server_ssl_sockets.size();
}

void SslServer::errorOccurredSslSlot(QAbstractSocket::SocketError error)
{
    QSslSocket * sender_socket = qobject_cast<QSslSocket*>(sender());
    qDebug() << "SSL Server" << sender_socket
             << ": socket error occured "
             << error;
}

void SslServer::readyReadSslSlot()
{
    QSslSocket *sender_socket = qobject_cast<QSslSocket*>(sender());
    QByteArray& data = this->server_ssl_sockets[sender_socket];
    emit serverReadyReadSsl(sender_socket, &data);
}
