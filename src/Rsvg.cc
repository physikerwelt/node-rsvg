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
    Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
    tpl->SetClassName(Nan::New<String>("Rsvg").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    // Add methods to prototype.
    Nan::SetPrototypeMethod(tpl, "getBaseURI", GetBaseURI);
    Nan::SetPrototypeMethod(tpl, "getBaseURI", GetBaseURI);
    Nan::SetPrototypeMethod(tpl, "setBaseURI", SetBaseURI);
    Nan::SetPrototypeMethod(tpl, "getDPI", GetDPI);
    Nan::SetPrototypeMethod(tpl, "setDPI", SetDPI);
    Nan::SetPrototypeMethod(tpl, "getDPIX", GetDPIX);
    Nan::SetPrototypeMethod(tpl, "setDPIX", SetDPIX);
    Nan::SetPrototypeMethod(tpl, "getDPIY", GetDPIY);
    Nan::SetPrototypeMethod(tpl, "setDPIY", SetDPIY);
    Nan::SetPrototypeMethod(tpl, "getWidth", GetWidth);
    Nan::SetPrototypeMethod(tpl, "getHeight", GetHeight);
    Nan::SetPrototypeMethod(tpl, "write", Write);
    Nan::SetPrototypeMethod(tpl, "close", Close);
    Nan::SetPrototypeMethod(tpl, "dimensions", Dimensions);
    Nan::SetPrototypeMethod(tpl, "hasElement", HasElement);
    Nan::SetPrototypeMethod(tpl, "autocrop", Autocrop);
    Nan::SetPrototypeMethod(tpl, "render", Render);
    // Export class.
    Local<Function> tplFunc = tpl->GetFunction();
    constructor.Reset(v8::Isolate::GetCurrent(), tplFunc); // TODO: кишки изолятов по идее не должны торчать наружу, т.к. скорее всего несовместимо с ранними версиями ноды
    exports->Set(Nan::New("Rsvg").ToLocalChecked(), tplFunc);
}

NAN_METHOD(Rsvg::New) {
    Nan::HandleScope scope;

    if (info.IsConstructCall()) {
        // Invoked as constructor: `new Rsvg(...)`
        RsvgHandle* handle;
        if (Buffer::HasInstance(info[0])) {
            const guint8* buffer =
                reinterpret_cast<guint8*>(Buffer::Data(info[0]));
            gsize length = Buffer::Length(info[0]);

            GError* error = NULL;
            handle = rsvg_handle_new_from_data(buffer, length, &error);

            if (error) {
                Nan::ThrowError(error->message);
                g_error_free(error);
                info.GetReturnValue().Set(Nan::Undefined());
            }
        } else {
            handle = rsvg_handle_new();
        }
        // Error handling.
        if (!handle) {
            Nan::ThrowError("Unable to create RsvgHandle instance.");
            info.GetReturnValue().Set(Nan::Undefined());
        }
        // Create object.
        Rsvg* obj = new Rsvg(handle);
        obj->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
    } else {
        // Invoked as plain function `Rsvg(...)`, turn into construct call.
        const int argc = 1;
        Local<Value> argv[argc] = { info[0] };
        info.GetReturnValue().Set(Nan::New<Function>(constructor)->NewInstance(argc, argv));
    }
}

NAN_METHOD(Rsvg::GetBaseURI) {
    Nan::HandleScope scope;
    info.GetReturnValue().Set(GetStringProperty("base-uri", info));
}

NAN_METHOD(Rsvg::SetBaseURI) {
    Nan::HandleScope scope;
    SetStringProperty("base-uri", info);
    info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Rsvg::GetDPI) {
    Nan::HandleScope scope;
    Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(info.This());
    gdouble dpiX = 0;
    gdouble dpiY = 0;
    g_object_get(
            G_OBJECT(obj->_handle),
            "dpi-x", &dpiX,
            "dpi-y", &dpiY,
            NULL
            );

    Handle<ObjectTemplate> dpi = Nan::New<ObjectTemplate>();
    dpi->Set(Nan::New("x").ToLocalChecked(), Nan::New<Number>(dpiX));
    dpi->Set(Nan::New("y").ToLocalChecked(), Nan::New<Number>(dpiY));

    info.GetReturnValue().Set(dpi->NewInstance());
}

NAN_METHOD(Rsvg::SetDPI) {
    Nan::HandleScope scope;
    Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(info.This());

    gdouble x = info[0]->NumberValue();
    if (std::isnan(x)) {
        x = 0;
    }

    gdouble y = x;
    if (info[1]->IsNumber()) {
        y = info[1]->NumberValue();
        if (std::isnan(y)) {
            y = 0;
        }
    }

    rsvg_handle_set_dpi_x_y(obj->_handle, x, y);
    info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Rsvg::GetDPIX) {
    Nan::HandleScope scope;
    info.GetReturnValue().Set(GetNumberProperty("dpi-x", info));
}

NAN_METHOD(Rsvg::SetDPIX) {
    Nan::HandleScope scope;
    SetNumberProperty("dpi-x", info);
    info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Rsvg::GetDPIY) {
    Nan::HandleScope scope;
    info.GetReturnValue().Set(GetNumberProperty("dpi-y", info));
}

NAN_METHOD(Rsvg::SetDPIY) {
    Nan::HandleScope scope;
    SetNumberProperty("dpi-y", info);
    info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Rsvg::GetWidth) {
    Nan::HandleScope scope;
    info.GetReturnValue().Set(GetIntegerProperty("width", info));
}

NAN_METHOD(Rsvg::GetHeight) {
    Nan::HandleScope scope;
    info.GetReturnValue().Set(GetIntegerProperty("height", info));
}

NAN_METHOD(Rsvg::Write) {
    Nan::HandleScope scope;
    Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(info.This());
    if (Buffer::HasInstance(info[0])) {
        const guchar* buffer =
            reinterpret_cast<guchar*>(Buffer::Data(info[0]));
        gsize length = Buffer::Length(info[0]);

        GError* error = NULL;
        gboolean success = rsvg_handle_write(obj->_handle, buffer, length, &error);

        if (error) {
            Nan::ThrowError(error->message);
            g_error_free(error);
        } else if (!success) {
            Nan::ThrowError("Failed to write data.");
        }
    } else {
        Nan::ThrowError("Invalid argument: buffer");
    }
    info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Rsvg::Close) {
    Nan::HandleScope scope;
    Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(info.This());

    GError* error = NULL;
    gboolean success = rsvg_handle_close(obj->_handle, &error);

    if (error) {
        Nan::ThrowError(error->message);
        g_error_free(error);
    } else if (!success) {
        Nan::ThrowError("Failed to close.");
    }
    info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(Rsvg::Dimensions) {
    Nan::HandleScope scope;
    Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(info.This());

    const char* id = NULL;
    String::Utf8Value idArg(info[0]);
    if (!(info[0]->IsUndefined() || info[0]->IsNull())) {
        id = *idArg;
        if (!id) {
            Nan::ThrowError("Invalid argument: id");
            info.GetReturnValue().Set(Nan::Undefined());
        }
    }

    RsvgPositionData _position = { 0, 0 };
    RsvgDimensionData _dimensions = { 0, 0, 0, 0 };

    gboolean hasPosition = rsvg_handle_get_position_sub(obj->_handle, &_position, id);
    gboolean hasDimensions = rsvg_handle_get_dimensions_sub(obj->_handle, &_dimensions, id);

    if (hasPosition || hasDimensions) {
        Handle<ObjectTemplate> dimensions = Nan::New<ObjectTemplate>();
        if (hasPosition) {
            dimensions->Set(Nan::New("x").ToLocalChecked(), Nan::New<Integer>(_position.x));
            dimensions->Set(Nan::New("y").ToLocalChecked(), Nan::New<Integer>(_position.y));
        }
        if (hasDimensions) {
            dimensions->Set(Nan::New("width").ToLocalChecked(), Nan::New<Integer>(_dimensions.width));
            dimensions->Set(Nan::New("height").ToLocalChecked(), Nan::New<Integer>(_dimensions.height));
        }
        info.GetReturnValue().Set(dimensions->NewInstance());
    } else {
        info.GetReturnValue().Set(Nan::Null());
    }
}

NAN_METHOD(Rsvg::HasElement) {
    Nan::HandleScope scope;
    Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(info.This());

    const char* id = NULL;
    String::Utf8Value idArg(info[0]);
    if (!(info[0]->IsUndefined() || info[0]->IsNull())) {
        id = *idArg;
        if (!id) {
            Nan::ThrowError("Invalid argument: id");
            info.GetReturnValue().Set(Nan::Undefined());
        }
    }

    gboolean exists = rsvg_handle_has_sub(obj->_handle, id);
    info.GetReturnValue().Set(Nan::New<Boolean>(exists));
}

NAN_METHOD(Rsvg::Render) {
    Nan::HandleScope scope;
    Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(info.This());

    int width = info[0]->Int32Value();
    int height = info[1]->Int32Value();

    if (width <= 0) {
        Nan::ThrowError("Expected width > 0.");
        info.GetReturnValue().Set(Nan::Undefined());
    }
    if (height <= 0) {
        Nan::ThrowError("Expected height > 0.");
        info.GetReturnValue().Set(Nan::Undefined());
    }

    String::Utf8Value formatArg(info[2]);
    const char* formatString = *formatArg;
    render_format_t renderFormat = RenderFormatFromString(formatString);
    cairo_format_t pixelFormat = CAIRO_FORMAT_INVALID;
    if (renderFormat == RENDER_FORMAT_RAW ||
            renderFormat == RENDER_FORMAT_PNG) {
        pixelFormat = CAIRO_FORMAT_ARGB32;
    } else if (renderFormat == RENDER_FORMAT_JPEG) {
        Nan::ThrowError("Format not supported: JPEG");
        info.GetReturnValue().Set(Nan::Undefined());
    } else if (
            renderFormat == RENDER_FORMAT_SVG ||
            renderFormat == RENDER_FORMAT_PDF) {
        pixelFormat = CAIRO_FORMAT_INVALID;
    } else if (renderFormat == RENDER_FORMAT_VIPS) {
        Nan::ThrowError("Format not supported: VIPS");
        info.GetReturnValue().Set(Nan::Undefined());
    } else {
        renderFormat = RENDER_FORMAT_RAW;
        pixelFormat = CairoFormatFromString(formatString);
        if (pixelFormat == CAIRO_FORMAT_INVALID) {
            Nan::ThrowError("Invalid argument: format");
            info.GetReturnValue().Set(Nan::Undefined());
        }
    }

    const char* id = NULL;
    String::Utf8Value idArg(info[3]);
    if (!(info[3]->IsUndefined() || info[3]->IsNull())) {
        id = *idArg;
        if (!id) {
            Nan::ThrowError("Invalid argument: id");
            info.GetReturnValue().Set(Nan::Undefined());
        }
        if (!rsvg_handle_has_sub(obj->_handle, id)) {
            Nan::ThrowError("SVG element with given id does not exists.");
            info.GetReturnValue().Set(Nan::Undefined());
        }
    }

    RsvgPositionData position = { 0, 0 };
    RsvgDimensionData dimensions = { 0, 0, 0, 0 };

    if (!rsvg_handle_get_position_sub(obj->_handle, &position, id)) {
        Nan::ThrowError("Could not get position of SVG element with given id.");
        info.GetReturnValue().Set(Nan::Undefined());
    }

    if (!rsvg_handle_get_dimensions_sub(obj->_handle, &dimensions, id)) {
        Nan::ThrowError("Could not get dimensions of SVG element or whole image.");
        info.GetReturnValue().Set(Nan::Undefined());
    }
    if (dimensions.width <= 0 || dimensions.height <= 0) {
        Nan::ThrowError("Got invalid dimensions of SVG element or whole image.");
        info.GetReturnValue().Set(Nan::Undefined());
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

        Nan::ThrowError(
                status ? cairo_status_to_string(status) : "Failed to render image."
                );
        info.GetReturnValue().Set(Nan::Undefined());
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
        Nan::ThrowError(
                "Rendered with invalid stride (byte size of row) for ARGB32 format."
                );
        info.GetReturnValue().Set(Nan::Undefined());
    }

    Handle<ObjectTemplate> image = ObjectTemplate::New();
    if (renderFormat == RENDER_FORMAT_SVG) {
        image->Set(Nan::New("data").ToLocalChecked(), Nan::New<String>(data.c_str()).ToLocalChecked());
    } else {
        image->Set(Nan::New("data").ToLocalChecked(), Nan::NewBuffer(const_cast<char*>(data.c_str()), data.length()).ToLocalChecked());
    }

    image->Set(Nan::New("format").ToLocalChecked(), RenderFormatToString(renderFormat));
    if (pixelFormat != CAIRO_FORMAT_INVALID) {
        image->Set(Nan::New("pixelFormat").ToLocalChecked(), CairoFormatToString(pixelFormat));
    }
    image->Set(Nan::New("width").ToLocalChecked(), Nan::New<Integer>(width));
    image->Set(Nan::New("height").ToLocalChecked(), Nan::New<Integer>(height));
    if (stride != -1) {
        image->Set(Nan::New("stride").ToLocalChecked(), Nan::New<Integer>(stride));
    }
    info.GetReturnValue().Set(image->NewInstance());
}

Handle<Value> Rsvg::GetStringProperty (const char* property, const ARGTYPE& info) {
    Nan::HandleScope scope;
    Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(info.This());
    gchar* value = NULL;
    g_object_get(G_OBJECT(obj->_handle), property, &value, NULL);
    Handle<Value> result = Nan::New<String>(value).ToLocalChecked();
    if (value) {
        g_free(value);
        return result;
    }
    return Nan::Null();
}

void Rsvg::SetStringProperty (const char* property, const ARGTYPE& info) {
    Nan::HandleScope scope;
    Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(info.This());
    gchar* value = NULL;
    String::Utf8Value arg0(info[0]);
    if (!(info[0]->IsNull() || info[0]->IsUndefined())) {
        value = *arg0;
    }
    g_object_set(G_OBJECT(obj->_handle), property, value, NULL);
}

Handle<Value> Rsvg::GetNumberProperty (const char* property, const ARGTYPE& info) {
    Nan::HandleScope scope;
    Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(info.This());
    gdouble value = 0;
    g_object_get(G_OBJECT(obj->_handle), property, &value, NULL);
    return Nan::New<Number>(value);
}

void Rsvg::SetNumberProperty (const char* property, const ARGTYPE& info) {
    Nan::HandleScope scope;
    Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(info.This());
    gdouble value = info[0]->NumberValue();
    if (std::isnan(value)) {
        value = 0;
    }
    g_object_set(G_OBJECT(obj->_handle), property, value, NULL);
}

Handle<Value> Rsvg::GetIntegerProperty (const char* property, const ARGTYPE& info) {
    Nan::HandleScope scope;
    Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(info.This());
    gint value = 0;
    g_object_get(G_OBJECT(obj->_handle), property, &value, NULL);
    return Nan::New<Integer>(value);
}

void Rsvg::SetIntegerProperty (const char* property, const ARGTYPE& info) {
    Nan::HandleScope scope;
    Rsvg* obj = ObjectWrap::Unwrap<Rsvg>(info.This());
    gint value = info[0]->Int32Value();
    g_object_set(G_OBJECT(obj->_handle), property, value, NULL);
}

NODE_MODULE(rsvg, Rsvg::Init)
