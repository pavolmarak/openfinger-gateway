#ifndef PREPROCESSINGTASK_H
#define PREPROCESSINGTASK_H

#include <QObject>
#include <QTcpSocket>
#include "Fingerprint.pb.h"
#include "PreprocessingRequest.pb.h"
#include "PreprocessingResponse.pb.h"
#include "Wrapper.pb.h"
#include "preprocessing.h"
#include "extraction.h"

class PreprocessingTask : public QObject
{
    Q_OBJECT
public:
    explicit PreprocessingTask(QObject *parent = nullptr);
    void start();

    Preprocessing preprocessor;
    OpenFinger::PreprocessingRequest request;
    OpenFinger::PreprocessingResponse response;
    QTcpSocket *socket;

private slots:
    void preprocessingDoneSlot(PREPROCESSING_ALL_RESULTS);

signals:
    void preprocessingResponseReady(OpenFinger::PreprocessingResponse&, QTcpSocket *);

};

#endif // PREPROCESSINGTASK_H


