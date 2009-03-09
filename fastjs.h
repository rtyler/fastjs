/*
 * fastjs - A v8-based JavaScript FastCGI server 
 *
 * Copyright 2009 - R. Tyler Ballance <tyler@monkeypox.org>
 */

#ifndef _FASTJS_H_
#define _FASTJS_H_

#define JQUERY_FILE "jquery-1.3.2.min.js"
#define JQUERY_COMPAT "if (typeof(window) == 'undefined'){window=new Object();document=window;self=window;navigator=new Object();navigator.userAgent=navigator.userAgent||'FastJS Server';location=new Object();location.href='file:///dev/null';location.protocol='file:';location.host = 'FastJS';}"

#define FASTJS_SUCCESS 0
#define FASTJS_COMPILE_ERROR -1
#define FASTJS_EXECUTION_ERROR -2


#endif
