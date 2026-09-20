#include <GLES2/gl2.h>
#include <stdio.h>
#include <stdlib.h>

// gfx.c - GLES2 only. No SDL includes; the platform layer owns the context.
#include "gfx.h"

#include "platform.h"

static GLuint fbo, tex, quad;
static GLuint scene_prog, crt_prog;
static GLint  scene_uTime;
static GLint  crt_uTex, crt_uSrc, crt_uTime;
static GLint  crt_uAberation;

static uint32_t pixels[SCREEN_W * SCREEN_H];
static int      flip_y;      // 1 when the last thing drawn was the CPU buffer
static GLint    crt_uFlip;

static char *load_asset_file(const char *asset_name) {
  char fp[512];
  size_t path_len = sizeof fp;

  if (!get_asset_path(fp, path_len, asset_name)) {
    return NULL;
  }

  // asset exists, load it into a malloc'd buffer
  LOG("asset_load: %s\n", fp);

  FILE *f = fopen(fp, "rb");
  if (!f) return NULL;

  fseek(f, 0, SEEK_END);
  long size = ftell(f);
  fseek(f, 0, SEEK_SET);

  if (size <= 0) {
    fclose(f);
    return NULL;
  }

  char *buf = malloc((size_t)size + 1);
  if (!buf) {
    fclose(f);
    return NULL;
  }

  if (fread(buf, 1, (size_t)size, f) != (size_t)size) {
    free(buf);
    fclose(f);
    return NULL;
  }

  buf[size] = '\0';
  fclose(f);
  return buf;
}


static GLuint compile(GLenum type, const char* src) {
  GLuint s = glCreateShader(type);
  glShaderSource(s, 1, &src, NULL);
  glCompileShader(s);

  GLint ok = 0;
  glGetShaderiv(s, GL_COMPILE_STATUS, &ok);

  if (!ok) {
    char log[1024];
    glGetShaderInfoLog(s, sizeof log, NULL, log);
    fprintf(stderr, "shader compile error:\n%s\n", log);
    glDeleteShader(s);

    return 0;
  }

  return s;
}


static GLuint make_program(const char* vs_src, const char* fs_src) {
  GLuint vs = compile(GL_VERTEX_SHADER, vs_src);
  GLuint fs = compile(GL_FRAGMENT_SHADER, fs_src);
  if (!vs || !fs) return 0;

  GLuint p = glCreateProgram();
  glAttachShader(p, vs);
  glAttachShader(p, fs);

  glBindAttribLocation(p, 0, "aPos");
  glLinkProgram(p);
  glDeleteShader(vs);
  glDeleteShader(fs);

  GLint ok = 0;
  glGetProgramiv(p, GL_LINK_STATUS, &ok);
  if (!ok) {
    char log[1024];
    
    glGetProgramInfoLog(p, sizeof log, NULL, log);
    fprintf(stderr, "program link error:\n%s\n", log);

    return 0;
  }

  return p;
}


static void draw_quad(void) {
  glBindBuffer(GL_ARRAY_BUFFER, quad);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}


static unsigned load_and_compile_shaders(void) {
  char *vertex;
  if (!(vertex = load_asset_file("shaders/screen.vs"))) {
    LOG("Failed to load screen.vs");
    return 0;
  }

  char *scene;
  if (!(scene = load_asset_file("shaders/scene.fs"))) {
    LOG("Failed to load scene.fs");
    free(vertex);
    return 0;
  }

  char *crt;
  if (!(crt = load_asset_file("shaders/crt.fs"))) {
    LOG("Failed to load crt.fs");
    free(vertex);
    free(scene);
    return 0;
  }

  scene_prog = make_program(vertex, scene);
  crt_prog   = make_program(vertex, crt);

  free(vertex);
  free(scene);
  free(crt);
  return 1;
}


int gfx_init(void) {
  // fullscreen quad
  static const float verts[] = { -1,-1,  1,-1,  -1,1,  1,1 };
  glGenBuffers(1, &quad);
  glBindBuffer(GL_ARRAY_BUFFER, quad);
  glBufferData(GL_ARRAY_BUFFER, sizeof verts, verts, GL_STATIC_DRAW);
  
  // low-res color target (nearest filtering = crisp pixels when scaled up)
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREEN_W, SCREEN_H, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D, tex, 0);
  
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    fprintf(stderr, "framebuffer incomplete\n");
    return 0;
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);


  if (!load_and_compile_shaders() || !scene_prog || !crt_prog) return 0;
  
  scene_uTime    = glGetUniformLocation(scene_prog, "uTime");
  crt_uTex       = glGetUniformLocation(crt_prog, "uTex");
  crt_uSrc       = glGetUniformLocation(crt_prog, "uSrc");
  crt_uTime      = glGetUniformLocation(crt_prog, "uTime");
  crt_uAberation = glGetUniformLocation(crt_prog, "uAberation");

  crt_uFlip = glGetUniformLocation(crt_prog, "uFlipY");

  return 1;
}


void gfx_shutdown(void) {
  glDeleteProgram(scene_prog);
  glDeleteProgram(crt_prog);
  glDeleteFramebuffers(1, &fbo);
  glDeleteTextures(1, &tex);
  glDeleteBuffers(1, &quad);
}


uint32_t *gfx_pixels(void) { return pixels; }


void gfx_upload(void) {
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, SCREEN_W, SCREEN_H,
                  GL_RGBA, GL_UNSIGNED_BYTE, pixels);
  flip_y = 1;   // buffer is top-down, GL textures are bottom-up
}


void gfx_begin_screen(void) {
  flip_y = 0;
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glViewport(0, 0, SCREEN_W, SCREEN_H);
}


void gfx_draw_startup(float t) {
  glUseProgram(scene_prog);
  glUniform1f(scene_uTime, t);
  draw_quad();
}


void gfx_present(int win_w, int win_h, float t) {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  // black bars, then a centered rect that keeps the 4:3 aspect
  glViewport(0, 0, win_w, win_h);
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  
  float sx = (float)win_w / SCREEN_W;
  float sy = (float)win_h / SCREEN_H;
  float s  = sx < sy ? sx : sy;
  int vw = (int)(SCREEN_W * s);
  int vh = (int)(SCREEN_H * s);
  glViewport((win_w - vw) / 2, (win_h - vh) / 2, vw, vh);
  
  glUseProgram(crt_prog);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex);
  glUniform1i(crt_uTex, 0);
  glUniform2f(crt_uSrc, (float)SCREEN_W, (float)SCREEN_H);
  glUniform1f(crt_uTime, t);
  glUniform1f(crt_uAberation, 0.0015);
  glUniform1f(crt_uFlip, (float)flip_y);

  draw_quad();
}
