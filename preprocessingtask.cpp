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

void PreprocessingTask::preprocessingDoneSlot(PREPROCESSING_ALL_RESULTS results)
{
    this->response.clear_results();
    OpenFinger::Fingerprint *fp = this->response.add_results();
    cv::Mat resp_img = results.imgSkeleton;
    fp->set_width(resp_img.cols);
    fp->set_height(resp_img.rows);
    fp->set_resolution(500);
    fp->set_color(OpenFinger::Fingerprint_Colorspace::Fingerprint_Colorspace_GRAYSCALE);
    fp->set_data((const char*)resp_img.data, resp_img.total() * resp_img.elemSize());

    emit preprocessingResponseReady(this->response, this->socket);
}
