#include <jni.h>
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/videoio.hpp>

#include "JNIHelpers.h"

using namespace cv;

extern "C" {

VideoWriter recorder;
Size frameSize;

JNIEXPORT void JNICALL
Java_com_kimjunu_homevideo_MainActivity_StartRecord(JNIEnv *env, jobject instance,
                                                    jstring filePath_) {
    JNIHelpers::String path(env, filePath_);

    recorder.open(path.str(), VideoWriter::fourcc('M', 'P', 'G', '4'),
                  30, frameSize, true);
}

JNIEXPORT void JNICALL
Java_com_kimjunu_homevideo_MainActivity_StopRecord(JNIEnv *env, jobject instance) {

    recorder.release();
}

JNIEXPORT jboolean JNICALL
Java_com_kimjunu_homevideo_MainActivity_IsRecording(JNIEnv *env, jobject instance) {

    if (recorder.isOpened())
        return JNI_TRUE;
    else
        return JNI_FALSE;
}

JNIEXPORT void JNICALL
Java_com_kimjunu_homevideo_MainActivity_ConvertScreenFilter(JNIEnv *env, jobject instance,
                                                            jlong matAddrInput, jlong matAddrResult) {

    Mat &matInput = *(Mat *) matAddrInput;
    Mat &matResult = *(Mat *) matAddrResult;

    int offset = 2;
    int scanningLine = 1;

    cvtColor(matInput, matInput, CV_RGBA2GRAY);
    cvtColor(matInput, matResult, CV_GRAY2RGB);

    float fOpacity = 0.9;

    // 분리된 각 체널의 이미지를 저장할 공간 생성
    Mat r(matResult.size(), CV_8UC1);
    Mat g(matResult.size(), CV_8UC1);
    Mat b(matResult.size(), CV_8UC1);

    // 체널을 분리하여 각각 저장
    Mat out[] = {r, g, b};
    int from_to[] = {0, 0, 1, 1, 2, 2};
    mixChannels(&matResult, 1, out, 3, from_to, 3);

    // 이동 시킨 이미지를 저장할 공간 생성
    Mat r_s(matResult.size(), CV_8UC1, Scalar(0));
    Mat b_s(matResult.size(), CV_8UC1, Scalar(0));

    // 적색과 청색만 좌우로 이동하여 새로운 공간에 저장
    r(Rect(offset, 0, matResult.cols - offset, matResult.rows)).copyTo(
            r_s(Rect(0, 0, matResult.cols - offset, matResult.rows)));
    b(Rect(0, 0, matResult.cols - offset, matResult.rows)).copyTo(
            b_s(Rect(offset, 0, matResult.cols - offset, matResult.rows)));

    // 이동 시킨 체널 2개와 기존 체널 1개를 결합
    Mat out_s[] = {r_s, g, b_s};
    Mat result(matResult.size(), CV_8UC3);
    mixChannels(out_s, 3, &result, 1, from_to, 3);

    // 주사선 느낌 내기 위해 4줄 Opacity 조절
    int line_step = matInput.rows / scanningLine;
    for (int i = 0; i < line_step; i++)
    {
        if (!(i % 2)) {
            result(Rect(0, i*scanningLine, result.cols, scanningLine)) *= fOpacity;
        }
    }

    frameSize = result.size();

    if (recorder.isOpened())
        recorder << result;

    matResult = result;
}
}
