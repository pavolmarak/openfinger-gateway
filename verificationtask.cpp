#include "verificationtask.h"
#include <QDir>

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

    connect(    &(this->matcher_remote_db),
                &Matcher::verificationDoneSignal,
                this,
                &VerificationTask::verificationDoneRemoteSlot);

    connect(    &(this->matcher_local_db),
                &Matcher::verificationDoneSignal,
                this,
                &VerificationTask::verificationDoneLocalSlot);

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

void VerificationTask::startRemoteDB()
{
    // 1. extract Level-2 vector of fingerprint
    cv::Mat img(  this->requestRemoteDB.fingerprint().height(),
                  this->requestRemoteDB.fingerprint().width(),
                  (this->requestRemoteDB.fingerprint().color() == OpenFinger::Fingerprint_Colorspace_GRAYSCALE
                   ? CV_8UC1 : CV_8UC3),
                  (void*)this->requestRemoteDB.fingerprint().data().data());
    if(img.rows <=0 || img.cols <=0){
        qDebug() << "Server: invalid image";
        return;
    }

    QVector<MINUTIA>& img_level2vector = this->preprocess_and_extract(img);


    // 2. convert Level-2 vectors of claimed identity in protobuf format to Openfinger format

    QVector<QVector<MINUTIA>> subjects_level2vectors;

    for(int i=0;i<this->requestRemoteDB.level2vectors_size();i++){
        subjects_level2vectors.push_back(convertProtoLevel2ToOpenFingerLevel2(this->requestRemoteDB.level2vectors(i)));
    }

    // 3. match vector against all vectors in request

    this->matcher_remote_db.setMatcher(MATCHER::bozorth3);
    //this->matcher.setBozorthThreshold(50);
    //this->matcher.setSupremaThreshold(0.10);
    this->matcher_remote_db.verify(img_level2vector,subjects_level2vectors);
}

void VerificationTask::startLocalDB()
{
    // 1. extract Level-2 vector of fingerprint
    cv::Mat img(  this->requestLocalDB.fingerprint().height(),
                  this->requestLocalDB.fingerprint().width(),
                  (this->requestLocalDB.fingerprint().color() == OpenFinger::Fingerprint_Colorspace_GRAYSCALE
                   ? CV_8UC1 : CV_8UC3),
                  (void*)this->requestLocalDB.fingerprint().data().data());
    if(img.rows <=0 || img.cols <=0){
        qDebug() << "Server: invalid image";
        return;
    }

    QVector<MINUTIA> img_level2vector = this->preprocess_and_extract(img);


    // 2. get Level-2 vectors of claimed identity from local DB

    QVector<QVector<MINUTIA>> subjects_level2vectors;
    subjects_level2vectors = getLevel2VectorsFromLocalDB(QString::fromStdString(this->requestLocalDB.login()));
    if(subjects_level2vectors.isEmpty()){
        this->responseLocalDB.set_requestid(this->requestLocalDB.requestid());
        this->responseLocalDB.set_success(false);
        this->responseLocalDB.set_score(0);
        emit verificationResponseReady(this->responseLocalDB,this->socket);
        return;
    }

    // 3. match vector against all vectors of requested user

    this->matcher_local_db.setMatcher(MATCHER::bozorth3);
    //this->matcher.setBozorthThreshold(50);
    //this->matcher.setSupremaThreshold(0.10);
    this->matcher_local_db.verify(img_level2vector,subjects_level2vectors);
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

void VerificationTask::verificationDoneRemoteSlot(bool success, float score)
{ 
    this->responseRemoteDB.set_result(success);
    this->responseRemoteDB.set_score(score);
    emit verificationResponseReady(this->responseRemoteDB,this->socket);
    this->responseRemoteDB.Clear();
}

void VerificationTask::verificationDoneLocalSlot(bool success, float score)
{
    this->responseLocalDB.set_requestid(this->requestLocalDB.requestid());
    this->responseLocalDB.set_success(success);
    this->responseLocalDB.set_score(score);
    emit verificationResponseReady(this->responseLocalDB,this->socket);
    this->responseLocalDB.Clear();
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

QVector<QVector<MINUTIA> > VerificationTask::getLevel2VectorsFromLocalDB(const QString &login)
{
    QVector<QVector<MINUTIA>> level2vectors;

    QString users_folder = "users";
    QString user_folder_path = users_folder+"/"+login;

    QDir user_dir = user_folder_path;
    if(!user_dir.exists()){
        return level2vectors;
    }
    QFileInfoList info_list = user_dir.entryInfoList(QStringList() << "*.level2",QDir::Files);

    QFile level2_file;
    for(const QFileInfo& info : info_list){
        level2_file.setFileName(info.absoluteFilePath());
        level2_file.open(QFile::ReadOnly);
        level2vectors.push_back({});
        QTextStream txt(&level2_file);
        QString line;
        while(!txt.atEnd()){
            level2vectors.last().push_back({});
            line = txt.readLine();
            QTextStream strm(&line);
            strm >> level2vectors.last().last().xy.rx();
            strm >> level2vectors.last().last().xy.ry();
            strm >> level2vectors.last().last().type;
            strm >> level2vectors.last().last().angle;
            strm >> level2vectors.last().last().quality;
            strm >> level2vectors.last().last().imgWH.rx();
            strm >> level2vectors.last().last().imgWH.ry();
        }
        level2_file.close();
    }
    return level2vectors;
}
