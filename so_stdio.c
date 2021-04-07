#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <paths.h>
#include "so_stdio.h"

#define BUFSIZE 4096


struct _so_file {
	char *buffer;
	int fd;			// file descriptor
	int offset;		// position in buffer
	int cursor;		// position in file
	int bytes_read;	// number of bytes returned from read
	int last_op;	// last operation: 1 - write, 2 - read
	int eof;		// mark reaching EOF
	int err;		// mark an error
};


int so_fileno(SO_FILE *stream) {
	/* Return file descriptor */
	return stream->fd;
}


SO_FILE *so_fopen(const char *pathname, const char *mode)
{
	int flag = 0, fd;
	SO_FILE *stream;

	/* Find the flag that matches the opening mode */
	if (!strcmp(mode, "r"))
		flag = O_RDONLY;
	else if (!strcmp(mode, "r+"))
		flag = O_RDWR;
	else if (!strcmp(mode, "w"))
		flag =  O_WRONLY | O_CREAT | O_TRUNC;
	else if (!strcmp(mode, "w+"))
		flag = O_RDWR | O_CREAT | O_TRUNC;
	else if (!strcmp(mode, "a"))
		flag = O_WRONLY | O_CREAT | O_APPEND;
	else if (!strcmp(mode, "a+"))
		flag = O_RDWR | O_CREAT | O_APPEND;
	else
		return NULL;


	/* Open the file */
	fd = open(pathname, flag, 0644);
	if (fd < 0)
		return NULL;

	/* Allocate space and initialize so_file structure */
	stream = (SO_FILE *) malloc(sizeof(SO_FILE));
	if (stream == NULL) {
		stream->err = 1;
		free(stream);
		return NULL;
	}

	stream->buffer = (char *) calloc(BUFSIZE, sizeof(char));
	if (stream->buffer == NULL) {
		stream->err = 1;
		free(stream->buffer);
		free(stream);
		return NULL;
	}

	stream->fd = fd;
	stream->offset = 0;
	stream->cursor = 0;
	stream->bytes_read = 0;
	stream->last_op = 0;
	stream->eof = 0;
	stream->err = 0;

	return stream;
}


int so_fclose(SO_FILE *stream)
{
	int ret;

	/* If the last operation was write, put remaining buffer content in file*/
	if (stream->last_op == 1) {
		ret = so_fflush(stream);
		if (ret == SO_EOF)  {
			stream->err = 1;
			free(stream->buffer);
			free(stream);
			return SO_EOF;
		}
	}

	/* Close the file */
	ret = close(stream->fd);
	if (ret < 0) {
		stream->err = 1;
		free(stream->buffer);
		free(stream);
		return SO_EOF;
	}

	/* Free space */
	free(stream->buffer);
	free(stream);
	return 0;
}


int so_fgetc(SO_FILE *stream)
{
	int ret, pos;

	/* Read if it is the first read operation or if all the characters from the
	 * buffer were processed and we need to read the next chunk
	 */
	if (stream->last_op != 2 || stream->offset == 0 || stream->offset == BUFSIZE) {
		memset(stream->buffer, 0, BUFSIZE);
		/* Read BUFSIZE characters from file */
		ret = read(stream->fd, stream->buffer, BUFSIZE);
		if (ret == 0) {
			stream->eof = 1;
			return SO_EOF;
		} else if (ret == -1) {
			stream->err = 1;
			return SO_EOF;
		}

		/* Place offset at position 0 in buffer and mark the read operation */
		stream->last_op = 2;
		stream->offset = 0;
		stream->bytes_read = ret;
	}

	/* End of file was reached */
	if (stream->offset == stream->bytes_read) {
		stream->eof = 1;
		return SO_EOF;
	}

	/* Move forward in buffer a position at a time */
	pos = stream->offset;
	stream->offset += 1;
	stream->cursor += 1;

	return (int) stream->buffer[pos];
}


size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	int i, ch;
	char *tmp = (char *) ptr;

	stream->bytes_read = 0;
	/* Read nmemb characters from file */
	for (i = 0; i < size * nmemb; i++) {
		/* Read a character at a time and store it in memory */
		ch = so_fgetc(stream);
		tmp[i] = ch;

		if (stream->err)
			break;
	}

	/* Return number of read characters */
	return i / size;
}


int so_fputc(int c, SO_FILE *stream)
{
	int ret;

	/* Write in file only if the buffer is full */
	if (stream->offset == BUFSIZE) {
		ret = so_fflush(stream);
		if (ret == SO_EOF) {
			stream->err = 1;
			return SO_EOF;
		}
	}

	/* Put each character in buffer at the correct position */
	stream->buffer[stream->offset] = c;
	stream->offset++;
	stream->cursor++;
	stream->last_op = 1;

	return c;
}


size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	int i;
	char *tmp = (char *)ptr;

	/*Write nmemb characters in file */
	for (i = 0; i < size * nmemb; i++) {
		/* Put each character in the buffer and write in file when it is full */
		so_fputc(tmp[i], stream);

		if (stream->eof || stream->err)
			break;
	}

	return nmemb;
}


int so_fflush(SO_FILE *stream)
{
	int ret;

	/* If the last operation was not write, no need to flush */
	if (stream->last_op != 1)
		return 0;

	/* Write in file the content of the buffer */
	ret = write(stream->fd, stream->buffer, stream->offset);
	if (ret == -1) {
		stream->err = 1;
		return SO_EOF;
	}

	memset(stream->buffer, 0, BUFSIZE);
	stream->offset = 0;

	return 0;
}


int so_fseek(SO_FILE *stream, long offset, int whence)
{
	int pos, ret;

	/* Flush the content of the buffer if last operation was write */
	if (stream->last_op == 1) {
		ret = so_fflush(stream);
		if (ret == SO_EOF)
			return SO_EOF;
	}

	/* Empty the buffer if last operation was read */
	else if (stream->last_op == 2) {
		stream->offset = 0;
		memset(stream->buffer, 0, BUFSIZE);
	}

	/* Move the position of the cursor */
	pos = lseek(stream->fd, offset, whence);
	if (pos < 0)
		return -1;
	stream->cursor = pos;

	return 0;
}


long so_ftell(SO_FILE *stream)
{
	/* Return the current position of the cursor */
	return stream->cursor;
}


int so_feof(SO_FILE *stream)
{
	/* Return a nonzero value if we reached end of file */
	return stream->eof;
}


int so_ferror(SO_FILE *stream)
{
	/* Return a nonzero value if there was an error */
	return stream->err;
}


SO_FILE *so_popen(const char *command, const char *type)
{
	return NULL;
}

int so_pclose(SO_FILE *stream)
{
	return 0;
}
