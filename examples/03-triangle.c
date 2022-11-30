#include "window.h"

#define GLAD_GL_IMPLEMENTATION
#include "glad.h"

static struct {
    float x, y;
    float r, g, b;
} const vertices[] = {
    { -0.5f, -0.5f, 1.0f, 0.0f, 0.0f },
    {  0.5f, -0.5f, 0.0f, 1.0f, 0.0f },
    {  0.0f,  0.5f, 0.0f, 0.0f, 1.0f }
};

static char const *vs_source =
    "#version 330\n"
    "layout(location = 0) in vec2 a_pos;\n"
    "layout(location = 1) in vec3 a_color;\n"
    "out vec4 v_color;\n"
    "void main() {\n"
    "    gl_Position = vec4(a_pos, 0.0, 1.0);\n"
    "    v_color = vec4(a_color, 1.0);\n"
    "}";

static char const *fs_source =
    "#version 330\n"
    "in vec4 v_color;\n"
    "out vec4 o_color;\n"
    "void main() {\n"
    "    o_color = v_color;\n"
    "}";

int main(void) {
    window_t window = {0};

    window_init(&window);
    window_make_current(window);
    gladLoadGL(window_proc_address);

    unsigned vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    unsigned vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof *vertices, (void *)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof *vertices, (void *)(2 * sizeof(float)));

    unsigned vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vs_source, 0);
    glCompileShader(vs);

    unsigned fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fs_source, 0);
    glCompileShader(fs);

    unsigned program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    while (!window.closed) {
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(vao);
        glUseProgram(program);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        window_swap_buffers(window);
        window_poll_events(&window);
    }

    glDeleteProgram(program);
    glDeleteShader(fs);
    glDeleteShader(vs);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    window_fini(window);
}
