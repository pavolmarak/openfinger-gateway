#ifndef OPENFINGERGATEWAY_H
#define OPENFINGERGATEWAY_H

#include <QObject>
#include <QSslSocket>
#include <QMap>

// OpenFinger protobufs
#include "Level2.pb.h"
#include "Fingerprint.pb.h"
#include "Wrapper.pb.h"
#include "PreprocessingRequest.pb.h"
#include "PreprocessingResponse.pb.h"
#include "ExtractionRequest.pb.h"
#include "ExtractionResponse.pb.h"
#include "RegistrationRequest.pb.h"
#include "RegistrationResponse.pb.h"
#include "VerificationRequest.pb.h"
#include "VerificationResponse.pb.h"
#include "VerificationRequestOlejarnikova.pb.h"
#include "VerificationResponseOlejarnikova.pb.h"
#include "IdentificationRequest.pb.h"
#include "IdentificationResponse.pb.h"

// OpenFinger library
#include "preprocessing.h"
#include "extraction.h"
#include "matcher.h"

// Task handlers
#include "preprocessingtask.h"
#include "extractiontask.h"
#include "registrationtask.h"
#include "verificationtask.h"
#include "identificationtask.h"

// SSL server
#include "sslserver.h"

class OpenFingerGateway : public QObject
{
    Q_OBJECT
public:
    explicit OpenFingerGateway(QObject *parent = nullptr);
    OpenFingerGateway(QString host, quint16 port);
    void readFromClient(QSslSocket*, QByteArray*);
private:
    QString host;
    quint16 port;
    SslServer ssl_server;

    PreprocessingTask preproc_task;
    ExtractionTask extract_task;
    VerificationTask verify_task;
    RegistrationTask register_task;
    IdentificationTask identify_task;

    void handlePreprocessingRequest(OpenFinger::Wrapper&, QSslSocket* socket, QByteArray& data);
    void handleExtractionRequest(OpenFinger::Wrapper&, QSslSocket* socket, QByteArray& data);
    void handleVerificationRequestRemoteDB(OpenFinger::Wrapper&, QSslSocket* socket, QByteArray& data);
    void handleVerificationRequestLocalDB(OpenFinger::Wrapper&, QSslSocket* socket, QByteArray& data);
    void handleRegistrationRequest(OpenFinger::Wrapper&, QSslSocket* socket, QByteArray& data);
    void handleIdentificationRequest(OpenFinger::Wrapper&, QSslSocket* socket, QByteArray& data);


signals:
private slots:
    void serverReadyReadSslSlot(QSslSocket *socket, QByteArray *data);
    void preprocessingResponseReadySlot(OpenFinger::PreprocessingResponse& response, QSslSocket * socket);
    void extractionResponseReadySlot(OpenFinger::ExtractionResponse& response, QSslSocket * socket);
    void verificationResponseReadySlot(OpenFinger::VerificationResponse response, QSslSocket * socket);
    void verificationResponseReadySlot(OpenFinger::VerificationResponseOlejarnikova response, QSslSocket * socket);
    void registrationResponseReadySlot(OpenFinger::RegistrationResponse& response, QSslSocket * socket);
    void identificationResponseReadySlot(OpenFinger::IdentificationResponse& response, QSslSocket * socket);

};

#endif // OPENFINGERGATEWAY_H
