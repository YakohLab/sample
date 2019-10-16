/*
 * glview.cpp
 * 画面表示を司る関数群
 */
#include <iostream>
#include <iomanip>
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
/* position and color information for each vertex */
struct vertex_info {
  float position[3];
  float color[3];
};

/* the vertex data is constant */
static const struct vertex_info vertex_data[] = {
  { {  0.0f,  0.500f, 0.0f }, { 1.f, 0.f, 0.f } },
  { {  0.5f, -0.366f, 0.0f }, { 0.f, 1.f, 0.f } },
  { { -0.5f, -0.366f, 0.0f }, { 0.f, 0.f, 1.f } },
};

static void
init_buffers (guint  position_index,
              guint  color_index,
              guint *vao_out)
{
  guint vao, buffer;

  /* we need to create a VAO to store the other buffers */
  glGenVertexArrays (1, &vao);
  glBindVertexArray (vao);

  /* this is the VBO that holds the vertex data */
  glGenBuffers (1, &buffer);
  glBindBuffer (GL_ARRAY_BUFFER, buffer);
  glBufferData (GL_ARRAY_BUFFER, sizeof (vertex_data), vertex_data, GL_STATIC_DRAW);

  /* enable and set the position attribute */
  glEnableVertexAttribArray (position_index);
  glVertexAttribPointer (position_index, 3, GL_FLOAT, GL_FALSE,
                         sizeof (struct vertex_info),
                         (GLvoid *) (G_STRUCT_OFFSET (struct vertex_info, position)));

  /* enable and set the color attribute */
  glEnableVertexAttribArray (color_index);
  glVertexAttribPointer (color_index, 3, GL_FLOAT, GL_FALSE,
                         sizeof (struct vertex_info),
                         (GLvoid *) (G_STRUCT_OFFSET (struct vertex_info, color)));

  /* reset the state; we will re-enable the VAO when needed */
  glBindBuffer (GL_ARRAY_BUFFER, 0);
  glBindVertexArray (0);

  /* the VBO is referenced by the VAO */
  glDeleteBuffers (1, &buffer);

  if (vao_out != NULL)
    *vao_out = vao;
}

static guint
create_shader (int          shader_type,
               const char  *source,
               GError     **error,
               guint       *shader_out)
{
  guint shader = glCreateShader (shader_type);
  glShaderSource (shader, 1, &source, NULL);
  glCompileShader (shader);

  int status;
  glGetShaderiv (shader, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE)
    {
      int log_len;
      glGetShaderiv (shader, GL_INFO_LOG_LENGTH, &log_len);

      char *buffer = (char *)g_malloc (log_len + 1);
      glGetShaderInfoLog (shader, log_len, NULL, buffer);
/*
      g_set_error (error, GLAREA_ERROR, GLAREA_ERROR_SHADER_COMPILATION,
                   "Compilation failure in %s shader: %s",
                   shader_type == GL_VERTEX_SHADER ? "vertex" : "fragment",
                   buffer);
*/
      g_free (buffer);

      glDeleteShader (shader);
      shader = 0;
    }

  if (shader_out != NULL)
    *shader_out = shader;

  return shader != 0;
}

static gboolean
init_shaders (guint   *program_out,
              guint   *mvp_location_out,
              guint   *position_location_out,
              guint   *color_location_out,
              GError **error)
{
  guint program = 0;
  guint mvp_location = 0;
  guint vertex = 0, fragment = 0;
  guint position_location = 0;
  guint color_location = 0;
  int status = 0;
  const char vertex_source[] = " \
#version 130 \
in vec3 position; \
in vec3 color; \
uniform mat4 mvp; \
smooth out vec4 vertexColor; \
void main() { \
  gl_Position = mvp * vec4(position, 1.0); \
  vertexColor = vec4(color, 1.0); \
}";
  const char fragment_source[] = " \
#version 130 \
smooth in vec4 vertexColor; \
out vec4 outputColor; \
void main() { \
  outputColor = vertexColor; \
}";
  /* load the vertex shader */
  create_shader (GL_VERTEX_SHADER, vertex_source, error, &vertex);
  if (vertex == 0)
    goto out;

  /* load the fragment shader */

  create_shader (GL_FRAGMENT_SHADER, fragment_source, error, &fragment);
  if (fragment == 0)
    goto out;

  /* link the vertex and fragment shaders together */
  program = glCreateProgram ();
  glAttachShader (program, vertex);
  glAttachShader (program, fragment);
  glLinkProgram (program);

  glGetProgramiv (program, GL_LINK_STATUS, &status);
  if (status == GL_FALSE)
    {
      int log_len = 0;
      glGetProgramiv (program, GL_INFO_LOG_LENGTH, &log_len);

      char *buffer = (char *)g_malloc (log_len + 1);
      glGetProgramInfoLog (program, log_len, NULL, buffer);
/*
      g_set_error (error, GLAREA_ERROR, GLAREA_ERROR_SHADER_LINK,
                   "Linking failure in program: %s", buffer);
*/
      g_free (buffer);

      glDeleteProgram (program);
      program = 0;

      goto out;
    }

  /* get the location of the "mvp" uniform */
  mvp_location = glGetUniformLocation (program, "mvp");

  /* get the location of the "position" and "color" attributes */
  position_location = glGetAttribLocation (program, "position");
  color_location = glGetAttribLocation (program, "color");

  /* the individual shaders can be detached and destroyed */
  glDetachShader (program, vertex);
  glDetachShader (program, fragment);

out:
  if (vertex != 0)
    glDeleteShader (vertex);
  if (fragment != 0)
    glDeleteShader (fragment);

  if (program_out != NULL)
    *program_out = program;
  if (mvp_location_out != NULL)
    *mvp_location_out = mvp_location;
  if (position_location_out != NULL)
    *position_location_out = position_location;
  if (color_location_out != NULL)
    *color_location_out = color_location;

  return program != 0;
}

static void
init_mvp (float *res)
{
  /* initialize a matrix as an identity matrix */
  res[0] = 1.f; res[4] = 0.f;  res[8] = 0.f; res[12] = 0.f;
  res[1] = 0.f; res[5] = 1.f;  res[9] = 0.f; res[13] = 0.f;
  res[2] = 0.f; res[6] = 0.f; res[10] = 1.f; res[14] = 0.f;
  res[3] = 0.f; res[7] = 0.f; res[11] = 0.f; res[15] = 1.f;
}
//=====
MyDrawingArea::MyDrawingArea(BaseObjectType* o,
		const Glib::RefPtr<Gtk::Builder>& g) :
		Gtk::DrawingArea(o) {
//	gl_config = gdk_gl_config_new_by_mode(
//			(GdkGLConfigMode) (GDK_GL_MODE_RGBA | GDK_GL_MODE_DEPTH));
//	gtk_widget_set_gl_capability(&(o->widget), gl_config, NULL, TRUE,
//			GDK_GL_RGBA_TYPE);
}

MyGLArea::MyGLArea(BaseObjectType* o,
		const Glib::RefPtr<Gtk::Builder>& g) :
		Gtk::GLArea(o) {
	std::cout << "GLArea is constructed." << std::endl;
}

bool MyGLArea::on_draw(const Cairo::RefPtr<Cairo::Context>&cc){
	Gtk::GLArea::on_draw(cc);

	cc->set_line_width(1.0);
	cc->set_source_rgb(0, 0, 0);
	cc->move_to(0, 0);
	cc->line_to(200, 100);
	cc->stroke();

	cc->set_font_size(16);
	cc->move_to(4, 20);
	cc->show_text(std::string("OpenGL"));

	return true;
}

void MyGLArea::on_realize(void) {
	Gtk::GLArea::on_realize();
	Gtk::GLArea::set_size_request(800, 600);

	ViewManager &vmr = ViewManager::getInstance();
	gtk_gl_area_make_current(GTK_GL_AREA(vmr.glArea->gobj()));

//=====
  /* initialize the shaders and retrieve the program data */
  GError *error = NULL;
  if (!init_shaders (&vmr.glArea->program,
                     &vmr.glArea->mvp_location,
                     &vmr.glArea->position_index,
                     &vmr.glArea->color_index,
                     &error))
    {
      /* set the GtkGLArea in error state, so we'll see the error message
       * rendered inside the viewport
       */
      gtk_gl_area_set_error (GTK_GL_AREA (vmr.glArea->gobj()), error);
      g_error_free (error);
      return;
    }

  /* initialize the vertex buffers */
  init_buffers (vmr.glArea->position_index, vmr.glArea->color_index, &vmr.glArea->vao);

    init_mvp (vmr.glArea->mvp);
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

bool MyGLArea::on_render(const Glib::RefPtr<Gdk::GLContext> &glc){
	Gtk::GLArea::on_render(glc);
	Manager &mgr = Manager::getInstance();
	Scene &scene=mgr.scene;

	ViewManager &vmr = ViewManager::getInstance();
	gtk_gl_area_make_current(GTK_GL_AREA(vmr.glArea->gobj()));
  glClearColor (0.5, 0.5, 0.5, 1.0);
  glClear (GL_COLOR_BUFFER_BIT);
/* load our program */
  glUseProgram (vmr.glArea->program);

  /* update the "mvp" matrix we use in the shader */
  glUniformMatrix4fv (vmr.glArea->mvp_location, 1, GL_FALSE, &(vmr.glArea->mvp[0]));

  /* use the buffers in the VAO */
  glBindVertexArray (vmr.glArea->vao);
  /* draw the three vertices as a triangle */
  glDrawArrays (GL_TRIANGLES, 0, 3);

  /* we finished using the buffers and program */
  glBindVertexArray (0);
  glUseProgram (0);
    glFlush ();
   return true; 

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

#ifdef HOGE
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
#endif
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
