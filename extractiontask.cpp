#include "extractiontask.h"

ExtractionTask::ExtractionTask(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<PREPROCESSING_ALL_RESULTS>("PREPROCESSING_ALL_RESULTS");
    connect(    &(this->preprocessor),
                qOverload<PREPROCESSING_ALL_RESULTS>(&Preprocessing::preprocessingDoneSignal),
                this,
                &ExtractionTask::preprocessingDoneSlot);

    connect(    &(this->extractor),
                qOverload<QVector<MINUTIA>>(&Extraction::extractionDoneSignal),
                this,
                qOverload<QVector<MINUTIA>>(&ExtractionTask::extractionDoneSlot));

    connect(    &(this->extractor),
                qOverload<cv::Mat>(&Extraction::extractionDoneSignal),
                this,
                qOverload<cv::Mat>(&ExtractionTask::extractionDoneSlot));

    connect(    this,
                &ExtractionTask::preprocessingComplete,
                &this->loop,
                &QEventLoop::quit);

    connect(    this,
                &ExtractionTask::extractionComplete,
                &this->loop,
                &QEventLoop::quit);

    connect(    this,
                &ExtractionTask::extractionImageComplete,
                &this->loop,
                &QEventLoop::quit);

    this->is_preprocessing_done = false;
    this->is_extraction_done = false;
    this->is_extraction_image_done = false;
}

void ExtractionTask::start()
{
    cv::Mat input_img(  this->request.fingerprint().height(),
                        this->request.fingerprint().width(),
                        (this->request.fingerprint().color() == OpenFinger::Fingerprint_Colorspace_GRAYSCALE
                        ? CV_8UC1 : CV_8UC3),
                        (void*)this->request.fingerprint().data().data());

    if(input_img.rows <=0 || input_img.cols <=0){
        qDebug() << "Server: invalid image";
        return;
    }

    this->preprocess_and_extract(input_img);

    // set Level-2 vector
    this->response.clear_level2vector();
    for(const MINUTIA& m : this->extract_result){
        OpenFinger::Level2 *level2 = this->response.mutable_level2vector()->add_level2vector();
        level2->set_x(m.xy.x());
        level2->set_y(m.xy.y());
        level2->set_type(m.type);
        level2->set_angle(m.angle);
        level2->set_quality(m.quality);
        level2->set_img_width(m.imgWH.x());
        level2->set_img_height(m.imgWH.y());
    }

    // set extraction image
    this->response.clear_extraction_image();
    this->cvColorMatToProtoFingerprint(this->extract_image,this->response.mutable_extraction_image());

    emit extractionResponseReady(this->response,this->socket);
}

void ExtractionTask::preprocess_and_extract(cv::Mat &input_img)
{
    this->is_preprocessing_done = false;
    this->is_extraction_done = false;
    this->is_extraction_image_done = false;

    this->preprocessor.setCPUOnly(true);
    this->preprocessor.loadInput(input_img);
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

    this->extractor.loadInput(r_basic);
    this->extractor.setCPUOnly(true);
    this->extractor.start();

    this->waitForExtractionComplete();
}

void ExtractionTask::preprocessingDoneSlot(PREPROCESSING_ALL_RESULTS results)
{
    this->preproc_results_all = results;
    this->is_preprocessing_done = true;
    emit this->preprocessingComplete();
}

void ExtractionTask::extractionDoneSlot(QVector<MINUTIA> level2vector)
{
    this->extract_result = level2vector;
    this->is_extraction_done = true;
    emit this->extractionComplete();
}

void ExtractionTask::extractionDoneSlot(cv::Mat extractionImage)
{
    this->extract_image = extractionImage;
    this->is_extraction_image_done = true;
    emit this->extractionImageComplete();
    //cv::imwrite("extraction_image.png", extractionImage);
}

void ExtractionTask::waitForPreprocessingComplete()
{
    if(this->is_preprocessing_done == false){
        this->loop.exec();
    }
}

void ExtractionTask::waitForExtractionComplete()
{
    if(this->is_extraction_done == false){
        this->loop.exec();
    }
    if(this->is_extraction_image_done == false){
        this->loop.exec();
    }
}


void ExtractionTask::cvColorMatToProtoFingerprint(cv::Mat &mat, OpenFinger::Fingerprint *fp)
{
    if(mat.empty()){
        qDebug() << "Server: invalid extraction image";
        return;
    }
    fp->set_width(mat.cols);
    fp->set_height(mat.rows);
    fp->set_resolution(500);
    fp->set_color(OpenFinger::Fingerprint_Colorspace::Fingerprint_Colorspace_RGB);
    uchar * rgb_data = new uchar[3*mat.cols*mat.rows];
    int cnt=0;
    for(int i=0; i<mat.rows;i++){
        for(int j=0; j<mat.cols;j++){
            rgb_data[cnt++] = mat.at<cv::Vec3b>(i,j)[2]; // R
            rgb_data[cnt++] = mat.at<cv::Vec3b>(i,j)[1]; // G
            rgb_data[cnt++] = mat.at<cv::Vec3b>(i,j)[0]; // B
        }
    }
    fp->set_data(rgb_data, 3*mat.cols*mat.rows);
}

