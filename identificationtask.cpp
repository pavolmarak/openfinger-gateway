#include <QMultiMap>
#include "identificationtask.h"

IdentificationTask::IdentificationTask(QObject *parent) : QObject(parent)
{
    connect(    &(this->preprocessor),
                qOverload<PREPROCESSING_ALL_RESULTS>(&Preprocessing::preprocessingDoneSignal),
                this,
                &IdentificationTask::preprocessingDoneSlot);

    connect(    &(this->extractor),
                qOverload<QVector<MINUTIA>>(&Extraction::extractionDoneSignal),
                this,
                &IdentificationTask::extractionDoneSlot);

    connect(    &(this->matcher),
                &Matcher::identificationDoneSignal,
                this,
                &IdentificationTask::identificationDoneSlot);

    connect(    this,
                &IdentificationTask::preprocessingComplete,
                &this->loop,
                &QEventLoop::quit);

    connect(    this,
                &IdentificationTask::extractionComplete,
                &this->loop,
                &QEventLoop::quit);

    this->is_preprocessing_done = false;
    this->is_extraction_done = false;
}

void IdentificationTask::start()
{
    // 1. extract Level-2 vector of fingerprint
    cv::Mat img(  this->request.fingerprint().height(),
                  this->request.fingerprint().width(),
                  (this->request.fingerprint().color() == OpenFinger::Fingerprint_Colorspace_GRAYSCALE
                   ? CV_8UC1 : CV_8UC3),
                  (void*)this->request.fingerprint().data().data());
    if(img.rows <=0 || img.cols <=0){
        qDebug() << "Server: invalid image";
        return;
    }

    QVector<MINUTIA>& img_level2vector = this->preprocess_and_extract(img);

    // 2. prepare username-fingerprint map

    QMultiMap<QString,QVector<MINUTIA>> map;

    for(int i=0;i<this->request.vectors_size();i++){
        map.insert(
                    QString::number(this->request.vectors(i).fingerprint_id()),
                    convertProtoLevel2ToOpenFingerLevel2(this->request.vectors(i).level2vector())
                    );
    }

    // 3. match vector against all vectors in request

    this->matcher.setMatcher(MATCHER::bozorth3);
    //this->matcher.setBozorthThreshold(50);
    //this->matcher.setSupremaThreshold(0.10);
    this->matcher.identify(img_level2vector,map);
}

QVector<MINUTIA>& IdentificationTask::preprocess_and_extract(const cv::Mat& img)
{
    this->is_preprocessing_done = false;
    this->is_extraction_done = false;

    this->preprocessor.setCPUOnly(true);
    this->preprocessor.loadInput(img);
    this->preprocessor.setFeatures(true);
    this->preprocessor.start();

    this->waitForPreprocessingComplete();

    PREPROCESSING_RESULTS r_basic{
        this->preproc_results_all.imgOriginal,
                this->preproc_results_all.imgSkeleton,
                this->preproc_results_all.imgSkeletonInverted,
                this->preproc_results_all.qualityMap,
                this->preproc_results_all.orientationMap
    };

//    cv::imwrite("orig.png",r_basic.imgOriginal);
//    cv::imwrite("skel.png",r_basic.imgSkeleton);
//    cv::imwrite("skelinv.png",r_basic.imgSkeletonInverted);
//    cv::imwrite("qmap.png",this->preproc_results_all.imgQualityMap);
//    cv::imwrite("omap.png",this->preproc_results_all.imgOrientationMap);

    this->extractor.loadInput(r_basic);
    this->extractor.setCPUOnly(true);
    this->extractor.start();

    this->waitForExtractionComplete();

    return this->extract_result;
}

void IdentificationTask::preprocessingDoneSlot(PREPROCESSING_ALL_RESULTS all_results)
{
    this->preproc_results_all = all_results;
    this->is_preprocessing_done = true;
    emit this->preprocessingComplete();
}

void IdentificationTask::extractionDoneSlot(QVector<MINUTIA> level2vector)
{
    this->extract_result = level2vector;
    this->is_extraction_done = true;
    emit this->extractionComplete();
}

void IdentificationTask::identificationDoneSlot(bool success, QString bestSubject, float bestScore)
{
    qDebug() << "Server" << this->socket
             << ": identification result: success=" << success
             << ", bestSubject=" << bestSubject
             << ", bestScore=" << bestScore;
    this->response.set_success(success);
    this->response.set_fingerprint_id(bestSubject.toInt());
    emit identificationResponseReady(this->response,this->socket);
}

void IdentificationTask::waitForPreprocessingComplete()
{
    if(this->is_preprocessing_done == false){
        this->loop.exec();
    }
}

void IdentificationTask::waitForExtractionComplete()
{
    if(this->is_extraction_done == false){
        this->loop.exec();
    }
}

QVector<MINUTIA> IdentificationTask::convertProtoLevel2ToOpenFingerLevel2(const OpenFinger::Level2Vector &level2vector)
{
    QVector<MINUTIA> minutiae_vector;
    for(int i=0;i<level2vector.level2vector_size();i++){
        minutiae_vector.append({
                                   {
                                       level2vector.level2vector(i).x(),
                                       level2vector.level2vector(i).y()
                                   },
                                   level2vector.level2vector(i).type(),
                                   level2vector.level2vector(i).angle(),
                                   level2vector.level2vector(i).quality(),
                                   {
                                       level2vector.level2vector(i).img_width(),
                                       level2vector.level2vector(i).img_height()
                                   }
                               });
    }
    return minutiae_vector;
}

