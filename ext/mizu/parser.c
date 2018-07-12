#include <ruby.h>
#include "picohttpparser/picohttpparser.c"

VALUE Mizu = Qnil;
VALUE MizuExceptions = Qnil;
VALUE MizuParser = Qnil;

VALUE HeadersTooLongError = Qnil;
VALUE ParseError = Qnil;

void Init_mizu_ext();
VALUE method_mizu_parser_alloc(VALUE klass);
VALUE method_mizu_parser_reset(VALUE self);
static void method_parser_mark(void *p);
static void method_mizu_parser_free(void *p);
VALUE method_mizu_parser_parse(VALUE self, VALUE data);
VALUE method_mizu_parser_version(VALUE self);
VALUE method_mizu_parser_offset(VALUE self);
VALUE method_mizu_parser_headers(VALUE self);
VALUE method_mizu_parser_method(VALUE self);
VALUE method_mizu_parser_path(VALUE self);
ID id_call;

typedef struct
{
  char _buf[4096];
  const char *_method;
  const char *_path;
  struct phr_header _headers[128];
  size_t _buflen;
  size_t _prevbuflen;
  size_t _method_len;
  size_t _path_len;
  size_t _num_headers;
  int _pret;
  int _minor_version;
  VALUE version;
  VALUE offset;
  VALUE headers;
  VALUE method;
  VALUE path;
} ParserWrapper;

void Init_mizu_ext()
{
  Mizu = rb_define_module("Mizu");
  MizuExceptions = rb_define_class_under(Mizu, "Exceptions", rb_cObject);
  MizuParser = rb_define_class_under(Mizu, "Parser", rb_cObject);
  HeadersTooLongError = rb_const_get(MizuExceptions, rb_intern("HeadersTooLongError"));
  ParseError = rb_const_get(MizuExceptions, rb_intern("ParseError"));
  rb_define_alloc_func(MizuParser, method_mizu_parser_alloc);
  rb_define_method(MizuParser, "<<", method_mizu_parser_parse, 1);
  rb_define_method(MizuParser, "reset!", method_mizu_parser_reset, 0);
  rb_define_method(MizuParser, "version", method_mizu_parser_version, 0);
  rb_define_method(MizuParser, "offset", method_mizu_parser_offset, 0);
  rb_define_method(MizuParser, "headers", method_mizu_parser_headers, 0);
  rb_define_method(MizuParser, "method", method_mizu_parser_method, 0);
  rb_define_method(MizuParser, "path", method_mizu_parser_path, 0);
  id_call = rb_intern("call");
}

VALUE method_mizu_parser_alloc(VALUE klass)
{
  ParserWrapper *ptr = ALLOC(ParserWrapper);
  ptr->_buflen = 0;
  ptr->_prevbuflen = 0;
  return Data_Wrap_Struct(klass, method_parser_mark, method_mizu_parser_free, ptr);
}

VALUE method_mizu_parser_reset(VALUE self)
{
  ParserWrapper *wrapper;
  Data_Get_Struct(self, ParserWrapper, wrapper);
  wrapper->_buflen = 0;
  wrapper->_prevbuflen = 0;
  return Qnil;
}

static void method_parser_mark(void *p)
{
  if (p)
  {
    ParserWrapper *wrapper = (ParserWrapper *)p;
    rb_gc_mark_maybe(wrapper->version);
    rb_gc_mark_maybe(wrapper->offset);
    rb_gc_mark_maybe(wrapper->headers);
    rb_gc_mark_maybe(wrapper->method);
    rb_gc_mark_maybe(wrapper->path);
  }
}

static void method_mizu_parser_free(void *p)
{
  xfree(p);
}

VALUE method_mizu_parser_parse(VALUE self, VALUE data)
{
  ParserWrapper *wrapper;
  Data_Get_Struct(self, ParserWrapper, wrapper);
  char *buffer = StringValueCStr(data);
  ssize_t rret = strlen(buffer);
  if (wrapper->_buflen + rret >= sizeof(wrapper->_buf))
  {
    rb_raise(HeadersTooLongError, "Request Headers Too Long");
  }
  memcpy(wrapper->_buf + wrapper->_buflen, buffer, rret);
  wrapper->_prevbuflen = wrapper->_buflen;
  wrapper->_buflen += rret;
  wrapper->_num_headers = sizeof(wrapper->_headers) / sizeof(wrapper->_headers[0]);
  wrapper->_pret = phr_parse_request(
      wrapper->_buf,
      wrapper->_buflen,
      &(wrapper->_method),
      &(wrapper->_method_len),
      &(wrapper->_path),
      &(wrapper->_path_len),
      &(wrapper->_minor_version),
      wrapper->_headers,
      &(wrapper->_num_headers),
      wrapper->_prevbuflen);

  if (wrapper->_pret > 0)
  {
    // Parsed
    size_t i;
    ParserWrapper *wrapper;
    Data_Get_Struct(self, ParserWrapper, wrapper);
    VALUE headers;

    wrapper->offset = ULONG2NUM(wrapper->_pret - wrapper->_prevbuflen);
    wrapper->version = rb_sprintf("1.%d", wrapper->_minor_version);
    wrapper->method = rb_str_new(wrapper->_method, wrapper->_method_len);
    wrapper->path = rb_str_new(wrapper->_path, wrapper->_path_len);

    headers = rb_hash_new();
    for (i = 0; i < wrapper->_num_headers; i++)
    {
      VALUE header_name = rb_str_new(wrapper->_headers[i].name, wrapper->_headers[i].name_len);
      VALUE header_value = rb_str_new(wrapper->_headers[i].value, wrapper->_headers[i].value_len);
      rb_hash_aset(headers, header_name, header_value);
    }

    wrapper->headers = headers;
    rb_funcall(rb_iv_get(self, "@callback"), id_call, 0);
  }
  else if (wrapper->_pret == -1)
  {
    rb_raise(ParseError, "Illegal HTTP/1.x Request");
  }
  else if (wrapper->_pret == -2)
  {
    return Qnil; // Incomplete parsing, needs more data.
  }

  if (wrapper->_buflen == sizeof(wrapper->_buf))
  {
    rb_raise(HeadersTooLongError, "Request Headers Too Long");
  }
  return Qnil;
}

VALUE method_mizu_parser_version(VALUE self)
{
  ParserWrapper *p;
  Data_Get_Struct(self, ParserWrapper, p);
  return p->version;
}

VALUE method_mizu_parser_offset(VALUE self)
{
  ParserWrapper *p;
  Data_Get_Struct(self, ParserWrapper, p);
  return p->offset;
}

VALUE method_mizu_parser_headers(VALUE self)
{
  ParserWrapper *p;
  Data_Get_Struct(self, ParserWrapper, p);
  return p->headers;
}

VALUE method_mizu_parser_method(VALUE self)
{
  ParserWrapper *p;
  Data_Get_Struct(self, ParserWrapper, p);
  return p->method;
}

VALUE method_mizu_parser_path(VALUE self)
{
  ParserWrapper *p;
  Data_Get_Struct(self, ParserWrapper, p);
  return p->path;
}
