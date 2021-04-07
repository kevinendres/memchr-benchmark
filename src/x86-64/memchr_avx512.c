#include <immintrin.h>
#include <x86intrin.h>

#define vector_t __m512i
#define mask_t __mmask64
#define vector_size 64
#define vector_eq(A, B)  _mm512_cmp_epi8_mask(A, B, 0)
#define mask_from_vector  _mm512_movepi8_mask
#define load_mask(C) _mm512_set1_epi8(C)
#define load_vector(X) _mm512_stream_load_si512( (vector_t * )(X))
#define apply_mask(A, B) mask_from_vector(vector_eq(A, B))
#define ctz(x) __builtin_ctz(x)


void * memchr_avx512(void * buffer, char c, size_t length){
	vector_t * buffer_cast =  (vector_t *) buffer;
	vector_t vmask = load_mask(c);
	mask_t mask;
	size_t i=0;
	for (;i < length/vector_size;i++){
		mask = vector_eq(load_vector(buffer_cast + i), vmask);
		if (mask)
			return  ((char *) (buffer + i)) + ctz(mask);
	}
	i = vector_size * vector_size;
	for (;i < length; i++)
		if (*((char *)(buffer + i)) == c)
			return buffer + i;
	return NULL;
}
