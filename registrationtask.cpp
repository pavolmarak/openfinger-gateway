#include <QFile>
#include <QDir>
#include <QProcess>
#include <QDateTime>
#include "registrationtask.h"

RegistrationTask::RegistrationTask(QObject *parent) : QObject(parent)
{
    connect(    &(this->preprocessor),
                qOverload<PREPROCESSING_ALL_RESULTS>(&Preprocessing::preprocessingDoneSignal),
                this,
                &RegistrationTask::preprocessingDoneSlot);

    connect(    &(this->extractor),
                qOverload<QVector<MINUTIA>>(&Extraction::extractionDoneSignal),
                this,
                &RegistrationTask::extractionDoneSlot);

    connect(    this,
                &RegistrationTask::preprocessingComplete,
                &this->loop,
                &QEventLoop::quit);

    connect(    this,
                &RegistrationTask::extractionComplete,
                &this->loop,
                &QEventLoop::quit);

    this->is_preprocessing_done = false;
    this->is_extraction_done = false;
}

void RegistrationTask::start()
{
    // 1. preprocess and extract features from registration fingerprint
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


    // 2. register user fingerprint to DB
    this->response.set_requestid(this->request.requestid());

    if(img_level2vector.isEmpty()){
        this->response.set_registration_status("REGISTRATION ERROR: fingerprint for user " + this->request.login() + " has not been registered. No Level-2 features detected.");
    }
    else{
        // save level-2 vector to file
        bool status = this->saveUserLevel2Vector(QString::fromStdString(this->request.login()), img_level2vector);
        if(status){
            this->response.set_registration_status("REGISTRATION SUCCESS: fingerprint for user " + this->request.login() + " has been registered succesfully.");
        }
        else{
            this->response.set_registration_status("REGISTRATION ERROR: fingerprint for user " + this->request.login() + " has not been registered. Error writing to DB.");
        }
    }

    emit registrationResponseReady(this->response,this->socket);
}

QVector<MINUTIA> &RegistrationTask::preprocess_and_extract(const cv::Mat &img)
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

bool RegistrationTask::saveUserLevel2Vector(const QString &username, const QVector<MINUTIA> &level2vector)
{
   // save new level2vector to file in folder 'users/username'
   // filename: username_timestamp.level2
    QString users_folder = "users";
    QString user_folder = users_folder+"/"+username;
    QProcess::execute("mkdir",QStringList() << "-p" << user_folder);
    QFile data(user_folder + "/" + username+"_"+QDateTime::currentDateTime().toString("dd.MM.yyyy.hh.mm.ss.zzz")+".level2");
    if (data.open(QFile::WriteOnly|QFile::Truncate)) {
        QTextStream out(&data);
        for(const MINUTIA& m : level2vector){
            out << m.xy.x() << " ";
            out << m.xy.y() << " ";
            out << m.type << " ";
            out << m.angle << " ";
            out << m.quality << " ";
            out << m.imgWH.x() << " ";
            out << m.imgWH.y() << "\n";
        }
        data.close();
    }
    else{
        return false;
    }
    return true;
}



void RegistrationTask::waitForPreprocessingComplete()
{
    if(this->is_preprocessing_done == false){
        this->loop.exec();
    }
}

void RegistrationTask::waitForExtractionComplete()
{
    if(this->is_extraction_done == false){
        this->loop.exec();
    }
}

void RegistrationTask::preprocessingDoneSlot(PREPROCESSING_ALL_RESULTS all_results)
{
    this->preproc_results_all = all_results;
    this->is_preprocessing_done = true;
    emit this->preprocessingComplete();
}

void RegistrationTask::extractionDoneSlot(QVector<MINUTIA> level2vector)
{
    this->extract_result = level2vector;
    this->is_extraction_done = true;
    emit this->extractionComplete();
}
