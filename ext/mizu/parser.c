#include <ruby.h>
#include <ruby/encoding.h>
#include "picohttpparser/picohttpparser.c"

VALUE Mizu = Qnil;
VALUE MizuExceptions = Qnil;
VALUE MizuParser = Qnil;

VALUE HeadersTooLongError = Qnil;
VALUE ParseError = Qnil;

void Init_mizu_ext();
VALUE method_mizu_parser_alloc(VALUE self);
static void method_parser_mark(void *p);
static void method_mizu_parser_free(void *p);
VALUE method_mizu_parser_parse(VALUE self, VALUE data);
ID id_complete;

VALUE sym_version;
VALUE sym_method;
VALUE sym_path;
VALUE sym_headers;
VALUE sym_offset;

typedef struct
{
  char buf[4096];
  const char *method;
  const char *path;
  struct phr_header headers[128];
  size_t buflen;
  size_t prevbuflen;
  size_t method_len;
  size_t path_len;
  size_t num_headers;
  int pret;
  int minor_version;
} ParserWrapper;

void Init_mizu_ext()
{
  Mizu = rb_define_module("Mizu");
  MizuExceptions = rb_define_class_under(Mizu, "Exceptions", rb_cObject);
  MizuParser = rb_define_class_under(Mizu, "Parser", rb_cObject);
  HeadersTooLongError = rb_const_get(MizuExceptions, rb_intern("HeadersTooLongError"));
  ParseError = rb_const_get(MizuExceptions, rb_intern("ParseError"));
  rb_define_alloc_func(MizuParser, method_mizu_parser_alloc);
  rb_define_private_method(MizuParser, "parse", method_mizu_parser_parse, 1);
  id_complete = rb_intern("complete");
  sym_version = ID2SYM(rb_intern("version"));
  sym_method = ID2SYM(rb_intern("method"));
  sym_path = ID2SYM(rb_intern("path"));
  sym_headers = ID2SYM(rb_intern("headers"));
  sym_offset = ID2SYM(rb_intern("offset"));
}

VALUE method_mizu_parser_alloc(VALUE klass)
{
  ParserWrapper *ptr = ALLOC(ParserWrapper);
  ptr->buflen = 0;
  ptr->prevbuflen = 0;
  return Data_Wrap_Struct(klass, method_parser_mark, method_mizu_parser_free, ptr);
}

static void method_parser_mark(void *p)
{
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
  memcpy(wrapper->buf + wrapper->buflen, buffer, rret);
  wrapper->prevbuflen = wrapper->buflen;
  wrapper->buflen += rret;
  wrapper->num_headers = sizeof(wrapper->headers) / sizeof(wrapper->headers[0]);
  wrapper->pret = phr_parse_request(
      wrapper->buf,
      wrapper->buflen,
      &(wrapper->method),
      &(wrapper->method_len),
      &(wrapper->path),
      &(wrapper->path_len),
      &(wrapper->minor_version),
      wrapper->headers,
      &(wrapper->num_headers),
      wrapper->prevbuflen);

  if (wrapper->pret > 0)
  {
    // Parsed
    size_t offset, i;
    VALUE result, headers;

    result = rb_hash_new();
    headers = rb_hash_new();
    offset = wrapper->pret - wrapper->prevbuflen;
    rb_hash_aset(result, sym_offset, ULONG2NUM(offset));
    rb_hash_aset(result, sym_version, rb_sprintf("1.%d", wrapper->minor_version));
    rb_hash_aset(result, sym_method, rb_str_new(wrapper->method, wrapper->method_len));
    rb_hash_aset(result, sym_path, rb_str_new(wrapper->path, wrapper->path_len));

    for (i = 0; i < wrapper->num_headers; i++)
    {
      VALUE header_name = rb_str_new(wrapper->headers[i].name, wrapper->headers[i].name_len);
      VALUE header_value = rb_enc_str_new(wrapper->headers[i].value, wrapper->headers[i].value_len, rb_utf8_encoding());
      rb_hash_aset(headers, header_name, header_value);
    }
    rb_hash_aset(result, sym_headers, headers);
    rb_funcall(self, id_complete, 1, result);
  }
  else if (wrapper->pret == -1)
  {
    rb_raise(ParseError, "Illegal HTTP/1.x Request");
  }
  else if (wrapper->pret == -2)
  {
    return Qnil; // Incomplete parsing, needs more data.
  }

  if (wrapper->buflen == sizeof(wrapper->buf))
  {
    rb_raise(HeadersTooLongError, "Request Headers Too Long");
  }
  return Qnil;
}
