#ifndef VERIFICATIONTASK_H
#define VERIFICATIONTASK_H

#include <QObject>
#include <QSslSocket>
#include <QEventLoop>
#include "Fingerprint.pb.h"
#include "VerificationRequest.pb.h"
#include "VerificationResponse.pb.h"
#include "VerificationRequestOlejarnikova.pb.h"
#include "VerificationResponseOlejarnikova.pb.h"
#include "Wrapper.pb.h"
#include "preprocessing.h"
#include "extraction.h"
#include "matcher.h"

class VerificationTask : public QObject
{
    Q_OBJECT
public:
    explicit VerificationTask(QObject *parent = nullptr);

    void startRemoteDB(const QString &local_hostname);
    void startLocalDB(const QString &local_hostname);
    QVector<MINUTIA> &preprocess_and_extract(const cv::Mat &img);
    void waitForPreprocessingComplete();
    void waitForExtractionComplete();

    QVector<MINUTIA> convertProtoLevel2ToOpenFingerLevel2(const OpenFinger::Level2Vector& level2vector);
    QVector<QVector<MINUTIA>> getLevel2VectorsFromLocalDB(const QString& login);

    QEventLoop loop;
    Preprocessing preprocessor;
    Extraction extractor;
    Matcher matcher_remote_db, matcher_local_db;

    bool is_preprocessing_done;
    bool is_extraction_done;
    PREPROCESSING_ALL_RESULTS preproc_results_all;
    QVector<MINUTIA> extract_result;

    OpenFinger::VerificationRequest requestRemoteDB;
    OpenFinger::VerificationResponse responseRemoteDB;
    OpenFinger::VerificationRequestOlejarnikova requestLocalDB;
    OpenFinger::VerificationResponseOlejarnikova responseLocalDB;
    QSslSocket *socket;
private slots:
    void preprocessingDoneSlot(PREPROCESSING_ALL_RESULTS);
    void extractionDoneSlot(QVector<MINUTIA>);
    void verificationDoneRemoteSlot(bool, float score);
    void verificationDoneLocalSlot(bool, float score);
signals:
    void verificationResponseReady(OpenFinger::VerificationResponse, QSslSocket *);
    void verificationResponseReady(OpenFinger::VerificationResponseOlejarnikova, QSslSocket *);

    void preprocessingComplete();
    void extractionComplete();

};

#endif // VERIFICATIONTASK_H
