#ifndef __RSVG_H__
#define __RSVG_H__

#include <node.h>
#include <node_version.h>
#include <node_object_wrap.h>
#include <librsvg/rsvg.h>
#include "nan.h"

#if NODE_VERSION_AT_LEAST(0, 11, 3)
    #define ARGTYPE v8::FunctionCallbackInfo<v8::Value>
#else
    #define ARGTYPE v8::Arguments
#endif

class Rsvg : public node::ObjectWrap {
public:
	static void Init(v8::Handle<v8::Object> exports);

private:
	explicit Rsvg(RsvgHandle* const handle);
	~Rsvg();

	static NAN_METHOD(New);
	static NAN_METHOD(GetBaseURI);
	static NAN_METHOD(SetBaseURI);
	static NAN_METHOD(GetDPI);
	static NAN_METHOD(SetDPI);
	static NAN_METHOD(GetDPIX);
	static NAN_METHOD(SetDPIX);
	static NAN_METHOD(GetDPIY);
	static NAN_METHOD(SetDPIY);
	static NAN_METHOD(GetWidth);
	static NAN_METHOD(GetHeight);
	static NAN_METHOD(Write);
	static NAN_METHOD(Close);
	static NAN_METHOD(Dimensions);
	static NAN_METHOD(HasElement);
	static NAN_METHOD(Autocrop);
	static NAN_METHOD(Render);
        static v8::Handle<v8::Value> GetStringProperty(const char* property, const ARGTYPE& args);
        static void SetStringProperty(const char* property, const ARGTYPE& args);
        static v8::Handle<v8::Value> GetNumberProperty(const char* property, const ARGTYPE& args);
        static void SetNumberProperty(const char* property, const ARGTYPE& args);
        static v8::Handle<v8::Value> GetIntegerProperty(const char* property, const ARGTYPE& args);
        static void SetIntegerProperty(const char* property, const ARGTYPE& args);
	static v8::Persistent<v8::Function> constructor;
	RsvgHandle* const _handle;
};

#endif /*__RSVG_H__*/
