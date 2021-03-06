#ifndef PREPROCESSINGTASK_H
#define PREPROCESSINGTASK_H

#include <QObject>
#include <QSslSocket>
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
    void cvMatToProtoFingerprint(cv::Mat& mat, OpenFinger::Fingerprint *fp, bool is_color);
//    void cvColorMatToProtoFingerprint(cv::Mat& mat, OpenFinger::Fingerprint *fp);

    Preprocessing preprocessor;
    OpenFinger::PreprocessingRequest request;
    OpenFinger::PreprocessingResponse response;
    QSslSocket *socket;

private slots:
    void preprocessingDoneSlot(PREPROCESSING_ALL_RESULTS);

signals:
    void preprocessingResponseReady(OpenFinger::PreprocessingResponse&, QSslSocket *);

};

#endif // PREPROCESSINGTASK_H


