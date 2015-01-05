// Excerpted from PDFium sample source with modification
//   - Remove unnecessary part
//     - Ouput formats except for BMP
//     - Command line handling
//   - Output to memory structure

// Original copyright follows:
//
// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <list>
#include <string>
#include <utility>
#include <vector>

#include "fpdfsdk/include/fpdf_dataavail.h"
#include "fpdfsdk/include/fpdf_ext.h"
#include "fpdfsdk/include/fpdfformfill.h"
#include "fpdfsdk/include/fpdftext.h"
#include "fpdfsdk/include/fpdfview.h"
#include "core/include/fxcrt/fx_system.h"
#include "v8/include/v8.h"

#ifdef _WIN32
#define snprintf _snprintf
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif

// Reads the entire contents of a file into a newly malloc'd buffer.
static char* GetFileContents(const char* filename, size_t* retlen) {
  FILE* file = fopen(filename, "rb");
  if (!file) {
    fprintf(stderr, "Failed to open: %s\n", filename);
    return NULL;
  }
  (void) fseek(file, 0, SEEK_END);
  size_t file_length = ftell(file);
  if (!file_length) {
    return NULL;
  }
  (void) fseek(file, 0, SEEK_SET);
  char* buffer = (char*) malloc(file_length);
  if (!buffer) {
    return NULL;
  }
  size_t bytes_read = fread(buffer, 1, file_length, file);
  (void) fclose(file);
  if (bytes_read != file_length) {
    fprintf(stderr, "Failed to read: %s\n", filename);
    free(buffer);
    return NULL;
  }
  *retlen = bytes_read;
  return buffer;
}

static void WriteToVec(std::vector<char> &v, const void* p, std::size_t size) {
  auto b = static_cast<const char*>(p);
  auto e = static_cast<const char*>(p) + size;
  v.insert(v.end(), b, e);
}

static bool WriteBmp(std::vector<char> &v, const void* buffer, int stride, int width, int height) {
  if (stride < 0 || width < 0 || height < 0)
    return false;
  if (height > 0 && width > INT_MAX / height)
    return false;
  int out_len = stride * height;
  if (out_len > INT_MAX / 3)
    return false;

  BITMAPINFO bmi = {0};
  bmi.bmiHeader.biSize = sizeof(bmi) - sizeof(RGBQUAD);
  bmi.bmiHeader.biWidth = width;
  bmi.bmiHeader.biHeight = -height;  // top-down image
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;
  bmi.bmiHeader.biSizeImage = 0;

  BITMAPFILEHEADER file_header = {0};
  file_header.bfType = 0x4d42;
  file_header.bfSize = sizeof(file_header) + bmi.bmiHeader.biSize + out_len;
  file_header.bfOffBits = file_header.bfSize - out_len;

  WriteToVec(v, &file_header, sizeof(file_header));
  WriteToVec(v, &bmi, bmi.bmiHeader.biSize);
  WriteToVec(v, buffer, out_len);

  return true;
}

int Form_Alert(IPDF_JSPLATFORM*, FPDF_WIDESTRING, FPDF_WIDESTRING, int, int) {
  printf("Form_Alert called.\n");
  return 0;
}

void Unsupported_Handler(UNSUPPORT_INFO*, int type) {
  std::string feature = "Unknown";
  switch (type) {
    case FPDF_UNSP_DOC_XFAFORM:
      feature = "XFA";
      break;
    case FPDF_UNSP_DOC_PORTABLECOLLECTION:
      feature = "Portfolios_Packages";
      break;
    case FPDF_UNSP_DOC_ATTACHMENT:
    case FPDF_UNSP_ANNOT_ATTACHMENT:
      feature = "Attachment";
      break;
    case FPDF_UNSP_DOC_SECURITY:
      feature = "Rights_Management";
      break;
    case FPDF_UNSP_DOC_SHAREDREVIEW:
      feature = "Shared_Review";
      break;
    case FPDF_UNSP_DOC_SHAREDFORM_ACROBAT:
    case FPDF_UNSP_DOC_SHAREDFORM_FILESYSTEM:
    case FPDF_UNSP_DOC_SHAREDFORM_EMAIL:
      feature = "Shared_Form";
      break;
    case FPDF_UNSP_ANNOT_3DANNOT:
      feature = "3D";
      break;
    case FPDF_UNSP_ANNOT_MOVIE:
      feature = "Movie";
      break;
    case FPDF_UNSP_ANNOT_SOUND:
      feature = "Sound";
      break;
    case FPDF_UNSP_ANNOT_SCREEN_MEDIA:
    case FPDF_UNSP_ANNOT_SCREEN_RICHMEDIA:
      feature = "Screen";
      break;
    case FPDF_UNSP_ANNOT_SIG:
      feature = "Digital_Signature";
      break;
  }
  printf("Unsupported feature: %s.\n", feature.c_str());
}

class TestLoader {
 public:
  TestLoader(const char* pBuf, size_t len);

  const char* m_pBuf;
  size_t m_Len;
};

TestLoader::TestLoader(const char* pBuf, size_t len)
    : m_pBuf(pBuf), m_Len(len) {
}

int Get_Block(void* param, unsigned long pos, unsigned char* pBuf,
              unsigned long size) {
  TestLoader* pLoader = (TestLoader*) param;
  if (pos + size < pos || pos + size > pLoader->m_Len) return 0;
  memcpy(pBuf, pLoader->m_pBuf + pos, size);
  return 1;
}

bool Is_Data_Avail(FX_FILEAVAIL* pThis, size_t offset, size_t size) {
  return true;
}

void Add_Segment(FX_DOWNLOADHINTS* pThis, size_t offset, size_t size) {
}

#include "Spi_api.h"
extern void SetErrorImage(std::vector<char> &v2);
extern void SetArchiveInfo(std::vector<SPI_FILEINFO> &v1, DWORD dwSize, DWORD dwPos, DWORD timestamp);

std::pair<double, double> GetFactor()
{
	HDC hDC = GetDC(NULL);
	int x = GetDeviceCaps(hDC, LOGPIXELSX);
	int y = GetDeviceCaps(hDC, LOGPIXELSY);
	ReleaseDC(NULL, hDC);
	return std::make_pair(1. / 72 * x, 1. / 72 * y);
}

void RenderPdf(std::vector<SPI_FILEINFO> &v1, std::vector<std::vector<char> > &v2, const char* pBuf, size_t len, DWORD timestamp) {
  auto scale = GetFactor();

  IPDF_JSPLATFORM platform_callbacks;
  memset(&platform_callbacks, '\0', sizeof(platform_callbacks));
  platform_callbacks.version = 1;
  platform_callbacks.app_alert = Form_Alert;

  FPDF_FORMFILLINFO form_callbacks;
  memset(&form_callbacks, '\0', sizeof(form_callbacks));
  form_callbacks.version = 1;
  form_callbacks.m_pJsPlatform = &platform_callbacks;

  TestLoader loader(pBuf, len);

  FPDF_FILEACCESS file_access;
  memset(&file_access, '\0', sizeof(file_access));
  file_access.m_FileLen = static_cast<unsigned long>(len);
  file_access.m_GetBlock = Get_Block;
  file_access.m_Param = &loader;

  FX_FILEAVAIL file_avail;
  memset(&file_avail, '\0', sizeof(file_avail));
  file_avail.version = 1;
  file_avail.IsDataAvail = Is_Data_Avail;

  FX_DOWNLOADHINTS hints;
  memset(&hints, '\0', sizeof(hints));
  hints.version = 1;
  hints.AddSegment = Add_Segment;

  FPDF_DOCUMENT doc;
  FPDF_AVAIL pdf_avail = FPDFAvail_Create(&file_avail, &file_access);

  (void) FPDFAvail_IsDocAvail(pdf_avail, &hints);

  if (!FPDFAvail_IsLinearized(pdf_avail)) {
    printf("Non-linearized path...\n");
    doc = FPDF_LoadCustomDocument(&file_access, NULL);
  } else {
    printf("Linearized path...\n");
    doc = FPDFAvail_GetDocument(pdf_avail, NULL);
  }

  (void) FPDF_GetDocPermissions(doc);
  (void) FPDFAvail_IsFormAvail(pdf_avail, &hints);

  FPDF_FORMHANDLE form = FPDFDOC_InitFormFillEnvironment(doc, &form_callbacks);
  FPDF_SetFormFieldHighlightColor(form, 0, 0xFFE4DD);
  FPDF_SetFormFieldHighlightAlpha(form, 100);

  int first_page = FPDFAvail_GetFirstPageNum(doc);
  (void) FPDFAvail_IsPageAvail(pdf_avail, first_page, &hints);

  int page_count = FPDF_GetPageCount(doc);
  for (int i = 0; i < page_count; ++i) {
    (void) FPDFAvail_IsPageAvail(pdf_avail, i, &hints);
  }

  FORM_DoDocumentJSAction(form);
  FORM_DoDocumentOpenAction(form);

  size_t rendered_pages = 0;
  size_t bad_pages = 0;
  for (int i = 0; i < page_count; ++i) {
	v2.push_back(std::vector<char>());
	FPDF_PAGE page = FPDF_LoadPage(doc, i);
    if (!page) {
		SetErrorImage(v2.back());
		SetArchiveInfo(v1, v2.back().size(), i, timestamp);
		bad_pages++;
        continue;
    }
    FPDF_TEXTPAGE text_page = FPDFText_LoadPage(page);
    FORM_OnAfterLoadPage(page, form);
    FORM_DoPageAAction(page, form, FPDFPAGE_AACTION_OPEN);

    int width = static_cast<int>(FPDF_GetPageWidth(page) * scale.first +.5);
    int height = static_cast<int>(FPDF_GetPageHeight(page) * scale.second +.5);
    FPDF_BITMAP bitmap = FPDFBitmap_Create(width, height, 0);
    FPDFBitmap_FillRect(bitmap, 0, 0, width, height, 0xFFFFFFFF);

    FPDF_RenderPageBitmap(bitmap, page, 0, 0, width, height, 0, 0);
    rendered_pages ++;

    FPDF_FFLDraw(form, bitmap, page, 0, 0, width, height, 0, 0);
    int stride = FPDFBitmap_GetStride(bitmap);
    const char* buffer =
        reinterpret_cast<const char*>(FPDFBitmap_GetBuffer(bitmap));

	if (!WriteBmp(v2.back(), buffer, stride, width, height)) {
      SetErrorImage(v2.back());
	}
	SetArchiveInfo(v1, v2.back().size(), i, timestamp);

    FPDFBitmap_Destroy(bitmap);

    FORM_DoPageAAction(page, form, FPDFPAGE_AACTION_CLOSE);
    FORM_OnBeforeClosePage(page, form);
    FPDFText_ClosePage(text_page);
    FPDF_ClosePage(page);
  }

  FORM_DoDocumentAAction(form, FPDFDOC_AACTION_WC);
  FPDFDOC_ExitFormFillEnvironment(form);
  FPDF_CloseDocument(doc);
  FPDFAvail_Destroy(pdf_avail);

  printf("Loaded, parsed and rendered %" PRIuS " pages.\n", rendered_pages);
  printf("Skipped %" PRIuS " bad pages.\n", bad_pages);
}

static UNSUPPORT_INFO unsuppored_info;

void InitPDFium() {
  v8::V8::InitializeICU();

  FPDF_InitLibrary();

  memset(&unsuppored_info, '\0', sizeof(unsuppored_info));
  unsuppored_info.version = 1;
  unsuppored_info.FSDK_UnSupport_Handler = Unsupported_Handler;

  FSDK_SetUnSpObjProcessHandler(&unsuppored_info);
}

void ProcessPDF(std::string filename, std::vector<SPI_FILEINFO> &v1, std::vector<std::vector<char> > &v2, DWORD timestamp) {
  size_t file_length = 0;
  char* file_contents = GetFileContents(filename.c_str(), &file_length);
  if (!file_contents)
	return;
  RenderPdf(v1, v2, file_contents, file_length, timestamp);
  free(file_contents);
}

void FreePDFium() {
  FPDF_DestroyLibrary();
}
