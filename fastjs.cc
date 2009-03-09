/*
 * fastjs - A v8-based JavaScript FastCGI server 
 *
 * Copyright 2009 - R. Tyler Ballance <tyler@monkeypox.org>
 */

#include <cstring>
#include <cstdio>
#include <cstdlib>

#include <v8.h>

#include "fastjs.h"

extern "C" {
	extern char **environ;
	#include <fcgi_config.h>
	#include <fcgiapp.h>
	#include <strings.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <fcntl.h>
	#include <unistd.h>

	#include <glib.h>
}

using namespace v8; // *puke*

/* Functions that will be exposed into the JavaScript environment */
Handle<Value> FastJS_Write(const Arguments& args);
Handle<Value> FastJS_Source(const Arguments& args);
Handle<Value> FastJS_Log(const Arguments& args);

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

static void log_exception(FILE *stream, TryCatch *context) 
{
	HandleScope scope;
	String::Utf8Value exception(context->Exception());
	Handle<Message> message = context->Message();

	if (message.IsEmpty()) {
		/* V8 didn't pass any extra fancy information back */
		fprintf(stream, *exception);
		return;
	}

	String::Utf8Value filename(message->GetScriptResourceName());
	int line = message->GetLineNumber();
	fprintf(stream, "%s:%i: %s\n", *filename, line, *exception);

	String::Utf8Value offender(message->GetSourceLine());
	fprintf(stream, "\t%s\n", *offender);
	return;
}

static int exec_javascript(const char *script) 
{
	HandleScope scope;
	TryCatch exception_ctx;

	Handle<Script> compiled = Script::Compile(String::New(script));

	if (compiled.IsEmpty()) {
		log_exception(stderr, &exception_ctx);
		return FASTJS_COMPILE_ERROR;
	}

	Handle<Value> result = compiled->Run();

	if (result.IsEmpty()) {
		log_exception(stderr, &exception_ctx);
		return FASTJS_EXECUTION_ERROR;
	}

	return FASTJS_SUCCESS;
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
	Handle<ObjectTemplate> _fastjs = ObjectTemplate::New();
	_fastjs->Set(String::New("write"), FunctionTemplate::New(FastJS_Write));
	_fastjs->Set(String::New("source"), FunctionTemplate::New(FastJS_Source));
	_fastjs->Set(String::New("log"), FunctionTemplate::New(FastJS_Log));

	_global->Set(String::New("fastjs"), _fastjs);
	Persistent<Context> ctx = Context::New(NULL, _global);
	Context::Scope ctx_scope(ctx);

	int rc = 0;
	rc = exec_javascript(JQUERY_COMPAT);
	rc = exec_javascript( (const char *)(_jQuery) );

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

	Handle<Value> results = script->Run();
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
    unsigned int count = 0;

	_jQuery = read_file_contents(JQUERY_FILE);

    while (FCGX_Accept(&in, &out, &err, &envp) >= 0) {
		_in_stream = in;
		_error_stream = err;
		_out_stream = out;
        char *contentLength = FCGX_GetParam("CONTENT_LENGTH", envp);
        unsigned int len = 0;

        FCGX_FPrintF(out, "Content-type: text/html\r\nX-FastJS-Request: %d\r\n"
					"X-FastJS-Process: %d\r\nX-FastJS-Engine: V8\r\n\r\n", ++count, getpid());

        if (contentLength != NULL)
            len = strtol(contentLength, NULL, 10);
	
		req_handle(out, envp);

		//PrintEnv(err, "Request environment", envp);
		//PrintEnv(err, "Initial", environ);

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

Handle<Value> FastJS_Source(const Arguments& args) {
	for (int i = 0; i < args.Length(); ++i) {
		HandleScope scope;
		String::Utf8Value file(args[i]);
		void *sourcebuf = read_file_contents(*file);

		if (sourcebuf == NULL)
			return ThrowException(String::New("Error loading the file!"));
		int rc = exec_javascript((const char *)(sourcebuf));
		if (rc != FASTJS_SUCCESS) 
			return ThrowException(String::New("Error executing the file!"));
	}
	return Undefined();
}

Handle<Value> FastJS_Log(const Arguments& args) {
	for (int i =0; i < args.Length(); ++i) {
		HandleScope scope;
		String::Utf8Value line(args[i]);

		FCGX_FPrintF(_error_stream, "FastJS_Log>> %s\n", *line);
	}

	return Undefined();
}
