/**
* Author: Adria Orenstein
* Assignment: Simple 2D Scene
* Date due: 2023-06-02, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

#define LOG(argument) std::cout << argument << '\n'

const int WINDOW_WIDTH = 640,
          WINDOW_HEIGHT = 480;

const float BG_RED = 0.1922f,
            BG_BLUE = 0.9000f,
            BG_GREEN = 0.4000f,
            BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;
const float DEGREES_PER_SECOND = 90.0f;

const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL = 0;  // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER = 0;   // this value MUST be zero

const char DOG_SPRITE[] = "sprites/dog.png";
const char SQUIRREL_SPRITE[] = "sprites/squirrel.png";

SDL_Window* display_window;
bool game_is_running = true;
bool d_going_up = true;
bool s_going_up = true;
int frame_counter = 0;
bool s_hits = false;

glm::mat4 view_matrix, projection_matrix, trans_matrix;

ShaderProgram dog_program;
glm::mat4 d_model_matrix;

ShaderProgram squirrel_program;
glm::mat4 s_model_matrix;

float previous_ticks = 0.0f;

GLuint dog_texture_id;
GLuint squirrel_texture_id;

// overall position
float dog_position_x = -0.75f;
float dog_position_y = -0.75f;
float squirrel_position_x = 1.0f;
float squirrel_position_y = 1.0f;

float squirrel_rotation = 0.0f;

float squirrel_scale_factor = 1.0f;

GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }
    
    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);
    
    return textureID;
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    
    display_window = SDL_CreateWindow("Hello, Project 1!",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(display_window);
    SDL_GL_MakeCurrent(display_window, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    dog_program.Load(V_SHADER_PATH, F_SHADER_PATH);
    squirrel_program.Load(V_SHADER_PATH, F_SHADER_PATH);
    
    d_model_matrix = glm::mat4(1.0f);
    s_model_matrix = glm::mat4(1.0f);
    
    view_matrix = glm::mat4(1.0f);  // Defines the position (location and orientation) of the camera
    projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);  // Defines the characteristics of your camera, such as clip planes, field of view, projection method etc.
    
    dog_program.SetProjectionMatrix(projection_matrix);
    dog_program.SetViewMatrix(view_matrix);

    squirrel_program.SetProjectionMatrix(projection_matrix);
    squirrel_program.SetViewMatrix(view_matrix);
    
    glUseProgram(dog_program.programID);
    glUseProgram(squirrel_program.programID);
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    dog_texture_id = load_texture(DOG_SPRITE);
    squirrel_texture_id = load_texture(SQUIRREL_SPRITE);
    
    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
        {
            game_is_running = false;
        }
    }
}

void update()
{
    float ticks = (float) SDL_GetTicks() / MILLISECONDS_IN_SECOND; // get the current number of ticks
    float delta_time = ticks - previous_ticks; // the delta time is the difference from the last frame
    previous_ticks = ticks;
    
    if (((dog_position_x > 4.0f) && (d_going_up)) | ((dog_position_x < -4.0f) && (!d_going_up))) {
        d_going_up = !d_going_up;
    }
    
    if (d_going_up) {
        dog_position_x += 1.0f * delta_time;
        dog_position_y += 1.0f * delta_time;
    } else {
        dog_position_x -= 1.0f * delta_time;
        dog_position_y -= 1.0f * delta_time;
    }
    
    d_model_matrix = glm::mat4(1.0f);
    
    d_model_matrix = glm::translate(d_model_matrix, glm::vec3(dog_position_x, dog_position_y, 0.0f));
    d_model_matrix = glm::scale(d_model_matrix, glm::vec3(1.5f, 1.5f, 1.0f));
    
    if (((squirrel_position_x > 4.0f) && (s_going_up)) | ((squirrel_position_x < -4.0f) && (!s_going_up))) {
        s_going_up = !s_going_up;
        s_hits = true;
    }
    
    if (s_going_up) {
        squirrel_position_x += 1.0f * delta_time;
        squirrel_position_y += 1.0f * delta_time;
    } else {
        squirrel_position_x -= 1.0f * delta_time;
        squirrel_position_y -= 1.0f * delta_time;
    }
    
    squirrel_rotation += DEGREES_PER_SECOND * delta_time;
    
    s_model_matrix = glm::mat4(1.0f);
    
    if (s_hits) {
        if (frame_counter == 200) {
            squirrel_scale_factor = 1.0f;
            frame_counter = 0;
            s_hits = false;
        } else {
            if (frame_counter < 100) {
                squirrel_scale_factor *= 1.01f;
            } else {
                squirrel_scale_factor *= 0.99f;
            }
            frame_counter += 1;
        }
    }
    
    s_model_matrix = glm::translate(s_model_matrix, glm::vec3(squirrel_position_x, squirrel_position_y, 0.0f));
    s_model_matrix = glm::scale(s_model_matrix, glm::vec3(squirrel_scale_factor, squirrel_scale_factor, 1.0f));
    s_model_matrix = glm::rotate(s_model_matrix, glm::radians(squirrel_rotation), glm::vec3(0.0f, 0.0f, 1.0f));
}

void draw_object(glm::mat4 &object_model_matrix, GLuint &object_texture_id, ShaderProgram& object_program)
{
    object_program.SetModelMatrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so we use 6 instead of 3
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Vertices
    float vertices[] = {
        -0.7f, -0.7f, 0.7f, -0.7f, 0.7f, 0.7f,  // triangle 1
        -0.7f, -0.7f, 0.7f, 0.7f, -0.7f, 0.7f   // triangle 2
    };

    // Textures
    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };
    
    glVertexAttribPointer(dog_program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(dog_program.positionAttribute);
    
    glVertexAttribPointer(dog_program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(dog_program.texCoordAttribute);
    
    glVertexAttribPointer(squirrel_program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(squirrel_program.positionAttribute);
    
    glVertexAttribPointer(squirrel_program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(squirrel_program.texCoordAttribute);
    
    // Bind texture
    draw_object(d_model_matrix, dog_texture_id, dog_program);
    draw_object(s_model_matrix, squirrel_texture_id, squirrel_program);
    
    // We disable two attribute arrays now
    glDisableVertexAttribArray(dog_program.positionAttribute);
    glDisableVertexAttribArray(dog_program.texCoordAttribute);
    
    glDisableVertexAttribArray(squirrel_program.positionAttribute);
    glDisableVertexAttribArray(squirrel_program.texCoordAttribute);
    
    SDL_GL_SwapWindow(display_window);
}

void shutdown()
{
    SDL_Quit();
}

int main(int argc, char* argv[])
{
    initialise();
    
    while (game_is_running)
    {
        process_input();
        update();
        render();
    }
    
    shutdown();
    return 0;
}
