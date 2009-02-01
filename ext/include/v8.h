// Copyright 2007-2008 the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/** \mainpage V8 API Reference Guide
 *
 * V8 is Google's open source JavaScript engine.
 *
 * This set of documents provides reference material generated from the
 * V8 header file, include/v8.h.
 *
 * For other documentation see http://code.google.com/apis/v8/
 */

#ifndef V8_H_
#define V8_H_

#include <stdio.h>

#ifdef _WIN32
typedef int int32_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;  // NOLINT
typedef long long int64_t;  // NOLINT

// Setup for Windows DLL export/import. When building the V8 DLL the
// BUILDING_V8_SHARED needs to be defined. When building a program which uses
// the V8 DLL USING_V8_SHARED needs to be defined. When either building the V8
// static library or building a program which uses the V8 static library neither
// BUILDING_V8_SHARED nor USING_V8_SHARED should be defined.
// The reason for having both EXPORT and EXPORT_INLINE is that classes which
// have their code inside this header file needs to have __declspec(dllexport)
// when building the DLL but cannot have __declspec(dllimport) when building
// a program which uses the DLL.
#if defined(BUILDING_V8_SHARED) && defined(USING_V8_SHARED)
#error both BUILDING_V8_SHARED and USING_V8_SHARED are set - please check the\
  build configuration to ensure that at most one of these is set
#endif

#ifdef BUILDING_V8_SHARED
#define EXPORT __declspec(dllexport)
#define EXPORT_INLINE __declspec(dllexport)
#elif USING_V8_SHARED
#define EXPORT __declspec(dllimport)
#define EXPORT_INLINE
#else
#define EXPORT
#define EXPORT_INLINE
#endif  // BUILDING_V8_SHARED

#else  // _WIN32

#include <stdint.h>

// Setup for Linux shared library export. There is no need to destinguish
// neither between building or using the V8 shared library nor between using
// the shared or static V8 library as there is on Windows. Therefore there is
// no checking of BUILDING_V8_SHARED and USING_V8_SHARED.
#if defined(__GNUC__) && (__GNUC__ >= 4)
#define EXPORT __attribute__ ((visibility("default")))
#define EXPORT_INLINE __attribute__ ((visibility("default")))
#else  // defined(__GNUC__) && (__GNUC__ >= 4)
#define EXPORT
#define EXPORT_INLINE
#endif  // defined(__GNUC__) && (__GNUC__ >= 4)

#endif  // _WIN32

/**
 * The v8 JavaScript engine.
 */
namespace v8 {

class Context;
class String;
class Value;
class Utils;
class Number;
class Object;
class Array;
class Int32;
class Uint32;
class External;
class Primitive;
class Boolean;
class Integer;
class Function;
class Date;
class ImplementationUtilities;
class Signature;
template <class T> class Handle;
template <class T> class Local;
template <class T> class Persistent;
class FunctionTemplate;
class ObjectTemplate;
class Data;


// --- W e a k  H a n d l e s


/**
 * A weak reference callback function.
 *
 * \param object the weak global object to be reclaimed by the garbage collector
 * \param parameter the value passed in when making the weak global object
 */
typedef void (*WeakReferenceCallback)(Persistent<Value> object,
                                      void* parameter);


// --- H a n d l e s ---

#define TYPE_CHECK(T, S)                              \
  while (false) {                                     \
    *(static_cast<T**>(0)) = static_cast<S*>(0);      \
  }

/**
 * An object reference managed by the v8 garbage collector.
 *
 * All objects returned from v8 have to be tracked by the garbage
 * collector so that it knows that the objects are still alive.  Also,
 * because the garbage collector may move objects, it is unsafe to
 * point directly to an object.  Instead, all objects are stored in
 * handles which are known by the garbage collector and updated
 * whenever an object moves.  Handles should always be passed by value
 * (except in cases like out-parameters) and they should never be
 * allocated on the heap.
 *
 * There are two types of handles: local and persistent handles.
 * Local handles are light-weight and transient and typically used in
 * local operations.  They are managed by HandleScopes.  Persistent
 * handles can be used when storing objects across several independent
 * operations and have to be explicitly deallocated when they're no
 * longer used.
 *
 * It is safe to extract the object stored in the handle by
 * dereferencing the handle (for instance, to extract the Object* from
 * an Handle<Object>); the value will still be governed by a handle
 * behind the scenes and the same rules apply to these values as to
 * their handles.
 */
template <class T> class EXPORT_INLINE Handle {
 public:

  /**
   * Creates an empty handle.
   */
  Handle();

  /**
   * Creates a new handle for the specified value.
   */
  explicit Handle(T* val) : val_(val) { }

  /**
   * Creates a handle for the contents of the specified handle.  This
   * constructor allows you to pass handles as arguments by value and
   * to assign between handles.  However, if you try to assign between
   * incompatible handles, for instance from a Handle<String> to a
   * Handle<Number> it will cause a compiletime error.  Assigning
   * between compatible handles, for instance assigning a
   * Handle<String> to a variable declared as Handle<Value>, is legal
   * because String is a subclass of Value.
   */
  template <class S> inline Handle(Handle<S> that)
      : val_(reinterpret_cast<T*>(*that)) {
    /**
     * This check fails when trying to convert between incompatible
     * handles. For example, converting from a Handle<String> to a
     * Handle<Number>.
     */
    TYPE_CHECK(T, S);
  }

  /**
   * Returns true if the handle is empty.
   */
  bool IsEmpty() const { return val_ == 0; }

  T* operator->() const;

  T* operator*() const;

  /**
   * Sets the handle to be empty. IsEmpty() will then return true.
   */
  void Clear() { this->val_ = 0; }

  /**
   * Checks whether two handles are the same.
   * Returns true if both are empty, or if the objects
   * to which they refer are identical.
   * The handles' references are not checked.
   */
  template <class S> bool operator==(Handle<S> that) const {
    void** a = reinterpret_cast<void**>(**this);
    void** b = reinterpret_cast<void**>(*that);
    if (a == 0) return b == 0;
    if (b == 0) return false;
    return *a == *b;
  }

  /**
   * Checks whether two handles are different.
   * Returns true if only one of the handles is empty, or if
   * the objects to which they refer are different.
   * The handles' references are not checked.
   */
  template <class S> bool operator!=(Handle<S> that) const {
    return !operator==(that);
  }

  template <class S> static inline Handle<T> Cast(Handle<S> that) {
    if (that.IsEmpty()) return Handle<T>();
    return Handle<T>(T::Cast(*that));
  }

 private:
  T* val_;
};


/**
 * A light-weight stack-allocated object handle.  All operations
 * that return objects from within v8 return them in local handles.  They
 * are created within HandleScopes, and all local handles allocated within a
 * handle scope are destroyed when the handle scope is destroyed.  Hence it
 * is not necessary to explicitly deallocate local handles.
 */
template <class T> class EXPORT_INLINE Local : public Handle<T> {
 public:
  Local();
  template <class S> inline Local(Local<S> that)
      : Handle<T>(reinterpret_cast<T*>(*that)) {
    /**
     * This check fails when trying to convert between incompatible
     * handles. For example, converting from a Handle<String> to a
     * Handle<Number>.
     */
    TYPE_CHECK(T, S);
  }
  template <class S> inline Local(S* that) : Handle<T>(that) { }
  template <class S> static inline Local<T> Cast(Local<S> that) {
    if (that.IsEmpty()) return Local<T>();
    return Local<T>(T::Cast(*that));
  }

  /** Create a local handle for the content of another handle.
   *  The referee is kept alive by the local handle even when
   *  the original handle is destroyed/disposed.
   */
  static Local<T> New(Handle<T> that);
};


/**
 * An object reference that is independent of any handle scope.  Where
 * a Local handle only lives as long as the HandleScope in which it was
 * allocated, a Persistent handle remains valid until it is explicitly
 * disposed.
 *
 * A persistent handle contains a reference to a storage cell within
 * the v8 engine which holds an object value and which is updated by
 * the garbage collector whenever the object is moved.  A new storage
 * cell can be created using Persistent::New and existing handles can
 * be disposed using Persistent::Dispose.  Since persistent handles
 * are passed by value you may have many persistent handle objects
 * that point to the same storage cell.  For instance, if you pass a
 * persistent handle as an argument to a function you will not get two
 * different storage cells but rather two references to the same
 * storage cell.
 */
template <class T> class EXPORT_INLINE Persistent : public Handle<T> {
 public:

  /**
   * Creates an empty persistent handle that doesn't point to any
   * storage cell.
   */
  Persistent();

  /**
   * Creates a persistent handle for the same storage cell as the
   * specified handle.  This constructor allows you to pass persistent
   * handles as arguments by value and to assign between persistent
   * handles.  However, attempting to assign between incompatible
   * persistent handles, for instance from a Persistent<String> to a
   * Persistent<Number> will cause a compiletime error.  Assigning
   * between compatible persistent handles, for instance assigning a
   * Persistent<String> to a variable declared as Persistent<Value>,
   * is allowed as String is a subclass of Value.
   */
  template <class S> inline Persistent(Persistent<S> that)
      : Handle<T>(reinterpret_cast<T*>(*that)) {
    /**
     * This check fails when trying to convert between incompatible
     * handles. For example, converting from a Handle<String> to a
     * Handle<Number>.
     */
    TYPE_CHECK(T, S);
  }

  template <class S> inline Persistent(S* that) : Handle<T>(that) { }

  /**
   * "Casts" a plain handle which is known to be a persistent handle
   * to a persistent handle.
   */
  template <class S> explicit inline Persistent(Handle<S> that)
      : Handle<T>(*that) { }

  template <class S> static inline Persistent<T> Cast(Persistent<S> that) {
    if (that.IsEmpty()) return Persistent<T>();
    return Persistent<T>(T::Cast(*that));
  }

  /**
   * Creates a new persistent handle for an existing local or
   * persistent handle.
   */
  static Persistent<T> New(Handle<T> that);

  /**
   * Releases the storage cell referenced by this persistent handle.
   * Does not remove the reference to the cell from any handles.
   * This handle's reference, and any any other references to the storage
   * cell remain and IsEmpty will still return false.
   */
  void Dispose();

  /**
   * Make the reference to this object weak.  When only weak handles
   * refer to the object, the garbage collector will perform a
   * callback to the given V8::WeakReferenceCallback function, passing
   * it the object reference and the given parameters.
   */
  void MakeWeak(void* parameters, WeakReferenceCallback callback);

  /** Clears the weak reference to this object.*/
  void ClearWeak();

  /**
   *Checks if the handle holds the only reference to an object.
   */
  bool IsNearDeath() const;

  /**
   * Returns true if the handle's reference is weak.
   */
  bool IsWeak() const;

 private:
  friend class ImplementationUtilities;
  friend class ObjectTemplate;
};


 /**
 * A stack-allocated class that governs a number of local handles.
 * After a handle scope has been created, all local handles will be
 * allocated within that handle scope until either the handle scope is
 * deleted or another handle scope is created.  If there is already a
 * handle scope and a new one is created, all allocations will take
 * place in the new handle scope until it is deleted.  After that,
 * new handles will again be allocated in the original handle scope.
 *
 * After the handle scope of a local handle has been deleted the
 * garbage collector will no longer track the object stored in the
 * handle and may deallocate it.  The behavior of accessing a handle
 * for which the handle scope has been deleted is undefined.
 */
class EXPORT HandleScope {
 public:
  HandleScope();

  ~HandleScope();

  /**
   * Closes the handle scope and returns the value as a handle in the
   * previous scope, which is the new current scope after the call.
   */
  template <class T> Local<T> Close(Handle<T> value);

  /**
   * Counts the number of allocated handles.
   */
  static int NumberOfHandles();

  /**
   * Creates a new handle with the given value.
   */
  static void** CreateHandle(void* value);

 private:
  // Make it impossible to create heap-allocated or illegal handle
  // scopes by disallowing certain operations.
  HandleScope(const HandleScope&);
  void operator=(const HandleScope&);
  void* operator new(size_t size);
  void operator delete(void*, size_t);

  // This Data class is accessible internally through a typedef in the
  // ImplementationUtilities class.
  class EXPORT Data {
   public:
    int extensions;
    void** next;
    void** limit;
    inline void Initialize() {
      extensions = -1;
      next = limit = NULL;
    }
  };

  Data previous_;

  // Allow for the active closing of HandleScopes which allows to pass a handle
  // from the HandleScope being closed to the next top most HandleScope.
  bool is_closed_;
  void** RawClose(void** value);

  friend class ImplementationUtilities;
};


// --- S p e c i a l   o b j e c t s ---


/**
 * The superclass of values and API object templates.
 */
class EXPORT Data {
 private:
  Data();
};


/**
 * Pre-compilation data that can be associated with a script.  This
 * data can be calculated for a script in advance of actually
 * compiling it, and can be stored between compilations.  When script
 * data is given to the compile method compilation will be faster.
 */
class EXPORT ScriptData {  // NOLINT
 public:
  virtual ~ScriptData() { }
  static ScriptData* PreCompile(const char* input, int length);
  static ScriptData* New(unsigned* data, int length);

  virtual int Length() = 0;
  virtual unsigned* Data() = 0;
};


/**
 * The origin, within a file, of a script.
 */
class EXPORT ScriptOrigin {
 public:
  ScriptOrigin(Handle<Value> resource_name,
               Handle<Integer> resource_line_offset = Handle<Integer>(),
               Handle<Integer> resource_column_offset = Handle<Integer>())
      : resource_name_(resource_name),
        resource_line_offset_(resource_line_offset),
        resource_column_offset_(resource_column_offset) { }
  inline Handle<Value> ResourceName() const;
  inline Handle<Integer> ResourceLineOffset() const;
  inline Handle<Integer> ResourceColumnOffset() const;
 private:
  Handle<Value> resource_name_;
  Handle<Integer> resource_line_offset_;
  Handle<Integer> resource_column_offset_;
};


/**
 * A compiled JavaScript script.
 */
class EXPORT Script {
 public:

  /**
   * Compiles the specified script. The ScriptOrigin* and ScriptData*
   * parameters are owned by the caller of Script::Compile. No
   * references to these objects are kept after compilation finishes.
   */
  static Local<Script> Compile(Handle<String> source,
                               ScriptOrigin* origin = NULL,
                               ScriptData* pre_data = NULL);

  /**
   * Compiles the specified script using the specified file name
   * object (typically a string) as the script's origin.
   */
  static Local<Script> Compile(Handle<String> source,
                               Handle<Value> file_name);

  /**
   * Runs the script returning the resulting value.
   */
  Local<Value> Run();
};


/**
 * An error message.
 */
class EXPORT Message {
 public:
  Local<String> Get() const;
  Local<String> GetSourceLine() const;

  Handle<Value> GetScriptResourceName() const;

  /**
   * Returns the number, 1-based, of the line where the error occurred.
   */
  int GetLineNumber() const;

  /**
   * Returns the index within the script of the first character where
   * the error occurred.
   */
  int GetStartPosition() const;

  /**
   * Returns the index within the script of the last character where
   * the error occurred.
   */
  int GetEndPosition() const;

  /**
   * Returns the index within the line of the first character where
   * the error occurred.
   */
  int GetStartColumn() const;

  /**
   * Returns the index within the line of the last character where
   * the error occurred.
   */
  int GetEndColumn() const;

  // TODO(1245381): Print to a string instead of on a FILE.
  static void PrintCurrentStackTrace(FILE* out);
};


// --- V a l u e ---


/**
 * The superclass of all JavaScript values and objects.
 */
class EXPORT Value : public Data {
 public:

  /**
   * Returns true if this value is the undefined value.  See ECMA-262
   * 4.3.10.
   */
  bool IsUndefined() const;

  /**
   * Returns true if this value is the null value.  See ECMA-262
   * 4.3.11.
   */
  bool IsNull() const;

   /**
   * Returns true if this value is true.
   */
  bool IsTrue() const;

  /**
   * Returns true if this value is false.
   */
  bool IsFalse() const;

  /**
   * Returns true if this value is an instance of the String type.
   * See ECMA-262 8.4.
   */
  bool IsString() const;

  /**
   * Returns true if this value is a function.
   */
  bool IsFunction() const;

  /**
   * Returns true if this value is an array.
   */
  bool IsArray() const;

  /**
   * Returns true if this value is an object.
   */
  bool IsObject() const;

  /**
   * Returns true if this value is boolean.
   */
  bool IsBoolean() const;

  /**
   * Returns true if this value is a number.
   */
  bool IsNumber() const;

  /**
   * Returns true if this value is external.
   */
  bool IsExternal() const;

  /**
   * Returns true if this value is a 32-bit signed integer.
   */
  bool IsInt32() const;

  /**
   * Returns true if this value is a Date.
   */
  bool IsDate() const;

  Local<Boolean> ToBoolean() const;
  Local<Number> ToNumber() const;
  Local<String> ToString() const;
  Local<String> ToDetailString() const;
  Local<Object> ToObject() const;
  Local<Integer> ToInteger() const;
  Local<Uint32> ToUint32() const;
  Local<Int32> ToInt32() const;

  /**
   * Attempts to convert a string to an array index.
   * Returns an empty handle if the conversion fails.
   */
  Local<Uint32> ToArrayIndex() const;

  bool BooleanValue() const;
  double NumberValue() const;
  int64_t IntegerValue() const;
  uint32_t Uint32Value() const;
  int32_t Int32Value() const;

  /** JS == */
  bool Equals(Handle<Value> that) const;
  bool StrictEquals(Handle<Value> that) const;
};


/**
 * The superclass of primitive values.  See ECMA-262 4.3.2.
 */
class EXPORT Primitive : public Value { };


/**
 * A primitive boolean value (ECMA-262, 4.3.14).  Either the true
 * or false value.
 */
class EXPORT Boolean : public Primitive {
 public:
  bool Value() const;
  static inline Handle<Boolean> New(bool value);
};


/**
 * A JavaScript string value (ECMA-262, 4.3.17).
 */
class EXPORT String : public Primitive {
 public:

  /**
   * Returns the number of characters in this string.
   */
  int Length() const;

  /**
   * Returns the number of bytes in the UTF-8 encoded
   * representation of this string.
   */
  int Utf8Length() const;

  /**
   * Write the contents of the string to an external buffer.
   * If no arguments are given, expects the buffer to be large
   * enough to hold the entire string and NULL terminator. Copies
   * the contents of the string and the NULL terminator into the
   * buffer.
   *
   * Copies up to length characters into the output buffer.
   * Only null-terminates if there is enough space in the buffer.
   *
   * \param buffer The buffer into which the string will be copied.
   * \param start The starting position within the string at which
   * copying begins.
   * \param length The number of bytes to copy from the string.
   * \return The number of characters copied to the buffer
   * excluding the NULL terminator.
   */
  int Write(uint16_t* buffer, int start = 0, int length = -1) const;  // UTF-16
  int WriteAscii(char* buffer, int start = 0, int length = -1) const;  // ASCII
  int WriteUtf8(char* buffer, int length = -1) const; // UTF-8

  /**
   * Returns true if the string is external
   */
  bool IsExternal() const;

  /**
   * Returns true if the string is both external and ascii
   */
  bool IsExternalAscii() const;
  /**
   * An ExternalStringResource is a wrapper around a two-byte string
   * buffer that resides outside V8's heap. Implement an
   * ExternalStringResource to manage the life cycle of the underlying
   * buffer.  Note that the string data must be immutable.
   */
  class EXPORT ExternalStringResource {  // NOLINT
   public:
    /**
     * Override the destructor to manage the life cycle of the underlying
     * buffer.
     */
    virtual ~ExternalStringResource() {}
    /** The string data from the underlying buffer.*/
    virtual const uint16_t* data() const = 0;
    /** The length of the string. That is, the number of two-byte characters.*/
    virtual size_t length() const = 0;
   protected:
    ExternalStringResource() {}
   private:
    // Disallow copying and assigning.
    ExternalStringResource(const ExternalStringResource&);
    void operator=(const ExternalStringResource&);
  };

  /**
   * An ExternalAsciiStringResource is a wrapper around an ascii
   * string buffer that resides outside V8's heap. Implement an
   * ExternalAsciiStringResource to manage the life cycle of the
   * underlying buffer.  Note that the string data must be immutable
   * and that the data must be strict 7-bit ASCII, not Latin1 or
   * UTF-8, which would require special treatment internally in the
   * engine and, in the case of UTF-8, do not allow efficient indexing.
   * Use String::New or convert to 16 bit data for non-ASCII.
   */

  class EXPORT ExternalAsciiStringResource {  // NOLINT
   public:
    /**
     * Override the destructor to manage the life cycle of the underlying
     * buffer.
     */
    virtual ~ExternalAsciiStringResource() {}
    /** The string data from the underlying buffer.*/
    virtual const char* data() const = 0;
    /** The number of ascii characters in the string.*/
    virtual size_t length() const = 0;
   protected:
    ExternalAsciiStringResource() {}
   private:
    // Disallow copying and assigning.
    ExternalAsciiStringResource(const ExternalAsciiStringResource&);
    void operator=(const ExternalAsciiStringResource&);
  };

  /**
   * Get the ExternalStringResource for an external string.  Only
   * valid if IsExternal() returns true.
   */
  ExternalStringResource* GetExternalStringResource() const;

  /**
   * Get the ExternalAsciiStringResource for an external ascii string.
   * Only valid if IsExternalAscii() returns true.
   */
  ExternalAsciiStringResource* GetExternalAsciiStringResource() const;

  static String* Cast(v8::Value* obj);

  /**
   * Allocates a new string from either utf-8 encoded or ascii data.
   * The second parameter 'length' gives the buffer length.
   * If the data is utf-8 encoded, the caller must
   * be careful to supply the length parameter.
   * If it is not given, the function calls
   * 'strlen' to determine the buffer length, it might be
   * wrong if 'data' contains a null character.
   */
  static Local<String> New(const char* data, int length = -1);

  /** Allocates a new string from utf16 data.*/
  static Local<String> New(const uint16_t* data, int length = -1);

  /** Creates a symbol. Returns one if it exists already.*/
  static Local<String> NewSymbol(const char* data, int length = -1);

  /**
   * Creates a new external string using the data defined in the given
   * resource. The resource is deleted when the external string is no
   * longer live on V8's heap. The caller of this function should not
   * delete or modify the resource. Neither should the underlying buffer be
   * deallocated or modified except through the destructor of the
   * external string resource.
   */
  static Local<String> NewExternal(ExternalStringResource* resource);

  /**
   * Creates a new external string using the ascii data defined in the given
   * resource. The resource is deleted when the external string is no
   * longer live on V8's heap. The caller of this function should not
   * delete or modify the resource. Neither should the underlying buffer be
   * deallocated or modified except through the destructor of the
   * external string resource.
   */
  static Local<String> NewExternal(ExternalAsciiStringResource* resource);

  /** Creates an undetectable string from the supplied ascii or utf-8 data.*/
  static Local<String> NewUndetectable(const char* data, int length = -1);

  /** Creates an undetectable string from the supplied utf-16 data.*/
  static Local<String> NewUndetectable(const uint16_t* data, int length = -1);

  /**
   * Converts an object to a utf8-encoded character array.  Useful if
   * you want to print the object.
   */
  class EXPORT Utf8Value {
   public:
    explicit Utf8Value(Handle<v8::Value> obj);
    ~Utf8Value();
    char* operator*() { return str_; }
    int length() { return length_; }
   private:
    char* str_;
    int length_;

    // Disallow copying and assigning.
    Utf8Value(const Utf8Value&);
    void operator=(const Utf8Value&);
  };

  /**
   * Converts an object to an ascii string.
   * Useful if you want to print the object.
   */
  class EXPORT AsciiValue {
   public:
    explicit AsciiValue(Handle<v8::Value> obj);
    ~AsciiValue();
    char* operator*() { return str_; }
    int length() { return length_; }
   private:
    char* str_;
    int length_;

    // Disallow copying and assigning.
    AsciiValue(const AsciiValue&);
    void operator=(const AsciiValue&);
  };

  /**
   * Converts an object to a two-byte string.
   */
  class EXPORT Value {
   public:
    explicit Value(Handle<v8::Value> obj);
    ~Value();
    uint16_t* operator*() { return str_; }
    int length() { return length_; }
   private:
    uint16_t* str_;
    int length_;

    // Disallow copying and assigning.
    Value(const Value&);
    void operator=(const Value&);
  };
};


/**
 * A JavaScript number value (ECMA-262, 4.3.20)
 */
class EXPORT Number : public Primitive {
 public:
  double Value() const;
  static Local<Number> New(double value);
  static Number* Cast(v8::Value* obj);
 private:
  Number();
};


/**
 * A JavaScript value representing a signed integer.
 */
class EXPORT Integer : public Number {
 public:
  static Local<Integer> New(int32_t value);
  int64_t Value() const;
  static Integer* Cast(v8::Value* obj);
 private:
  Integer();
};


/**
 * A JavaScript value representing a 32-bit signed integer.
 */
class EXPORT Int32 : public Integer {
 public:
  int32_t Value() const;
 private:
  Int32();
};


/**
 * A JavaScript value representing a 32-bit unsigned integer.
 */
class EXPORT Uint32 : public Integer {
 public:
  uint32_t Value() const;
 private:
  Uint32();
};


/**
 * An instance of the built-in Date constructor (ECMA-262, 15.9).
 */
class EXPORT Date : public Value {
 public:
  static Local<Value> New(double time);

  /**
   * A specialization of Value::NumberValue that is more efficient
   * because we know the structure of this object.
   */
  double NumberValue() const;

  static Date* Cast(v8::Value* obj);
};


enum PropertyAttribute {
  None       = 0,
  ReadOnly   = 1 << 0,
  DontEnum   = 1 << 1,
  DontDelete = 1 << 2
};

/**
 * A JavaScript object (ECMA-262, 4.3.3)
 */
class EXPORT Object : public Value {
 public:
  bool Set(Handle<Value> key,
           Handle<Value> value,
           PropertyAttribute attribs = None);
  Local<Value> Get(Handle<Value> key);

  // TODO(1245389): Replace the type-specific versions of these
  // functions with generic ones that accept a Handle<Value> key.
  bool Has(Handle<String> key);
  bool Delete(Handle<String> key);
  bool Has(uint32_t index);
  bool Delete(uint32_t index);

  /**
   * Returns an array containing the names of the enumerable properties
   * of this object, including properties from prototype objects.  The
   * array returned by this method contains the same values as would
   * be enumerated by a for-in statement over this object.
   */
  Local<Array> GetPropertyNames();

  /**
   * Get the prototype object.  This does not skip objects marked to
   * be skipped by __proto__ and it does not consult the security
   * handler.
   */
  Local<Value> GetPrototype();

  /**
   * Call builtin Object.prototype.toString on this object.
   * This is different from Value::ToString() that may call
   * user-defined toString function. This one does not.
   */
  Local<String> ObjectProtoToString();

  /** Gets the number of internal fields for this Object. */
  int InternalFieldCount();
  /** Gets the value in an internal field. */
  Local<Value> GetInternalField(int index);
  /** Sets the value in an internal field. */
  void SetInternalField(int index, Handle<Value> value);

  // Testers for local properties.
  bool HasRealNamedProperty(Handle<String> key);
  bool HasRealIndexedProperty(uint32_t index);
  bool HasRealNamedCallbackProperty(Handle<String> key);

  /**
   * If result.IsEmpty() no real property was located in the prototype chain.
   * This means interceptors in the prototype chain are not called.
   */
  Handle<Value> GetRealNamedPropertyInPrototypeChain(Handle<String> key);

  /** Tests for a named lookup interceptor.*/
  bool HasNamedLookupInterceptor();

  /** Tests for an index lookup interceptor.*/
  bool HasIndexedLookupInterceptor();

  /**
   * Turns on access check on the object if the object is an instance of
   * a template that has access check callbacks. If an object has no
   * access check info, the object cannot be accessed by anyone.
   */
  void TurnOnAccessCheck();

  static Local<Object> New();
  static Object* Cast(Value* obj);
 private:
  Object();
};


/**
 * An instance of the built-in array constructor (ECMA-262, 15.4.2).
 */
class EXPORT Array : public Object {
 public:
  uint32_t Length() const;

  static Local<Array> New(int length = 0);
  static Array* Cast(Value* obj);
 private:
  Array();
};


/**
 * A JavaScript function object (ECMA-262, 15.3).
 */
class EXPORT Function : public Object {
 public:
  Local<Object> NewInstance() const;
  Local<Object> NewInstance(int argc, Handle<Value> argv[]) const;
  Local<Value> Call(Handle<Object> recv, int argc, Handle<Value> argv[]);
  void SetName(Handle<String> name);
  Handle<Value> GetName() const;
  static Function* Cast(Value* obj);
 private:
  Function();
};


/**
 * A JavaScript value that wraps a c++ void*.  This type of value is
 * mainly used to associate c++ data structures with JavaScript
 * objects.
 */
class EXPORT External : public Value {
 public:
  static Local<External> New(void* value);
  static External* Cast(Value* obj);
  void* Value() const;
 private:
  External();
};


// --- T e m p l a t e s ---


/**
 * The superclass of object and function templates.
 */
class EXPORT Template : public Data {
 public:
  /** Adds a property to each instance created by this template.*/
  void Set(Handle<String> name, Handle<Data> value,
           PropertyAttribute attributes = None);
  inline void Set(const char* name, Handle<Data> value);
 private:
  Template();

  friend class ObjectTemplate;
  friend class FunctionTemplate;
};


/**
 * The argument information given to function call callbacks.  This
 * class provides access to information about the context of the call,
 * including the receiver, the number and values of arguments, and
 * the holder of the function.
 */
class EXPORT Arguments {
 public:
  inline int Length() const;
  inline Local<Value> operator[](int i) const;
  inline Local<Function> Callee() const;
  inline Local<Object> This() const;
  inline Local<Object> Holder() const;
  inline bool IsConstructCall() const;
  inline Local<Value> Data() const;
 private:
  Arguments();
  friend class ImplementationUtilities;
  inline Arguments(Local<Value> data,
                   Local<Object> holder,
                   Local<Function> callee,
                   bool is_construct_call,
                   void** values, int length);
  Local<Value> data_;
  Local<Object> holder_;
  Local<Function> callee_;
  bool is_construct_call_;
  void** values_;
  int length_;
};


/**
 * The information passed to an accessor callback about the context
 * of the property access.
 */
class EXPORT AccessorInfo {
 public:
  inline AccessorInfo(Local<Object> self,
                      Local<Value> data,
                      Local<Object> holder)
      : self_(self), data_(data), holder_(holder) { }
  inline Local<Value> Data() const;
  inline Local<Object> This() const;
  inline Local<Object> Holder() const;
 private:
  Local<Object> self_;
  Local<Value> data_;
  Local<Object> holder_;
};


typedef Handle<Value> (*InvocationCallback)(const Arguments& args);

typedef int (*LookupCallback)(Local<Object> self, Local<String> name);

/**
 * Accessor[Getter|Setter] are used as callback functions when
 * setting|getting a particular property. See objectTemplate::SetAccessor.
 */
typedef Handle<Value> (*AccessorGetter)(Local<String> property,
                                        const AccessorInfo& info);


typedef void (*AccessorSetter)(Local<String> property,
                               Local<Value> value,
                               const AccessorInfo& info);


/**
 * NamedProperty[Getter|Setter] are used as interceptors on object.
 * See ObjectTemplate::SetNamedPropertyHandler.
 */
typedef Handle<Value> (*NamedPropertyGetter)(Local<String> property,
                                             const AccessorInfo& info);


/**
 * Returns the value if the setter intercepts the request.
 * Otherwise, returns an empty handle.
 */
typedef Handle<Value> (*NamedPropertySetter)(Local<String> property,
                                             Local<Value> value,
                                             const AccessorInfo& info);


/**
 * Returns a non-empty handle if the interceptor intercepts the request.
 * The result is true if the property exists and false otherwise.
 */
typedef Handle<Boolean> (*NamedPropertyQuery)(Local<String> property,
                                              const AccessorInfo& info);


/**
 * Returns a non-empty handle if the deleter intercepts the request.
 * The return value is true if the property could be deleted and false
 * otherwise.
 */
typedef Handle<Boolean> (*NamedPropertyDeleter)(Local<String> property,
                                                const AccessorInfo& info);

/**
 * Returns an array containing the names of the properties the named
 * property getter intercepts.
 */
typedef Handle<Array> (*NamedPropertyEnumerator)(const AccessorInfo& info);


/**
 * Returns the value of the property if the getter intercepts the
 * request.  Otherwise, returns an empty handle.
 */
typedef Handle<Value> (*IndexedPropertyGetter)(uint32_t index,
                                               const AccessorInfo& info);


/**
 * Returns the value if the setter intercepts the request.
 * Otherwise, returns an empty handle.
 */
typedef Handle<Value> (*IndexedPropertySetter)(uint32_t index,
                                               Local<Value> value,
                                               const AccessorInfo& info);


/**
 * Returns a non-empty handle if the interceptor intercepts the request.
 * The result is true if the property exists and false otherwise.
 */
typedef Handle<Boolean> (*IndexedPropertyQuery)(uint32_t index,
                                                const AccessorInfo& info);

/**
 * Returns a non-empty handle if the deleter intercepts the request.
 * The return value is true if the property could be deleted and false
 * otherwise.
 */
typedef Handle<Boolean> (*IndexedPropertyDeleter)(uint32_t index,
                                                  const AccessorInfo& info);

/**
 * Returns an array containing the indices of the properties the
 * indexed property getter intercepts.
 */
typedef Handle<Array> (*IndexedPropertyEnumerator)(const AccessorInfo& info);


/**
 * Access control specifications.
 *
 * Some accessors should be accessible across contexts.  These
 * accessors have an explicit access control parameter which specifies
 * the kind of cross-context access that should be allowed.
 *
 * Additionally, for security, accessors can prohibit overwriting by
 * accessors defined in JavaScript.  For objects that have such
 * accessors either locally or in their prototype chain it is not
 * possible to overwrite the accessor by using __defineGetter__ or
 * __defineSetter__ from JavaScript code.
 */
enum AccessControl {
  DEFAULT               = 0,
  ALL_CAN_READ          = 1,
  ALL_CAN_WRITE         = 1 << 1,
  PROHIBITS_OVERWRITING = 1 << 2
};


/**
 * Access type specification.
 */
enum AccessType {
  ACCESS_GET,
  ACCESS_SET,
  ACCESS_HAS,
  ACCESS_DELETE,
  ACCESS_KEYS
};


/**
 * Returns true if cross-context access should be allowed to the named
 * property with the given key on the global object.
 */
typedef bool (*NamedSecurityCallback)(Local<Object> global,
                                      Local<Value> key,
                                      AccessType type,
                                      Local<Value> data);


/**
 * Returns true if cross-context access should be allowed to the indexed
 * property with the given index on the global object.
 */
typedef bool (*IndexedSecurityCallback)(Local<Object> global,
                                        uint32_t index,
                                        AccessType type,
                                        Local<Value> data);


/**
 * A FunctionTemplate is used to create functions at runtime. There
 * can only be one function created from a FunctionTemplate in a
 * context.
 *
 * A FunctionTemplate can have properties, these properties are added to the
 * function object when it is created.
 *
 * A FunctionTemplate has a corresponding instance template which is
 * used to create object instances when the function is used as a
 * constructor. Properties added to the instance template are added to
 * each object instance.
 *
 * A FunctionTemplate can have a prototype template. The prototype template
 * is used to create the prototype object of the function.
 *
 * The following example shows how to use a FunctionTemplate:
 *
 * \code
 *    v8::Local<v8::FunctionTemplate> t = v8::FunctionTemplate::New();
 *    t->Set("func_property", v8::Number::New(1));
 *
 *    v8::Local<v8::Template> proto_t = t->PrototypeTemplate();
 *    proto_t->Set("proto_method", v8::FunctionTemplate::New(InvokeCallback));
 *    proto_t->Set("proto_const", v8::Number::New(2));
 *
 *    v8::Local<v8::ObjectTemplate> instance_t = t->InstanceTemplate();
 *    instance_t->SetAccessor("instance_accessor", InstanceAccessorCallback);
 *    instance_t->SetNamedPropertyHandler(PropertyHandlerCallback, ...);
 *    instance_t->Set("instance_property", Number::New(3));
 *
 *    v8::Local<v8::Function> function = t->GetFunction();
 *    v8::Local<v8::Object> instance = function->NewInstance();
 * \endcode
 *
 * Let's use "function" as the JS variable name of the function object
 * and "instance" for the instance object created above.  The function
 * and the instance will have the following properties:
 *
 * \code
 *   func_property in function == true;
 *   function.func_property == 1;
 *
 *   function.prototype.proto_method() invokes 'InvokeCallback'
 *   function.prototype.proto_const == 2;
 *
 *   instance instanceof function == true;
 *   instance.instance_accessor calls 'InstanceAccessorCallback'
 *   instance.instance_property == 3;
 * \endcode
 *
 * A FunctionTemplate can inherit from another one by calling the
 * FunctionTemplate::Inherit method.  The following graph illustrates
 * the semantics of inheritance:
 *
 * \code
 *   FunctionTemplate Parent  -> Parent() . prototype -> { }
 *     ^                                                  ^
 *     | Inherit(Parent)                                  | .__proto__
 *     |                                                  |
 *   FunctionTemplate Child   -> Child()  . prototype -> { }
 * \endcode
 *
 * A FunctionTemplate 'Child' inherits from 'Parent', the prototype
 * object of the Child() function has __proto__ pointing to the
 * Parent() function's prototype object. An instance of the Child
 * function has all properties on Parent's instance templates.
 *
 * Let Parent be the FunctionTemplate initialized in the previous
 * section and create a Child FunctionTemplate by:
 *
 * \code
 *   Local<FunctionTemplate> parent = t;
 *   Local<FunctionTemplate> child = FunctionTemplate::New();
 *   child->Inherit(parent);
 *
 *   Local<Function> child_function = child->GetFunction();
 *   Local<Object> child_instance = child_function->NewInstance();
 * \endcode
 *
 * The Child function and Child instance will have the following
 * properties:
 *
 * \code
 *   child_func.prototype.__proto__ == function.prototype;
 *   child_instance.instance_accessor calls 'InstanceAccessorCallback'
 *   child_instance.instance_property == 3;
 * \endcode
 */
class EXPORT FunctionTemplate : public Template {
 public:
  /** Creates a function template.*/
  static Local<FunctionTemplate> New(
      InvocationCallback callback = 0,
      Handle<Value> data = Handle<Value>(),
      Handle<Signature> signature = Handle<Signature>());
  /** Returns the unique function instance in the current execution context.*/
  Local<Function> GetFunction();

  /**
   * Set the call-handler callback for a FunctionTemplate.  This
   * callback is called whenever the function created from this
   * FunctionTemplate is called.
   */
  void SetCallHandler(InvocationCallback callback,
                      Handle<Value> data = Handle<Value>());

  /** Get the InstanceTemplate. */
  Local<ObjectTemplate> InstanceTemplate();

  /** Causes the function template to inherit from a parent function template.*/
  void Inherit(Handle<FunctionTemplate> parent);

  /**
   * A PrototypeTemplate is the template used to create the prototype object
   * of the function created by this template.
   */
  Local<ObjectTemplate> PrototypeTemplate();


  /**
   * Set the class name of the FunctionTemplate.  This is used for
   * printing objects created with the function created from the
   * FunctionTemplate as its constructor.
   */
  void SetClassName(Handle<String> name);

  /**
   * Determines whether the __proto__ accessor ignores instances of
   * the function template.  If instances of the function template are
   * ignored, __proto__ skips all instances and instead returns the
   * next object in the prototype chain.
   *
   * Call with a value of true to make the __proto__ accessor ignore
   * instances of the function template.  Call with a value of false
   * to make the __proto__ accessor not ignore instances of the
   * function template.  By default, instances of a function template
   * are not ignored.
   */
  void SetHiddenPrototype(bool value);

  /**
   * Returns true if the given object is an instance of this function
   * template.
   */
  bool HasInstance(Handle<Value> object);

 private:
  FunctionTemplate();
  void AddInstancePropertyAccessor(Handle<String> name,
                                   AccessorGetter getter,
                                   AccessorSetter setter,
                                   Handle<Value> data,
                                   AccessControl settings,
                                   PropertyAttribute attributes);
  void SetNamedInstancePropertyHandler(NamedPropertyGetter getter,
                                       NamedPropertySetter setter,
                                       NamedPropertyQuery query,
                                       NamedPropertyDeleter remover,
                                       NamedPropertyEnumerator enumerator,
                                       Handle<Value> data);
  void SetIndexedInstancePropertyHandler(IndexedPropertyGetter getter,
                                         IndexedPropertySetter setter,
                                         IndexedPropertyQuery query,
                                         IndexedPropertyDeleter remover,
                                         IndexedPropertyEnumerator enumerator,
                                         Handle<Value> data);
  void SetInstanceCallAsFunctionHandler(InvocationCallback callback,
                                        Handle<Value> data);

  friend class Context;
  friend class ObjectTemplate;
};


/**
 * An ObjectTemplate is used to create objects at runtime.
 *
 * Properties added to an ObjectTemplate are added to each object
 * created from the ObjectTemplate.
 */
class EXPORT ObjectTemplate : public Template {
 public:
  /** Creates an ObjectTemplate. */
  static Local<ObjectTemplate> New();

  /** Creates a new instance of this template.*/
  Local<Object> NewInstance();

  /**
   * Sets an accessor on the object template.
   *
   * Whenever the property with the given name is accessed on objects
   * created from this ObjectTemplate the getter and setter callbacks
   * are called instead of getting and setting the property directly
   * on the JavaScript object.
   *
   * \param name The name of the property for which an accessor is added.
   * \param getter The callback to invoke when getting the property.
   * \param setter The callback to invoke when setting the property.
   * \param data A piece of data that will be passed to the getter and setter
   *   callbacks whenever they are invoked.
   * \param settings Access control settings for the accessor. This is a bit
   *   field consisting of one of more of
   *   DEFAULT = 0, ALL_CAN_READ = 1, or ALL_CAN_WRITE = 2.
   *   The default is to not allow cross-context access.
   *   ALL_CAN_READ means that all cross-context reads are allowed.
   *   ALL_CAN_WRITE means that all cross-context writes are allowed.
   *   The combination ALL_CAN_READ | ALL_CAN_WRITE can be used to allow all
   *   cross-context access.
   * \param attribute The attributes of the property for which an accessor
   *   is added.
   */
  void SetAccessor(Handle<String> name,
                   AccessorGetter getter,
                   AccessorSetter setter = 0,
                   Handle<Value> data = Handle<Value>(),
                   AccessControl settings = DEFAULT,
                   PropertyAttribute attribute = None);

  /**
   * Sets a named property handler on the object template.
   *
   * Whenever a named property is accessed on objects created from
   * this object template, the provided callback is invoked instead of
   * accessing the property directly on the JavaScript object.
   *
   * \param getter The callback to invoke when getting a property.
   * \param setter The callback to invoke when setting a property.
   * \param query The callback to invoke to check is an object has a property.
   * \param deleter The callback to invoke when deleting a property.
   * \param enumerator The callback to invoke to enumerate all the named
   *   properties of an object.
   * \param data A piece of data that will be passed to the callbacks
   *   whenever they are invoked.
   */
  void SetNamedPropertyHandler(NamedPropertyGetter getter,
                               NamedPropertySetter setter = 0,
                               NamedPropertyQuery query = 0,
                               NamedPropertyDeleter deleter = 0,
                               NamedPropertyEnumerator enumerator = 0,
                               Handle<Value> data = Handle<Value>());

  /**
   * Sets an indexed property handler on the object template.
   *
   * Whenever an indexed property is accessed on objects created from
   * this object template, the provided callback is invoked instead of
   * accessing the property directly on the JavaScript object.
   *
   * \param getter The callback to invoke when getting a property.
   * \param setter The callback to invoke when setting a property.
   * \param query The callback to invoke to check is an object has a property.
   * \param deleter The callback to invoke when deleting a property.
   * \param enumerator The callback to invoke to enumerate all the indexed
   *   properties of an object.
   * \param data A piece of data that will be passed to the callbacks
   *   whenever they are invoked.
   */
  void SetIndexedPropertyHandler(IndexedPropertyGetter getter,
                                 IndexedPropertySetter setter = 0,
                                 IndexedPropertyQuery query = 0,
                                 IndexedPropertyDeleter deleter = 0,
                                 IndexedPropertyEnumerator enumerator = 0,
                                 Handle<Value> data = Handle<Value>());
  /**
   * Sets the callback to be used when calling instances created from
   * this template as a function.  If no callback is set, instances
   * behave like normal JavaScript objects that cannot be called as a
   * function.
   */
  void SetCallAsFunctionHandler(InvocationCallback callback,
                                Handle<Value> data = Handle<Value>());

  /**
   * Mark object instances of the template as undetectable.
   *
   * In many ways, undetectable objects behave as though they are not
   * there.  They behave like 'undefined' in conditionals and when
   * printed.  However, properties can be accessed and called as on
   * normal objects.
   */
  void MarkAsUndetectable();

  /**
   * Sets access check callbacks on the object template.
   *
   * When accessing properties on instances of this object template,
   * the access check callback will be called to determine whether or
   * not to allow cross-context access to the properties.
   * The last parameter specifies whether access checks are turned
   * on by default on instances. If access checks are off by default,
   * they can be turned on on individual instances by calling
   * Object::TurnOnAccessCheck().
   */
  void SetAccessCheckCallbacks(NamedSecurityCallback named_handler,
                               IndexedSecurityCallback indexed_handler,
                               Handle<Value> data = Handle<Value>(),
                               bool turned_on_by_default = true);

  /**
   * Gets the number of internal fields for objects generated from
   * this template.
   */
  int InternalFieldCount();

  /**
   * Sets the number of internal fields for objects generated from
   * this template.
   */
  void SetInternalFieldCount(int value);

 private:
  ObjectTemplate();
  static Local<ObjectTemplate> New(Handle<FunctionTemplate> constructor);
  friend class FunctionTemplate;
};


/**
 * A Signature specifies which receivers and arguments a function can
 * legally be called with.
 */
class EXPORT Signature : public Data {
 public:
  static Local<Signature> New(Handle<FunctionTemplate> receiver =
                                  Handle<FunctionTemplate>(),
                              int argc = 0,
                              Handle<FunctionTemplate> argv[] = 0);
 private:
  Signature();
};


/**
 * A utility for determining the type of objects based on the template
 * they were constructed from.
 */
class EXPORT TypeSwitch : public Data {
 public:
  static Local<TypeSwitch> New(Handle<FunctionTemplate> type);
  static Local<TypeSwitch> New(int argc, Handle<FunctionTemplate> types[]);
  int match(Handle<Value> value);
 private:
  TypeSwitch();
};


// --- E x t e n s i o n s ---


/**
 * Ignore
 */
class EXPORT Extension {  // NOLINT
 public:
  Extension(const char* name,
            const char* source = 0,
            int dep_count = 0,
            const char** deps = 0);
  virtual ~Extension() { }
  virtual v8::Handle<v8::FunctionTemplate>
      GetNativeFunction(v8::Handle<v8::String> name) {
    return v8::Handle<v8::FunctionTemplate>();
  }

  const char* name() { return name_; }
  const char* source() { return source_; }
  int dependency_count() { return dep_count_; }
  const char** dependencies() { return deps_; }
  void set_auto_enable(bool value) { auto_enable_ = value; }
  bool auto_enable() { return auto_enable_; }

 private:
  const char* name_;
  const char* source_;
  int dep_count_;
  const char** deps_;
  bool auto_enable_;

  // Disallow copying and assigning.
  Extension(const Extension&);
  void operator=(const Extension&);
};


void EXPORT RegisterExtension(Extension* extension);


/**
 * Ignore
 */
class EXPORT DeclareExtension {
 public:
  inline DeclareExtension(Extension* extension) {
    RegisterExtension(extension);
  }
};


// --- S t a t i c s ---


Handle<Primitive> EXPORT Undefined();
Handle<Primitive> EXPORT Null();
Handle<Boolean> EXPORT True();
Handle<Boolean> EXPORT False();


/**
 * A set of constraints that specifies the limits of the runtime's
 * memory use.
 */
class EXPORT ResourceConstraints {
 public:
  ResourceConstraints();
  int max_young_space_size() const { return max_young_space_size_; }
  void set_max_young_space_size(int value) { max_young_space_size_ = value; }
  int max_old_space_size() const { return max_old_space_size_; }
  void set_max_old_space_size(int value) { max_old_space_size_ = value; }
  uint32_t* stack_limit() const { return stack_limit_; }
  void set_stack_limit(uint32_t* value) { stack_limit_ = value; }
 private:
  int max_young_space_size_;
  int max_old_space_size_;
  uint32_t* stack_limit_;
};


bool SetResourceConstraints(ResourceConstraints* constraints);


// --- E x c e p t i o n s ---


typedef void (*FatalErrorCallback)(const char* location, const char* message);


typedef void (*MessageCallback)(Handle<Message> message, Handle<Value> data);


/**
 * Schedules an exception to be thrown when returning to JavaScript.  When an
 * exception has been scheduled it is illegal to invoke any JavaScript
 * operation; the caller must return immediately and only after the exception
 * has been handled does it become legal to invoke JavaScript operations.
 */
Handle<Value> EXPORT ThrowException(Handle<Value> exception);

/**
 * Create new error objects by calling the corresponding error object
 * constructor with the message.
 */
class EXPORT Exception {
 public:
  static Local<Value> RangeError(Handle<String> message);
  static Local<Value> ReferenceError(Handle<String> message);
  static Local<Value> SyntaxError(Handle<String> message);
  static Local<Value> TypeError(Handle<String> message);
  static Local<Value> Error(Handle<String> message);
};


// --- C o u n t e r s  C a l l b a c k s ---

typedef int* (*CounterLookupCallback)(const char* name);

// --- F a i l e d A c c e s s C h e c k C a l l b a c k ---
typedef void (*FailedAccessCheckCallback)(Local<Object> target,
                                          AccessType type,
                                          Local<Value> data);

// --- G a r b a g e C o l l e c t i o n  C a l l b a c k s

/**
 * Applications can register a callback function which is called
 * before and after a major garbage collection.  Allocations are not
 * allowed in the callback function, you therefore cannot manipulate
 * objects (set or delete properties for example) since it is possible
 * such operations will result in the allocation of objects.
 */
typedef void (*GCCallback)();


// --- E x t e r n a l  S y m b o l  C a l l b a c k ---

/**
 * Callback used to allocate certain V8 symbols as external strings.
 *
 * The data passed to the callback is utf8 encoded.
 *
 * Allocations are not allowed in the callback function, you therefore
 * cannot manipulate objects (set or delete properties for example)
 * since it is possible such operations will result in the allocation
 * of objects.
 */
typedef String::ExternalStringResource* (*ExternalSymbolCallback)(
    const char* utf8,
    size_t length);


// --- C o n t e x t  G e n e r a t o r ---

/**
 * Applications must provide a callback function which is called to generate
 * a context if a context was not deserialized from the snapshot.
 */
typedef Persistent<Context> (*ContextGenerator)();


/**
 * Container class for static utility functions.
 */
class EXPORT V8 {
 public:
  /** Set the callback to invoke in case of fatal errors. */
  static void SetFatalErrorHandler(FatalErrorCallback that);

  /**
   * Ignore out-of-memory exceptions.
   *
   * V8 running out of memory is treated as a fatal error by default.
   * This means that the fatal error handler is called and that V8 is
   * terminated.
   *
   * IgnoreOutOfMemoryException can be used to not treat a
   * out-of-memory situation as a fatal error.  This way, the contexts
   * that did not cause the out of memory problem might be able to
   * continue execution.
   */
  static void IgnoreOutOfMemoryException();

  /**
   * Check if V8 is dead and therefore unusable.  This is the case after
   * fatal errors such as out-of-memory situations.
   */
  static bool IsDead();

  /**
   * Adds a message listener.
   *
   * The same message listener can be added more than once and it that
   * case it will be called more than once for each message.
   */
  static bool AddMessageListener(MessageCallback that,
                                 Handle<Value> data = Handle<Value>());

  /**
   * Remove all message listeners from the specified callback function.
   */
  static void RemoveMessageListeners(MessageCallback that);

  /**
   * Sets V8 flags from a string.
   */
  static void SetFlagsFromString(const char* str, int length);

  /**
   * Sets V8 flags from the command line.
   */
  static void SetFlagsFromCommandLine(int* argc,
                                      char** argv,
                                      bool remove_flags);

  /** Get the version string. */
  static const char* GetVersion();

  /**
   * Enables the host application to provide a mechanism for recording
   * statistics counters.
   */
  static void SetCounterFunction(CounterLookupCallback);

  /**
   * Enables the computation of a sliding window of states. The sliding
   * window information is recorded in statistics counters.
   */
  static void EnableSlidingStateWindow();

  /** Callback function for reporting failed access checks.*/
  static void SetFailedAccessCheckCallbackFunction(FailedAccessCheckCallback);

  /**
   * Enables the host application to receive a notification before a
   * major garbage colletion.  Allocations are not allowed in the
   * callback function, you therefore cannot manipulate objects (set
   * or delete properties for example) since it is possible such
   * operations will result in the allocation of objects.
   */
  static void SetGlobalGCPrologueCallback(GCCallback);

  /**
   * Enables the host application to receive a notification after a
   * major garbage collection.  Allocations are not allowed in the
   * callback function, you therefore cannot manipulate objects (set
   * or delete properties for example) since it is possible such
   * operations will result in the allocation of objects.
   */
  static void SetGlobalGCEpilogueCallback(GCCallback);

  /**
   * Applications can register a callback that will be used when
   * allocating most of the V8 symbols.  The callback must return an
   * external string resource that represents the symbols.
   *
   * Most often when performing a property lookup the key will be a
   * symbol.  Allocating symbols as external strings can reduce the
   * amount of string conversions needed when using interceptors and
   * accessors.
   *
   * \note This is an experimental feature and it might be removed.
   */
  static void SetExternalSymbolCallback(ExternalSymbolCallback);

  /**
   * Allows the host application to group objects together. If one
   * object in the group is alive, all objects in the group are alive.
   * After each garbage collection, object groups are removed. It is
   * intended to be used in the before-garbage-collection callback
   * function, for instance to simulate DOM tree connections among JS
   * wrapper objects.
   */
  static void AddObjectGroup(Persistent<Value>* objects, size_t length);

  /**
   * Initializes from snapshot if possible. Otherwise, attempts to
   * initialize from scratch.
   */
  static bool Initialize();

  /**
   * Adjusts the amount of registered external memory.  Used to give
   * V8 an indication of the amount of externally allocated memory
   * that is kept alive by JavaScript objects.  V8 uses this to decide
   * when to perform global garbage collections.  Registering
   * externally allocated memory will trigger global garbage
   * collections more often than otherwise in an attempt to garbage
   * collect the JavaScript objects keeping the externally allocated
   * memory alive.
   *
   * \param change_in_bytes the change in externally allocated memory
   *   that is kept alive by JavaScript objects.
   * \returns the adjusted value.
   */
  static int AdjustAmountOfExternalAllocatedMemory(int change_in_bytes);

 private:
  V8();

  static void** GlobalizeReference(void** handle);
  static void DisposeGlobal(void** global_handle);
  static void MakeWeak(void** global_handle, void* data, WeakReferenceCallback);
  static void ClearWeak(void** global_handle);
  static bool IsGlobalNearDeath(void** global_handle);
  static bool IsGlobalWeak(void** global_handle);

  template <class T> friend class Handle;
  template <class T> friend class Local;
  template <class T> friend class Persistent;
  friend class Context;
};


/**
 * An external exception handler.
 */
class EXPORT TryCatch {
 public:

  /**
   * Creates a new try/catch block and registers it with v8.
   */
  TryCatch();

  /**
   * Unregisters and deletes this try/catch block.
   */
  ~TryCatch();

  /**
   * Returns true if an exception has been caught by this try/catch block.
   */
  bool HasCaught() const;

  /**
   * Returns the exception caught by this try/catch block.  If no exception has
   * been caught an empty handle is returned.
   *
   * The returned handle is valid until this TryCatch block has been destroyed.
   */
  Local<Value> Exception() const;

  /**
   * Returns the message associated with this exception.  If there is
   * no message associated an empty handle is returned.
   *
   * The returned handle is valid until this TryCatch block has been
   * destroyed.
   */
  Local<v8::Message> Message() const;

  /**
   * Clears any exceptions that may have been caught by this try/catch block.
   * After this method has been called, HasCaught() will return false.
   *
   * It is not necessary to clear a try/catch block before using it again; if
   * another exception is thrown the previously caught exception will just be
   * overwritten.  However, it is often a good idea since it makes it easier
   * to determine which operation threw a given exception.
   */
  void Reset();

  /**
   * Set verbosity of the external exception handler.
   *
   * By default, exceptions that are caught by an external exception
   * handler are not reported.  Call SetVerbose with true on an
   * external exception handler to have exceptions caught by the
   * handler reported as if they were not caught.
   */
  void SetVerbose(bool value);

  /**
   * Set whether or not this TryCatch should capture a Message object
   * which holds source information about where the exception
   * occurred.  True by default.
   */
  void SetCaptureMessage(bool value);

 public:
  TryCatch* next_;
  void* exception_;
  void* message_;
  bool is_verbose_;
  bool capture_message_;
  void* js_handler_;
};


// --- C o n t e x t ---


/**
 * Ignore
 */
class EXPORT ExtensionConfiguration {
 public:
  ExtensionConfiguration(int name_count, const char* names[])
      : name_count_(name_count), names_(names) { }
 private:
  friend class ImplementationUtilities;
  int name_count_;
  const char** names_;
};


/**
 * A sandboxed execution context with its own set of built-in objects
 * and functions.
 */
class EXPORT Context {
 public:
  /** Returns the global object of the context. */
  Local<Object> Global();

  /**
   * Detaches the global object from its context before
   * the global object can be reused to create a new context.
   */
  void DetachGlobal();

  /** Creates a new context. */
  static Persistent<Context> New(
      ExtensionConfiguration* extensions = 0,
      Handle<ObjectTemplate> global_template = Handle<ObjectTemplate>(),
      Handle<Value> global_object = Handle<Value>());

  /** Returns the last entered context. */
  static Local<Context> GetEntered();

  /** Returns the context that is on the top of the stack. */
  static Local<Context> GetCurrent();

  /**
   * Sets the security token for the context.  To access an object in
   * another context, the security tokens must match.
   */
  void SetSecurityToken(Handle<Value> token);

  /** Restores the security token to the default value. */
  void UseDefaultSecurityToken();

  /** Returns the security token of this context.*/
  Handle<Value> GetSecurityToken();

  /**
   * Enter this context.  After entering a context, all code compiled
   * and run is compiled and run in this context.  If another context
   * is already entered, this old context is saved so it can be
   * restored when the new context is exited.
   */
  void Enter();

  /**
   * Exit this context.  Exiting the current context restores the
   * context that was in place when entering the current context.
   */
  void Exit();

  /** Returns true if the context has experienced an out of memory situation. */
  bool HasOutOfMemoryException();

  /** Returns true if V8 has a current context. */
  static bool InContext();

  /**
   * Stack-allocated class which sets the execution context for all
   * operations executed within a local scope.
   */
  class EXPORT Scope {
   public:
    inline Scope(Handle<Context> context) : context_(context) {
      context_->Enter();
    }
    inline ~Scope() { context_->Exit(); }
   private:
    Handle<Context> context_;
  };

 private:
  friend class Value;
  friend class Script;
  friend class Object;
  friend class Function;
};


/**
 * Multiple threads in V8 are allowed, but only one thread at a time
 * is allowed to use V8.  The definition of 'using V8' includes
 * accessing handles or holding onto object pointers obtained from V8
 * handles.  It is up to the user of V8 to ensure (perhaps with
 * locking) that this constraint is not violated.
 *
 * If you wish to start using V8 in a thread you can do this by constructing
 * a v8::Locker object.  After the code using V8 has completed for the
 * current thread you can call the destructor.  This can be combined
 * with C++ scope-based construction as follows:
 *
 * \code
 * ...
 * {
 *   v8::Locker locker;
 *   ...
 *   // Code using V8 goes here.
 *   ...
 * } // Destructor called here
 * \endcode
 *
 * If you wish to stop using V8 in a thread A you can do this by either
 * by destroying the v8::Locker object as above or by constructing a
 * v8::Unlocker object:
 *
 * \code
 * {
 *   v8::Unlocker unlocker;
 *   ...
 *   // Code not using V8 goes here while V8 can run in another thread.
 *   ...
 * } // Destructor called here.
 * \endcode
 *
 * The Unlocker object is intended for use in a long-running callback
 * from V8, where you want to release the V8 lock for other threads to
 * use.
 *
 * The v8::Locker is a recursive lock.  That is, you can lock more than
 * once in a given thread.  This can be useful if you have code that can
 * be called either from code that holds the lock or from code that does
 * not.  The Unlocker is not recursive so you can not have several
 * Unlockers on the stack at once, and you can not use an Unlocker in a
 * thread that is not inside a Locker's scope.
 *
 * An unlocker will unlock several lockers if it has to and reinstate
 * the correct depth of locking on its destruction. eg.:
 *
 * \code
 * // V8 not locked.
 * {
 *   v8::Locker locker;
 *   // V8 locked.
 *   {
 *     v8::Locker another_locker;
 *     // V8 still locked (2 levels).
 *     {
 *       v8::Unlocker unlocker;
 *       // V8 not locked.
 *     }
 *     // V8 locked again (2 levels).
 *   }
 *   // V8 still locked (1 level).
 * }
 * // V8 Now no longer locked.
 * \endcode
 */
class EXPORT Unlocker {
 public:
  Unlocker();
  ~Unlocker();
};


class EXPORT Locker {
 public:
  Locker();
  ~Locker();

  /**
   * Start preemption.
   *
   * When preemption is started, a timer is fired every n milli seconds
   * that will switch between multiple threads that are in contention
   * for the V8 lock.
   */
  static void StartPreemption(int every_n_ms);

  /**
   * Stop preemption.
   */
  static void StopPreemption();

  /**
   * Returns whether or not the locker is locked by the current thread.
   */
  static bool IsLocked();

  /**
   * Returns whether v8::Locker is being used by this V8 instance.
   */
  static bool IsActive() { return active_; }

 private:
  bool has_lock_;
  bool top_level_;

  static bool active_;

  // Disallow copying and assigning.
  Locker(const Locker&);
  void operator=(const Locker&);
};



// --- I m p l e m e n t a t i o n ---

template <class T>
Handle<T>::Handle() : val_(0) { }


template <class T>
Local<T>::Local() : Handle<T>() { }


template <class T>
Local<T> Local<T>::New(Handle<T> that) {
  if (that.IsEmpty()) return Local<T>();
  void** p = reinterpret_cast<void**>(*that);
  return Local<T>(reinterpret_cast<T*>(HandleScope::CreateHandle(*p)));
}


template <class T>
Persistent<T> Persistent<T>::New(Handle<T> that) {
  if (that.IsEmpty()) return Persistent<T>();
  void** p = reinterpret_cast<void**>(*that);
  return Persistent<T>(reinterpret_cast<T*>(V8::GlobalizeReference(p)));
}


template <class T>
bool Persistent<T>::IsNearDeath() const {
  if (this->IsEmpty()) return false;
  return V8::IsGlobalNearDeath(reinterpret_cast<void**>(**this));
}


template <class T>
bool Persistent<T>::IsWeak() const {
  if (this->IsEmpty()) return false;
  return V8::IsGlobalWeak(reinterpret_cast<void**>(**this));
}


template <class T>
void Persistent<T>::Dispose() {
  if (this->IsEmpty()) return;
  V8::DisposeGlobal(reinterpret_cast<void**>(**this));
}


template <class T>
Persistent<T>::Persistent() : Handle<T>() { }

template <class T>
void Persistent<T>::MakeWeak(void* parameters, WeakReferenceCallback callback) {
  V8::MakeWeak(reinterpret_cast<void**>(**this), parameters, callback);
}

template <class T>
void Persistent<T>::ClearWeak() {
  V8::ClearWeak(reinterpret_cast<void**>(**this));
}

template <class T>
T* Handle<T>::operator->() const {
  return val_;
}


template <class T>
T* Handle<T>::operator*() const {
  return val_;
}


Local<Value> Arguments::operator[](int i) const {
  if (i < 0 || length_ <= i) return Local<Value>(*Undefined());
  return Local<Value>(reinterpret_cast<Value*>(values_ - i));
}


Local<Function> Arguments::Callee() const {
  return callee_;
}


Local<Object> Arguments::This() const {
  return Local<Object>(reinterpret_cast<Object*>(values_ + 1));
}


Local<Object> Arguments::Holder() const {
  return holder_;
}


Local<Value> Arguments::Data() const {
  return data_;
}


bool Arguments::IsConstructCall() const {
  return is_construct_call_;
}


int Arguments::Length() const {
  return length_;
}


Local<Value> AccessorInfo::Data() const {
  return data_;
}


Local<Object> AccessorInfo::This() const {
  return self_;
}


Local<Object> AccessorInfo::Holder() const {
  return holder_;
}


template <class T>
Local<T> HandleScope::Close(Handle<T> value) {
  void** after = RawClose(reinterpret_cast<void**>(*value));
  return Local<T>(reinterpret_cast<T*>(after));
}

Handle<Value> ScriptOrigin::ResourceName() const {
  return resource_name_;
}


Handle<Integer> ScriptOrigin::ResourceLineOffset() const {
  return resource_line_offset_;
}


Handle<Integer> ScriptOrigin::ResourceColumnOffset() const {
  return resource_column_offset_;
}


Handle<Boolean> Boolean::New(bool value) {
  return value ? True() : False();
}


void Template::Set(const char* name, v8::Handle<Data> value) {
  Set(v8::String::New(name), value);
}


/**
 * \example shell.cc
 * A simple shell that takes a list of expressions on the
 * command-line and executes them.
 */


/**
 * \example process.cc
 */


}  // namespace v8


#undef EXPORT
#undef EXPORT_INLINE
#undef TYPE_CHECK


#endif  // V8_H_
