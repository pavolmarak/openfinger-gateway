#ifndef SSLSERVER_H
#define SSLSERVER_H

#include <QTcpServer>
#include <QSslSocket>
#include <QObject>

class SslServer : public QTcpServer
{
    Q_OBJECT
    QString ip;
    quint16 port;
    QMap<QSslSocket*,QByteArray> server_ssl_sockets;
    //QMap<QTcpSocket*,QByteArray> server_sockets;
public:
    explicit SslServer(QObject *parent = nullptr);
    void startListening();
    quint16 getPort() const;
    void setPort(quint16 newPort);

    const QString &getIp() const;
    void setIp(const QString &newIp);

protected:
    void incomingConnection(qintptr socketDescriptor) override;
private slots:
    void newConnectionSlot();
    void encryptedSlot();
    void sslErrorsSlot(const QList<QSslError> &errors);
    void disconnectedSslSlot();
    void errorOccurredSslSlot(QAbstractSocket::SocketError error);
    void readyReadSslSlot();

signals:
    void serverReadyReadSsl(QSslSocket*, QByteArray*);
};

#endif // SSLSERVER_H
