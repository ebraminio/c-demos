#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <hb.h>
#include <hb-ft.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define FONT_SIZE 300
#define MARGIN (FONT_SIZE * .5)

int main(int argc, char* argv[]) {
  const char *fontfile;
  const char *text;


  if (argc < 3) {
    fprintf(stderr, "usage: hello-harfbuzz font-file.ttf text\n");
    exit(1);
  }

  fontfile = argv[1];
  text = argv[2];
  
  /* Initialize FreeType and create FreeType font face. */
  FT_Library ft_library;
  FT_Face ft_face;
  FT_Error ft_error;


  if ((ft_error = FT_Init_FreeType(&ft_library)))
    abort();
  if ((ft_error = FT_New_Face(ft_library, fontfile, 0, &ft_face)))
    abort();
  if ((ft_error = FT_Set_Char_Size(ft_face, FONT_SIZE * 64, FONT_SIZE * 64, 0, 0)))
    abort();

  /* Create hb-ft font. */
  hb_font_t *hb_font;
  hb_font = hb_ft_font_create(ft_face, NULL);

  /* Create hb-buffer and populate. */
  hb_buffer_t *hb_buffer;
  hb_buffer = hb_buffer_create();
  hb_buffer_add_utf8(hb_buffer, text, -1, 0, -1);
  hb_buffer_guess_segment_properties(hb_buffer);

  /* Shape it! */
  hb_shape(hb_font, hb_buffer, NULL, 0);

  /* Print glyph names and positions out. */
  unsigned int len;
  hb_glyph_info_t *infos;
  hb_glyph_position_t *positions;
  len = hb_buffer_get_length(hb_buffer);
  infos = hb_buffer_get_glyph_infos(hb_buffer, NULL);
  positions = hb_buffer_get_glyph_positions(hb_buffer, NULL);

  /* Draw */
  int width = 2 * MARGIN;
  int height = 2 * MARGIN;
  for (unsigned int i = 0; i < len; i++)
  {
    width += positions[i].x_advance / 64.;
    height -= positions[i].y_advance / 64.;
  }
  if (HB_DIRECTION_IS_HORIZONTAL(hb_buffer_get_direction(hb_buffer)))
    height += FONT_SIZE;
  else
    width += FONT_SIZE;
  
  width = pow(2, (int)log2(width) + 1);
  int image_len = width * height;
  unsigned char *image = (unsigned char*)calloc(image_len, sizeof(unsigned char));

  FT_GlyphSlot slot = ft_face->glyph;

  int x = MARGIN, y = MARGIN;
  if (HB_DIRECTION_IS_HORIZONTAL(hb_buffer_get_direction(hb_buffer)))
    y += (FONT_SIZE - ft_face->size->metrics.height / 64.) * .5 +
      ft_face->size->metrics.ascender / 64.;
  else
    x += FONT_SIZE * .5;

  for (int i = 0; i < len; ++i) {
    FT_Load_Glyph(ft_face, infos[i].codepoint, FT_LOAD_DEFAULT);
    FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL);

    const int src_x = x + (positions[i].x_offset / 64.) + slot->bitmap_left;
    const int src_y = y - (positions[i].y_offset / 64.) - slot->bitmap_top;
    const int src_height = slot->bitmap.rows;
    const int src_width = slot->bitmap.width;
    const int width_diff = width - src_width;
    int dst_index = src_y * width + src_x;
    int src_index = 0;
    for (int row = 0; row < src_height; dst_index += width_diff, ++row)
      for (int index = 0; index < src_width; ++index, ++dst_index, ++src_index)
        if (dst_index < image_len && dst_index > 0)
          image[dst_index] |= slot->bitmap.buffer[src_index];

    x += positions[i].x_advance / 64.;
    y -= positions[i].y_advance / 64.;
  }

  /* Display */
  // http://antongerdelan.net/opengl
  GLFWwindow* window = NULL;
  GLuint vao;
  GLuint vbo;
  
  // position and texture coords at the same time
  GLfloat points[] = {
    0.0, 0.0,
    1.0, 0.0,
    0.0, 1.0,
    1.0, 1.0,
  };
  const char* vertex_shader =
    "#version 400\n"
    "uniform float time;"
    "in vec2 vp;"
    "out vec2 vTexCoord;"
    "void main () {"
    "  gl_Position = vec4(vp * 2.0 - vec2(1.0, 1.0), 0.0, 1.0);"
    "  gl_Position.x *= sin(time);"
    "  vTexCoord = vp;"
    "}";
  const char* fragment_shader =
    "#version 400\n"
    "uniform sampler2D basic_texture;"
    "in vec2 vTexCoord;"
    "out vec4 frag_color;"
    "void main () {"
    "  frag_color = texture(basic_texture, vec2(vTexCoord.x, 1 - vTexCoord.y));"
    "  frag_color = vec4(frag_color.r, frag_color.r, frag_color.r, 1.0);"
    "}";
  GLuint vs, fs;
  GLuint shader_programme;

  if (!glfwInit()) {
    fprintf(stderr, "ERROR: could not start GLFW3\n");
    return 1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  glfwWindowHint(GLFW_SAMPLES, 4);

  window = glfwCreateWindow(width, height, text, NULL, NULL);
  if (!window) {
    fprintf(stderr, "ERROR: could not open window with GLFW3\n");
    glfwTerminate();
    return 1;
  }
  glfwMakeContextCurrent(window);
  glewExperimental = GL_TRUE;
  glewInit();
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), points, GL_STATIC_DRAW);

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

  vs = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vs, 1, &vertex_shader, NULL);
  glCompileShader(vs);
  fs = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fs, 1, &fragment_shader, NULL);
  glCompileShader(fs);
  shader_programme = glCreateProgram();
  glAttachShader(shader_programme, fs);
  glAttachShader(shader_programme, vs);
  glLinkProgram(shader_programme);

  GLuint tex = 0;
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED,
    GL_UNSIGNED_BYTE, image);
  glUniform1i(glGetUniformLocation(shader_programme, "image"), tex);
  glGenerateMipmap(GL_TEXTURE_2D);

  GLuint time_uniform = glGetUniformLocation(shader_programme, "time");

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glUseProgram(shader_programme);
    glUniform1f(time_uniform, glfwGetTime());
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glfwPollEvents();
    glfwSwapBuffers(window);
  }
  
  /* free resources */
  free(image);

  hb_buffer_destroy(hb_buffer);
  hb_font_destroy(hb_font);

  FT_Done_Face(ft_face);
  FT_Done_FreeType(ft_library);

  glDeleteProgram(shader_programme);
  glfwTerminate();

  return 0;
}
