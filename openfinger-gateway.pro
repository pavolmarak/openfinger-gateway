QT += gui
QT += network

CONFIG += c++17 console
CONFIG -= app_bundle
QMAKE_CFLAGS_ISYSTEM=

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        ../openfinger-proto/cpp/ExtractionRequest.pb.cc \
        ../openfinger-proto/cpp/ExtractionResponse.pb.cc \
        ../openfinger-proto/cpp/Fingerprint.pb.cc \
        ../openfinger-proto/cpp/IdentificationRequest.pb.cc \
        ../openfinger-proto/cpp/IdentificationResponse.pb.cc \
        ../openfinger-proto/cpp/Level2.pb.cc \
        ../openfinger-proto/cpp/Level2Vector.pb.cc \
        ../openfinger-proto/cpp/PreprocessingRequest.pb.cc \
        ../openfinger-proto/cpp/PreprocessingResponse.pb.cc \
        ../openfinger-proto/cpp/RegistrationRequest.pb.cc \
        ../openfinger-proto/cpp/RegistrationResponse.pb.cc \
        ../openfinger-proto/cpp/VerificationRequest.pb.cc \
        ../openfinger-proto/cpp/VerificationRequestOlejarnikova.pb.cc \
        ../openfinger-proto/cpp/VerificationResponse.pb.cc \
        ../openfinger-proto/cpp/VerificationResponseOlejarnikova.pb.cc \
        ../openfinger-proto/cpp/Wrapper.pb.cc \
        extractiontask.cpp \
        identificationtask.cpp \
        main.cpp \
        openfingergateway.cpp \
        preprocessingtask.cpp \
        registrationtask.cpp \
        sslserver.cpp \
        verificationtask.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    ../openfinger-proto/cpp/ExtractionRequest.pb.h \
    ../openfinger-proto/cpp/ExtractionResponse.pb.h \
    ../openfinger-proto/cpp/Fingerprint.pb.h \
    ../openfinger-proto/cpp/IdentificationRequest.pb.h \
    ../openfinger-proto/cpp/IdentificationResponse.pb.h \
    ../openfinger-proto/cpp/Level2.pb.h \
    ../openfinger-proto/cpp/Level2Vector.pb.h \
    ../openfinger-proto/cpp/PreprocessingRequest.pb.h \
    ../openfinger-proto/cpp/PreprocessingResponse.pb.h \
    ../openfinger-proto/cpp/RegistrationRequest.pb.h \
    ../openfinger-proto/cpp/RegistrationResponse.pb.h \
    ../openfinger-proto/cpp/VerificationRequest.pb.h \
    ../openfinger-proto/cpp/VerificationRequestOlejarnikova.pb.h \
    ../openfinger-proto/cpp/VerificationResponse.pb.h \
    ../openfinger-proto/cpp/VerificationResponseOlejarnikova.pb.h \
    ../openfinger-proto/cpp/Wrapper.pb.h \
    extractiontask.h \
    identificationtask.h \
    openfingergateway.h \
    preprocessingtask.h \
    registrationtask.h \
    sslserver.h \
    verificationtask.h


CONFIG += manjaro
#CONFIG += debian

manjaro{
    # CUDA - Manjaro (as a package)
    unix:!macx: LIBS += -L/usr/local/cuda-10.2/lib64/ -lcudart
    unix:!macx: LIBS += -L/usr/local/cuda-10.2/lib64/ -lcublasLt
    INCLUDEPATH += /usr/local/cuda-10.2/include
    DEPENDPATH += /usr/local/cuda-10.2/include

    #ArrayFire - Manjaro (as a package)
    unix:!macx: LIBS += -L/usr/lib/ -lafcuda
    INCLUDEPATH += /usr/include
    DEPENDPATH += /usr/include

    #OpenCV - Manjaro (as a package)
    unix:!macx: LIBS += -L/usr/lib/ -lopencv_core
    unix:!macx: LIBS += -L/usr/lib/ -lopencv_imgproc
    unix:!macx: LIBS += -L/usr/lib/ -lopencv_imgcodecs
    unix:!macx: LIBS += -L/usr/lib/ -lopencv_highgui
    INCLUDEPATH += /usr/include/opencv4
    DEPENDPATH += /usr/include/opencv4

    #Caffe - Manjaro (built from source)
    unix:!macx: LIBS += -L/usr/local/lib/ -lcaffe
    INCLUDEPATH += /usr/local/include
    DEPENDPATH += /usr/local/include

    #glog, protobuf, boost - Manjaro (as a package)
    unix:!macx: LIBS += -L/usr/lib/ -lglog
    unix:!macx: LIBS += -L/usr/lib/ -lprotobuf
    unix:!macx: LIBS += -L/usr/lib/ -lboost_system
    INCLUDEPATH += /usr/include
    DEPENDPATH += /usr/include

    # openfinger-preprocessor - Manjaro
    unix:!macx: LIBS += -L/home/pavol/Downloads/src/openfinger-preprocessor -lofpreproc
    INCLUDEPATH += /home/pavol/Downloads/src/openfinger-preprocessor
    DEPENDPATH += /home/pavol/Downloads/src/openfinger-preprocessor

    # openfinger-extractor - Manjaro
    unix:!macx: LIBS += -L/home/pavol/Downloads/src/openfinger-extractor -lExtraction
    INCLUDEPATH += /home/pavol/Downloads/src/openfinger-extractor
    DEPENDPATH += /home/pavol/Downloads/src/openfinger-extractor

    # openfinger-matcher - Manjaro
    unix:!macx: LIBS += -L/home/pavol/Downloads/src/openfinger-matcher -lMatcher
    INCLUDEPATH += /home/pavol/Downloads/src/openfinger-matcher
    DEPENDPATH += /home/pavol/Downloads/src/openfinger-matcher

    # openfinger-proto - Manjaro
    INCLUDEPATH += /home/pavol/Downloads/src/openfinger-proto/cpp

    # Suprema - Manjaro
    INCLUDEPATH += /usr/local/include/suprema
}

debian{
    # CUDA - Debian (custom)
    unix:!macx: LIBS += -L/usr/local/cuda-10.2/lib64/ -lcudart
    unix:!macx: LIBS += -L/usr/local/cuda-10.2/lib64/ -lcublasLt
    INCLUDEPATH += /usr/local/cuda-10.2/include
    DEPENDPATH += /usr/local/cuda-10.2/include

    #ArrayFire - Debian (custom)
    unix:!macx: LIBS += -L/usr/local/lib/ -lafcuda
    INCLUDEPATH += /usr/local/include
    DEPENDPATH += /usr/local/include

    #OpenCV - Debian (package)
    unix:!macx: LIBS += -L/usr/lib/x86_64-linux-gnu -lopencv_core
    unix:!macx: LIBS += -L/usr/lib/x86_64-linux-gnu -lopencv_imgproc
    unix:!macx: LIBS += -L/usr/lib/x86_64-linux-gnu -lopencv_imgcodecs
    unix:!macx: LIBS += -L/usr/lib/x86_64-linux-gnu -lopencv_highgui
    INCLUDEPATH += /usr/include/opencv4
    DEPENDPATH += /usr/include/opencv4

    #Caffe - Debian (custom)
    unix:!macx: LIBS += -L/usr/local/lib/ -lcaffe
    INCLUDEPATH += /usr/local/include
    DEPENDPATH += /usr/local/include

    #glog, protobuf, boost - Debian (package)
    unix:!macx: LIBS += -L/usr/lib/x86_64-linux-gnu -lglog
    unix:!macx: LIBS += -L/usr/lib/x86_64-linux-gnu -lprotobuf
    unix:!macx: LIBS += -L/usr/lib/x86_64-linux-gnu -lboost_system
    INCLUDEPATH += /usr/include
    DEPENDPATH += /usr/include

    # openfinger-preprocessor - Debian
    unix:!macx: LIBS += -L/home/dualuser/Downloads/src/openfinger-preprocessor -lofpreproc
    INCLUDEPATH += /home/dualuser/Downloads/src/openfinger-preprocessor
    DEPENDPATH += /home/dualuser/Downloads/src/openfinger-preprocessor

    # openfinger-extractor - Debian
    unix:!macx: LIBS += -L/home/dualuser/Downloads/src/openfinger-extractor -lExtraction
    INCLUDEPATH += /home/dualuser/Downloads/src/openfinger-extractor
    DEPENDPATH += /home/dualuser/Downloads/src/openfinger-extractor

    # openfinger-matcher - Debian
    unix:!macx: LIBS += -L/home/dualuser/Downloads/src/openfinger-matcher -lMatcher
    INCLUDEPATH += /home/dualuser/Downloads/src/openfinger-matcher
    DEPENDPATH += /home/dualuser/Downloads/src/openfinger-matcher

    # openfinger-proto - Debian
    INCLUDEPATH += /home/dualuser/Downloads/src/openfinger-proto/cpp

    # Suprema - Debian
    INCLUDEPATH += /usr/local/include/suprema
}
