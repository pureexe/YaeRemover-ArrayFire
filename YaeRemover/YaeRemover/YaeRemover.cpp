// YaeRemover.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "YaeRemover.h"

AVSValue __cdecl Create_YaeRemover(AVSValue args, void* user_data, IScriptEnvironment* env) {
	return new YaeRemover(args[0].AsClip(), env);
}

const AVS_Linkage *AVS_linkage = 0;

extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit3(IScriptEnvironment* env, const AVS_Linkage* const vectors) {
	AVS_linkage = vectors;
	env->AddFunction("YaeRemover", "c", Create_YaeRemover, 0);
	return "YaeRemover plugin";
}
