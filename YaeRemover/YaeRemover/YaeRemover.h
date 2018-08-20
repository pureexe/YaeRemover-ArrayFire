#pragma once
#include "stdafx.h"
#include "detector/InRange.h"
#include "detector/OldYaeAlgorithm.h"
#include "inpainter/RecursiveInpainter.h"

class YaeRemover : public GenericVideoFilter {
public:
	YaeRemover(PClip _child, IScriptEnvironment* env);
	PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
	array frameToArray(PVideoFrame &frame);
	PVideoFrame arrayToFrame(array &frameArray, IScriptEnvironment* env);
};

YaeRemover::YaeRemover(PClip _child, IScriptEnvironment* env) :
	GenericVideoFilter(_child) {
	if (vi.IsPlanar() || !vi.IsRGB()) {
		env->ThrowError("YaeRemover: input must be RGB24. Precede YaeRemover() with ConvertRGB24()");
	}
	//setBackend(AF_BACKEND_CUDA);
	setBackend(AF_BACKEND_OPENCL);
}

PVideoFrame __stdcall YaeRemover::GetFrame(int n, IScriptEnvironment* env) {
	int left = 280, right = 1000, up = 660, down = 720;
	PVideoFrame src = child->GetFrame(n, env);
	array frameArray = frameToArray(src);
	seq row(0,59), col(280,999),row2(70,129);
	array subtitleFrame = frameArray(row, col);
	array inpaintDomainMask = inRange(subtitleFrame, 0, 0, 0, 20, 20, 20);
	if (inpaintDomainMask.nonzeros()) {
		inpaintDomainMask = OldYaeAlgorithm(inpaintDomainMask);
		inpaintDomainMask = dilate(inpaintDomainMask * 255, constant(1, 11,11,u8));
		
		double lambda = 250;
		double theta = 5;
		double tolerant = 5 * 1e-4;
		int max_iteration = 100;
		int max_guass_seidel = 1;
		double omega = 1.3;

		inpaintDomainMask /= 255.0;
		subtitleFrame /= 255.0;

		array inpaintDomainBuff(inpaintDomainMask.dims(0), inpaintDomainMask.dims(1), 3, f64);
		inpaintDomainBuff(span, span, 0) = inpaintDomainMask;
		inpaintDomainBuff(span, span, 1) = inpaintDomainMask;
		inpaintDomainBuff(span, span, 2) = inpaintDomainMask;
		inpaintDomainMask = inpaintDomainBuff;

		array resultImage = RecusriveInpainter(subtitleFrame, inpaintDomainMask, lambda, theta, tolerant, omega, max_guass_seidel, max_iteration, 4, 1);
		resultImage *= 255;

		frameArray(row, col) = resultImage.as(u8);
		
		/*

		frameArray(row, col, 0) = inpaintDomainMask;
		frameArray(row, col, 1) = inpaintDomainMask;
		frameArray(row, col, 2) = inpaintDomainMask;
		
		*/
	}
	PVideoFrame resultFrame = arrayToFrame(frameArray, env);
	return resultFrame;
}

array YaeRemover::frameToArray(PVideoFrame &frame) {
	const int width = frame->GetRowSize() / 3;
	const int height = frame->GetHeight();
	const int count = width * height * 3;
	BYTE* frameData = (BYTE*)frame->GetReadPtr();
	array frameArray(height,width,3);
	array rawArray = array(count, frameData).as(u8);
	seq row(height), col(width), b(0, count - 1, 3), g(1, count - 1, 3), r(2, count - 1, 3);
	frameArray(row, col, 0) = moddims(rawArray(r),width,height).T();
	frameArray(row, col, 1) = moddims(rawArray(g), width, height).T();
	frameArray(row, col, 2) = moddims(rawArray(b), width, height).T();
	return frameArray;
}

PVideoFrame YaeRemover::arrayToFrame(array &frameArray, IScriptEnvironment* env) {
	std::wstring str;
	const int height = frameArray.dims(0);
	const int width = frameArray.dims(1);
	const int channels = frameArray.dims(2);
	const int count = height * width * channels;
	seq row(height), col(width), b(0, count - 1, 3), g(1, count - 1, 3), r(2, count - 1, 3);
	array rawData(count,u8);
	rawData(r) = flat(frameArray(row, col, 0).T());
	rawData(g) = flat(frameArray(row, col, 1).T());
	rawData(b) = flat(frameArray(row, col, 2).T());
	PVideoFrame frame = env->NewVideoFrame(vi);
	BYTE* rawDataPtr = frame->GetWritePtr();
	unsigned char* frameData = rawData.host<unsigned char>();
	memcpy(rawDataPtr, frameData, count);
	freeHost(frameData); //garbage collection
	return frame;
}

//std::wstring str = L"REMOVE SUBTITLE: BEFORE frameToArray";OutputDebugString(str.data());
