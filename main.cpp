#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_image.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

int textureWidth = 256;
int textureHeight = 256;

// Function to initialize SDL and create an OpenGL context
int initSDL(SDL_Window** window, SDL_GLContext* context) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 0;
    }

    // Initialize SDL_image
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        fprintf(stderr, "IMG_Init Error: %s\n", IMG_GetError());
        SDL_Quit();
        return 0;
    }

    // Set SDL to use OpenGL 3.3 core
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Create SDL Window
    *window = SDL_CreateWindow("OpenGL Texture Update", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, textureWidth, textureHeight, SDL_WINDOW_OPENGL);
    if (!*window) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    // Create OpenGL context
    *context = SDL_GL_CreateContext(*window);
    if (!*context) {
        fprintf(stderr, "SDL_GL_CreateContext Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(*window);
        SDL_Quit();
        return 0;
    }

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
        SDL_GL_DeleteContext(*context);
        SDL_DestroyWindow(*window);
        SDL_Quit();
        return 0;
    }

    return 1;
}

// Function to clean up SDL and OpenGL resources
void cleanup(SDL_Window* window, SDL_GLContext context) {
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

// Function to flip the image data vertically
void flipImageVertical(GLubyte* data, int width, int height) {
    int stride = width * 4; // Assuming 4 bytes per pixel (RGBA)
    GLubyte* temp = (GLubyte*)malloc(stride);
    for (int y = 0; y < height / 2; y++) {
        memcpy(temp, data + y * stride, stride);
        memcpy(data + y * stride, data + (height - 1 - y) * stride, stride);
        memcpy(data + (height - 1 - y) * stride, temp, stride);
    }
    free(temp);
}

GLuint loadTexture(const char* path, GLubyte** outPixels, int* width, int* height) {
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        fprintf(stderr, "IMG_Load Error: %s\n", IMG_GetError());
        return 0;
    }

    *width = surface->w;
    *height = surface->h;

    GLubyte* pixels = (GLubyte*)malloc(*width * *height * 4 * sizeof(GLubyte));
    memcpy(pixels, surface->pixels, *width * *height * 4 * sizeof(GLubyte));
    *outPixels = pixels;

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    GLint mode = surface->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, mode, *width, *height, 0, mode, GL_UNSIGNED_BYTE, surface->pixels);

    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    SDL_FreeSurface(surface);

    flipImageVertical(*outPixels, *width, *height);

    return texture;
}

// Shader compilation function
GLuint compileShader(const GLchar* source, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        fprintf(stderr, "ERROR::SHADER::COMPILATION_FAILED\n%s\n", infoLog);
    }
    return shader;
}

// Function to update texture pixels using PBOs
void updateTexture(GLuint texture, GLuint pbo, GLubyte* pixels, int width, int height) {
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);

    // Map the buffer object into client's memory
    GLubyte* ptr = (GLubyte*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
    if (ptr) {
        // Update the pixel buffer with new data
        memcpy(ptr, pixels, width * height * 4);
        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER); // Release the buffer
    }

    // Transfer pixel data from the PBO to the texture
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

int main(int argc, char* argv[]) {
    SDL_Window* window = NULL;
    SDL_GLContext glContext;

    if (!initSDL(&window, &glContext)) {
        return -1;
    }

    // Set up the viewport
    glViewport(0, 0, textureWidth, textureHeight);

    // Create a texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLubyte* pixels = (GLubyte*)malloc(textureWidth * textureHeight * 4 * sizeof(GLubyte));

    // Initial texture setup
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    // Create PBO
    GLuint pbo;
    glGenBuffers(1, &pbo);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, textureWidth * textureHeight * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    // Vertex data for a full-screen quad
    GLfloat vertices[] = {
        // Positions          // Texture Coords
        -1.0f, -1.0f, 0.0f,   0.0f, 0.0f,
         1.0f, -1.0f, 0.0f,   1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,   1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f,   0.0f, 1.0f
    };

    GLuint indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Shader setup (vertex and fragment shaders)
    const GLchar* vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec2 texCoord;
        out vec2 TexCoord;
        void main() {
            gl_Position = vec4(position, 1.0);
            TexCoord = texCoord;
        }
    )";

    const GLchar* fragmentShaderSource = R"(
        #version 330 core
        in vec2 TexCoord;
        out vec4 color;
        uniform sampler2D ourTexture;
        void main() {
            color = texture(ourTexture, TexCoord);
        }
    )";

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // test test texture

    GLubyte* myTexture;
    int myTextureWidth;
    int myTextureHeight;
    loadTexture("Untitled.png", &myTexture, &myTextureWidth, &myTextureHeight);

    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // Update pixels (example: fill with random colors)
        for (int y = 0; y < textureHeight; ++y) {
            for (int x = 0; x < textureWidth; ++x) {
                int index = (y * textureWidth + x) * 4;
                pixels[index] = rand() % 256;      // Red
                pixels[index + 1] = rand() % 256;  // Green
                pixels[index + 2] = rand() % 256;  // Blue
                pixels[index + 3] = 255;        // Alpha
            }
        }

        // draw loaded texture
        for (int y = 0; y < myTextureHeight; ++y) {
            for (int x = 0; x < myTextureWidth; ++x) {
                int index = (y * myTextureWidth + x) * 4;

                float alpha = myTexture[index + 3] / 255.0f;  // Normalized alpha value
                pixels[index] = (GLubyte)((myTexture[index] * alpha) + (pixels[index] * (1.0f - alpha)));       // Red
                pixels[index + 1] = (GLubyte)((myTexture[index + 1] * alpha) + (pixels[index + 1] * (1.0f - alpha))); // Green
                pixels[index + 2] = (GLubyte)((myTexture[index + 2] * alpha) + (pixels[index + 2] * (1.0f - alpha))); // Blue
                pixels[index + 3] = 255;  // Preserve alpha as 255
            }
        }

        // Update the texture with new pixel data using PBO
        updateTexture(texture, pbo, pixels, textureWidth, textureHeight);

        // Render
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        SDL_GL_SwapWindow(window);
    }

    // Clean up
    free(pixels);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    glDeleteTextures(1, &texture);
    glDeleteBuffers(1, &pbo);

    cleanup(window, glContext);

    return 0;
}

