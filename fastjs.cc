/*
 * fastjs - A v8-based JavaScript FastCGI server 
 *
 * Copyright 2009 - R. Tyler Ballance <tyler@monkeypox.org>
 */

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

#include <v8.h>

extern "C" {
	extern char **environ;
	#include <fcgi_config.h>
	#include <fcgiapp.h>
}

using namespace v8; // *puke*

static void PrintEnv(FCGX_Stream *out, char *label, char **envp)
{
    FCGX_FPrintF(out, "%s:<br>\n<pre>\n", label);
    for( ; *envp != NULL; envp++) {
        FCGX_FPrintF(out, "%s\n", *envp);
    }
    FCGX_FPrintF(out, "</pre><p>\n");
}

static void req_handle(FCGX_Stream *out, char **environment) 
{
	HandleScope scope;
	Persistent<Context> ctx = Context::New();
	Context::Scope ctx_scope(ctx);

	Handle<String> source = String::New("(function() { return 'OMG'; })()");
	Handle<Script> script = Script::Compile(source);

	Handle<Value> rc = script->Run();
	ctx.Dispose();

	String::AsciiValue rc_str(rc);

	FCGX_FPrintF(out, "Our test() JavaScript function returned:\n");
	FCGX_FPrintF(out, *rc_str);

	PrintEnv(out, "foo", environment);
}

int main ()
{
    FCGX_Stream *in, *out, *err;
    FCGX_ParamArray envp;
    int count = 0;

    while (FCGX_Accept(&in, &out, &err, &envp) >= 0) {
        char *contentLength = FCGX_GetParam("CONTENT_LENGTH", envp);
        int len = 0;

        FCGX_FPrintF(out,
           "Content-type: text/html\r\n"
           "\r\n"
           "<title>FastJS Sample Page"
           "<h1>FastJS</h1>\n"
           "Request number %d,  Process ID: %d<p>\n", ++count, getpid());

        if (contentLength != NULL)
            len = strtol(contentLength, NULL, 10);
	
		req_handle(out, envp);
    } /* while */

    return 0;
}
