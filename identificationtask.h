#ifndef IDENTIFICATIONTASK_H
#define IDENTIFICATIONTASK_H

#include <QObject>
#include <QTcpSocket>
#include <QEventLoop>
#include "Fingerprint.pb.h"
#include "IdentificationRequest.pb.h"
#include "IdentificationResponse.pb.h"
#include "Wrapper.pb.h"
#include "preprocessing.h"
#include "extraction.h"
#include "matcher.h"

class IdentificationTask : public QObject
{
    Q_OBJECT
public:
    explicit IdentificationTask(QObject *parent = nullptr);

    void start();
    QVector<MINUTIA>& preprocess_and_extract(const cv::Mat &img);
    void waitForPreprocessingComplete();
    void waitForExtractionComplete();

    QVector<MINUTIA> convertProtoLevel2ToOpenFingerLevel2(const OpenFinger::Level2Vector& level2vector);

    QEventLoop loop;
    Preprocessing preprocessor;
    Extraction extractor;
    Matcher matcher;

    bool is_preprocessing_done;
    bool is_extraction_done;
    PREPROCESSING_ALL_RESULTS preproc_results_all;
    QVector<MINUTIA> extract_result;

    OpenFinger::IdentificationRequest request;
    OpenFinger::IdentificationResponse response;
    QTcpSocket *socket;
private slots:
    void preprocessingDoneSlot(PREPROCESSING_ALL_RESULTS);
    void extractionDoneSlot(QVector<MINUTIA>);
    void identificationDoneSlot(bool, QString, float);
signals:
    void identificationResponseReady(OpenFinger::IdentificationResponse&, QTcpSocket *);
    void preprocessingComplete();
    void extractionComplete();
};

#endif // IDENTIFICATIONTASK_H
