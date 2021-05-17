#ifndef EXTRACTIONTASK_H
#define EXTRACTIONTASK_H

#include <QObject>
#include <QTcpSocket>
#include "Fingerprint.pb.h"
#include "ExtractionRequest.pb.h"
#include "ExtractionResponse.pb.h"
#include "Wrapper.pb.h"
#include "preprocessing.h"
#include "extraction.h"

class ExtractionTask : public QObject
{
    Q_OBJECT
public:
    explicit ExtractionTask(QObject *parent = nullptr);
    void start();

    Preprocessing preprocessor;
    Extraction extractor;

    OpenFinger::ExtractionRequest request;
    OpenFinger::ExtractionResponse response;
    QTcpSocket *socket;

private slots:
    void preprocessingDoneSlot(PREPROCESSING_ALL_RESULTS);
    void extractionDoneSlot(QVector<MINUTIA>);

signals:
     void extractionResponseReady(OpenFinger::ExtractionResponse&, QTcpSocket *);

};

#endif // EXTRACTIONTASK_H

