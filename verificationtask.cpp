#include "verificationtask.h"

VerificationTask::VerificationTask(QObject *parent) : QObject(parent)
{
    connect(    &(this->preprocessor),
                qOverload<PREPROCESSING_ALL_RESULTS>(&Preprocessing::preprocessingDoneSignal),
                this,
                &VerificationTask::preprocessingDoneSlot);

    connect(    &(this->extractor),
                qOverload<QVector<MINUTIA>>(&Extraction::extractionDoneSignal),
                this,
                &VerificationTask::extractionDoneSlot);

    connect(    &(this->matcher),
                &Matcher::verificationDoneSignal,
                this,
                &VerificationTask::verificationDoneSlot);

    connect(    this,
                &VerificationTask::preprocessingComplete,
                &this->loop,
                &QEventLoop::quit);

    connect(    this,
                &VerificationTask::extractionComplete,
                &this->loop,
                &QEventLoop::quit);

    this->is_preprocessing_done = false;
    this->is_extraction_done = false;
}

void VerificationTask::start()
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

    // 2. convert Level-2 vectors of claimed identity in protobuf format to Openfinger format

    QVector<QVector<MINUTIA>> subjects_level2vectors;

    for(int i=0;i<this->request.level2vectors_size();i++){
        subjects_level2vectors.push_back(convertProtoLevel2ToOpenFingerLevel2(this->request.level2vectors(i)));
    }

    // 3. match vector against all vectors in request

    this->matcher.setMatcher(MATCHER::bozorth3);
    //this->matcher.setBozorthThreshold(50);
    //this->matcher.setSupremaThreshold(0.10);
    this->matcher.verify(img_level2vector,subjects_level2vectors);
}

QVector<MINUTIA>& VerificationTask::preprocess_and_extract(const cv::Mat& img)
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

void VerificationTask::preprocessingDoneSlot(PREPROCESSING_ALL_RESULTS all_results)
{
    this->preproc_results_all = all_results;
    this->is_preprocessing_done = true;
    emit this->preprocessingComplete();
}

void VerificationTask::extractionDoneSlot(QVector<MINUTIA> level2vector)
{
    this->extract_result = level2vector;
    this->is_extraction_done = true;
    emit this->extractionComplete();
}

void VerificationTask::verificationDoneSlot(bool success, float score)
{
    this->response.set_result(success);
    this->response.set_score(score);
    emit verificationResponseReady(this->response,this->socket);
}

void VerificationTask::waitForPreprocessingComplete()
{
    if(this->is_preprocessing_done == false){
        this->loop.exec();
    }
}

void VerificationTask::waitForExtractionComplete()
{
    if(this->is_extraction_done == false){
        this->loop.exec();
    }
}

QVector<MINUTIA> VerificationTask::convertProtoLevel2ToOpenFingerLevel2(const OpenFinger::Level2Vector &level2vector)
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
