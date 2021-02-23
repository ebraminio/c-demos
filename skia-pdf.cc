// vcpkg install skia
/* clang++ -I ~/vcpkg/installed/x64-osx/include/ -std=c++14 \
   -L ~/vcpkg/installed/x64-osx/lib -framework CoreText -framework CoreFoundation \
   -framework CoreGraphics -framework ApplicationServices -lbrotlicommon-static \
   -lbrotlidec-static -lbrotlienc-static -lbz2 -lexpat -lfreetype -lharfbuzz-icu \
    -lharfbuzz-subset -lharfbuzz -licudata -licui18n -licuio -licutu -licuuc \
   -lintl -ljpeg -lpng -lpng16 -lskia -lturbojpeg -lwebp -lwebpdecoder \
   -lwebpdemux -lwebpmux -lz skia-pdf.cc -Wall && ./a.out
*/
// https://skia.org/user/sample/pdf
#include "skia/core/SkCanvas.h"
#include "skia/core/SkFont.h"
#include "skia/core/SkGraphics.h"
#include "skia/core/SkPath.h"
#include "skia/core/SkSurface.h"
#include "skia/docs/SkPDFDocument.h"
#include "skia/effects/SkGradientShader.h"

int main() {
  SkDynamicMemoryWStream buffer;
  SkPDF::Metadata metadata;
  metadata.fTitle = "SkPDF Example";
  metadata.fCreator = "Example WritePDF() Function";
  metadata.fCreation = {0, 2019, 1, 4, 31, 12, 34, 56};
  metadata.fModified = {0, 2019, 1, 4, 31, 12, 34, 56};
  auto pdfDocument = SkPDF::MakeDocument(&buffer, metadata);
  SkASSERT(pdfDocument);
  for (unsigned page = 0; page < 2; ++page) {
    constexpr SkSize ansiLetterSize{8.5f * 72, 11.0f * 72};
    SkCanvas *pageCanvas =
        pdfDocument->beginPage(ansiLetterSize.width(), ansiLetterSize.height());
    {
      const SkScalar R = 115.2f, C = 128.0f;
      SkPath path;
      path.moveTo(C + R, C);
      for (int i = 1; i < 8; ++i) {
        SkScalar a = 2.6927937f * i;
        path.lineTo(C + R * cos(a), C + R * sin(a));
      }
      SkPaint paint;
      paint.setStyle(SkPaint::kStroke_Style);
      pageCanvas->drawPath(path, paint);
    }
    pdfDocument->endPage();
  }
  pdfDocument->close();

  sk_sp<SkData> pdfData = buffer.detachAsData();
  SkData *data = pdfData.get();
  FILE *f = fopen("out.pdf", "wb");
  fwrite(data->bytes(), 1, data->size(), f);
  fclose(f);
  return 0;
}
