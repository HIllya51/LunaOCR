#define __CLIB__
#ifdef __CLIB__

#include "OcrLiteCApi.h"
#include "OcrLite.h"

extern "C"
{

_QM_OCR_API OCR_HANDLE
OcrInit(const char *szDetModel, const char *szClsModel, const char *szRecModel, const char *szKeyPath, int nThreads) {

    auto *pOcrObj = new OcrLite;
    if (pOcrObj) {
        pOcrObj->setNumThread(nThreads);

        pOcrObj->initModels(szDetModel, szClsModel, szRecModel, szKeyPath);

        return pOcrObj;
    } else {
        return nullptr;
    }

}

_QM_OCR_API OCR_BOOL
OcrDetect(OCR_HANDLE handle, const void* binptr, size_t size ,int doAngle, int* num, ocrpoints** points, char*** strings){

    auto pOcrObj = (OcrLite *) handle;
    if (!pOcrObj)
        return FALSE;

    OCR_PARAM Param  ;
    Param.padding = 50;
    Param.maxSideLen = 1024;
    Param.boxScoreThresh = 0.1;
    Param.boxThresh =0.1;
    Param.unClipRatio = 2.0;
    Param.doAngle = 1;
    Param.mostAngle = 1;

    auto result = pOcrObj->detect(binptr, size, Param.padding, Param.maxSideLen,
                                              Param.boxScoreThresh, Param.boxThresh, Param.unClipRatio,
                                              doAngle==1, Param.mostAngle != 0);
    *num=result.size();
    *strings=new char*[result.size()];
    *points=new ocrpoints[result.size()];
    int idx=0;
    for(auto item:result){
        (*points)[idx]={item.boxPoint[0].x, item.boxPoint[0].y,
            item.boxPoint[1].x, item.boxPoint[1].y,
            item.boxPoint[2].x, item.boxPoint[2].y,
            item.boxPoint[3].x, item.boxPoint[3].y};
        (*strings)[idx]=new char[item.text.size()+1];
        strcpy((*strings)[idx],item.text.c_str());
        idx+=1;
    }
    return TRUE;
}

_QM_OCR_API void OcrDestroy(OCR_HANDLE handle) {
    auto pOcrObj = (OcrLite *) handle;
    if (pOcrObj)
        delete pOcrObj;
}
_QM_OCR_API void OcrFreeptr( int num,ocrpoints* ps,char** strings){
    for(int i=0;i<num;i++){
        delete strings[i];
    }
    delete ps;
    delete strings;
}

};
#endif

