#include <iostream>
#include <vector>
#include <cmath>
#include <signal.h>

#include "glcontext.h"
#include "program.h"
#include "texture.h"
#include "buffer.h"
#include "vertexarray.h"

volatile sig_atomic_t quit = 0;

void sighandler(int signal)
{
	std::cout << "Caught signal " << signal << ", setting flaq to quit.\n";
	quit = true;
}

int main (int argc, char* argv[])
{
	signal(SIGINT, sighandler);
	signal(SIGTERM, sighandler);

    glContext context(200, 200);
    std::cout << "Frame size:(" << context.getWidth() << ", " << context.getHeight() << ")" << std::endl;
    
    Texture tex2d;
    if(argc > 1)
    {
        tex2d.Load(argv[1]);
    }
    else
    {
        uint8_t image[] =
        {
            0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff,
            0x00, 0xff, 0x00, 0xff, 0x00, 0x00, 0xff, 0xff,
        };
        tex2d.TexImage(image, 0, GL_RGBA, 2, 2);
    }
    
    Shader shaders[] =
    {
        Shader(SHADER_SOURCE(
attribute vec2 pos;\n
attribute vec2 tc0;\n
\n
varying vec2 tc;\n
\n
void main()\n
{\n
    gl_Position = vec4(pos, 0.0, 1.0);\n
    tc = tc0;\n
}\n
        ), 0, GL_VERTEX_SHADER),
    
        Shader(SHADER_SOURCE(
//#precision mediump float;\n
\n
varying vec2 tc;\n
\n
uniform sampler2D tex;\n
\n
void main()\n
{\n
    gl_FragColor = texture2D(tex, tc);\n
}\n
        ), 0, GL_FRAGMENT_SHADER)
    };
    
    Program program(shaders, 2);
    program.Use();
    program.setUniform("tex", 0);
    
    float l, b, r, t;
    
    float rs = float(context.getWidth()) / float(context.getHeight());
    float ri = float(tex2d.getWidth()) / float(tex2d.getHeight());
    
    if(ri > rs)
    {
        l = -1.0f;
        r =  1.0f;

        t =  rs / ri; 
        b = -t;
    }
    else if(ri < rs)
    {
        b = -1.0f;
        t =  1.0f; 

        r =  ri / rs;
        l = -r;
    }
    else
    {
        l = -1.0f;
        b = -1.0f;
        r =  1.0f;
        t =  1.0f; 
    }
    
    float vertices[] =
    {
        l,    t,
        l,    b,
        r,    t,
        r,    b,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
    };
    Buffer vb(vertices, sizeof(vertices), GL_ARRAY_BUFFER);
    
    uint16_t indices[] =
    {
        0, 1, 2, 3,
    };
    Buffer ib(indices, sizeof(indices), GL_ELEMENT_ARRAY_BUFFER);
    
    VertexArray vao;
    vao.Bind();
    vb.Bind();
    vao.VertexAttrib(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    vao.VertexAttrib(1, 2, GL_FLOAT, GL_FALSE, 0, sizeof(float) * 2 * 4);
    ib.Bind();
    vao.Unbind();
		
	while (!quit)
	{
	    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	    
	    tex2d.Bind();
	    
	    vao.Bind();
	    program.Use();
	    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);
	    vao.Unbind();
	    
	    context.SwapBuffers();
	}
	return 0;
}
