#include "indexBuffer.h"

#include "GL\glew.h"
#include "GLFW\glfw3.h"

#include "../util/log.h"

IndexBuffer::IndexBuffer() {
	id = 0;
	Log::debug("Created empty index buffer");
};

IndexBuffer::IndexBuffer(const unsigned int* data, unsigned int size, unsigned int mode) {
	glGenBuffers(1, &id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
	if (size != 0)
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * sizeof(unsigned int), data, mode);
}

IndexBuffer::IndexBuffer(const unsigned int* data, unsigned int size) : IndexBuffer(data, size, GL_STATIC_DRAW) {}

IndexBuffer::~IndexBuffer() {
	close();
	Log::warn("Deleted index buffer with id (%d)", id);
}

void IndexBuffer::bind() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
}

void IndexBuffer::unbind() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void IndexBuffer::close() {
	unbind();
	glDeleteBuffers(1, &id);
}