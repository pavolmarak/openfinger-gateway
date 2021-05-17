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
                &ExtractionTask::extractionDoneSlot);
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

    this->preprocessor.setCPUOnly(true);
    this->preprocessor.loadInput(input_img);
    this->preprocessor.setFeatures(true);
    this->preprocessor.start();
}

void ExtractionTask::preprocessingDoneSlot(PREPROCESSING_ALL_RESULTS results)
{
    PREPROCESSING_RESULTS r_basic{
        results.imgOriginal,
        results.imgSkeleton,
        results.imgSkeletonInverted,
        results.qualityMap,
        results.orientationMap
    };

    this->extractor.loadInput(r_basic);
    this->extractor.setCPUOnly(true);
    this->extractor.start();
}

void ExtractionTask::extractionDoneSlot(QVector<MINUTIA> level2vector)
{
    this->response.clear_level2vector();

    for(const MINUTIA& m : level2vector){
        OpenFinger::Level2 *level2 = this->response.mutable_level2vector()->add_level2vector();
        level2->set_x(m.xy.x());
        level2->set_y(m.xy.y());
        level2->set_type(m.type);
        level2->set_angle(m.angle);
        level2->set_quality(m.quality);
        level2->set_img_width(m.imgWH.x());
        level2->set_img_height(m.imgWH.y());
    }

    emit extractionResponseReady(this->response,this->socket);
}

