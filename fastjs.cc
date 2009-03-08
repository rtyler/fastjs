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
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <fcntl.h>
	#include <unistd.h>
}

using namespace v8; // *puke*

#define JQUERY_FILE "jquery-1.3.2.min.js"
#define JQUERY_COMPAT "if (typeof(window) == 'undefined'){window=new Object();document=window;self=window;navigator=new Object();navigator.userAgent=navigator.userAgent||'FastJS Server';location=new Object();location.href='file:///dev/null';location.protocol='file:';location.host = 'FastJS';};"

/* Functions that will be exposed into the JavaScript environment */
Handle<Value> FastJS_Write(const Arguments& args);
Handle<Value> FastJS_Load(const Arguments& args);

FCGX_Stream *_current_stream = NULL; /* Unfortunately we need a global to access the stream inside JavaScript callbacks */

static void PrintEnv(FCGX_Stream *out, char *label, char **envp)
{
    FCGX_FPrintF(out, "%s:<br>\n<pre>\n", label);
    for( ; *envp != NULL; envp++) {
        FCGX_FPrintF(out, "%s\n", *envp);
    }
    FCGX_FPrintF(out, "</pre><p>\n");
}

static void *load_jquery() 
{
	struct stat attributes;
	int fd = 0;
	int rc = 0;

	rc = stat(JQUERY_FILE, &attributes);
	if (rc != 0) {
		/* I should do something with the error here */
		return NULL;
	}

	void *jquery = malloc(sizeof(char) * (attributes.st_size + 2));
	fd = open(JQUERY_FILE, 0);
	if (fd == 0) {
		/* I should do something with the error here */
		close(fd);
		return NULL;
	}
	
	rc = read(fd, jquery, attributes.st_size); 
	close(fd);
	return jquery;
}


static void req_handle(FCGX_Stream *out, char **environment) 
{
	void *jq = load_jquery();
	if (jq == NULL) {
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

	Handle<String> jquery = String::New((const char *)(jq));
	Handle<Script> jqscript = Script::Compile(jquery);
	jqscript->Run();

	Handle<String> source = String::New("(function() { write('Looping over location:'); $.each(location, function(index) { write(this); }); })();");
	Handle<Script> script = Script::Compile(source);
	
	Handle<Value> rc = script->Run();
	ctx.Dispose();
	/*
	String::AsciiValue rc_str(rc);
	FCGX_FPrintF(out, *rc_str);
	*/

	PrintEnv(out, "<br/><br/>FastCGI Environment:", environment);

}

int main ()
{
    FCGX_Stream *in, *out, *err;
    FCGX_ParamArray envp;
    int count = 0;

    while (FCGX_Accept(&in, &out, &err, &envp) >= 0) {
		_current_stream = out;
        char *contentLength = FCGX_GetParam("CONTENT_LENGTH", envp);
        int len = 0;

        FCGX_FPrintF(out,
           "Content-type: text/html\r\n"
           "\r\n"
           "<html><title>FastJS Sample Page</title>"
           "<h1>FastJS</h1>\n"
           "Request number %d,  Process ID: %d<p>\n", ++count, getpid());

        if (contentLength != NULL)
            len = strtol(contentLength, NULL, 10);
	
		req_handle(out, envp);
		_current_stream = NULL;
    } /* while */

    return 0;
}

Handle<Value> FastJS_Write(const Arguments& args)
{
	for (int i = 0; i < args.Length(); ++i) {
		String::Utf8Value str(args[i]);
		FCGX_FPrintF(_current_stream, *str);
	}
	FCGX_FPrintF(_current_stream, "\n");
	return Undefined();
}
