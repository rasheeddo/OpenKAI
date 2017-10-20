/*
 * _Caffe.h
 *
 *  Created on: Aug 17, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_DNN_Classifier__Caffe_H_
#define OpenKAI_src_DNN_Classifier__Caffe_H_

#include "../../Base/common.h"

#ifdef USE_CAFFE

#include <cuda_runtime.h>
#include <caffe/caffe.hpp>
#include <caffe/blob.hpp>
#include <caffe/common.hpp>
#include <caffe/util/io.hpp>
#include <caffe/proto/caffe.pb.h>

#include "../../Detector/_DetectorBase.h"
#include "../../Vision/_VisionBase.h"

namespace kai
{

using namespace caffe;
using caffe::Blob;
using caffe::Caffe;
using caffe::Net;
using caffe::shared_ptr;
using caffe::vector;
using std::string;
using namespace std;
using namespace cv;
using namespace cuda;

// Pair (label, confidence) representing a prediction
typedef std::pair<string, float> Prediction;

class _Caffe: public _DetectorBase
{
public:
	_Caffe();
	~_Caffe();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(void);

	bool setup(void);
	void updateMode(void);
	std::vector<vector<Prediction> > Classify(const vector<GpuMat> vImg);

private:
	void detect(void);
	void SetMean(const string& meanFile);
	std::vector<float> Predict(const vector<GpuMat> vImg);
	void WrapInputLayer(std::vector<std::vector<GpuMat> > *vvInput);
	void Preprocess(const vector<GpuMat> vImg, std::vector<std::vector<GpuMat> >* vvInputBatch);

	void update(void);
	static void* getUpdateThread(void* This) {
		((_Caffe*) This)->update();
		return NULL;
	}

private:
	shared_ptr<Net<float> > m_pNet;
	cv::Size m_inputGeometry;
	int m_nChannel;
	GpuMat m_mMean;
	vector<string> m_vLabel;
	int m_nClass;
	int m_nBatch;

	Frame* m_pRGBA;
};
}
#endif
#endif
