/*
 * glview.cpp
 * 画面表示を司る関数群
 */
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <cmath>
#include <sstream>

#include <epoxy/gl.h>
#include "view.h"
#include "manager.h"
#include "mynetwork.h"

#if !GTKMM3
const int ss_divisor = 3; // frames
#endif

GLuint texnames[1];
Glib::RefPtr<Gdk::Pixbuf> img;
//=====

enum {
  X_AXIS,
  Y_AXIS,
  Z_AXIS,

  N_AXIS
};

struct vertex_info {
	float position[3];
	float color[3];
};

static const struct vertex_info vertex_data[] = {
  { {  0.0f,  0.500f, 0.0f }, { 1.f, 0.f, 0.f } },
  { {  0.5f, -0.366f, 0.0f }, { 0.f, 1.f, 0.f } },
  { { -0.5f, -0.366f, 0.0f }, { 0.f, 0.f, 1.f } },
};

static void compute_mvp(float *res,
                        float phi,
                        float theta,
                        float psi)
{
  float x       {phi * ((float)G_PI / 180.f)};
  float y       {theta * ((float)G_PI / 180.f)};
  float z       {psi * ((float)G_PI / 180.f)};
  float c1      {cosf (x)};
  float s1      {sinf (x)};
  float c2      {cosf (y)};
  float s2      {sinf (y)};
  float c3      {cosf (z)};
  float s3      {sinf (z)};
  float c3c2    {c3 * c2};
  float s3c1    {s3 * c1};
  float c3s2s1  {c3 * s2 * s1};
  float s3s1    {s3 * s1};
  float c3s2c1  {c3 * s2 * c1};
  float s3c2    {s3 * c2};
  float c3c1    {c3 * c1};
  float s3s2s1  {s3 * s2 * s1};
  float c3s1    {c3 * s1};
  float s3s2c1  {s3 * s2 * c1};
  float c2s1    {c2 * s1};
  float c2c1    {c2 * c1};

  /* apply all three rotations using the three matrices:
   *
   * ⎡  c3 s3 0 ⎤ ⎡ c2  0 -s2 ⎤ ⎡ 1   0  0 ⎤
   * ⎢ -s3 c3 0 ⎥ ⎢  0  1   0 ⎥ ⎢ 0  c1 s1 ⎥
   * ⎣   0  0 1 ⎦ ⎣ s2  0  c2 ⎦ ⎣ 0 -s1 c1 ⎦
   */
  res[0] = c3c2;  res[4] = s3c1 + c3s2s1;  res[8] = s3s1 - c3s2c1; res[12] = 0.f;
  res[1] = -s3c2; res[5] = c3c1 - s3s2s1;  res[9] = c3s1 + s3s2c1; res[13] = 0.f;
  res[2] = s2;    res[6] = -c2s1;         res[10] = c2c1;          res[14] = 0.f;
  res[3] = 0.f;   res[7] = 0.f;           res[11] = 0.f;           res[15] = 1.f;
}

void MyGLArea::init_buffers()
{
  glGenVertexArrays(1, &m_Vao);
  glBindVertexArray(m_Vao);

  glGenBuffers(1, &m_Buffer);
  glBindBuffer(GL_ARRAY_BUFFER, m_Buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);

  glEnableVertexAttribArray(position_index);
  glVertexAttribPointer (position_index, 3, GL_FLOAT, GL_FALSE,
                         sizeof (struct vertex_info),
                         (GLvoid *) (G_STRUCT_OFFSET (struct vertex_info, position)));

   glEnableVertexAttribArray (color_index);
   glVertexAttribPointer (color_index, 3, GL_FLOAT, GL_FALSE,
                         sizeof (struct vertex_info),
                         (GLvoid *) (G_STRUCT_OFFSET (struct vertex_info, color)));

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

static GLuint create_shader(int type, const char *src)
{
  auto shader = glCreateShader(type);
  glShaderSource(shader, 1, &src, nullptr);
  glCompileShader(shader);

  int status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if(status == GL_FALSE)
  {
    int log_len;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);

    std::string log_space(log_len+1, ' ');
    glGetShaderInfoLog(shader, log_len, nullptr, (GLchar*)log_space.c_str());

    std::cerr << "Compile failure in " <<
      (type == GL_VERTEX_SHADER ? "vertex" : "fragment") <<
      " shader: " << log_space << std::endl;

    glDeleteShader(shader);

    return 0;
  }

  return shader;
}

void MyGLArea::init_shaders()
{
	char vshader[] = " \
#version 330 \n\
layout (location=0) in vec3 vertex_pos; \n\
layout (location=1) in vec4 vertex_color; \n\
out vec4 color; \n\
void main() \n\
{ \n\
    gl_Position = vec4(vertex_pos, 1.0); \n\
    color = vertex_color; \n\
}";
  auto vertex = create_shader(GL_VERTEX_SHADER, vshader);

  if(vertex == 0)
  {
    m_Program = 0;
    return;
  }

  char fshader[] = " \n\
#version 330 \n\
in vec4 color; \n\
layout (location=0) out vec4 frag_color; \n\
void main() \n\
{ \n\
    frag_color = color; \n\
}";
  auto fragment = create_shader(GL_FRAGMENT_SHADER, fshader);

  if(fragment == 0)
  {
    glDeleteShader(vertex);
    m_Program = 0;
    return;
  }

  m_Program = glCreateProgram();
  glAttachShader(m_Program, vertex);
  glAttachShader(m_Program, fragment);

  glLinkProgram(m_Program);

  int status;
  glGetProgramiv(m_Program, GL_LINK_STATUS, &status);
  if(status == GL_FALSE)
  {
    int log_len;
    glGetProgramiv(m_Program, GL_INFO_LOG_LENGTH, &log_len);

    std::string log_space(log_len+1, ' ');
    glGetProgramInfoLog(m_Program, log_len, nullptr, (GLchar*)log_space.c_str());

    std::cerr << "Linking failure: " << log_space << std::endl;

    glDeleteProgram(m_Program);
    m_Program = 0;
  }
  else
  {
    /* Get the location of the "mvp" uniform */
    m_Mvp = glGetUniformLocation(m_Program, "mvp");

    glDetachShader(m_Program, vertex);
    glDetachShader(m_Program, fragment);
  }
  glDeleteShader(vertex);
  glDeleteShader(fragment);
}
//=====

MyGLArea::MyGLArea(BaseObjectType* o,
		const Glib::RefPtr<Gtk::Builder>& g) :
		Gtk::GLArea(o) {
}

void MyGLArea::on_realize(void) {
	Gtk::GLArea::on_realize();
	make_current();
	Gtk::GLArea::set_size_request(600, 400);

  try
  {
    throw_if_error();
    init_buffers();
    init_shaders();
  }
  catch(const Gdk::GLError& gle)
  {
    std::cerr << "An error occured making the context current during realize:" << std::endl;
    std::cerr << gle.domain() << "-" << gle.code() << "-" << gle.what() << std::endl;
  }
  
//=====

//=====
//	GdkGLContext *gl_context = gtk_widget_get_gl_context(
//			(GtkWidget *) this->gobj());
//	GdkGLDrawable *gl_drawable =
//			gtk_widget_get_gl_drawable((GtkWidget *)this->gobj());
//	gdk_gl_drawable_gl_begin(gl_drawable, gl_context);
//	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//	if (gdk_gl_drawable_is_double_buffered(gl_drawable)) {
//		gdk_gl_drawable_swap_buffers(gl_drawable);
//	} else {
//		glFlush();
//	}
//	gdk_gl_drawable_gl_end(gl_drawable);

	img = Gdk::Pixbuf::create_from_file("sample.jpg");
	glGenTextures(1, texnames);
	glBindTexture(GL_TEXTURE_2D, texnames[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
//	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	if (img->get_n_channels() == 3) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img->get_width(),
				img->get_height(), 0, GL_RGB, GL_UNSIGNED_BYTE,
				img->get_pixels());
	} else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img->get_width(),
				img->get_height(), 0, GL_RGBA, GL_UNSIGNED_BYTE,
				img->get_pixels());
	}
	return;
}

void showPlayer(int x, int y) {
	GLfloat color[4];
	GLUquadricObj *q;
	q = gluNewQuadric();
	color[0] = 0.0f;
	color[1] = 0.0f;
	color[2] = 1.0f;
	color[3] = 0.3f;
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
	glPushMatrix();
	glRotated(90, 1, 0, 0);
	glTranslated(x, y, -100);
	gluCylinder(q, 0, 20, 100, 100, 20);
	gluSphere(q, 10, 100, 100);
	glPopMatrix();
}

void MyGLArea::draw_triangle()
{
  float mvp[16];

  compute_mvp(mvp, 0, 0, 0);

  glUseProgram(m_Program);

  glUniformMatrix4fv(m_Mvp, 1, GL_FALSE, &mvp[0]);

  glBindVertexArray(m_Vao);

//  glBindBuffer(GL_ARRAY_BUFFER, m_Vao);
//  glEnableVertexAttribArray(m_Vao);
//  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

  glDrawArrays(GL_TRIANGLES, 0, 3);

//  glDisableVertexAttribArray(0);
//  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glUseProgram(0);
}


bool MyGLArea::on_render(const Glib::RefPtr<Gdk::GLContext> &glc){
	Gtk::GLArea::on_render(glc);
	Manager &mgr = Manager::getInstance();
	Scene &scene=mgr.scene;

#define HOGE
#ifdef HOGE
	make_current();
  try
  {
    throw_if_error();
    glClearColor(scene.tm.tm_sec/60.0, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    draw_triangle();

    glFlush();

    return true;
  }
  catch(const Gdk::GLError& gle)
  {
    std::cerr << "An error occurred in the render callback of the GLArea" << std::endl;
    std::cerr << gle.domain() << "-" << gle.code() << "-" << gle.what() << std::endl;
    return false;
  }
#endif
	if (!scene.valid) {
//		GdkGLDrawable *gl_drawable =
//				gtk_widget_get_gl_drawable((GtkWidget *)this->gobj());
//		gdk_gl_drawable_gl_begin(gl_drawable, gl_context);
		glClearColor(0.4f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//		if (gdk_gl_drawable_is_double_buffered(gl_drawable)) {
//			gdk_gl_drawable_swap_buffers(gl_drawable);
//		} else {
			glFlush();
//		}
//		gdk_gl_drawable_gl_end(gl_drawable);
        glBindVertexArray (0);
        glUseProgram (0);
  		return true;
	}

	int ls = (int) fmin(this->get_width() * 0.5, this->get_height() * 0.5);
	int lm = (int) fmin(this->get_width() * 0.4, this->get_height() * 0.4);
	int lh = (int) fmin(this->get_width() * 0.25, this->get_height() * 0.25);

	int z = ls / 30;
//	GdkGLContext *gl_context = gtk_widget_get_gl_context(
//			(GtkWidget *) this->gobj());
//	GdkGLDrawable *gl_drawable =
//			gtk_widget_get_gl_drawable((GtkWidget *)this->gobj());

	// std::cout << scene.p[0].scale << ", " << scene.p[0].angle << std::endl;

//	gdk_gl_drawable_gl_begin(gl_drawable, gl_context);
	glViewport(0, 0, this->get_width(), this->get_height());

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(40, 1, ls / 10, 5.0 * ls);
	gluLookAt(0, 3.0 * ls, 2.0 * ls, 0.0, 0.0, 0.0, 0.0, 9.0, 0.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
//	glClearColor(0.2f, 0.4f, 0.2f, 1.0f);
	glClearColor(scene.tm.tm_sec/60.0, 0.4f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);

	GLfloat position[4] = { -(GLfloat) ls, (GLfloat) ls, (GLfloat) ls,
			(GLfloat) 1 };
	GLfloat color[4];
	GLUquadricObj *q;

	glLightfv(GL_LIGHT0, GL_POSITION, position);
//	color[0]=0.0; color[1]=0.0; color[2]=0.0; color[3]=1.0;
//	glLightfv(GL_LIGHT0, GL_AMBIENT, color);
//	color[0]=0.7; color[1]=0.7; color[2]=0.7; color[3]=1.0;
//	glLightfv(GL_LIGHT0, GL_DIFFUSE, color);
	color[0] = 1.0;
	color[1] = 1.0;
	color[2] = 1.0;
	color[3] = 1.0;
	glLightfv(GL_LIGHT0, GL_SPECULAR, color);
	q = gluNewQuadric();

	//	color[0]=0.2; color[1]=0.2; color[2]=0.2; color[3]=1.0;
	//	glMaterialfv(GL_FRONT, GL_AMBIENT, color);
	//	color[0]=0.8; color[1]=0.8; color[2]=0.8; color[3]=1.0;
	//	glMaterialfv(GL_FRONT, GL_DIFFUSE, color);
	color[0] = 1.0;
	color[1] = 1.0;
	color[2] = 1.0;
	color[3] = 0.0;
	glMaterialfv(GL_FRONT, GL_SPECULAR, color);
	color[0] = 128.0;
	glMaterialfv(GL_FRONT, GL_SHININESS, color);
	color[0] = 0;
	color[1] = 0;
	color[2] = 0;
	color[3] = 1.0;
	glMaterialfv(GL_FRONT, GL_EMISSION, color);

	color[0] = 0.8f;
	color[1] = 0.8f;
	color[2] = 0.8f;
	color[3] = 1.0f;
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
	glPushMatrix();
	glTranslated(0, -z / 2, 0);
	glRotated(270, 1, 0, 0);
	gluDisk(q, 0, ls, 120, 10);
	glPopMatrix();

	for (int i = 0; i < 12; ++i) {
		glPushMatrix();
		glRotated(-30 * i + 180, 0.0, 1.0, 0.0);
		glTranslated(0, -z / 2, lm);
		gluCylinder(q, z, z, ls - lm, 10, 10);
		glPopMatrix();
	}

	color[0] = 0.0f;
	color[1] = 0.0f;
	color[2] = 1.0f;
	color[3] = 0.8f;
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
	glPushMatrix();
	glTranslated(0, z, 0);
	glRotated(-6.0 * scene.tm.tm_sec + 180, 0.0, 1.0, 0.0);
	gluCylinder(q, z / 2, z / 3, ls, 10, 10);
	glPopMatrix();

	{
		int x, y;
		x = (int) (200 * sin(2.0 * M_PI * scene.tm.tm_sec / 60.0));
		y = (int) (-200 * cos(2.0 * M_PI * scene.tm.tm_sec / 60.0));
		showPlayer(x, y);
	}

	color[0] = 0.0f;
	color[1] = 1.0f;
	color[2] = 0.0f;
	color[3] = 0.8f;
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
	glPushMatrix();
	glTranslated(0, 2 * z, 0);
	glRotated(-6.0 * (scene.tm.tm_min + scene.tm.tm_sec / 60.0) + 180, 0.0, 1.0,
			0.0);
	gluCylinder(q, z / 2, z / 3, lm, 10, 10);
	glPopMatrix();

	color[0] = 1.0f;
	color[1] = 0.0f;
	color[2] = 0.0f;
	color[3] = 0.8f;
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
	glPushMatrix();
	glTranslated(0, 3 * z, 0);
	glRotated(-30.0 * (scene.tm.tm_hour + scene.tm.tm_min / 60.0) + 180, 0.0,
			1.0, 0.0);
	gluCylinder(q, z / 2, z / 3, lh, 10, 10);
	glPopMatrix();

	///画像の読み込み
	color[0] = 0.8f;
	color[1] = 1.0f;
	color[2] = 1.0f;
	color[3] = 0.8f;
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);

	//テクスチャ貼り付け
	glNormal3d(0, 1, 0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texnames[0]);
	glColor3d(0, 0, 0);
	glBegin(GL_QUADS);
	glTexCoord2d(0, 1);
	glVertex3d(ls / 2, 10, ls / 2);
	glTexCoord2d(0, 0);
	glVertex3d(ls / 2, 10, 0);
	glTexCoord2d(1, 0);
	glVertex3d(0, 10, 0);
	glTexCoord2d(1, 1);
	glVertex3d(0, 10, ls / 2);
	glEnd();
	glDisable(GL_TEXTURE_2D);

#ifdef USE_OPENGLUT
	// 文字列を3次元座標系で表示する
	std::string s("SD Experiments F");
	glColor3f(1, 0, 0);
	glRasterPos3i(-120, 1, 0);
	for (unsigned int i = 0; i < s.length(); ++i) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, s.c_str()[i]);
	}
#endif

	for (Players::iterator it = scene.p.begin(); it != scene.p.end(); ++it) {
		Player &p=it->second;
		color[0] = (float) p.r;
		color[1] = (float) p.g;
		color[2] = (float) p.b;
		color[3] = (float) 1.0;
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
		for (int j = 0; j < max_dots; ++j) {
			if (p.dots[j].visible == 1) {
				glPushMatrix();
				glTranslated((p.dots[j].x - this->get_width() / 2) / 2,
						0, (p.dots[j].y - this->get_height() / 2) / 2);
				glRotated(270, 1, 0, 0);
				gluDisk(q, z, 2 * z, 120, 10);
				glPopMatrix();
			}
		}
	}

	color[0] = 0.8f;
	color[1] = 0.8f;
	color[2] = 0.8f;
	color[3] = 0.8f;
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);
	glCullFace(GL_FRONT);
	glPushMatrix();
	double roll = atan2(scene.p[0].ax, scene.p[0].az) * 180 / M_PI;
	double pitch = atan2(scene.p[0].ay, scene.p[0].az) * 180 / M_PI;
	glRotated(pitch, 1.0, 0.0, 0.0);
	glRotated(roll, 0.0, 0.0, 1.0);
	glRotated(-scene.p[0].angle * 180 / M_PI, 0.0, 1.0, 0.0);
//	gdk_gl_draw_teapot(true, lh / 4 * scene.p[0].scale);
	glPopMatrix();

	// 文字列を2次元座標系で表示する
#ifdef USE_OPENGLUT
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, this->get_width(), this->get_height());
	glOrtho(0, this->get_width(), 0, this->get_height(), 100.0, -100.0);
	glDisable(GL_LIGHTING);

	std::stringstream ss;
	ss << "Now " << std::setw(2) << std::setfill('0') << scene.tm.tm_hour
			<< ":";
	ss << std::setw(2) << std::setfill('0') << scene.tm.tm_min << ":";
	ss << std::setw(2) << std::setfill('0') << scene.tm.tm_sec;
	glColor3f(1, 1, 1);
	glRasterPos3i(10, this->get_height() - 34, 0);
	for (unsigned int i = 0; i < ss.str().length(); ++i) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, ss.str().c_str()[i]);
	}
#endif

//	if (gdk_gl_drawable_is_double_buffered(gl_drawable)) {
//		gdk_gl_drawable_swap_buffers(gl_drawable);
//	} else {
		glFlush();
//	}
//	gdk_gl_drawable_gl_end(gl_drawable);

#if GTKMM3
#else
	MySmartphone &smapho=MySmartphone::getInstance();
	static int frame=0;
	if((++frame%ss_divisor)==0 && smapho.isConnected() && mgr.get_mode()==Manager::Server && mgr.members.size()==1) {
		Glib::RefPtr<Gdk::Pixmap> pixmap=this->get_snapshot();
		Glib::RefPtr<Gdk::Pixbuf> pixbuf=Gdk::Pixbuf::create((Glib::RefPtr<Gdk::Drawable>)pixmap,
				0, 0, this->get_width(), this->get_height());
		pixbuf=pixbuf->scale_simple(smapho.get_width(), smapho.get_height(), Gdk::INTERP_NEAREST);
		smapho.sendPixbuf((Glib::RefPtr<Gdk::Pixbuf>)pixbuf);
	}
#endif
	glFlush();
    glBindVertexArray (0);
    glUseProgram (0);
  	return true;
}
