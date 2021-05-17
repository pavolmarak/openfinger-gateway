#ifndef OPENFINGERGATEWAY_H
#define OPENFINGERGATEWAY_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
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

class OpenFingerGateway : public QObject
{
    Q_OBJECT
public:
    explicit OpenFingerGateway(QObject *parent = nullptr);
    OpenFingerGateway(QString host, quint16 port);
private:
    QTcpServer server;
    QMap<QTcpSocket*,QByteArray> server_sockets;

    PreprocessingTask preproc_task;
    ExtractionTask extract_task;
    VerificationTask verify_task;
    RegistrationTask register_task;
    IdentificationTask identify_task;

    void handlePreprocessingRequest(OpenFinger::Wrapper&, QTcpSocket* socket, QByteArray& data);
    void handleExtractionRequest(OpenFinger::Wrapper&, QTcpSocket* socket, QByteArray& data);
    void handleVerificationRequest(OpenFinger::Wrapper&, QTcpSocket* socket, QByteArray& data);
    void handleRegistrationRequest(OpenFinger::Wrapper&, QTcpSocket* socket, QByteArray& data);
    void handleIdentificationRequest(OpenFinger::Wrapper&, QTcpSocket* socket, QByteArray& data);


signals:
private slots:
    void newConnectionSlot();
    void readyReadSlot();
    void disconnectedSlot();
    void errorOccurredSlot(QAbstractSocket::SocketError);
    void preprocessingResponseReadySlot(OpenFinger::PreprocessingResponse& response, QTcpSocket * socket);
    void extractionResponseReadySlot(OpenFinger::ExtractionResponse& response, QTcpSocket * socket);
    void verificationResponseReadySlot(OpenFinger::VerificationResponse& response, QTcpSocket * socket);
    void registrationResponseReadySlot(OpenFinger::RegistrationResponse& response, QTcpSocket * socket);
    void identificationResponseReadySlot(OpenFinger::IdentificationResponse& response, QTcpSocket * socket);

};

#endif // OPENFINGERGATEWAY_H
