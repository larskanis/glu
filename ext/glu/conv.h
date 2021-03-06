/*
 * Copyright (C) 2007 Jan Dvorak <jan.dvorak@kraxnet.cz>
 *
 * This program is distributed under the terms of the MIT license.
 * See the included MIT-LICENSE file for the terms of this license.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/* Functions and macros for datatype conversion between Ruby and C */

/*
	Fast inline conversion functions as a replacement for the ones in libruby.
	FIXNUM_P is simple logical AND check so it comes first, TYPE() is simple function,
	and specified in header file so it can be inlined. For conversion, FIX2LONG is
	simple right shift, and RFLOAT()-> just pointer dereference. For converting
	Fixnum and Float types (which accounts for 99.9% of things you would want to pass
	to OpenGL), there is large performance boost as result.

	Also ruby 'true' and 'false' are converted to GL_TRUE/GL_FALSE for compatibility, and
	finally, we fallback to library functions for any other data types (and error handling).
*/

#if HAVE_STRUCT_RFLOAT_FLOAT_VALUE
#define	FLOAT_VAL_ACCESS(val) RFLOAT(val)->float_value
#else
#define	FLOAT_VAL_ACCESS(val) RFLOAT(val)->value
#endif

/* For conversion between ruby and GL boolean values */
#define GLBOOL2RUBY(x) \
	(x) == GL_TRUE ? Qtrue : ((x)==GL_FALSE ? Qfalse : INT2NUM((x)))

#define RUBYBOOL2GL(x) \
	(x) == Qtrue ? GL_TRUE : GL_FALSE

#define cond_GLBOOL2RUBY_FUNC(_name_,_type_,_conv_) \
static inline VALUE _name_(GLenum pname,_type_ value) \
{ \
	switch (pname) { \
	case GL_DELETE_STATUS: \
	case GL_LINK_STATUS: \
	case GL_VALIDATE_STATUS: \
	case GL_COMPILE_STATUS: \
	case GL_MINMAX_SINK: \
	case GL_HISTOGRAM_SINK: \
	case GL_COORD_REPLACE: \
	case GL_TEXTURE_COMPRESSED: \
	case GL_GENERATE_MIPMAP: \
	case GL_TEXTURE_RESIDENT: \
	case GL_BUFFER_MAPPED: \
	case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED: \
	case GL_VERTEX_ATTRIB_ARRAY_ENABLED: \
	case GL_QUERY_RESULT_AVAILABLE: \
	case GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB: \
	case GL_FRAMEBUFFER_ATTACHMENT_LAYERED_EXT: \
	case GL_FENCE_STATUS_NV: \
	case GL_TEXTURE_FLOAT_COMPONENTS_NV: \
	case GL_SHADER_CONSISTENT_NV: \
	case GL_TEXTURE_COMPARE_SGIX: \
	case GLU_TESS_BOUNDARY_ONLY: \
	case GLU_CULLING: \
	case GLU_AUTO_LOAD_MATRIX: \
		return GLBOOL2RUBY(value); \
	default: \
		return _conv_(value); \
	} \
}

cond_GLBOOL2RUBY_FUNC(cond_GLBOOL2RUBY,GLint,INT2NUM)
cond_GLBOOL2RUBY_FUNC(cond_GLBOOL2RUBY_U,GLuint,UINT2NUM)
cond_GLBOOL2RUBY_FUNC(cond_GLBOOL2RUBY_F,GLfloat,rb_float_new)
cond_GLBOOL2RUBY_FUNC(cond_GLBOOL2RUBY_D,GLdouble,rb_float_new)


/* For conversion between ruby array (or object that can be converted to
 * array) and C array.
 *
 * The C array has to be preallocated by calling function. */
#define ARY2CTYPE(_type_,_convert_) \
static inline long ary2c##_type_( arg, cary, maxlen ) \
VALUE arg; \
GL##_type_ cary[]; \
long maxlen; \
{ \
	long i; \
	VALUE ary = rb_Array(arg); \
	if (maxlen < 1) \
		maxlen = RARRAY_LEN(ary); \
	else \
		maxlen = maxlen < RARRAY_LEN(ary) ? maxlen : RARRAY_LEN(ary); \
	for (i=0; i < maxlen; i++) \
		cary[i] = (GL##_type_)_convert_(rb_ary_entry(ary,i)); \
	return i; \
}

ARY2CTYPE(int,NUM2INT)
ARY2CTYPE(uint,NUM2UINT)
ARY2CTYPE(byte,NUM2INT)
ARY2CTYPE(ubyte,NUM2INT)
ARY2CTYPE(short,NUM2INT)
ARY2CTYPE(ushort,NUM2INT)
ARY2CTYPE(boolean,GLBOOL2RUBY)
ARY2CTYPE(float,NUM2DBL)
ARY2CTYPE(double,NUM2DBL)

#define ary2cflt ary2cfloat
#define ary2cdbl ary2cdouble

#undef ARY2CTYPE

/* Converts either array or object responding to #to_a to C-style array */
#define ARY2CMAT(_type_) \
static inline void ary2cmat##_type_(rary, cary, cols, rows) \
VALUE rary; \
_type_ cary[]; \
int cols,rows; \
{ \
	int i; \
\
	rary = rb_Array(rary); \
	rary = rb_funcall(rary,rb_intern("flatten"),0); \
\
	if (RARRAY_LEN(rary) != cols*rows) \
		rb_raise(rb_eArgError, "passed array/matrix must have %i*%i elements",cols,rows); \
\
	for (i=0; i < cols*rows; i++) \
		cary[i] = (_type_) NUM2DBL(rb_ary_entry(rary,i)); \
}

ARY2CMAT(double)
ARY2CMAT(float)
#undef ARY2CMAT

#define EMPTY
#define FREE(_x_) xfree(_x_);

