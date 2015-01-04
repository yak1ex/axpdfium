/***********************************************************************/
/*                                                                     */
/* Spi_api.h: Header file for Susie plugin                             */
/*                                                                     */
/*     Copyright (C) 2015 Yak! / Yasutaka ATARASHI                     */
/*                                                                     */
/*     This software is distributed under the terms of a zlib/libpng   */
/*     License.                                                        */
/*                                                                     */
/*     $Id: fe98a65917bceb68f479ad7442f34aa9ab86b27c $                 */
/*                                                                     */
/***********************************************************************/

#ifndef SPI_API_H
#define SPI_API_H

#include <windef.h>

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************/
/* Constant                                                            */
/***********************************************************************/

#define SPI_ERR_NO_ERROR              0     /* ����I��*/
#define SPI_ERR_NOT_IMPLEMENTED       (-1)  /* ���̋@�\�̓C���v�������g����Ă��Ȃ� */
#define SPI_ERR_ABORT_BY_CALLBACK     1     /* �R�[���o�b�N�֐�����0��Ԃ����̂œW�J�𒆎~���� */
#define SPI_ERR_UNKNOWN_FORMAT        2     /* ���m�̃t�H�[�}�b�g */
#define SPI_ERR_BROKEN_DATA           3     /* �f�[�^�����Ă��� */
#define SPI_ERR_NO_MEMORY             4     /* �������[���m�ۏo���Ȃ� */
#define SPI_ERR_MEMORY_ERROR          5     /* �������[�G���[�iLock�o���Ȃ��A���j */
#define SPI_ERR_READ_ERROR            6     /* �t�@�C�����[�h�G���[ */
#define SPI_ERR_CANT_OPEN_WINDOW      7     /* �����J���Ȃ� (undocumented) */
#define SPI_ERR_INTERNAL_ERROR        8     /* �����G���[ */
#define SPI_ERR_WRITE_ERROR           9     /* �������݃G���[ (undocumented) */
#define SPI_ERR_END_OF_FILE           10    /* �t�@�C���I�[ (undocumented) */

/* For undocumented error codes, see http://www.asahi-net.or.jp/~kh4s-smz/spi/make_spi.html */

#define SPI_SUPPORT_YES    1
#define SPI_SUPPORT_NO     0

/***********************************************************************/
/* Type                                                                */
/***********************************************************************/

#include <pshpack1.h> /* Set alignment as 1 byte boundary. Probably, this is the safest way on Windows environment */
typedef struct
{
    BYTE  method[8];
    DWORD position;
    DWORD compsize;
    DWORD filesize;
    DWORD timestamp; /* seconds from epoch (1970/1/1 00:00:00 UTC) */
    CHAR  path[200];
    CHAR  filename[200];
    DWORD crc;
} SPI_FILEINFO;

typedef struct
{
	LONG  left;
	LONG  top;
	LONG  width;
	LONG  height;
	WORD  x_density;
	WORD  y_density;
	SHORT colorDepth;
	HLOCAL hInfo;
} SPI_PICTINFO;
#include <poppack.h>

/***********************************************************************/
/* Interface                                                           */
/***********************************************************************/

/* Common */
__declspec(dllexport) INT PASCAL GetPluginInfo(INT infono, LPSTR buf, INT buflen);
__declspec(dllexport) INT PASCAL IsSupported(LPSTR filename, DWORD dw);
__declspec(dllexport) INT PASCAL ConfigurationDlg(HWND parent, INT fnc);

/* 00IN */
__declspec(dllexport) INT PASCAL GetPictureInfo(LPSTR buf, LONG len, UINT flag, SPI_PICTINFO *lpInfo);
__declspec(dllexport) INT PASCAL GetPicture(LPSTR buf, LONG len, UINT flag, HANDLE *pHBInfo, HANDLE *pHBm, FARPROC lpPrgressCallback, LONG lData);
__declspec(dllexport) INT PASCAL GetPreview(LPSTR buf, LONG len, UINT flag, HANDLE *pHBInfo, HANDLE *pHBm, FARPROC lpPrgressCallback, LONG lData);

/* 00AM */
__declspec(dllexport) INT PASCAL GetArchiveInfo(LPSTR buf, LONG len, UINT flag, HLOCAL *lphInf);
__declspec(dllexport) INT PASCAL GetFileInfo(LPSTR buf, LONG len, LPSTR filename, UINT flag, SPI_FILEINFO *lpInfo);
__declspec(dllexport) INT PASCAL GetFile(LPSTR buf, LONG len, LPSTR dest, UINT flag, FARPROC prgressCallback, LONG lData);


#ifdef __cplusplus
};
#endif

#endif
