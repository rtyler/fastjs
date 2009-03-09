/*
 * fastjs - A v8-based JavaScript FastCGI server 
 *
 * Copyright 2009 - R. Tyler Ballance <tyler@monkeypox.org>
 */

#include <cstring>
#include <cstdio>
#include <cstdlib>


#include <v8.h>

extern "C" {
	extern char **environ;
	#include <fcgi_config.h>
	#include <fcgiapp.h>
	#include <strings.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <fcntl.h>
	#include <unistd.h>
}

using namespace v8; // *puke*

#define JQUERY_FILE "jquery-1.3.2.min.js"
#define JQUERY_COMPAT "if (typeof(window) == 'undefined'){window=new Object();document=window;self=window;navigator=new Object();navigator.userAgent=navigator.userAgent||'FastJS Server';location=new Object();location.href='file:///dev/null';location.protocol='file:';location.host = 'FastJS';}"

/* Functions that will be exposed into the JavaScript environment */
Handle<Value> FastJS_Write(const Arguments& args);
Handle<Value> FastJS_Load(const Arguments& args);

/* Unfortunately we need a few global pointers to our current streams for the JavaScript callbacks */
FCGX_Stream *_in_stream = NULL;
FCGX_Stream *_out_stream = NULL; 
FCGX_Stream *_error_stream = NULL;
void *_jQuery = NULL; /* Global jQuery buffer to prevent needing to re-read the file per-request */

static void PrintEnv(FCGX_Stream *out, char *label, char **envp)
{
    FCGX_FPrintF(out, "%s:<br>\n<pre>\n", label);
    for( ; *envp != NULL; envp++) {
        FCGX_FPrintF(out, "%s\n", *envp);
    }
    FCGX_FPrintF(out, "</pre><p>\n");
}

static void *read_file_contents(const char *filepath)
{
	struct stat attributes;
	int fd = 0;
	int rc = 0;

	rc = stat(filepath, &attributes);
	if (rc != 0) {
		/* I should do something with the error here */
		return NULL;
	}
	
	void *buffer = malloc(sizeof(char) * (attributes.st_size + 1));
	bzero(buffer, (attributes.st_size + 1));
	fd = open(filepath, 0);
	if (fd == 0) {
		/* I should do something with the error here */
		close(fd);
		return NULL;
	}

	rc = read(fd, buffer, attributes.st_size);
	close(fd);
	return buffer;
}	


static void req_handle(FCGX_Stream *out, char **environment) 
{
	if (_jQuery == NULL) {
		/* FAIL! */
		FCGX_FPrintF(out, "FAILED TO PROPERLY LOAD JQUERY!");
		return;
	}

	HandleScope scope;
	Handle<ObjectTemplate> _global = ObjectTemplate::New();
	_global->Set(String::New("write"), FunctionTemplate::New(FastJS_Write));
	Persistent<Context> ctx = Context::New(NULL, _global);
	Context::Scope ctx_scope(ctx);

	Handle<String> prereq = String::New(JQUERY_COMPAT);
	Handle<Script> prereqs = Script::Compile(prereq);
	prereqs->Run();

	Handle<String> jquery = String::New((const char *)(_jQuery));
	Handle<Script> jqscript = Script::Compile(jquery);
	jqscript->Run();

	void *index = read_file_contents("pages/index.fjs");
	Handle<String> source = String::New( (const char *)(index) );
	TryCatch ex;
	Handle<Script> script = Script::Compile(source);

	if (script.IsEmpty()) {
		Handle<Value> exception = ex.Exception();
		String::AsciiValue ex_str(exception);
		fprintf(stderr, *ex_str);
		fprintf(stderr, "\n\n");
	}

	Handle<Value> rc = script->Run();
	ctx.Dispose();

	/*
	String::AsciiValue rc_str(rc);
	fprintf(stderr, *rc_str);
	*/
}

int main ()
{
    FCGX_Stream *in, *out, *err;
    FCGX_ParamArray envp;
    int count = 0;

	_jQuery = read_file_contents(JQUERY_FILE);

    while (FCGX_Accept(&in, &out, &err, &envp) >= 0) {
		_in_stream = in;
		_error_stream = err;
		_out_stream = out;
        char *contentLength = FCGX_GetParam("CONTENT_LENGTH", envp);
        int len = 0;

        FCGX_FPrintF(out,
           "Content-type: text/html\r\n"
           "\r\n"
		   );

        if (contentLength != NULL)
            len = strtol(contentLength, NULL, 10);
	
		req_handle(out, envp);

		_error_stream = NULL;
		_in_stream = NULL;
		_out_stream = NULL;
    } /* while */

	if (_jQuery)
		free(_jQuery);
    return 0;
}

Handle<Value> FastJS_Write(const Arguments& args)
{
	for (int i = 0; i < args.Length(); ++i) {
		String::Utf8Value str(args[i]);
		FCGX_FPrintF(_out_stream, *str);
	}
	FCGX_FPrintF(_out_stream, "\n");
	return Undefined();
}
/*
Handle<Value> FastJS_Load(const Arguments& args) {
	for (int i = 0; i < args.Length(); ++i;) {
		HandleScope handle_scope;
		String::Utf8Value file(args[i]);
		Handle<String> source = String::New(read_file(*file));
	}
	return Undefined();
}

v8::Handle<v8::Value> Load(const v8::Arguments& args) {
  for (int i = 0; i < args.Length(); i++) {
    v8::HandleScope handle_scope;
    v8::String::Utf8Value file(args[i]);
    v8::Handle<v8::String> source = ReadFile(*file);
    if (source.IsEmpty()) {
      return v8::ThrowException(v8::String::New("Error loading file"));
    }
    if (!ExecuteString(source, v8::String::New(*file), false, false)) {
      return v8::ThrowException(v8::String::New("Error executing  file"));
    }
  }
  return v8::Undefined();
}
*/
