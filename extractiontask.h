#ifndef EXTRACTIONTASK_H
#define EXTRACTIONTASK_H

#include <QObject>
#include <QSslSocket>
#include <QEventLoop>
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
    void preprocess_and_extract(cv::Mat&);
    void cvMatToProtoFingerprint(cv::Mat &mat, OpenFinger::Fingerprint *fp, bool is_color);

    void waitForPreprocessingComplete();
    void waitForExtractionComplete();

    Preprocessing preprocessor;
    Extraction extractor;

    OpenFinger::ExtractionRequest request;
    OpenFinger::ExtractionResponse response;
    QSslSocket *socket;
    PREPROCESSING_ALL_RESULTS preproc_results_all;
    QVector<MINUTIA> extract_result;
    cv::Mat extract_image;
    QEventLoop loop;
    bool is_preprocessing_done;
    bool is_extraction_done;
    bool is_extraction_image_done;

private slots:
    void preprocessingDoneSlot(PREPROCESSING_ALL_RESULTS);
    void extractionDoneSlot(QVector<MINUTIA>);
    void extractionDoneSlot(cv::Mat);

signals:
     void extractionResponseReady(OpenFinger::ExtractionResponse&, QSslSocket *);
     void preprocessingComplete();
     void extractionComplete();
     void extractionImageComplete();
};

#endif // EXTRACTIONTASK_H

