/* node-lzf (C) 2011 Ian Babrou <ibobrik@gmail.com>  */

#include <stdlib.h>

#ifdef __APPLE__
#include <malloc/malloc.h>
#endif

#include "nan.h"

#include "lzf/lzf.h"


using namespace v8;
using namespace node;
using namespace Nan;


// Handle<Value> ThrowNodeError(const char* what = NULL) {
//     return NanThrowError(Exception::Error(NanNew<String>(what)));
// }
NAN_METHOD(compress) {
    if (info.Length() < 1 || !Buffer::HasInstance(info[0])) {
        ThrowError("First argument must be a Buffer");
        return;
    }

    Local<Object> bufferIn = info[0]->ToObject();
    size_t bytesIn         = Buffer::Length(bufferIn);
    char * dataPointer     = Buffer::Data(bufferIn);
    size_t bytesCompressed = bytesIn + 100;
    char * bufferOut       = (char*) malloc(bytesCompressed);

    if (!bufferOut) {
        ThrowError("LZF malloc failed!");
        return;
    }

    unsigned result = lzf_compress(dataPointer, bytesIn, bufferOut, bytesCompressed);

    if (!result) {
        free(bufferOut);
        ThrowError("Compression failed, probably too small buffer");
        return;
    }

    Nan::MaybeLocal<Object> BufferOut = Nan::NewBuffer(bufferOut, result);
    info.GetReturnValue().Set(BufferOut.ToLocalChecked());
}


NAN_METHOD(decompress) {
    if (info.Length() < 1 || !Buffer::HasInstance(info[0])) {
        Nan::ThrowError("First argument must be a Buffer");
        return;
    }

    Local<Object> bufferIn = info[0]->ToObject();

    size_t bytesUncompressed = 999 * 1024 * 1024; // it's about max size that V8 supports

    if (info.Length() > 1 && info[1]->IsNumber()) { // accept dest buffer size
        bytesUncompressed = info[1]->Uint32Value();
    }


    char * bufferOut = (char*) malloc(bytesUncompressed);
    if (!bufferOut) {
        Nan::ThrowError("LZF malloc failed!");
        return;
    }

    unsigned result = lzf_decompress(Buffer::Data(bufferIn), Buffer::Length(bufferIn), bufferOut, bytesUncompressed);

    if (!result) {
        Nan::ThrowError("Unrompression failed, probably too small buffer");
        return;
    }

    Nan::MaybeLocal<Object> BufferOut = Nan::NewBuffer(bufferOut, result);
    info.GetReturnValue().Set(BufferOut.ToLocalChecked());
}

extern "C" void
init (Handle<Object> target) {
  Nan::SetMethod(target, "compress",    compress);
  Nan::SetMethod(target, "decompress",  decompress);
}

NODE_MODULE(lzf, init)
