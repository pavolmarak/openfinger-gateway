#ifndef REGISTRATIONTASK_H
#define REGISTRATIONTASK_H

#include <QObject>
#include <QTcpSocket>
#include <QEventLoop>
#include "Fingerprint.pb.h"
#include "RegistrationRequest.pb.h"
#include "RegistrationResponse.pb.h"
#include "Wrapper.pb.h"
#include "preprocessing.h"
#include "extraction.h"

class RegistrationTask : public QObject
{
    Q_OBJECT
public:
    explicit RegistrationTask(QObject *parent = nullptr);
    void start();
    QVector<MINUTIA>& preprocess_and_extract(const cv::Mat &img);
    bool saveUserLevel2Vector(const QString& username, const QVector<MINUTIA>& level2vector);
    void waitForPreprocessingComplete();
    void waitForExtractionComplete();

    QEventLoop loop;
    Preprocessing preprocessor;
    Extraction extractor;
    bool is_preprocessing_done;
    bool is_extraction_done;
    PREPROCESSING_ALL_RESULTS preproc_results_all;
    QVector<MINUTIA> extract_result;

    OpenFinger::RegistrationRequest request;
    OpenFinger::RegistrationResponse response;
    QTcpSocket *socket;

private slots:
    void preprocessingDoneSlot(PREPROCESSING_ALL_RESULTS);
    void extractionDoneSlot(QVector<MINUTIA>);
signals:
    void registrationResponseReady(OpenFinger::RegistrationResponse&, QTcpSocket *);
    void preprocessingComplete();
    void extractionComplete();
};

#endif // REGISTRATIONTASK_H
