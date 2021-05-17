#ifndef VERIFICATIONTASK_H
#define VERIFICATIONTASK_H

#include <QObject>
#include <QTcpSocket>
#include <QEventLoop>
#include "Fingerprint.pb.h"
#include "VerificationRequest.pb.h"
#include "VerificationResponse.pb.h"
#include "Wrapper.pb.h"
#include "preprocessing.h"
#include "extraction.h"
#include "matcher.h"

class VerificationTask : public QObject
{
    Q_OBJECT
public:
    explicit VerificationTask(QObject *parent = nullptr);

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

    OpenFinger::VerificationRequest request;
    OpenFinger::VerificationResponse response;
    QTcpSocket *socket;
private slots:
    void preprocessingDoneSlot(PREPROCESSING_ALL_RESULTS);
    void extractionDoneSlot(QVector<MINUTIA>);
    void verificationDoneSlot(bool, float score);
signals:
    void verificationResponseReady(OpenFinger::VerificationResponse&, QTcpSocket *);
    void preprocessingComplete();
    void extractionComplete();

};

#endif // VERIFICATIONTASK_H
