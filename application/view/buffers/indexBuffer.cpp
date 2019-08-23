#include "indexBuffer.h"

#include "GL\glew.h"
#include "GLFW\glfw3.h"

#include "../util/log.h"

IndexBuffer::IndexBuffer() {
	id = 0;
	Log::debug("Created empty index buffer");
};

IndexBuffer::IndexBuffer(const unsigned int* data, size_t size, unsigned int mode) {
	glGenBuffers(1, &id);
	bind();
	if (size != 0)
		fill(data, size, mode);
}

IndexBuffer::IndexBuffer(const unsigned int* data, size_t size) : IndexBuffer(data, size, GL_STATIC_DRAW) {}

IndexBuffer::~IndexBuffer() {
	close();
	Log::warn("Deleted index buffer with id (%d)", id);
}

void IndexBuffer::fill(const unsigned int* data, size_t size, unsigned int mode) {
	bind();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * sizeof(unsigned int), data, mode);
}

void IndexBuffer::update(const unsigned int* data, size_t size, int offset) {
	bind();
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, size * sizeof(unsigned int), data);
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