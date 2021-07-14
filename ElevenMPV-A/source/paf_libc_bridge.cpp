#include <paf.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

extern "C" {

	static int s_fakeErrno = 0;

	FILE * sce_paf_fopen(const char * filename, const char * mode);
	int sce_paf_fclose(FILE * stream);
	size_t sce_paf_fread(void * ptr, size_t size, size_t count, FILE * stream);
	int sce_paf_fseek(FILE * stream, long int offset, int origin);
	void *sce_paf_realloc(void *ptr, size_t new_size);
	long int sce_paf_ftell(FILE * stream);
	int sce_paf_fgetc(FILE * stream);
	char * sce_paf_strcpy(char * destination, const char * source);
	char * sce_paf_strcat(char * destination, const char * source);
	float sce_paf_roundf(float x);

	FILE * fopen(const char * filename, const char * mode)
	{
		return sce_paf_fopen(filename, mode);
	}

	int fclose(FILE * stream)
	{
		return sce_paf_fclose(stream);
	}

	size_t fread(void * ptr, size_t size, size_t count, FILE * stream)
	{
		return sce_paf_fread(ptr, size, count, stream);
	}

	int fseek(FILE * stream, long int offset, int origin)
	{
		return sce_paf_fseek(stream,  offset,  origin);
	}

	long int ftell(FILE * stream)
	{
		return sce_paf_ftell(stream);
	}

	int fgetc(FILE * stream)
	{
		return sce_paf_fgetc(stream);
	}

	int ferror(FILE *stream)
	{
		return 0;
	}

	void* malloc(size_t size)
	{
		return sce_paf_malloc(size);
	}

	void* calloc(size_t num, size_t size)
	{
		void *ret = sce_paf_malloc(num * size);
		sce_paf_memset(ret, 0, num * size);
		return ret;
	}

	void free(void *ptr)
	{
		sce_paf_free(ptr);
	}

	void *realloc(void *ptr, size_t new_size)
	{
		return sce_paf_realloc(ptr, new_size);
	}

	void* memcpy(void* dest, const void* src, size_t count)
	{
		return sce_paf_memcpy(dest, src, count);
	}

	void* memset(void * ptr, int value, size_t num)
	{
		return sce_paf_memset(ptr, value, num);
	}

	int rand(void)
	{
		return sce_paf_rand();
	}

	size_t strlen(const char *str)
	{
		return sce_paf_strlen(str);
	}

	int strcmp(const char * str1, const char * str2)
	{
		return sce_paf_strcmp(str1, str2);
	}

	char * strcpy(char * destination, const char * source)
	{
		return sce_paf_strcpy(destination, source);
	}

	char * strcat(char * destination, const char * source)
	{
		return sce_paf_strcat(destination, source);
	}

	int strncmp(const char * str1, const char * str2, size_t num)
	{
		return sce_paf_strncmp(str1, str2, num);
	}

	char * strncpy(char * destination, const char * source, size_t num)
	{
		return sce_paf_strncpy(destination, source, num);
	}

	char *strdup(const char *str1)
	{
		SceInt32 len = sce_paf_strlen(str1);
		char *ret = (char *)sce_paf_malloc(len);
		if (ret != SCE_NULL)
			sce_paf_strcpy(ret, str1);
		return ret;
	}

	int memcmp(const void * ptr1, const void * ptr2, size_t num)
	{
		return sce_paf_memcmp(ptr1, ptr2, num);
	}

	void* memmove(void* dest, const void* src, std::size_t count)
	{
		return sce_paf_memmove(dest, src, count);
	}

	int abs(int x)
	{
		return sce_paf_abs(x);
	}

	long int labs(long int x)
	{
		return sce_paf_abs(x);
	}

	void qsort(void *ptr, size_t count, size_t size,
		int(*comp)(const void *, const void *))
	{
		sce_paf_qsort(ptr, count, size, comp);
	}

	double floor(double x)
	{
		return (double)sce_paf_floorf((float)x);
	}

	double ceil(double arg)
	{
		return (double)sce_paf_ceilf((float)arg);
	}

	double sqrt(double x)
	{
		return (float)sqrtf((float)x);
	}

	double ldexp(double x, int y)
	{
		return (float)ldexpf((float)x, y);
	}

	double rint(double x)
	{
		return (double)sce_paf_roundf((float)x);
	}

	int *_sceLibcErrnoLoc()
	{
		return &s_fakeErrno;
	}

	int sce_paf_look_ctype_table(int c)
	{
		return 0x20;
	}
}