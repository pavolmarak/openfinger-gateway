#include "openfingergateway.h"

OpenFingerGateway::OpenFingerGateway(QObject *parent) : QObject(parent)
{

}

OpenFingerGateway::OpenFingerGateway(QString host, quint16 port){
    connect(    &(this->server),
                &QTcpServer::newConnection,
                this,
                &OpenFingerGateway::newConnectionSlot);

    connect(    &(this->preproc_task),
                &PreprocessingTask::preprocessingResponseReady,
                this,
                &OpenFingerGateway::preprocessingResponseReadySlot);

    connect(    &(this->extract_task),
                &ExtractionTask::extractionResponseReady,
                this,
                &OpenFingerGateway::extractionResponseReadySlot);

    connect(    &(this->verify_task),
                qOverload<OpenFinger::VerificationResponse, QTcpSocket *>(&VerificationTask::verificationResponseReady),
                this,
                qOverload<OpenFinger::VerificationResponse, QTcpSocket *>(&OpenFingerGateway::verificationResponseReadySlot));

    connect(    &(this->verify_task),
                qOverload<OpenFinger::VerificationResponseOlejarnikova, QTcpSocket *>(&VerificationTask::verificationResponseReady),
                this,
                qOverload<OpenFinger::VerificationResponseOlejarnikova, QTcpSocket *>(&OpenFingerGateway::verificationResponseReadySlot));

    connect(    &(this->register_task),
                &RegistrationTask::registrationResponseReady,
                this,
                &OpenFingerGateway::registrationResponseReadySlot);

    connect(    &(this->identify_task),
                &IdentificationTask::identificationResponseReady,
                this,
                &OpenFingerGateway::identificationResponseReadySlot);

    if(!this->server.listen(QHostAddress(host), port)){
        qDebug() << "Server failed to listen on IP:" << host << "and port:" << port ;
        exit(EXIT_FAILURE);
    }
    else{
        qDebug() << "Server is listening on IP:" << host << "and port:" << port;
    }
}

void OpenFingerGateway::newConnectionSlot()
{
    QMap<QTcpSocket*,QByteArray>::iterator i = this->server_sockets.insert(this->server.nextPendingConnection(),{});
    connect(    i.key(),
                &QTcpSocket::readyRead,
                this,
                &OpenFingerGateway::readyReadSlot);

    connect(    i.key(),
                &QTcpSocket::disconnected,
                this,
                &OpenFingerGateway::disconnectedSlot);

    connect(    i.key(),
                &QTcpSocket::errorOccurred,
                this,
                &OpenFingerGateway::errorOccurredSlot);

    qDebug() << "\nServer: NEW CONNECTION" << i.key()
             << "from" << i.key()->peerAddress()
             << ", socket descriptor:" << i.key()->socketDescriptor();
    qDebug() << "Server: number of connected sockets is" << this->server_sockets.size();
}

void OpenFingerGateway::disconnectedSlot()
{
    QTcpSocket * sender_socket = qobject_cast<QTcpSocket*>(sender());
    qDebug() << "Server" << sender_socket
             << ": disconnected.";
    this->server_sockets.remove(sender_socket);
    qDebug() << "Server: number of connected sockets is" << this->server_sockets.size();
}

void OpenFingerGateway::errorOccurredSlot(QAbstractSocket::SocketError error)
{
    QTcpSocket * sender_socket = qobject_cast<QTcpSocket*>(sender());
    qDebug() << "Server" << sender_socket
             << ": error occured "
             << error;
}

void OpenFingerGateway::readyReadSlot()
{
    QTcpSocket * sender_socket = qobject_cast<QTcpSocket*>(sender());
    QByteArray& data = this->server_sockets[sender_socket];
    QByteArray tmp = sender_socket->readAll();
    data += tmp;
    qDebug()    << "Server" << sender_socket
                << ": read" << tmp.size() << "B from"
                << sender_socket->peerAddress().toString();

    OpenFinger::Wrapper wrap;
    bool parse_status = wrap.ParseFromString(data.toStdString());

    if(parse_status){
        qDebug() << "Server" << sender_socket
                 << ": wrapper parsed successfully.";
        switch(wrap.body_case()){
        case OpenFinger::Wrapper::kPreprocRequest:
            handlePreprocessingRequest(wrap, sender_socket, data);
            break;
        case OpenFinger::Wrapper::kPreprocResponse:
            break;
        case OpenFinger::Wrapper::kExtractRequest:
            handleExtractionRequest(wrap, sender_socket, data);
            break;
        case OpenFinger::Wrapper::kExtractResponse:
            break;
        case OpenFinger::Wrapper::kVerifyRequest:
            handleVerificationRequestRemoteDB(wrap, sender_socket, data);
            break;
        case OpenFinger::Wrapper::kVerifyResponse:
            break;
        case OpenFinger::Wrapper::kVerifyRequestOlejarnikova:
            handleVerificationRequestLocalDB(wrap, sender_socket, data);
            break;
        case OpenFinger::Wrapper::kVerifyResponseOlejarnikova:
            break;
        case OpenFinger::Wrapper::kIdentifyRequest:
            handleIdentificationRequest(wrap, sender_socket, data);
            break;
        case OpenFinger::Wrapper::kIdentifyResponse:
            break;
        case OpenFinger::Wrapper::kRegisterRequest:
            handleRegistrationRequest(wrap, sender_socket, data);
            break;
        case OpenFinger::Wrapper::kRegisterResponse:
            break;
        default:
            break;
        }
        data.clear();
    }
}

void OpenFingerGateway::handlePreprocessingRequest(OpenFinger::Wrapper& wrap, QTcpSocket* socket, QByteArray& data)
{
    this->preproc_task.request = wrap.preproc_request();
    this->preproc_task.socket = socket;

    qDebug() << "Server" << socket
             << ": preprocessing request received, size:" << data.size() << "bytes";
    qDebug() << "Server" << socket
             << ": image to be processed:"
             << this->preproc_task.request.fingerprint().width() << "(W)"
             << this->preproc_task.request.fingerprint().height() << "(H)"
             << this->preproc_task.request.fingerprint().resolution() << "(ppi)"
             << this->preproc_task.request.fingerprint().color() << "(0-grayscale, 1-rgb)"
             << this->preproc_task.request.fingerprint().data().size() << "bytes";
    qDebug() << "Server" << socket
             << ": preprocessing params:"
             << "\nblock_size:" << this->preproc_task.request.params().block_size()
             << "\ngabor_lambda:" << this->preproc_task.request.params().gabor_lambda()
             << "\ngabor_sigma:" << this->preproc_task.request.params().gabor_sigma();
    qDebug() << "Server" << socket
             << ": preprocessing request processing started.";

    this->preproc_task.start();
}

void OpenFingerGateway::handleExtractionRequest(OpenFinger::Wrapper& wrap, QTcpSocket* socket, QByteArray& data)
{
    this->extract_task.request = wrap.extract_request();
    this->extract_task.socket = socket;

    qDebug() << "Server" << socket
             << ": extraction request received, size:" << data.size() << "bytes";
    qDebug() << "Server" << socket
             << ": image to be processed:"
             << this->extract_task.request.fingerprint().width() << "(W)"
             << this->extract_task.request.fingerprint().height() << "(H)"
             << this->extract_task.request.fingerprint().resolution() << "(ppi)"
             << this->extract_task.request.fingerprint().color() << "(0-grayscale, 1-rgb)"
             << this->extract_task.request.fingerprint().data().size() << "bytes";
    qDebug() << "Server" << socket
             << ": extraction request processing started.";

    this->extract_task.start();
}

void OpenFingerGateway::handleVerificationRequestRemoteDB(OpenFinger::Wrapper &wrap, QTcpSocket *socket, QByteArray &data)
{
    this->verify_task.requestRemoteDB = wrap.verify_request();
    this->verify_task.socket = socket;

    qDebug() << "Server" << socket
             << ": verification (remote DB) request received, size:" << data.size() << "bytes";
    qDebug() << "Server" << socket
             << ": image to be processed:"
             << this->verify_task.requestRemoteDB.fingerprint().width() << "(W)"
             << this->verify_task.requestRemoteDB.fingerprint().height() << "(H)"
             << this->verify_task.requestRemoteDB.fingerprint().resolution() << "(ppi)"
             << this->verify_task.requestRemoteDB.fingerprint().color() << "(0-grayscale, 1-rgb)"
             << this->verify_task.requestRemoteDB.fingerprint().data().size() << "bytes";
    qDebug() << "Server" << socket
             << ": number of Level-2 vectors to match is:"
             << this->verify_task.requestRemoteDB.level2vectors_size();
    qDebug() << "Server" << socket
             << ": verification request processing started.";

    this->verify_task.startRemoteDB();
}

void OpenFingerGateway::handleVerificationRequestLocalDB(OpenFinger::Wrapper &wrap, QTcpSocket *socket, QByteArray &data)
{
    this->verify_task.requestLocalDB = wrap.verify_request_olejarnikova();
    this->verify_task.socket = socket;

    qDebug() << "Server" << socket
             << ": verification (local DB) request received, size:" << data.size() << "bytes";
    qDebug() << "Server" << socket
             << ": image to be processed:"
             << this->verify_task.requestLocalDB.fingerprint().width() << "(W)"
             << this->verify_task.requestLocalDB.fingerprint().height() << "(H)"
             << this->verify_task.requestLocalDB.fingerprint().resolution() << "(ppi)"
             << this->verify_task.requestLocalDB.fingerprint().color() << "(0-grayscale, 1-rgb)"
             << this->verify_task.requestLocalDB.fingerprint().data().size() << "bytes";
    qDebug() << "Server" << socket
             << ": claimed identity is:"
             << this->verify_task.requestLocalDB.login().data();
    qDebug() << "Server" << socket
             << ": verification request processing started.";

    this->verify_task.startLocalDB();
}

void OpenFingerGateway::handleIdentificationRequest(OpenFinger::Wrapper &wrap, QTcpSocket *socket, QByteArray &data)
{
    this->identify_task.request = wrap.identify_request();
    this->identify_task.socket = socket;

    qDebug() << "Server" << socket
             << ": identification request received, size:" << data.size() << "bytes";
    qDebug() << "Server" << socket
             << ": image to be processed:"
             << this->identify_task.request.fingerprint().width() << "(W)"
             << this->identify_task.request.fingerprint().height() << "(H)"
             << this->identify_task.request.fingerprint().resolution() << "(ppi)"
             << this->identify_task.request.fingerprint().color() << "(0-grayscale, 1-rgb)"
             << this->identify_task.request.fingerprint().data().size() << "bytes";
    qDebug() << "Server" << socket
             << ": number of Level-2 vectors to match is:"
             << this->identify_task.request.vectors_size();
    qDebug() << "Server" << socket
             << ": identification request processing started.";

    this->identify_task.start();
}

void OpenFingerGateway::handleRegistrationRequest(OpenFinger::Wrapper &wrap, QTcpSocket *socket, QByteArray &data)
{
    this->register_task.request = wrap.register_request();
    this->register_task.socket = socket;

    qDebug() << "Server" << socket
             << ": registration request received, size:" << data.size() << "bytes";
    qDebug() << "Server" << socket
             << ": username to be registered:"
             << this->register_task.request.login().data();
    qDebug() << "Server" << socket
             << ": image to be registered:"
             << this->register_task.request.fingerprint().width() << "(W)"
             << this->register_task.request.fingerprint().height() << "(H)"
             << this->register_task.request.fingerprint().resolution() << "(ppi)"
             << this->register_task.request.fingerprint().color() << "(0-grayscale, 1-rgb)"
             << this->register_task.request.fingerprint().data().size() << "bytes";
    qDebug() << "Server" << socket
             << ": registration request processing started.";

    this->register_task.start();
}

void OpenFingerGateway::preprocessingResponseReadySlot(OpenFinger::PreprocessingResponse &response, QTcpSocket *socket)
{
    std::string str;
    OpenFinger::Wrapper wrap;
    wrap.set_header(OpenFinger::Wrapper_Header_PREPROCESSING_RESPONSE);
    OpenFinger::PreprocessingResponse * resp = wrap.mutable_preproc_response();
    *resp = response;
    wrap.SerializeToString(&str);
    qDebug()   << "Server" << socket
               << ": preprocessing request processed successfully.";
    qDebug()   << "Server" << socket
               << ": preprocessing response sent ("
                << socket->write(QByteArray::fromStdString(str))
                << "B)";
}

void OpenFingerGateway::extractionResponseReadySlot(OpenFinger::ExtractionResponse &response, QTcpSocket *socket)
{
    std::string str;
    OpenFinger::Wrapper wrap;
    wrap.set_header(OpenFinger::Wrapper_Header_EXTRACTION_RESPONSE);
    OpenFinger::ExtractionResponse * resp = wrap.mutable_extract_response();
    *resp = response;
    wrap.SerializeToString(&str);
    qDebug()   << "Server" << socket
               << ": extraction request processed successfully.";
    qDebug()   << "Server" << socket
               << ": extraction response sent ("
               << socket->write(QByteArray::fromStdString(str))
               << "B) ..." << resp->level2vector().level2vector_size()
               << "Level-2 features extracted.";
}

void OpenFingerGateway::verificationResponseReadySlot(OpenFinger::VerificationResponse response, QTcpSocket *socket)
{
    std::string str;
    OpenFinger::Wrapper wrap;
    wrap.set_header(OpenFinger::Wrapper_Header_VERIFICATION_RESPONSE);
    OpenFinger::VerificationResponse * resp = wrap.mutable_verify_response();
    *resp = response;
    wrap.SerializeToString(&str);
    qDebug()   << "Server" << socket
               << ": verification (remote DB) request processed successfully.";
    qDebug()   << "Server" << socket
               << ": verification response sent ("
               << socket->write(QByteArray::fromStdString(str))
               << "B) ... result:"
               << resp->result()
               << ", score:"
               << resp->score();
}

void OpenFingerGateway::verificationResponseReadySlot(OpenFinger::VerificationResponseOlejarnikova response, QTcpSocket *socket)
{
    std::string str;
    OpenFinger::Wrapper wrap;
    wrap.set_header(OpenFinger::Wrapper_Header_VERIFICATION_RESPONSE_OLEJARNIKOVA);
    OpenFinger::VerificationResponseOlejarnikova * resp = wrap.mutable_verify_response_olejarnikova();
    *resp = response;
    wrap.SerializeToString(&str);
    qDebug()   << "Server" << socket
               << ": verification (local DB) request processed successfully.";
    qDebug()   << "Server" << socket
               << ": verification response sent ("
               << socket->write(QByteArray::fromStdString(str))
               << "B) ... success:"
               << resp->success()
               << ", score:"
               << resp->score();
}

void OpenFingerGateway::identificationResponseReadySlot(OpenFinger::IdentificationResponse &response, QTcpSocket *socket)
{
    std::string str;
    OpenFinger::Wrapper wrap;
    wrap.set_header(OpenFinger::Wrapper_Header_IDENTIFICATION_RESPONSE);
    OpenFinger::IdentificationResponse * resp = wrap.mutable_identify_response();
    *resp = response;
    wrap.SerializeToString(&str);
    qDebug()   << "Server" << socket
               << ": identification request processed successfully.";
    qDebug()   << "Server" << socket
               << ": identification response sent ("
               << socket->write(QByteArray::fromStdString(str))
               << "B) ... success:"
               << resp->success()
               << ", fingerprint ID:"
               << resp->fingerprint_id();
}

void OpenFingerGateway::registrationResponseReadySlot(OpenFinger::RegistrationResponse &response, QTcpSocket *socket)
{
    std::string str;
    OpenFinger::Wrapper wrap;
    wrap.set_header(OpenFinger::Wrapper_Header_REGISTRATION_RESPONSE);
    OpenFinger::RegistrationResponse * resp = wrap.mutable_register_response();
    *resp = response;
    wrap.SerializeToString(&str);
    qDebug()   << "Server" << socket
               << ": registration request processed successfully.";
    qDebug()   << "Server" << socket
               << ": registration response sent ("
               << socket->write(QByteArray::fromStdString(str))
               << "B)";
}
