// gcc ffmpeg-frames.c `pkg-config --cflags --libs libavutil libavformat` -Wall && ./a.out
// https://github.com/FFmpeg/FFmpeg/blob/master/doc/examples/demuxing_decoding.c
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>

static AVCodecContext *video_dec_ctx = NULL;
static int width, height;
static enum AVPixelFormat pix_fmt;
static AVStream *video_stream = NULL;

static uint8_t *video_dst_data[4] = {NULL};
static int video_dst_linesize[4];
static int video_dst_bufsize;

static int video_stream_idx = -1;
static AVFrame *frame = NULL;
static AVPacket pkt;

void callback(uint8_t *ptr, unsigned len, unsigned width, unsigned height, unsigned pix_fmt);

static int output_video_frame(AVFrame *frame) {
  if (frame->width != width || frame->height != height ||
      frame->format != pix_fmt) {
    return -1;
  }
  av_image_copy(video_dst_data, video_dst_linesize,
                (const uint8_t **)(frame->data), frame->linesize, pix_fmt,
                width, height);
  callback(video_dst_data[0], video_dst_bufsize, width, height, pix_fmt);
  return 0;
}

static int decode_packet(AVCodecContext *dec, const AVPacket *pkt) {
  int ret = avcodec_send_packet(dec, pkt);
  if (ret < 0) {
    return ret;
  }
  while (ret >= 0) {
    ret = avcodec_receive_frame(dec, frame);
    if (ret < 0) {
      if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
        return 0;
      return ret;
    }
    if (dec->codec->type == AVMEDIA_TYPE_VIDEO)
      ret = output_video_frame(frame);
    av_frame_unref(frame);
    if (ret < 0)
      return ret;
  }

  return 0;
}

static int open_codec_context(int *stream_idx, AVCodecContext **dec_ctx,
                              AVFormatContext *fmt_ctx, enum AVMediaType type) {
  AVStream *st;
  AVCodec *dec = NULL;
  AVDictionary *opts = NULL;
  int ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
  if (ret < 0) {
    return ret;
  } else {
    int stream_index = ret;
    st = fmt_ctx->streams[stream_index];
    dec = avcodec_find_decoder(st->codecpar->codec_id);
    if (!dec)
      return AVERROR(EINVAL);
    *dec_ctx = avcodec_alloc_context3(dec);
    if (!*dec_ctx)
      return AVERROR(ENOMEM);
    if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0)
      return ret;
    if ((ret = avcodec_open2(*dec_ctx, dec, &opts)) < 0)
      return ret;
    *stream_idx = stream_index;
  }

  return 0;
}

struct buffer_data {
  uint8_t *ptr;
  size_t size;
};

static int read_packet(void *opaque, uint8_t *buf, int buf_size) {
  struct buffer_data *bd = (struct buffer_data *)opaque;
  buf_size = FFMIN(buf_size, bd->size);
  memcpy(buf, bd->ptr, buf_size);
  bd->ptr += buf_size;
  bd->size -= buf_size;
  return buf_size;
}

int decode(uint8_t *buf, unsigned len) {
  struct buffer_data buffer = {.ptr = buf, .size = len};
  // probably not the best just that good enough for here
  uint8_t *file_stream_buffer = (uint8_t *)av_malloc(len);
  AVIOContext *io_context =
      avio_alloc_context(file_stream_buffer, len, 0, &buffer,
                         read_packet, NULL, NULL);
  if (!io_context)
    abort();
  AVFormatContext *fmt_ctx = avformat_alloc_context();
  fmt_ctx->pb = io_context;
  fmt_ctx->flags |= AVFMT_FLAG_CUSTOM_IO;
  if (avformat_open_input(&fmt_ctx, "", 0, 0) < 0)
    abort();
  if (avformat_find_stream_info(fmt_ctx, NULL) < 0)
    abort();
  int ret = 0;
  if (open_codec_context(&video_stream_idx, &video_dec_ctx, fmt_ctx,
                         AVMEDIA_TYPE_VIDEO) >= 0) {
    video_stream = fmt_ctx->streams[video_stream_idx];
    width = video_dec_ctx->width;
    height = video_dec_ctx->height;
    pix_fmt = video_dec_ctx->pix_fmt;
    ret = av_image_alloc(video_dst_data, video_dst_linesize, width, height,
                         pix_fmt, 1);
    if (ret < 0)
      goto end;
    video_dst_bufsize = ret;
  }
  if (!video_stream) {
    ret = 1;
    goto end;
  }
  frame = av_frame_alloc();
  if (!frame) {
    ret = AVERROR(ENOMEM);
    goto end;
  }
  av_init_packet(&pkt);
  pkt.data = NULL;
  pkt.size = 0;
  while (av_read_frame(fmt_ctx, &pkt) >= 0) {
    if (pkt.stream_index == video_stream_idx)
      ret = decode_packet(video_dec_ctx, &pkt);
    av_packet_unref(&pkt);
    if (ret < 0)
      break;
  }
  if (video_dec_ctx)
    decode_packet(video_dec_ctx, NULL);

end:
  avcodec_free_context(&video_dec_ctx);
  avformat_close_input(&fmt_ctx);
  av_frame_free(&frame);
  av_free(video_dst_data[0]);
  av_free(io_context);
  return ret < 0;
}

#ifndef NO_MAIN
void callback(uint8_t *ptr, unsigned len, unsigned width, unsigned height, unsigned pix_fmt) {
  printf("%d %d %d %d: %d %d %d %d...\n", width, height, pix_fmt, len, ptr[0], ptr[1], ptr[2], ptr[3]);
}
int main() {
  uint8_t *buf;
  unsigned len;
  {
    FILE *f = fopen("input/test.mp4", "rb");
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    rewind(f);
    buf = (uint8_t *)malloc(len);
    len = fread(buf, 1, len, f);
    fclose(f);
  }
  decode(buf, len);
  free(buf);
  return 0;
}
#endif
