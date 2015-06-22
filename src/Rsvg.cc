
#include "Rsvg.h"
#include "RsvgCairo.h"
#include "Enums.h"
#include <node_buffer.h>
#include <cairo-pdf.h>
#include <cairo-svg.h>
#include <string>
#include <cmath>

#ifdef _MSC_VER
#include "win32-math.h"
#endif

using namespace v8;
using namespace node;

cairo_status_t GetDataChunks(void* closure, const unsigned char* chunk, unsigned int length) {
	std::string* data = reinterpret_cast<std::string*>(closure);
	data->append(reinterpret_cast<const char *>(chunk), length);
	return CAIRO_STATUS_SUCCESS;
}

Persistent<Function> Rsvg::constructor;

Rsvg::Rsvg(RsvgHandle* const handle) : _handle(handle) {}

Rsvg::~Rsvg() {
	g_object_unref(G_OBJECT(_handle));
}

void Rsvg::Init(Handle<Object> exports) {

#if !GLIB_CHECK_VERSION(2, 36, 0)
	// Initialize GObject types.
	g_type_init();
#endif

	// Prepare constructor template.
	Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(New);
	tpl->SetClassName(NanNew<String>("Rsvg"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);
	// Add methods to prototype.
        NODE_SET_PROTOTYPE_METHOD(tpl, "getBaseURI", GetBaseURI);
        NODE_SET_PROTOTYPE_METHOD(tpl, "setBaseURI", SetBaseURI);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getDPI", GetDPI);
        NODE_SET_PROTOTYPE_METHOD(tpl, "setDPI", SetDPI);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getDPIX", GetDPIX);
        NODE_SET_PROTOTYPE_METHOD(tpl, "setDPIX", SetDPIX);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getDPIY", GetDPIY);
        NODE_SET_PROTOTYPE_METHOD(tpl, "setDPIY", SetDPIY);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getWidth", GetWidth);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getHeight", GetHeight);
        NODE_SET_PROTOTYPE_METHOD(tpl, "write", Write);
        NODE_SET_PROTOTYPE_METHOD(tpl, "close", Close);
        NODE_SET_PROTOTYPE_METHOD(tpl, "dimensions", Dimensions);
        NODE_SET_PROTOTYPE_METHOD(tpl, "hasElement", HasElement);
        NODE_SET_PROTOTYPE_METHOD(tpl, "autocrop", Autocrop);
        NODE_SET_PROTOTYPE_METHOD(tpl, "render", Render);
	// Export class.
        Local<Function> tplFunc = tpl->GetFunction();
	NanAssignPersistent(constructor, tplFunc);
	exports->Set(NanNew("Rsvg"), tplFunc);
}

NAN_METHOD(Rsvg::New) {
	NanScope();

	if (args.IsConstructCall()) {
		// Invoked as constructor: `new Rsvg(...)`
		RsvgHandle* handle;
		if (Buffer::HasInstance(args[0])) {
			const guint8* buffer =
				reinterpret_cast<guint8*>(Buffer::Data(args[0]));
			gsize length = Buffer::Length(args[0]);

			GError* error = NULL;
			handle = rsvg_handle_new_from_data(buffer, length, &error);

			if (error) {
				NanThrowError(error->message);
				g_error_free(error);
                                NanReturnUndefined();
			}
		} else {
			handle = rsvg_handle_new();
		}
		// Error handling.
		if (!handle) {
			NanThrowError("Unable to create RsvgHandle instance.");
			NanReturnUndefined();
		}
		// Create object.
		Rsvg* obj = new Rsvg(handle);
		obj->Wrap(args.This());
		NanReturnValue(args.This());
	} else {
		// Invoked as plain function `Rsvg(...)`, turn into construct call.
		const int argc = 1;
		Local<Value> argv[argc] = { args[0] };
		NanReturnValue(NanNew<Function>(constructor)->NewInstance(argc, argv));
	}
}

NAN_METHOD(Rsvg::GetBaseURI) {
        NanScope();
	NanReturnValue(GetStringProperty("base-uri", args));
}

NAN_METHOD(Rsvg::SetBaseURI) {
	NanScope();
	SetStringProperty("base-uri", args);
	NanReturnUndefined();
}

NAN_METHOD(Rsvg::GetDPI) {
	NanScope();
	Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(args.This());
	gdouble dpiX = 0;
	gdouble dpiY = 0;
	g_object_get(
		G_OBJECT(obj->_handle),
		"dpi-x", &dpiX,
		"dpi-y", &dpiY,
		NULL
	);

	Handle<ObjectTemplate> dpi = NanNew<ObjectTemplate>();
	dpi->Set(NanNew("x"), NanNew<Number>(dpiX));
	dpi->Set(NanNew("y"), NanNew<Number>(dpiY));

	NanReturnValue(dpi->NewInstance());
}

NAN_METHOD(Rsvg::SetDPI) {
	NanScope();
	Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(args.This());

	gdouble x = args[0]->NumberValue();
	if (std::isnan(x)) {
		x = 0;
	}

	gdouble y = x;
	if (args[1]->IsNumber()) {
		y = args[1]->NumberValue();
		if (std::isnan(y)) {
			y = 0;
		}
	}

	rsvg_handle_set_dpi_x_y(obj->_handle, x, y);
	NanReturnUndefined();
}

NAN_METHOD(Rsvg::GetDPIX) {
        NanScope();
	NanReturnValue(GetNumberProperty("dpi-x", args));
}

NAN_METHOD(Rsvg::SetDPIX) {
	NanScope();
	SetNumberProperty("dpi-x", args);
	NanReturnUndefined();
}

NAN_METHOD(Rsvg::GetDPIY) {
        NanScope();
	NanReturnValue(GetNumberProperty("dpi-y", args));
}

NAN_METHOD(Rsvg::SetDPIY) {
	NanScope();
	SetNumberProperty("dpi-y", args);
	NanReturnUndefined();
}

NAN_METHOD(Rsvg::GetWidth) {
        NanScope();
	NanReturnValue(GetIntegerProperty("width", args));
}

NAN_METHOD(Rsvg::GetHeight) {
        NanScope();
	NanReturnValue(GetIntegerProperty("height", args));
}

NAN_METHOD(Rsvg::Write) {
	NanScope();
	Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(args.This());
	if (Buffer::HasInstance(args[0])) {
		const guchar* buffer =
			reinterpret_cast<guchar*>(Buffer::Data(args[0]));
		gsize length = Buffer::Length(args[0]);

		GError* error = NULL;
		gboolean success = rsvg_handle_write(obj->_handle, buffer, length, &error);

		if (error) {
			NanThrowError(error->message);
			g_error_free(error);
		} else if (!success) {
			NanThrowError("Failed to write data.");
		}
	} else {
		NanThrowError("Invalid argument: buffer");
        }
        NanReturnUndefined();
}

NAN_METHOD(Rsvg::Close) {
	NanScope();
	Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(args.This());

	GError* error = NULL;
	gboolean success = rsvg_handle_close(obj->_handle, &error);

	if (error) {
		NanThrowError(error->message);
		g_error_free(error);
	} else if (!success) {
		NanThrowError("Failed to close.");
	}
	NanReturnUndefined();
}

NAN_METHOD(Rsvg::Dimensions) {
	NanScope();
	Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(args.This());

	const char* id = NULL;
	String::Utf8Value idArg(args[0]);
	if (!(args[0]->IsUndefined() || args[0]->IsNull())) {
		id = *idArg;
		if (!id) {
			NanThrowError("Invalid argument: id");
                        NanReturnUndefined();
		}
	}

	RsvgPositionData _position = { 0, 0 };
	RsvgDimensionData _dimensions = { 0, 0, 0, 0 };

	gboolean hasPosition = rsvg_handle_get_position_sub(obj->_handle, &_position, id);
	gboolean hasDimensions = rsvg_handle_get_dimensions_sub(obj->_handle, &_dimensions, id);

	if (hasPosition || hasDimensions) {
		Handle<ObjectTemplate> dimensions = NanNew<ObjectTemplate>();
		if (hasPosition) {
			dimensions->Set(NanNew("x"), NanNew<Integer>(_position.x));
			dimensions->Set(NanNew("y"), NanNew<Integer>(_position.y));
		}
		if (hasDimensions) {
			dimensions->Set(NanNew("width"), NanNew<Integer>(_dimensions.width));
			dimensions->Set(NanNew("height"), NanNew<Integer>(_dimensions.height));
		}
		NanReturnValue(dimensions->NewInstance());
	} else {
		NanReturnNull();
	}
}

NAN_METHOD(Rsvg::HasElement) {
	NanScope();
	Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(args.This());

	const char* id = NULL;
	String::Utf8Value idArg(args[0]);
	if (!(args[0]->IsUndefined() || args[0]->IsNull())) {
		id = *idArg;
		if (!id) {
			NanThrowError("Invalid argument: id");
                        NanReturnUndefined();
		}
	}

	gboolean exists = rsvg_handle_has_sub(obj->_handle, id);
	NanReturnValue(NanNew<Boolean>(exists));
}

NAN_METHOD(Rsvg::Render) {
	NanScope();
	Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(args.This());

	int width = args[0]->Int32Value();
	int height = args[1]->Int32Value();

	if (width <= 0) {
		NanThrowError("Expected width > 0.");
                NanReturnUndefined();
	}
	if (height <= 0) {
		NanThrowError("Expected height > 0.");
		NanReturnUndefined();
	}

	String::Utf8Value formatArg(args[2]);
	const char* formatString = *formatArg;
	render_format_t renderFormat = RenderFormatFromString(formatString);
	cairo_format_t pixelFormat = CAIRO_FORMAT_INVALID;
	if (renderFormat == RENDER_FORMAT_RAW ||
			renderFormat == RENDER_FORMAT_PNG) {
		pixelFormat = CAIRO_FORMAT_ARGB32;
	} else if (renderFormat == RENDER_FORMAT_JPEG) {
		NanThrowError("Format not supported: JPEG");
                NanReturnUndefined();
	} else if (
			renderFormat == RENDER_FORMAT_SVG ||
			renderFormat == RENDER_FORMAT_PDF) {
		pixelFormat = CAIRO_FORMAT_INVALID;
	} else if (renderFormat == RENDER_FORMAT_VIPS) {
		NanThrowError("Format not supported: VIPS");
                NanReturnUndefined();
	} else {
		renderFormat = RENDER_FORMAT_RAW;
		pixelFormat = CairoFormatFromString(formatString);
		if (pixelFormat == CAIRO_FORMAT_INVALID) {
			NanThrowError("Invalid argument: format");
                        NanReturnUndefined();
		}
	}

	const char* id = NULL;
	String::Utf8Value idArg(args[3]);
	if (!(args[3]->IsUndefined() || args[3]->IsNull())) {
		id = *idArg;
		if (!id) {
			NanThrowError("Invalid argument: id");
                        NanReturnUndefined();
		}
		if (!rsvg_handle_has_sub(obj->_handle, id)) {
			NanThrowError("SVG element with given id does not exists.");
                        NanReturnUndefined();
		}
	}

	RsvgPositionData position = { 0, 0 };
	RsvgDimensionData dimensions = { 0, 0, 0, 0 };

	if (!rsvg_handle_get_position_sub(obj->_handle, &position, id)) {
		NanThrowError("Could not get position of SVG element with given id.");
                NanReturnUndefined();
	}

	if (!rsvg_handle_get_dimensions_sub(obj->_handle, &dimensions, id)) {
		NanThrowError("Could not get dimensions of SVG element or whole image.");
                NanReturnUndefined();
	}
	if (dimensions.width <= 0 || dimensions.height <= 0) {
		NanThrowError("Got invalid dimensions of SVG element or whole image.");
                NanReturnUndefined();
	}

	std::string data;
	cairo_surface_t* surface;

	if (renderFormat == RENDER_FORMAT_SVG) {
		surface = cairo_svg_surface_create_for_stream(GetDataChunks, &data, width, height);
		cairo_svg_surface_restrict_to_version(surface, CAIRO_SVG_VERSION_1_1);
	} else if (renderFormat == RENDER_FORMAT_PDF) {
		surface = cairo_pdf_surface_create_for_stream(GetDataChunks, &data, width, height);
#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 10, 0)
		cairo_pdf_surface_restrict_to_version(surface, CAIRO_PDF_VERSION_1_4);
#endif
	} else {
		surface = cairo_image_surface_create(pixelFormat, width, height);
	}

	cairo_t* cr = cairo_create(surface);
	// printf(
	// 	"%s: (%d, %d) %dx%d, render: %dx%d\n",
	// 	id ? id : "SVG",
	// 	position.x,
	// 	position.y,
	// 	dimensions.width,
	// 	dimensions.height,
	// 	width,
	// 	height
	// );
	double scaleX = double(width) / double(dimensions.width);
	double scaleY = double(height) / double(dimensions.height);
	// printf("scale=%.3fx%.3f\n", scaleX, scaleY);
	double scale = MIN(scaleX, scaleY);
	int bboxWidth = round(scale * dimensions.width);
	int bboxHeight = round(scale * dimensions.height);
	int bboxX = (width - bboxWidth) / 2;
	int bboxY = (height - bboxHeight) / 2;
	// printf("bbox=(%d, %d) %dx%d\n", bboxX, bboxY, bboxWidth, bboxHeight);
	cairo_translate(cr, bboxX, bboxY);
	cairo_scale(cr, scale, scale);
	cairo_translate(cr, -position.x, -position.y);

	gboolean success;
	if (id) {
		success = rsvg_handle_render_cairo_sub(obj->_handle, cr, id);
	} else {
		success = rsvg_handle_render_cairo(obj->_handle, cr);
	}
	cairo_surface_flush(surface);

	cairo_status_t status = cairo_status(cr);
	if (status || !success) {
		cairo_destroy(cr);
		cairo_surface_destroy(surface);

		NanThrowError(
			status ? cairo_status_to_string(status) : "Failed to render image."
		);
                NanReturnUndefined();
	}

	int stride = -1;
	if (renderFormat == RENDER_FORMAT_RAW) {
		stride = cairo_image_surface_get_stride(surface);
		data.append(
			reinterpret_cast<char*>(cairo_image_surface_get_data(surface)),
			stride * height
		);
	} else if (renderFormat == RENDER_FORMAT_PNG) {
		cairo_surface_write_to_png_stream(surface, GetDataChunks, &data);
	}

	cairo_destroy(cr);
	cairo_surface_destroy(surface);

	if (renderFormat == RENDER_FORMAT_RAW &&
			pixelFormat == CAIRO_FORMAT_ARGB32 &&
			stride != width * 4) {
		NanThrowError(
			"Rendered with invalid stride (byte size of row) for ARGB32 format."
		);
                NanReturnUndefined();
	}

	Handle<ObjectTemplate> image = ObjectTemplate::New();
	if (renderFormat == RENDER_FORMAT_SVG) {
		image->Set(NanNew("data"), NanNew<String>(data.c_str()));
	} else {
		image->Set(NanNew("data"), NanNewBufferHandle(data.c_str(), data.length()));
	}

	image->Set(NanNew("format"), RenderFormatToString(renderFormat));
	if (pixelFormat != CAIRO_FORMAT_INVALID) {
		image->Set(NanNew("pixelFormat"), CairoFormatToString(pixelFormat));
	}
	image->Set(NanNew("width"), NanNew<Integer>(width));
	image->Set(NanNew("height"), NanNew<Integer>(height));
	if (stride != -1) {
		image->Set(NanNew("stride"), NanNew<Integer>(stride));
	}
	NanReturnValue(image->NewInstance());
}

Handle<Value> Rsvg::GetStringProperty (const char* property, const ARGTYPE& args) {
	NanScope();
	Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(args.This());
	gchar* value = NULL;
	g_object_get(G_OBJECT(obj->_handle), property, &value, NULL);
	Handle<Value> result = NanNew<String>(value);
	if (value) {
		g_free(value);
	        return result;
	}
        return NanNull();
}

void Rsvg::SetStringProperty (const char* property, const ARGTYPE& args) {
        NanScope();
	Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(args.This());
	gchar* value = NULL;
	String::Utf8Value arg0(args[0]);
	if (!(args[0]->IsNull() || args[0]->IsUndefined())) {
		value = *arg0;
	}
	g_object_set(G_OBJECT(obj->_handle), property, value, NULL);
}

Handle<Value> Rsvg::GetNumberProperty (const char* property, const ARGTYPE& args) {
        NanScope();
	Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(args.This());
	gdouble value = 0;
	g_object_get(G_OBJECT(obj->_handle), property, &value, NULL);
	return NanNew<Number>(value);
}

void Rsvg::SetNumberProperty (const char* property, const ARGTYPE& args) {
        NanScope();
	Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(args.This());
	gdouble value = args[0]->NumberValue();
	if (std::isnan(value)) {
		value = 0;
	}
	g_object_set(G_OBJECT(obj->_handle), property, value, NULL);
}

Handle<Value> Rsvg::GetIntegerProperty (const char* property, const ARGTYPE& args) {
        NanScope();
	Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(args.This());
	gint value = 0;
	g_object_get(G_OBJECT(obj->_handle), property, &value, NULL);
	return NanNew<Integer>(value);
}

void Rsvg::SetIntegerProperty (const char* property, const ARGTYPE& args) {
        NanScope();
	Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(args.This());
	gint value = args[0]->Int32Value();
	g_object_set(G_OBJECT(obj->_handle), property, value, NULL);
}

NODE_MODULE(rsvg, Rsvg::Init)
