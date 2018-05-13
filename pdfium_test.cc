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

#include <iterator>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "public/cpp/fpdf_scopers.h"
#include "public/fpdf_annot.h"
#include "public/fpdf_attachment.h"
#include "public/fpdf_dataavail.h"
#include "public/fpdf_edit.h"
#include "public/fpdf_ext.h"
#include "public/fpdf_formfill.h"
#include "public/fpdf_progressive.h"
#include "public/fpdf_structtree.h"
#include "public/fpdf_text.h"
#include "public/fpdfview.h"

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#ifdef _WIN32
#define access _access
#define snprintf _snprintf
#define R_OK 4
#endif

///////////////////////////////////////////////////////////////////////
#include <cstdarg>
#include <Strsafe.h>
#ifndef NUM_OUTPUT_DEBUG_PRINTF_BUFFER
#define NUM_OUTPUT_DEBUG_PRINTF_BUFFER 2048
#endif
inline void OutputDebugPrintf(LPCTSTR format, ...)
{
	std::va_list ap;
	va_start(ap, format);
	TCHAR buf[NUM_OUTPUT_DEBUG_PRINTF_BUFFER];
	StringCbVPrintf(buf, sizeof(buf), format, ap);
	OutputDebugString(buf);
	va_end(ap);
}
inline void OutputDebugVPrintf(LPCTSTR format, std::va_list ap)
{
	TCHAR buf[NUM_OUTPUT_DEBUG_PRINTF_BUFFER];
	StringCbVPrintf(buf, sizeof(buf), format, ap);
	OutputDebugString(buf);
}
///////////////////////////////////////////////////////////////////////

// from testing/test_support.h
namespace pdfium {
// Used with std::unique_ptr to free() objects that can't be deleted.
struct FreeDeleter {
  inline void operator()(void* ptr) const { free(ptr); }
};
}  // namespace pdfium

// from testing/test_support.cpp
static std::unique_ptr<char, pdfium::FreeDeleter> GetFileContents(const char* filename,
                                                           size_t* retlen) {
  FILE* file = fopen(filename, "rb");
  if (!file) {
    fprintf(stderr, "Failed to open: %s\n", filename);
    return nullptr;
  }
  (void)fseek(file, 0, SEEK_END);
  size_t file_length = ftell(file);
  if (!file_length) {
    return nullptr;
  }
  (void)fseek(file, 0, SEEK_SET);
  std::unique_ptr<char, pdfium::FreeDeleter> buffer(
      static_cast<char*>(malloc(file_length)));
  if (!buffer) {
    return nullptr;
  }
  size_t bytes_read = fread(buffer.get(), 1, file_length, file);
  (void)fclose(file);
  if (bytes_read != file_length) {
    fprintf(stderr, "Failed to read: %s\n", filename);
    return nullptr;
  }
  *retlen = bytes_read;
  return buffer;
}

// from testing/test_support.h
class TestLoader {
 public:
  TestLoader(const char* pBuf, size_t len);
  static int GetBlock(void* param,
                      unsigned long pos,
                      unsigned char* pBuf,
                      unsigned long size);

 private:
  const char* const m_pBuf;
  const size_t m_Len;
};

// from testing/test_support.cpp
TestLoader::TestLoader(const char* pBuf, size_t len)
    : m_pBuf(pBuf), m_Len(len) {
}

// from testing/test_support.cpp
// static
int TestLoader::GetBlock(void* param,
                         unsigned long pos,
                         unsigned char* pBuf,
                         unsigned long size) {
  TestLoader* pLoader = static_cast<TestLoader*>(param);
  if (pos + size < pos || pos + size > pLoader->m_Len)
    return 0;

  memcpy(pBuf, pLoader->m_pBuf + pos, size);
  return 1;
}

// from samples/pdfium_test_write_helper.cc
static bool CheckDimensions(int stride, int width, int height) {
  if (stride < 0 || width < 0 || height < 0)
    return false;
  if (height > 0 && width > INT_MAX / height)
    return false;
  return true;
}

static void WriteToVec(std::vector<char> &v, const void* p, std::size_t size) {
  auto b = static_cast<const char*>(p);
  auto e = static_cast<const char*>(p) + size;
  v.insert(v.end(), b, e);
}

// from samples/pdfium_test_write_helper.cc with modification
static bool WriteBmp(std::vector<char> &v,
                     const void* buffer,
                     int stride,
                     int width,
                     int height) {
  if (!CheckDimensions(stride, width, height))
    return "";

  int out_len = stride * height;
  if (out_len > INT_MAX / 3)
    return false;

  BITMAPINFO bmi = {};
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

struct FPDF_FORMFILLINFO_PDFiumTest : public FPDF_FORMFILLINFO {
  // Hold a map of the currently loaded pages in order to avoid them
  // to get loaded twice.
  std::map<int, ScopedFPDFPage> loaded_pages;

  // Hold a pointer of FPDF_FORMHANDLE so that PDFium app hooks can
  // make use of it.
  FPDF_FORMHANDLE form_handle;
};

FPDF_FORMFILLINFO_PDFiumTest* ToPDFiumTestFormFillInfo(
    FPDF_FORMFILLINFO* form_fill_info) {
  return static_cast<FPDF_FORMFILLINFO_PDFiumTest*>(form_fill_info);
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

void PrintLastError() {
  unsigned long err = FPDF_GetLastError();
  OutputDebugPrintf("Load pdf docs unsuccessful: ");
  switch (err) {
    case FPDF_ERR_SUCCESS:
      OutputDebugPrintf("Success");
      break;
    case FPDF_ERR_UNKNOWN:
      OutputDebugPrintf("Unknown error");
      break;
    case FPDF_ERR_FILE:
      OutputDebugPrintf("File not found or could not be opened");
      break;
    case FPDF_ERR_FORMAT:
      OutputDebugPrintf("File not in PDF format or corrupted");
      break;
    case FPDF_ERR_PASSWORD:
      OutputDebugPrintf("Password required or incorrect password");
      break;
    case FPDF_ERR_SECURITY:
      OutputDebugPrintf("Unsupported security scheme");
      break;
    case FPDF_ERR_PAGE:
      OutputDebugPrintf("Page not found or content error");
      break;
    default:
      OutputDebugPrintf("Unknown error %ld", err);
  }
  return;
}

FPDF_BOOL Is_Data_Avail(FX_FILEAVAIL* avail, size_t offset, size_t size) {
  return true;
}

void Add_Segment(FX_DOWNLOADHINTS* hints, size_t offset, size_t size) {}

FPDF_PAGE GetPageForIndex(FPDF_FORMFILLINFO* param,
                          FPDF_DOCUMENT doc,
                          int index) {
  FPDF_FORMFILLINFO_PDFiumTest* form_fill_info =
      ToPDFiumTestFormFillInfo(param);
  auto& loaded_pages = form_fill_info->loaded_pages;
  auto iter = loaded_pages.find(index);
  if (iter != loaded_pages.end())
    return iter->second.get();

  ScopedFPDFPage page(FPDF_LoadPage(doc, index));
  if (!page)
    return nullptr;

  // Mark the page as loaded first to prevent infinite recursion.
  FPDF_PAGE page_ptr = page.get();
  loaded_pages[index] = std::move(page);

  FPDF_FORMHANDLE& form_handle = form_fill_info->form_handle;
  FORM_OnAfterLoadPage(page_ptr, form_handle);
  FORM_DoPageAAction(page_ptr, form_handle, FPDFPAGE_AACTION_OPEN);
  return page_ptr;
}

#include "Spi_api.h"
extern void SetErrorImage(std::vector<char> &v2);
extern void SetArchiveInfo(std::vector<SPI_FILEINFO> &v1, DWORD dwSize, DWORD dwPos, DWORD timestamp);
extern void SetProgress(HWND hwnd, int nNum, int nDenom);

std::pair<double, double> GetFactor()
{
    HDC hDC = GetDC(NULL);
    int x = GetDeviceCaps(hDC, LOGPIXELSX);
    int y = GetDeviceCaps(hDC, LOGPIXELSY);
    ReleaseDC(NULL, hDC);
    return std::make_pair(1. / 72 * x, 1. / 72 * y);
}

bool RenderPage(std::vector<char> &v,
                FPDF_DOCUMENT doc,
                FPDF_FORMHANDLE form,
                FPDF_FORMFILLINFO_PDFiumTest* form_fill_info,
                const int page_index) {
  FPDF_PAGE page = GetPageForIndex(form_fill_info, doc, page_index);
  if (!page)
    return false;

  ScopedFPDFTextPage text_page(FPDFText_LoadPage(page));
  auto scale = GetFactor();

  auto width = static_cast<int>(FPDF_GetPageWidth(page) * scale.first +.5);
  auto height = static_cast<int>(FPDF_GetPageHeight(page) * scale.second +.5);
  int alpha = FPDFPage_HasTransparency(page) ? 1 : 0;
  ScopedFPDFBitmap bitmap(FPDFBitmap_Create(width, height, alpha));

  if (bitmap) {
    FPDF_DWORD fill_color = alpha ? 0x00000000 : 0xFFFFFFFF;
    FPDFBitmap_FillRect(bitmap.get(), 0, 0, width, height, fill_color);

    // Note, client programs probably want to use this method instead of the
    // progressive calls. The progressive calls are if you need to pause the
    // rendering to update the UI, the PDF renderer will break when possible.
    FPDF_RenderPageBitmap(bitmap.get(), page, 0, 0, width, height, 0,
                          FPDF_ANNOT);

    FPDF_FFLDraw(form, bitmap.get(), page, 0, 0, width, height, 0, FPDF_ANNOT);

    int stride = FPDFBitmap_GetStride(bitmap.get());
    const char* buffer =
        reinterpret_cast<const char*>(FPDFBitmap_GetBuffer(bitmap.get()));

    WriteBmp(v, buffer, stride, width, height);
  } else {
    OutputDebugPrintf("Page was too large to be rendered.\n");
  }

  FORM_DoPageAAction(page, form, FPDFPAGE_AACTION_CLOSE);
  FORM_OnBeforeClosePage(page, form);
  return !!bitmap;
}

void RenderPdf(std::vector<SPI_FILEINFO> &v1,
               std::vector<std::vector<char> > &v2,
               const char* pBuf,
               size_t len,
               DWORD timestamp,
               HWND hwnd) {
  TestLoader loader(pBuf, len);

  FPDF_FILEACCESS file_access = {};
  file_access.m_FileLen = static_cast<unsigned long>(len);
  file_access.m_GetBlock = TestLoader::GetBlock;
  file_access.m_Param = &loader;

  FX_FILEAVAIL file_avail = {};
  file_avail.version = 1;
  file_avail.IsDataAvail = Is_Data_Avail;

  FX_DOWNLOADHINTS hints = {};
  hints.version = 1;
  hints.AddSegment = Add_Segment;

  // The pdf_avail must outlive doc.
  ScopedFPDFAvail pdf_avail(FPDFAvail_Create(&file_avail, &file_access));

  // The document must outlive |form_callbacks.loaded_pages|.
  ScopedFPDFDocument doc;

  int nRet = PDF_DATA_NOTAVAIL;
  bool bIsLinearized = false;
  if (FPDFAvail_IsLinearized(pdf_avail.get()) == PDF_LINEARIZED) {
    doc.reset(FPDFAvail_GetDocument(pdf_avail.get(), nullptr));
    if (doc) {
      while (nRet == PDF_DATA_NOTAVAIL)
        nRet = FPDFAvail_IsDocAvail(pdf_avail.get(), &hints);

      if (nRet == PDF_DATA_ERROR) {
        v2.push_back(std::vector<char>());
        SetErrorImage(v2.back());
        SetArchiveInfo(v1, v2.back().size(), 0, timestamp);
        OutputDebugPrintf("Unknown error in checking if doc was available.\n");
        return;
      }
      nRet = FPDFAvail_IsFormAvail(pdf_avail.get(), &hints);
      if (nRet == PDF_FORM_ERROR || nRet == PDF_FORM_NOTAVAIL) {
        v2.push_back(std::vector<char>());
        SetErrorImage(v2.back());
        SetArchiveInfo(v1, v2.back().size(), 0, timestamp);
        OutputDebugPrintf("Unknown error in checking if doc was available.\n");
        OutputDebugPrintf(
                "Error %d was returned in checking if form was available.\n",
                nRet);
        return;
      }
      bIsLinearized = true;
    }
  } else {
    doc.reset(FPDF_LoadCustomDocument(&file_access, nullptr));
  }

  if (!doc) {
    v2.push_back(std::vector<char>());
    SetErrorImage(v2.back());
    SetArchiveInfo(v1, v2.back().size(), 0, timestamp);
    OutputDebugPrintf("Unknown error in checking if doc was available.\n");
    PrintLastError();
    return;
  }

  (void)FPDF_GetDocPermissions(doc.get());

  FPDF_FORMFILLINFO_PDFiumTest form_callbacks = {};
  form_callbacks.version = 1;
  form_callbacks.FFI_GetPage = GetPageForIndex;

  ScopedFPDFFormHandle form(
      FPDFDOC_InitFormFillEnvironment(doc.get(), &form_callbacks));
  form_callbacks.form_handle = form.get();

  FPDF_SetFormFieldHighlightColor(form.get(), FPDF_FORMFIELD_UNKNOWN, 0xFFE4DD);
  FPDF_SetFormFieldHighlightAlpha(form.get(), 100);
  FORM_DoDocumentJSAction(form.get());
  FORM_DoDocumentOpenAction(form.get());

  int page_count = FPDF_GetPageCount(doc.get());
  SetProgress(hwnd, 0, page_count);
  int rendered_pages = 0;
  int bad_pages = 0;
  int first_page = 0;
  int last_page = page_count;
  for (int i = first_page; i < last_page; ++i) {
    SetProgress(hwnd, i, page_count);
    v2.push_back(std::vector<char>());
    if (bIsLinearized) {
      nRet = PDF_DATA_NOTAVAIL;
      while (nRet == PDF_DATA_NOTAVAIL)
        nRet = FPDFAvail_IsPageAvail(pdf_avail.get(), i, &hints);

      if (nRet == PDF_DATA_ERROR) {
        OutputDebugPrintf("Unknown error in checking if page %d is available.\n",
                i);
        SetErrorImage(v2.back());
        SetArchiveInfo(v1, v2.back().size(), i, timestamp);
        bad_pages++;
        continue;
      }
    }
    if (RenderPage(v2.back(), doc.get(), form.get(), &form_callbacks, i)) {
      ++rendered_pages;
    } else {
      SetErrorImage(v2.back());
      ++bad_pages;
    }
    SetArchiveInfo(v1, v2.back().size(), i, timestamp);
  }

  FORM_DoDocumentAAction(form.get(), FPDFDOC_AACTION_WC);
  OutputDebugPrintf("Rendered %d pages.\n", rendered_pages);
  if (bad_pages)
    OutputDebugPrintf("Skipped %d bad pages.\n", bad_pages);
}

static UNSUPPORT_INFO unsuppored_info;

void InitPDFium() {
  FPDF_LIBRARY_CONFIG config;
  config.version = 2;
  config.m_pUserFontPaths = nullptr;
  config.m_pIsolate = nullptr;
  config.m_v8EmbedderSlot = 0;

  FPDF_InitLibraryWithConfig(&config);

  memset(&unsuppored_info, '\0', sizeof(unsuppored_info));
  unsuppored_info.version = 1;
  unsuppored_info.FSDK_UnSupport_Handler = Unsupported_Handler;

  FSDK_SetUnSpObjProcessHandler(&unsuppored_info);
}

void ProcessPDF(std::string filename, std::vector<SPI_FILEINFO> &v1, std::vector<std::vector<char> > &v2, DWORD timestamp, HWND hwnd) {
  size_t file_length = 0;
  std::unique_ptr<char, pdfium::FreeDeleter> file_contents =
    GetFileContents(filename.c_str(), &file_length);
  if (!file_contents)
    return;
  RenderPdf(v1, v2, file_contents.get(), file_length, timestamp, hwnd);
}

void FreePDFium() {
  FPDF_DestroyLibrary();
}
