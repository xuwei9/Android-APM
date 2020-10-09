/*
 * XkApm.cpp
 *
 *  Created on: 2019-7-10
 *      Author: Admin
 */

#include "XkApm.h"
#include "XkApm_Core.h"

int XkApm_Open(XkApm_Handle* pHandle)
{
	XkAPM_Core* apm = new XkAPM_Core();
	*pHandle = apm;
	return 0;
}

int XkApm_Close(XkApm_Handle handle)
{
	XkAPM_Core* apm = (XkAPM_Core*)handle;
	if(apm == NULL)
		return -1;
	apm->close();
	delete apm;
	return 0;
}

int XkApm_Reset(XkApm_Handle handle)
{
	XkAPM_Core* apm = (XkAPM_Core*)handle;
	if(apm == NULL)
		return -1;
	apm->reset();
	return 0;
}

int XkApm_Ns(XkApm_Handle handle,XkApm_Frame* pFrame)
{
	XkAPM_Core* apm = (XkAPM_Core*)handle;
	if(apm == NULL)
		return -1;
	apm->ns_process(pFrame->iDir,pFrame->pData,pFrame->iFrameLen);
	return 0;
}

int XkApm_Aec(XkApm_Handle handle,XkApm_Frame* pFrame)
{
	XkAPM_Core* apm = (XkAPM_Core*)handle;
	if(apm == NULL)
		return -1;
	apm->aec_process(pFrame->iDir,pFrame->pData,pFrame->iFrameLen);
	return 0;
}


int XkApm_Vad(XkApm_Handle handle,XkApm_Frame* pFrame)
{
	XkAPM_Core* apm = (XkAPM_Core*)handle;
	if(apm == NULL)
		return -1;
	return apm->vad_process(pFrame->iDir,pFrame->pData,pFrame->iFrameLen);
}

int XkApm_Agc(XkApm_Handle handle,XkApm_Frame* pFrame)
{
	XkAPM_Core* apm = (XkAPM_Core*)handle;
	if(apm == NULL)
		return -1;
	apm->agc_process(pFrame->iDir,pFrame->pData,pFrame->iFrameLen);
	return 0;
}

int XkApm_Config(XkApm_Handle handle, XkApm_Param config)
{
	XkAPM_Core* apm = (XkAPM_Core*)handle;
	if(apm == NULL)
		return -1;
	apm->config(config);
	return 0;
}
