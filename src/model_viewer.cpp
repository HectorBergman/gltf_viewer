// Model viewer code for the assignments in Computer Graphics 1TD388/1MD150.
//
// Modify this and other source files according to the tasks in the instructions.
//

#include "gltf_io.h"
#include "gltf_scene.h"
#include "gltf_render.h"
#include "cg_utils.h"
#include "cg_trackball.h"

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstdlib>
#include <iostream>

struct ShadowCastingLight {
    glm::vec3 position;      // Light source position
    glm::mat4 shadowMatrix;  // Camera matrix for shadowmap
    GLuint shadowmap;        // Depth texture
    GLuint shadowFBO;        // Depth framebuffer
    float shadowBias;        // Bias for depth comparison
};



// Struct for our application context
struct Context {
    int width = 512;
    int height = 512;
    GLFWwindow *window;
    gltf::GLTFAsset asset;
    gltf::DrawableList drawables;
    cg::Trackball trackball;
    GLuint program;
    GLuint emptyVAO;
    float elapsedTime;
    std::string gltfFilename = "gargo.gltf";
    glm::vec3 ambient  = glm::vec3(20.0f/255.0f, 0.0f, 0.0f);
    glm::vec3 diffuse  = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 specular = glm::vec3(100.0f/255.0f, 67.0f/255.0f, 67.0f/255.0f);
    glm::float32 specularPow = 10.0f;
    bool showNormals = false;
    bool orthographicProjection = false;
    bool reflective = false;
    bool UV = false;
    bool useTexture = false;
    glm::float32 zoom = 1.0f;
    GLuint cubemap = 0; 
    int current_cubemap = 3;
    int previous_cubemap = -1;
    std::string cubemap_chosen = "8";
    gltf::TextureList textures;

    ShadowCastingLight light;
    GLuint shadowProgram;
    bool showShadowmap = false;
};

// Returns the absolute path to the src/shader directory
std::string shader_dir(void)
{
    std::string rootDir = cg::get_env_var("MODEL_VIEWER_ROOT");
    if (rootDir.empty()) {
        std::cout << "Error: MODEL_VIEWER_ROOT is not set." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return rootDir + "/src/shaders/";
}

// Returns the absolute path to the src/shader directory
std::string cubemap_dir(void)
{
    std::string rootDir = cg::get_env_var("MODEL_VIEWER_ROOT");
    if (rootDir.empty()) {
        std::cout << "Error: MODEL_VIEWER_ROOT is not set." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return rootDir + "/assets/cubemaps/";
}

// Returns the absolute path to the assets/gltf directory
std::string gltf_dir(void)
{
    std::string rootDir = cg::get_env_var("MODEL_VIEWER_ROOT");
    if (rootDir.empty()) {
        std::cout << "Error: MODEL_VIEWER_ROOT is not set." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return rootDir + "/assets/gltf/";
}

void do_initialization(Context &ctx)
{
    ctx.program = cg::load_shader_program(shader_dir() + "mesh.vert", shader_dir() + "mesh.frag");

    gltf::load_gltf_asset(ctx.gltfFilename, gltf_dir(), ctx.asset);
    gltf::create_drawables_from_gltf_asset(ctx.drawables, ctx.asset);

    gltf::create_textures_from_gltf_asset(ctx.textures, ctx.asset);
    // Initial cubemap using YOUR actual folder structure
    ctx.current_cubemap = 3;
    ctx.previous_cubemap = -1;
    ctx.cubemap_chosen = "8";

    ctx.shadowProgram =
        cg::load_shader_program(shader_dir() + "shadow.vert", shader_dir() + "shadow.frag");

    ctx.light.shadowmap = cg::create_depth_texture(2048, 2048);
    ctx.light.shadowFBO = cg::create_depth_framebuffer(ctx.light.shadowmap);

    ctx.light.position = glm::vec3(5.0f, 5.0f, 5.0f);
    ctx.light.shadowMatrix = glm::mat4(1.0f);
    ctx.light.shadowBias = 0.0f;

    std::string path = cubemap_dir() + "RomeChurch/prefiltered/" + ctx.cubemap_chosen + "/";
    std::cout << "=== INITIAL CUBEMAP PATH: " << path << std::endl;
    ctx.cubemap = cg::load_cubemap(path);
}

void draw_scene(Context &ctx)
{
    // Activate shader program
    glUseProgram(ctx.program);
    
    // CUBEMAP
    const char* values[] = { "0.125", "0.5", "2", "8", "32", "128", "512", "2048" };
    ImGui::SliderInt("Cubemap Roughness", &ctx.current_cubemap, 0, 7, values[ctx.current_cubemap]);

    if (ctx.previous_cubemap != ctx.current_cubemap) {
        ctx.previous_cubemap = ctx.current_cubemap;
        ctx.cubemap_chosen = values[ctx.current_cubemap];

        std::string path = cubemap_dir() + "RomeChurch/prefiltered/" + ctx.cubemap_chosen + "/";
        std::cout << "=== SWITCHED CUBEMAP TO: " << path << std::endl;

        ctx.cubemap = cg::load_cubemap(path);
    }

    
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ctx.cubemap);
    glUniform1i(glGetUniformLocation(ctx.program, "u_cubemap"), 0);
    ////////////////////////////////
    
    // Set render state
    glEnable(GL_DEPTH_TEST);  // Enable Z-buffering

    // Define per-scene uniforms
    glUniform1f(glGetUniformLocation(ctx.program, "u_time"), ctx.elapsedTime);

    glm::mat4 view1 = glm::mat4(ctx.trackball.orient);
    //glUniformMatrix4fv(glGetUniformLocation(ctx.program, "u_view"), 1, GL_FALSE, &view[0][0]);

    glm::mat4 view2 = glm::lookAt(
        glm::vec3(0.0f,0.0f,3.0f), //camera position
        glm::vec3(0.0f,0.0f,0.0f), //point looking at
        glm::vec3(0.0f,1.0f,0.0f)  //camera up direction
    );
    glm::mat4 view = view2 * view1;


    // Bind shadowmap to texture unit 2
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, ctx.light.shadowmap);
    glUniform1i(glGetUniformLocation(ctx.program, "u_shadowmap"), 2);

    // u_shadowFromView = lightMatrix * inverse(view)
    // Transforms: view space -> world space -> light clip space
    glm::mat4 shadowFromView = ctx.light.shadowMatrix * glm::inverse(view);
    glUniformMatrix4fv(glGetUniformLocation(ctx.program, "u_shadowFromView"), 1, GL_FALSE, &shadowFromView[0][0]);

    glUniform1f(glGetUniformLocation(ctx.program, "u_shadowBias"), ctx.light.shadowBias);


    //todo: go to assets/cubemaps/romechurch, observe that
    //they are 0.125, 0.5, 2, 8, etc., add a slider or some equivalent
    //to go from 0.125, 0.5, 2, 8, etc. to change cubemaps.
    //"the next texture should go into GL_TEXTURE0"

    //binding it





    glUniform3fv(glGetUniformLocation(ctx.program, "u_ambientColor"), 1, &ctx.ambient[0]);
    glUniform3fv(glGetUniformLocation(ctx.program, "u_diffuseColor"), 1, &ctx.diffuse[0]);
    glUniform3fv(glGetUniformLocation(ctx.program, "u_specularColor"), 1, &ctx.specular[0]);
    glUniform1f(glGetUniformLocation(ctx.program, "u_specularPower"), ctx.specularPow);

    ImGui::ColorEdit3("Ambient Color", &ctx.ambient[0]);
    ImGui::ColorEdit3("Diffuse Color", &ctx.diffuse[0]);
    ImGui::ColorEdit3("Specular Color", &ctx.specular[0]);
    ImGui::InputFloat("Specular Power", &ctx.specularPow);
    ImGui::InputFloat("Zoom", &ctx.zoom);
    ImGui::Checkbox("Show Normals", &ctx.showNormals);
    ImGui::Checkbox("Toggle Orthographic Projection", &ctx.orthographicProjection);
    ImGui::Checkbox("Toggle Reflective Environment", &ctx.reflective);
    ImGui::Checkbox("Toggle UV", &ctx.UV);
    ImGui::Checkbox("Toggle Texture", &ctx.useTexture);
    ImGui::Checkbox("Show Shadowmap", &ctx.showShadowmap);
    ImGui::SliderFloat("Shadow Bias", &ctx.light.shadowBias, 0.0f, 0.01f, "%.4f");









    float near = 0.1f;
    float far = 100.0f;
    float aspectRatio = (float)ctx.width / (float)ctx.height;
    float baseFov = glm::radians(125.0f);
    float fov = baseFov / ctx.zoom;
    glm::mat4 projection = glm::perspective(
        fov,
        aspectRatio, 
        near,
        far
    );

    glm::mat4 orthoProj = glm::ortho(
        -(float)ctx.zoom * aspectRatio,
        (float)ctx.zoom * aspectRatio,
        -(float)ctx.zoom,
        (float)ctx.zoom,
        near,
        far
    );
    glUniformMatrix4fv(glGetUniformLocation(ctx.program, "u_orthoProjection"), 1, GL_FALSE, &orthoProj[0][0]);

    glUniform1i(
        glGetUniformLocation(ctx.program, "u_showNormals"),
        ctx.showNormals
    );
    glUniform1i(
        glGetUniformLocation(ctx.program, "u_toggleOrtho"),
        ctx.orthographicProjection
    );
    glUniform1i(
        glGetUniformLocation(ctx.program, "u_toggleReflective"),
        ctx.reflective
    );
    glUniform1i(
        glGetUniformLocation(ctx.program, "u_toggleUV"),
        ctx.UV
    );
    glUniform1i(
        glGetUniformLocation(ctx.program, "u_toggleTexture"),
        ctx.useTexture
    );
    glUniformMatrix4fv(glGetUniformLocation(ctx.program, "u_view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(ctx.program, "u_projection"), 1, GL_FALSE, &projection[0][0]);
    glUniform1i(glGetUniformLocation(ctx.program, "u_baseColorTexture"), 1);
    glm::vec3 lightPos = glm::vec3(5.0f,5.0f,5.0f);
    // Transform to view space

    glUniform3fv(glGetUniformLocation(ctx.program, "u_lightPosition"), 1, &lightPos[0]);

    
    //glm::mat4 projection = glm::mat4(1.0f);
    //glUniformMatrix4fv(glGetUniformLocation(ctx.program, "u_projection"), 1, GL_FALSE, &projection[0][0]);
    glm::mat4 model =
        glm::translate(glm::mat4(1.0f), glm::vec3(0.2f,0.0f,0.0f)) *
        glm::rotate(glm::mat4(1.0f), glm::radians(160.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
        glm::scale(glm::mat4(1.0f), glm::vec3(1.2f));
    glUniformMatrix4fv(glGetUniformLocation(ctx.program, "u_model"), 1, GL_FALSE, &model[0][0]);

    // Draw scene
    for (unsigned i = 0; i < ctx.asset.nodes.size(); ++i) {
        const gltf::Node &node = ctx.asset.nodes[i];
        const gltf::Drawable &drawable = ctx.drawables[node.mesh];
        
        // Define per-object uniforms
        const gltf::Mesh &mesh = ctx.asset.meshes[node.mesh];
        if (mesh.primitives[0].hasMaterial) {
            const gltf::Primitive &primitive = mesh.primitives[0];
            const gltf::Material &material = ctx.asset.materials[primitive.material];
            const gltf::PBRMetallicRoughness &pbr = material.pbrMetallicRoughness;

            // Define material textures and uniforms
            if (pbr.hasBaseColorTexture) {
                GLuint texture_id = ctx.textures[pbr.baseColorTexture.index];
                // Bind texture and define uniforms...
                glActiveTexture(GL_TEXTURE1);
                if (ctx.useTexture && pbr.hasBaseColorTexture) {
                    glBindTexture(GL_TEXTURE_2D, texture_id);//0);//baseTextureID);
                } else {
                    glBindTexture(GL_TEXTURE_2D, 0);   // unbind when we don't want the texture
                }
            } else {
                // Need to handle this case as well, by telling
                // the shader that no texture is available
            }
        }
        

        // Draw object
        glBindVertexArray(drawable.vao);
        glDrawElements(GL_TRIANGLES, drawable.indexCount, drawable.indexType,
                       (GLvoid *)(intptr_t)drawable.indexByteOffset);
        glBindVertexArray(0);
    }
    
    

    // Clean up
    cg::reset_gl_render_state();
    glUseProgram(0);
}

// Update the shadowmap and shadow matrix for a light source
void update_shadowmap(Context &ctx, ShadowCastingLight &light, GLuint shadowFBO)
{
    // Set up rendering to shadowmap framebuffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, shadowFBO);
    if (shadowFBO) glViewport(0, 0, 2048, 2048);  // TODO Set viewport to shadowmap size
    glClear(GL_DEPTH_BUFFER_BIT);               // Clear depth values to 1.0

    // Set up pipeline
    glUseProgram(ctx.shadowProgram);
    glEnable(GL_DEPTH_TEST);  // Enable Z-buffering

    glm::mat4 view = glm::lookAt(
        ctx.light.position,
        glm::vec3(0.0f, 0.0f, 0.0f),  // Target point (adjust if your model isn't centered at origin)
        glm::vec3(0.0f, 1.0f, 0.0f)   // Up vector
    );

    float fov = glm::radians(90.0f);  // Start with 90 degrees; experiment if needed
    float aspect = 1.0f;              // Square shadowmap
    float near_plane = 0.1f;          // Adjust if precision issues arise
    float far_plane = 10.0f;          // Ensure this covers the scene; experiment if visualization is all white
    glm::mat4 proj = glm::perspective(fov, aspect, near_plane, far_plane);
    glUniformMatrix4fv(glGetUniformLocation(ctx.shadowProgram, "u_view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(ctx.shadowProgram, "u_proj"), 1, GL_FALSE, &proj[0][0]);

    // Store updated shadow matrix for use in draw_scene()
    light.shadowMatrix = proj * view;

    // Draw scene
    for (unsigned i = 0; i < ctx.asset.nodes.size(); ++i) {
        const gltf::Node &node = ctx.asset.nodes[i];
        const gltf::Drawable &drawable = ctx.drawables[node.mesh];

        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.2f, 0.0f, 0.0f)) *
                          glm::rotate(glm::mat4(1.0f), glm::radians(160.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
                          glm::scale(glm::mat4(1.0f), glm::vec3(1.2f));
        glUniformMatrix4fv(glGetUniformLocation(ctx.shadowProgram, "u_model"), 1, GL_FALSE, &model[0][0]);

        // Draw object
        glBindVertexArray(drawable.vao);
        glDrawElements(GL_TRIANGLES, drawable.indexCount, drawable.indexType,
                       (GLvoid *)(intptr_t)drawable.indexByteOffset);
        glBindVertexArray(0);
    }

    // Clean up
    cg::reset_gl_render_state();
    glUseProgram(0);
    glViewport(0, 0, ctx.width, ctx.height);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}



void do_rendering(Context &ctx)
{
    cg::reset_gl_render_state();
    glClearColor(0.5f, 0.2f, 0.2f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    update_shadowmap(ctx, ctx.light, ctx.light.shadowFBO);
    draw_scene(ctx);  // <-- draw scene first

    if (ctx.showShadowmap) {
        // NOW overwrite the screen with the depth visualization
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        update_shadowmap(ctx, ctx.light, 0);
    }
}


void reload_shaders(Context *ctx)
{
    glDeleteProgram(ctx->program);
    ctx->program = cg::load_shader_program(shader_dir() + "mesh.vert", shader_dir() + "mesh.frag");
}

void error_callback(int /*error*/, const char *description)
{
    std::cerr << description << std::endl;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    // Forward event to ImGui
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    if (ImGui::GetIO().WantCaptureKeyboard) return;

    Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
    if (key == GLFW_KEY_R && action == GLFW_PRESS) { reload_shaders(ctx); }
}

void char_callback(GLFWwindow *window, unsigned int codepoint)
{
    // Forward event to ImGui
    ImGui_ImplGlfw_CharCallback(window, codepoint);
    if (ImGui::GetIO().WantTextInput) return;
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    // Forward event to ImGui
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
    if (ImGui::GetIO().WantCaptureMouse) return;

    double x, y;
    glfwGetCursorPos(window, &x, &y);

    Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        ctx->trackball.center = glm::vec2(x, y);
        ctx->trackball.tracking = (action == GLFW_PRESS);
    }
}

void cursor_pos_callback(GLFWwindow *window, double x, double y)
{
    // Forward event to ImGui
    if (ImGui::GetIO().WantCaptureMouse) return;

    Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
    cg::trackball_move(ctx->trackball, float(x), float(y));
}

void scroll_callback(GLFWwindow *window, double x, double y)
{
    // Forward event to ImGui

    ImGui_ImplGlfw_ScrollCallback(window, x, y);
    if (ImGui::GetIO().WantCaptureMouse) return;

    Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
    ctx->zoom += (float)y * 0.2f;
    ctx->zoom = glm::max(ctx->zoom, 0.4f); 
}

void resize_callback(GLFWwindow *window, int width, int height)
{
    // Update window size and viewport rectangle
    Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
    ctx->width = width;
    ctx->height = height;
    glViewport(0, 0, width, height);
}


int main(int argc, char *argv[])
{
    Context ctx = Context();
    if (argc > 1) { ctx.gltfFilename = std::string(argv[1]); }

    // Create a GLFW window
    glfwSetErrorCallback(error_callback);
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    ctx.window = glfwCreateWindow(ctx.width, ctx.height, "Model viewer", nullptr, nullptr);
    glfwMakeContextCurrent(ctx.window);
    glfwSetWindowUserPointer(ctx.window, &ctx);
    glfwSetKeyCallback(ctx.window, key_callback);
    glfwSetCharCallback(ctx.window, char_callback);
    glfwSetMouseButtonCallback(ctx.window, mouse_button_callback);
    glfwSetCursorPosCallback(ctx.window, cursor_pos_callback);
    glfwSetScrollCallback(ctx.window, scroll_callback);
    glfwSetFramebufferSizeCallback(ctx.window, resize_callback);

    // Load OpenGL functions
    if (gl3wInit() || !gl3wIsSupported(3, 3) /*check OpenGL version*/) {
        std::cerr << "Error: failed to initialize OpenGL" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

    // Initialize ImGui
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(ctx.window, false /*do not install callbacks*/);
    ImGui_ImplOpenGL3_Init("#version 330" /*GLSL version*/);

    // Initialize rendering
    glGenVertexArrays(1, &ctx.emptyVAO);
    glBindVertexArray(ctx.emptyVAO);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    do_initialization(ctx);

    // Start rendering loop
    while (!glfwWindowShouldClose(ctx.window)) {
        glfwPollEvents();
        ctx.elapsedTime = glfwGetTime();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        //ImGui::ShowDemoWindow();
        do_rendering(ctx);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(ctx.window);
    }

    // Shutdown
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(ctx.window);
    glfwTerminate();
    std::exit(EXIT_SUCCESS);
}
