#pragma once
void loadFromFile(const char* url, char** target) {
	std::ifstream stream(url, std::ios::binary);    //open binary file stream
	stream.seekg(0, stream.end);				    // seek to end of file
	int total = stream.tellg();					    // retrieve length of file
	*target = new char[total + 1];					// create buffer length + 1
	stream.seekg(0, stream.beg);					// seek to start of file
	stream.read(*target, total);					// read length of file into buffer
	(*target)[total] = '\0';						// length + 1 is the "null terminator"
	stream.close();									// close stream
}
unsigned int loadTexture(std::string url, GLenum type, int comp)
{
	// Generate & Bind Texture ID
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	// Set Texture Params
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// STBI Load From File
	int width, height, nrChannels;
	unsigned char* data = stbi_load(url.c_str(), &width, &height, &nrChannels, comp);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, type, width, height, 0, type,
			GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Texture failed to load at path: " << url << std::endl;
	}
	// Free loaded data
	stbi_image_free(data);
	// Unbind Texture ID
	glBindTexture(GL_TEXTURE_2D, 0);
	return textureID;
}
unsigned int GeneratePlane(const char* heightmap, GLenum format, int comp, float
	hScale, float xzScale, unsigned int& numIndices, unsigned int& heightmapID) {
	int width, height, channels;
	unsigned char* data = nullptr;
	if (heightmap != nullptr) {
		data = stbi_load(heightmap, &width, &height, &channels, comp);
		if (data) {
			glGenTextures(1, &heightmapID);
			glBindTexture(GL_TEXTURE_2D, heightmapID);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
				GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		stbi_image_free(data);
	}
	float* vertices = new float[(width * height) * 8];
	unsigned int* indices = new unsigned int[(width - 1) * (height - 1) * 6];
	int index = 0;

	for (int i = 0; i < (width * height); i++) {
		int x = i % width;
		int z = i / width;
		// position
		vertices[index++] = (float)x * xzScale;
		vertices[index++] = 0.0f;
		vertices[index++] = (float)z * xzScale;
		// normal
		vertices[index++] = 0.0f;
		vertices[index++] = 1.0f;
		vertices[index++] = 0.0f;
		// uv
		vertices[index++] = ((float)x) / (width - 1);
		vertices[index++] = ((float)z) / (height - 1);
	}

	// Calculate Normals
	for (int i = 0; i < (width * height); i++) {
		int x = i % width;
		int z = i / width;
		glm::vec3 vPos;
		vPos.x = vertices[i * 8 + 0];
		vPos.y = vertices[i * 8 + 1];
		vPos.z = vertices[i * 8 + 2];
		glm::vec3 normal;

		//bottom-right corner
		if (x == width - 1 && z == height - 1) {
			glm::vec3 lPos, uPos;
			lPos.x = vertices[(i - 1) * 8 + 0];
			lPos.y = vertices[(i - 1) * 8 + 1];
			lPos.z = vertices[(i - 1) * 8 + 2];
			uPos.x = vertices[(i - width) * 8 + 0];
			uPos.y = vertices[(i - width) * 8 + 1];
			uPos.z = vertices[(i - width) * 8 + 2];
			normal = glm::cross(glm::normalize(uPos - vPos),
				glm::normalize(lPos - vPos));
		}
		//bottom
		else if (z == height - 1) {
			glm::vec3 rPos, uPos;
			rPos.x = vertices[(i + 1) * 8 + 0];
			rPos.y = vertices[(i + 1) * 8 + 1];
			rPos.z = vertices[(i + 1) * 8 + 2];
			uPos.x = vertices[(i - width) * 8 + 0];
			uPos.y = vertices[(i - width) * 8 + 1];
			uPos.z = vertices[(i - width) * 8 + 2];
			normal = glm::cross(glm::normalize(rPos - vPos),
				glm::normalize(uPos - vPos));
		}

		//right
		else if (x == width - 1) {
			glm::vec3 lPos, dPos;
			lPos.x = vertices[(i - 1) * 8 + 0];
			lPos.y = vertices[(i - 1) * 8 + 1];
			lPos.z = vertices[(i - 1) * 8 + 2];
			dPos.x = vertices[(i + width) * 8 + 0];
			dPos.y = vertices[(i + width) * 8 + 1];
			dPos.z = vertices[(i + width) * 8 + 2];
			normal = glm::cross(glm::normalize(lPos - vPos),
				glm::normalize(dPos - vPos));
		}
		
		//rest
		else {
			glm::vec3 rPos, dPos;
			rPos.x = vertices[(i + 1) * 8 + 0];
			rPos.y = vertices[(i + 1) * 8 + 1];
			rPos.z = vertices[(i + 1) * 8 + 2];
			dPos.x = vertices[(i + width) * 8 + 0];
			dPos.y = vertices[(i + width) * 8 + 1];
			dPos.z = vertices[(i + width) * 8 + 2];
			normal = glm::cross(glm::normalize(dPos - vPos),
				glm::normalize(rPos - vPos));
			if (x > 0 && z > 0) {
				glm::vec3 lPos, uPos;
				lPos.x = vertices[(i - 1) * 8 + 0];
				lPos.y = vertices[(i - 1) * 8 + 1];
				lPos.z = vertices[(i - 1) * 8 + 2];
				uPos.x = vertices[(i - width) * 8 + 0];
				uPos.y = vertices[(i - width) * 8 + 1];
				uPos.z = vertices[(i - width) * 8 + 2];
				normal += glm::cross(glm::normalize(uPos - vPos),
					glm::normalize(lPos - vPos));
				normal += glm::cross(glm::normalize(rPos - vPos),
					glm::normalize(uPos - vPos));
				normal += glm::cross(glm::normalize(lPos - vPos),
					glm::normalize(dPos - vPos));
				normal /= 4.0f;
			}
		}
		vertices[(i * 8) + 3] = normal.x;
		vertices[(i * 8) + 4] = normal.y;
		vertices[(i * 8) + 5] = normal.z;
	}
	index = 0;
	for (int i = 0; i < (width - 1) * (height - 1); i++) {
		int x = i % (width - 1);
		int y = i / (height - 1);
		int vertex = x + y * width;
		indices[index++] = vertex;
		indices[index++] = vertex + width + 1;
		indices[index++] = vertex + 1;
		indices[index++] = vertex;
		indices[index++] = vertex + width;
		indices[index++] = vertex + width + 1;
	}
	unsigned int numVerts = width * height * 8 * sizeof(float);
	numIndices = ((width - 1) * (height - 1) * 6) * sizeof(unsigned int);
	unsigned int VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, numVerts, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices, indices, GL_STATIC_DRAW);
	// vertex information!
	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, 0);
	glEnableVertexAttribArray(0);
	// normal
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)
		(sizeof(float) * 3));
	glEnableVertexAttribArray(1);
	// uv
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)
		(sizeof(float) * 6));
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	delete[] vertices;
	delete[] indices;
	return VAO;
}
unsigned int GenerateCube(unsigned int& size) {
	unsigned int VAO;
	// need 24 vertices for normal/uv-mapped Cube
	float vertices[] = {
		// positions            //colors            // tex coords   // normals
		0.5f, -0.5f, -0.5f,     1.0f, 1.0f, 1.0f,   1.f, 0.f,       0.f, -1.f,
		0.f,
		0.5f, -0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,       0.f, -1.f,
		0.f,
		-0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 1.0f,   0.f, 1.f,       0.f, -1.f,
		0.f,
		-0.5f, -0.5f, -.5f,     1.0f, 1.0f, 1.0f,   0.f, 0.f,       0.f, -1.f,
		0.f,
		0.5f, 0.5f, -0.5f,      1.0f, 1.0f, 1.0f,   2.f, 0.f,       1.f, 0.f,
		0.f,
		0.5f, 0.5f, 0.5f,       1.0f, 1.0f, 1.0f,   2.f, 1.f,       1.f, 0.f,
		0.f,
		0.5f, 0.5f, 0.5f,       1.0f, 1.0f, 1.0f,   1.f, 2.f,       0.f, 0.f,
		1.f,
		-0.5f, 0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   0.f, 2.f,       0.f, 0.f,
		1.f,
		-0.5f, 0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   -1.f, 1.f,      -1.f, 0.f,
		0.f,
		-0.5f, 0.5f, -.5f,      1.0f, 1.0f, 1.0f,   -1.f, 0.f,      -1.f, 0.f,
		0.f,
		-0.5f, 0.5f, -.5f,      1.0f, 1.0f, 1.0f,   0.f, -1.f,      0.f, 0.f, -
		1.f,
		0.5f, 0.5f, -0.5f,      1.0f, 1.0f, 1.0f,   1.f, -1.f,      0.f, 0.f, -
		1.f,
		-0.5f, 0.5f, -.5f,      1.0f, 1.0f, 1.0f,   3.f, 0.f,       0.f, 1.f,
		0.f,
		-0.5f, 0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   3.f, 1.f,       0.f, 1.f,
		0.f,
		0.5f, -0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,       0.f, 0.f,
		1.f,
		-0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 1.0f,   0.f, 1.f,       0.f, 0.f,
		1.f,
		-0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 1.0f,   0.f, 1.f,       -1.f, 0.f,
		0.f,
		-0.5f, -0.5f, -.5f,     1.0f, 1.0f, 1.0f,   0.f, 0.f,       -1.f, 0.f,
		0.f,
		-0.5f, -0.5f, -.5f,     1.0f, 1.0f, 1.0f,   0.f, 0.f,       0.f, 0.f, -
		1.f,
		0.5f, -0.5f, -0.5f,     1.0f, 1.0f, 1.0f,   1.f, 0.f,       0.f, 0.f, -
		1.f,
		0.5f, -0.5f, -0.5f,     1.0f, 1.0f, 1.0f,   1.f, 0.f,       1.f, 0.f,
		0.f,
		0.5f, -0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,       1.f, 0.f,
		0.f,
		0.5f, 0.5f, -0.5f,      1.0f, 1.0f, 1.0f,   2.f, 0.f,       0.f, 1.f,
		0.f,
		0.5f, 0.5f, 0.5f,       1.0f, 1.0f, 1.0f,   2.f, 1.f,       0.f, 1.f,
		0.f
	};
	unsigned int indices[] = {  // note that we start from 0!
	// DOWN
	0, 1, 2,   // first triangle
	0, 2, 3,    // second triangle
	// BACK
	14, 6, 7,   // first triangle
	14, 7, 15,    // second triangle
	// RIGHT
	20, 4, 5,   // first triangle
	20, 5, 21,    // second triangle
	// LEFT
	16, 8, 9,   // first triangle
	16, 9, 17,    // second triangle
	// FRONT
	18, 10, 11,   // first triangle
	18, 11, 19,    // second triangle
	// UP
	22, 12, 13,   // first triangle
	22, 13, 23,    // second triangle
	};
	unsigned int VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
		GL_STATIC_DRAW);
	int stride = 11 * sizeof(float);
	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
	glEnableVertexAttribArray(0);
	// color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 *
		sizeof(float)));
	glEnableVertexAttribArray(1);
	// uv
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 *
		sizeof(float)));
	glEnableVertexAttribArray(2);
	// normal
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(8 *
		sizeof(float)));
	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	size = sizeof(indices);
	return VAO;
}
void CreateShader(const char* url, GLenum type, unsigned int& shader) {
	static int success;
	static char infoLog[512];
	char* target;
	loadFromFile(url, &target);
	shader = glCreateShader(type);
	glShaderSource(shader, 1, &target, nullptr);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog
			<< std::endl;
	}
}
