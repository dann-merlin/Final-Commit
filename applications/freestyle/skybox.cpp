#include "skybox.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

using namespace std;

GLuint Skybox::vao, Skybox::sky_tex;
std::shared_ptr<Drawelement> Skybox::prototype;

Skybox::Skybox() {
    vector<vector<float>> points = {{
            -1.0f, 1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f
		}, {
            -1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f
		}, {
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, -1.0f,
            1.0f, -1.0f, -1.0f
		}, {
            -1.0f, -1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f
		}, {
            -1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, -1.0f
		}, {
            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, 1.0f
		}
    };

	prototype = make_drawelement("skybox");

	int i=0;
	for(auto &verticies : points) {
		auto mesh = make_mesh("skybox_mesh" + to_string(i++));
		mesh->add_vertex_buffer(GL_FLOAT, 3, verticies.size(), verticies.data(), GL_STATIC_DRAW);
		vector<uint> indices;
		for(uint j = 0; j < verticies.size()/3; j++) {
			indices.push_back(j);
		}
		mesh->add_index_buffer(6, indices.data());
		mesh->set_primitive_type(GL_TRIANGLES);
		prototype->add_mesh(mesh);
	}

    // GLuint vbo;
    // glGenBuffers(1, &vbo);
    // glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // glBufferData(GL_ARRAY_BUFFER, 3 * 36 * sizeof(float), &points, GL_STATIC_DRAW);

    // glGenVertexArrays(1, &vao);
    // glBindVertexArray(vao);
    // glEnableVertexAttribArray(0);
    // glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    // vector<string> sky_textures{
    //         "render-data/images/skybox/skygrid_1.png",
    //         "render-data/images/skybox/skygrid_1.png",
    //         "render-data/images/skybox/skygrid_1.png",
    //         "render-data/images/skybox/skygrid_1.png",
    //         "render-data/images/skybox/skygrid_1.png",
    //         "render-data/images/skybox/skygrid_1.png"
    // };

    // glGenTextures(1, &sky_tex);
    // glBindTexture(GL_TEXTURE_CUBE_MAP, sky_tex);
    // int width, height, channels;
    // unsigned char *skyData;
    // for (GLuint i = 0; i < sky_textures.size(); i++) {
    //     stbi_set_flip_vertically_on_load(0);
    //     skyData = stbi_load(sky_textures[i].c_str(), &width, &height, &channels, 0);
    //     if (!skyData) cout << "failed to load image" << endl;
    //     glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE,
    //                  skyData);
    //     stbi_image_free(skyData);
    // }
    // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

Skybox::~Skybox() {}

void Skybox::draw(std::shared_ptr<Shader> &shader) {

	static float time_matrix = 0;
	time_matrix += Context::frame_time();
	// prototype->use_shader(shader);
    auto cam = Camera::current();
    glm::mat4 view = cam->view;
    glm::mat4 proj = cam->proj;
    view[3] = glm::vec4(0, 0, 0, 1);
    glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	shader->bind();
	shader->uniform("iTime", (float) time_matrix/ 2000);
	shader->uniform("iResolution", Context::resolution());
    shader->uniform("view", view);
    shader->uniform("proj", proj);
	auto model = glm::scale(glm::vec3(10000)) * glm::mat4(1);
	shader->uniform("model", model);
	shader->uniform("model_normal", glm::transpose(glm::inverse(model)));
	shader->uniform("default_trafo", glm::mat4(1));
	int i = 0;
	for (const auto &mesh : prototype->meshes) {
		shader->uniform("plane_id", i++);
		mesh->bind();
		mesh->draw(shader);
		mesh->unbind();
	}
	// prototype->draw(glm::mat4(1));
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_CUBE_MAP, sky_tex);
    // glBindVertexArray(vao);
    // glDrawArrays(GL_TRIANGLES, 0, 36);

    shader->unbind();
	glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
}
