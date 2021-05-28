#include "preprocessingtask.h"

PreprocessingTask::PreprocessingTask(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<PREPROCESSING_ALL_RESULTS>("PREPROCESSING_ALL_RESULTS");
    connect(    &(this->preprocessor),
                qOverload<PREPROCESSING_ALL_RESULTS>(&Preprocessing::preprocessingDoneSignal),
                this,
                &PreprocessingTask::preprocessingDoneSlot);

}

void PreprocessingTask::start()
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
    if(this->request.params().block_size()%2 == 0){
        this->request.mutable_params()->set_block_size(this->request.params().block_size()+1);
    }
    this->preprocessor.setPreprocessingParams(
                this->request.params().block_size()<=0 ? 13 : this->request.params().block_size(),
                this->request.params().gabor_lambda()<=0 ? 9 : this->request.params().gabor_lambda(),
                this->request.params().gabor_sigma()<=0 ? 3 : this->request.params().gabor_sigma()
                );
    this->preprocessor.start();
}

void PreprocessingTask::cvMatToProtoFingerprint(cv::Mat &mat, OpenFinger::Fingerprint *fp, bool is_color = false)
{
    fp->set_width(mat.cols);
    fp->set_height(mat.rows);
    fp->set_resolution(500);
    if(is_color){
        fp->set_color(OpenFinger::Fingerprint_Colorspace::Fingerprint_Colorspace_RGB);
    }
    else{
        fp->set_color(OpenFinger::Fingerprint_Colorspace::Fingerprint_Colorspace_GRAYSCALE);
    }
    fp->set_data((const char*)mat.data, mat.total() * mat.elemSize());
}

void PreprocessingTask::preprocessingDoneSlot(PREPROCESSING_ALL_RESULTS results)
{
    this->response.clear_results();
    OpenFinger::PreprocessingResult *result = nullptr;
    cv::Mat resp_img;

    // imgQualityMap
    resp_img = results.imgQualityMap;
    result = this->response.add_results();
    result->mutable_info()->clear();
    result->mutable_info()->append("Fingerprint image quality map.");
    this->cvMatToProtoFingerprint(resp_img,result->mutable_fingerprint());

    // imgContrastEnhanced
    resp_img = results.imgContrastEnhanced;
    result = this->response.add_results();
    result->mutable_info()->clear();
    result->mutable_info()->append("Fingerprint image after contrast enhancement.");
    this->cvMatToProtoFingerprint(resp_img,result->mutable_fingerprint());

    // imgOrientationMap
    resp_img = results.imgOrientationMap;
    result = this->response.add_results();
    result->mutable_info()->clear();
    result->mutable_info()->append("Fingerprint image orientation map.");
    this->cvMatToProtoFingerprint(resp_img,result->mutable_fingerprint(), true);

    // imgEnhanced
    resp_img = results.imgEnhanced;
    result = this->response.add_results();
    result->mutable_info()->clear();
    result->mutable_info()->append("Fingerprint image after Gabor filter enhancement.");
    this->cvMatToProtoFingerprint(resp_img,result->mutable_fingerprint());

    // imgBinarized
    resp_img = results.imgBinarized;
    result = this->response.add_results();
    result->mutable_info()->clear();
    result->mutable_info()->append("Fingerprint image after binarization.");
    this->cvMatToProtoFingerprint(resp_img,result->mutable_fingerprint());

    // imgSkeleton
    resp_img = results.imgSkeleton;
    result = this->response.add_results();
    result->mutable_info()->clear();
    result->mutable_info()->append("Fingerprint image skeleton.");
    this->cvMatToProtoFingerprint(resp_img,result->mutable_fingerprint());

    emit preprocessingResponseReady(this->response, this->socket);
}
