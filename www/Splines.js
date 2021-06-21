var Module = typeof Module !== "undefined" ? Module : {};

var moduleOverrides = {};

var key;

for (key in Module) {
 if (Module.hasOwnProperty(key)) {
  moduleOverrides[key] = Module[key];
 }
}

var arguments_ = [];

var thisProgram = "./this.program";

var quit_ = function(status, toThrow) {
 throw toThrow;
};

var ENVIRONMENT_IS_WEB = false;

var ENVIRONMENT_IS_WORKER = false;

var ENVIRONMENT_IS_NODE = false;

var ENVIRONMENT_IS_SHELL = false;

ENVIRONMENT_IS_WEB = typeof window === "object";

ENVIRONMENT_IS_WORKER = typeof importScripts === "function";

ENVIRONMENT_IS_NODE = typeof process === "object" && typeof process.versions === "object" && typeof process.versions.node === "string";

ENVIRONMENT_IS_SHELL = !ENVIRONMENT_IS_WEB && !ENVIRONMENT_IS_NODE && !ENVIRONMENT_IS_WORKER;

var scriptDirectory = "";

function locateFile(path) {
 if (Module["locateFile"]) {
  return Module["locateFile"](path, scriptDirectory);
 }
 return scriptDirectory + path;
}

var read_, readAsync, readBinary, setWindowTitle;

var nodeFS;

var nodePath;

if (ENVIRONMENT_IS_NODE) {
 if (ENVIRONMENT_IS_WORKER) {
  scriptDirectory = require("path").dirname(scriptDirectory) + "/";
 } else {
  scriptDirectory = __dirname + "/";
 }
 read_ = function shell_read(filename, binary) {
  if (!nodeFS) nodeFS = require("fs");
  if (!nodePath) nodePath = require("path");
  filename = nodePath["normalize"](filename);
  return nodeFS["readFileSync"](filename, binary ? null : "utf8");
 };
 readBinary = function readBinary(filename) {
  var ret = read_(filename, true);
  if (!ret.buffer) {
   ret = new Uint8Array(ret);
  }
  assert(ret.buffer);
  return ret;
 };
 if (process["argv"].length > 1) {
  thisProgram = process["argv"][1].replace(/\\/g, "/");
 }
 arguments_ = process["argv"].slice(2);
 if (typeof module !== "undefined") {
  module["exports"] = Module;
 }
 process["on"]("uncaughtException", function(ex) {
  if (!(ex instanceof ExitStatus)) {
   throw ex;
  }
 });
 process["on"]("unhandledRejection", abort);
 quit_ = function(status) {
  process["exit"](status);
 };
 Module["inspect"] = function() {
  return "[Emscripten Module object]";
 };
} else if (ENVIRONMENT_IS_SHELL) {
 if (typeof read != "undefined") {
  read_ = function shell_read(f) {
   return read(f);
  };
 }
 readBinary = function readBinary(f) {
  var data;
  if (typeof readbuffer === "function") {
   return new Uint8Array(readbuffer(f));
  }
  data = read(f, "binary");
  assert(typeof data === "object");
  return data;
 };
 if (typeof scriptArgs != "undefined") {
  arguments_ = scriptArgs;
 } else if (typeof arguments != "undefined") {
  arguments_ = arguments;
 }
 if (typeof quit === "function") {
  quit_ = function(status) {
   quit(status);
  };
 }
 if (typeof print !== "undefined") {
  if (typeof console === "undefined") console = {};
  console.log = print;
  console.warn = console.error = typeof printErr !== "undefined" ? printErr : print;
 }
} else if (ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER) {
 if (ENVIRONMENT_IS_WORKER) {
  scriptDirectory = self.location.href;
 } else if (typeof document !== "undefined" && document.currentScript) {
  scriptDirectory = document.currentScript.src;
 }
 if (scriptDirectory.indexOf("blob:") !== 0) {
  scriptDirectory = scriptDirectory.substr(0, scriptDirectory.lastIndexOf("/") + 1);
 } else {
  scriptDirectory = "";
 }
 {
  read_ = function(url) {
   var xhr = new XMLHttpRequest();
   xhr.open("GET", url, false);
   xhr.send(null);
   return xhr.responseText;
  };
  if (ENVIRONMENT_IS_WORKER) {
   readBinary = function(url) {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", url, false);
    xhr.responseType = "arraybuffer";
    xhr.send(null);
    return new Uint8Array(xhr.response);
   };
  }
  readAsync = function(url, onload, onerror) {
   var xhr = new XMLHttpRequest();
   xhr.open("GET", url, true);
   xhr.responseType = "arraybuffer";
   xhr.onload = function() {
    if (xhr.status == 200 || xhr.status == 0 && xhr.response) {
     onload(xhr.response);
     return;
    }
    onerror();
   };
   xhr.onerror = onerror;
   xhr.send(null);
  };
 }
 setWindowTitle = function(title) {
  document.title = title;
 };
} else {}

var out = Module["print"] || console.log.bind(console);

var err = Module["printErr"] || console.warn.bind(console);

for (key in moduleOverrides) {
 if (moduleOverrides.hasOwnProperty(key)) {
  Module[key] = moduleOverrides[key];
 }
}

moduleOverrides = null;

if (Module["arguments"]) arguments_ = Module["arguments"];

if (Module["thisProgram"]) thisProgram = Module["thisProgram"];

if (Module["quit"]) quit_ = Module["quit"];

function warnOnce(text) {
 if (!warnOnce.shown) warnOnce.shown = {};
 if (!warnOnce.shown[text]) {
  warnOnce.shown[text] = 1;
  err(text);
 }
}

var wasmBinary;

if (Module["wasmBinary"]) wasmBinary = Module["wasmBinary"];

var noExitRuntime = Module["noExitRuntime"] || true;

if (typeof WebAssembly !== "object") {
 abort("no native wasm support detected");
}

var wasmMemory;

var ABORT = false;

var EXITSTATUS;

function assert(condition, text) {
 if (!condition) {
  abort("Assertion failed: " + text);
 }
}

var UTF8Decoder = typeof TextDecoder !== "undefined" ? new TextDecoder("utf8") : undefined;

function UTF8ArrayToString(heap, idx, maxBytesToRead) {
 var endIdx = idx + maxBytesToRead;
 var endPtr = idx;
 while (heap[endPtr] && !(endPtr >= endIdx)) ++endPtr;
 if (endPtr - idx > 16 && heap.subarray && UTF8Decoder) {
  return UTF8Decoder.decode(heap.subarray(idx, endPtr));
 } else {
  var str = "";
  while (idx < endPtr) {
   var u0 = heap[idx++];
   if (!(u0 & 128)) {
    str += String.fromCharCode(u0);
    continue;
   }
   var u1 = heap[idx++] & 63;
   if ((u0 & 224) == 192) {
    str += String.fromCharCode((u0 & 31) << 6 | u1);
    continue;
   }
   var u2 = heap[idx++] & 63;
   if ((u0 & 240) == 224) {
    u0 = (u0 & 15) << 12 | u1 << 6 | u2;
   } else {
    u0 = (u0 & 7) << 18 | u1 << 12 | u2 << 6 | heap[idx++] & 63;
   }
   if (u0 < 65536) {
    str += String.fromCharCode(u0);
   } else {
    var ch = u0 - 65536;
    str += String.fromCharCode(55296 | ch >> 10, 56320 | ch & 1023);
   }
  }
 }
 return str;
}

function UTF8ToString(ptr, maxBytesToRead) {
 return ptr ? UTF8ArrayToString(HEAPU8, ptr, maxBytesToRead) : "";
}

function stringToUTF8Array(str, heap, outIdx, maxBytesToWrite) {
 if (!(maxBytesToWrite > 0)) return 0;
 var startIdx = outIdx;
 var endIdx = outIdx + maxBytesToWrite - 1;
 for (var i = 0; i < str.length; ++i) {
  var u = str.charCodeAt(i);
  if (u >= 55296 && u <= 57343) {
   var u1 = str.charCodeAt(++i);
   u = 65536 + ((u & 1023) << 10) | u1 & 1023;
  }
  if (u <= 127) {
   if (outIdx >= endIdx) break;
   heap[outIdx++] = u;
  } else if (u <= 2047) {
   if (outIdx + 1 >= endIdx) break;
   heap[outIdx++] = 192 | u >> 6;
   heap[outIdx++] = 128 | u & 63;
  } else if (u <= 65535) {
   if (outIdx + 2 >= endIdx) break;
   heap[outIdx++] = 224 | u >> 12;
   heap[outIdx++] = 128 | u >> 6 & 63;
   heap[outIdx++] = 128 | u & 63;
  } else {
   if (outIdx + 3 >= endIdx) break;
   heap[outIdx++] = 240 | u >> 18;
   heap[outIdx++] = 128 | u >> 12 & 63;
   heap[outIdx++] = 128 | u >> 6 & 63;
   heap[outIdx++] = 128 | u & 63;
  }
 }
 heap[outIdx] = 0;
 return outIdx - startIdx;
}

function stringToUTF8(str, outPtr, maxBytesToWrite) {
 return stringToUTF8Array(str, HEAPU8, outPtr, maxBytesToWrite);
}

function lengthBytesUTF8(str) {
 var len = 0;
 for (var i = 0; i < str.length; ++i) {
  var u = str.charCodeAt(i);
  if (u >= 55296 && u <= 57343) u = 65536 + ((u & 1023) << 10) | str.charCodeAt(++i) & 1023;
  if (u <= 127) ++len; else if (u <= 2047) len += 2; else if (u <= 65535) len += 3; else len += 4;
 }
 return len;
}

function allocateUTF8OnStack(str) {
 var size = lengthBytesUTF8(str) + 1;
 var ret = stackAlloc(size);
 stringToUTF8Array(str, HEAP8, ret, size);
 return ret;
}

var buffer, HEAP8, HEAPU8, HEAP16, HEAPU16, HEAP32, HEAPU32, HEAPF32, HEAPF64;

function updateGlobalBufferAndViews(buf) {
 buffer = buf;
 Module["HEAP8"] = HEAP8 = new Int8Array(buf);
 Module["HEAP16"] = HEAP16 = new Int16Array(buf);
 Module["HEAP32"] = HEAP32 = new Int32Array(buf);
 Module["HEAPU8"] = HEAPU8 = new Uint8Array(buf);
 Module["HEAPU16"] = HEAPU16 = new Uint16Array(buf);
 Module["HEAPU32"] = HEAPU32 = new Uint32Array(buf);
 Module["HEAPF32"] = HEAPF32 = new Float32Array(buf);
 Module["HEAPF64"] = HEAPF64 = new Float64Array(buf);
}

var INITIAL_MEMORY = Module["INITIAL_MEMORY"] || 16777216;

var wasmTable;

var __ATPRERUN__ = [];

var __ATINIT__ = [];

var __ATMAIN__ = [];

var __ATEXIT__ = [];

var __ATPOSTRUN__ = [];

var runtimeInitialized = false;

var runtimeExited = false;

function preRun() {
 if (Module["preRun"]) {
  if (typeof Module["preRun"] == "function") Module["preRun"] = [ Module["preRun"] ];
  while (Module["preRun"].length) {
   addOnPreRun(Module["preRun"].shift());
  }
 }
 callRuntimeCallbacks(__ATPRERUN__);
}

function initRuntime() {
 runtimeInitialized = true;
 callRuntimeCallbacks(__ATINIT__);
}

function preMain() {
 callRuntimeCallbacks(__ATMAIN__);
}

function exitRuntime() {
 runtimeExited = true;
}

function postRun() {
 if (Module["postRun"]) {
  if (typeof Module["postRun"] == "function") Module["postRun"] = [ Module["postRun"] ];
  while (Module["postRun"].length) {
   addOnPostRun(Module["postRun"].shift());
  }
 }
 callRuntimeCallbacks(__ATPOSTRUN__);
}

function addOnPreRun(cb) {
 __ATPRERUN__.unshift(cb);
}

function addOnInit(cb) {
 __ATINIT__.unshift(cb);
}

function addOnPostRun(cb) {
 __ATPOSTRUN__.unshift(cb);
}

var runDependencies = 0;

var runDependencyWatcher = null;

var dependenciesFulfilled = null;

function getUniqueRunDependency(id) {
 return id;
}

function addRunDependency(id) {
 runDependencies++;
 if (Module["monitorRunDependencies"]) {
  Module["monitorRunDependencies"](runDependencies);
 }
}

function removeRunDependency(id) {
 runDependencies--;
 if (Module["monitorRunDependencies"]) {
  Module["monitorRunDependencies"](runDependencies);
 }
 if (runDependencies == 0) {
  if (runDependencyWatcher !== null) {
   clearInterval(runDependencyWatcher);
   runDependencyWatcher = null;
  }
  if (dependenciesFulfilled) {
   var callback = dependenciesFulfilled;
   dependenciesFulfilled = null;
   callback();
  }
 }
}

Module["preloadedImages"] = {};

Module["preloadedAudios"] = {};

function abort(what) {
 if (Module["onAbort"]) {
  Module["onAbort"](what);
 }
 what += "";
 err(what);
 ABORT = true;
 EXITSTATUS = 1;
 what = "abort(" + what + "). Build with -s ASSERTIONS=1 for more info.";
 var e = new WebAssembly.RuntimeError(what);
 throw e;
}

var dataURIPrefix = "data:application/octet-stream;base64,";

function isDataURI(filename) {
 return filename.startsWith(dataURIPrefix);
}

function isFileURI(filename) {
 return filename.startsWith("file://");
}

var wasmBinaryFile = "Splines.wasm";

if (!isDataURI(wasmBinaryFile)) {
 wasmBinaryFile = locateFile(wasmBinaryFile);
}

function getBinary(file) {
 try {
  if (file == wasmBinaryFile && wasmBinary) {
   return new Uint8Array(wasmBinary);
  }
  if (readBinary) {
   return readBinary(file);
  } else {
   throw "both async and sync fetching of the wasm failed";
  }
 } catch (err) {
  abort(err);
 }
}

function getBinaryPromise() {
 if (!wasmBinary && (ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER)) {
  if (typeof fetch === "function" && !isFileURI(wasmBinaryFile)) {
   return fetch(wasmBinaryFile, {
    credentials: "same-origin"
   }).then(function(response) {
    if (!response["ok"]) {
     throw "failed to load wasm binary file at '" + wasmBinaryFile + "'";
    }
    return response["arrayBuffer"]();
   }).catch(function() {
    return getBinary(wasmBinaryFile);
   });
  } else {
   if (readAsync) {
    return new Promise(function(resolve, reject) {
     readAsync(wasmBinaryFile, function(response) {
      resolve(new Uint8Array(response));
     }, reject);
    });
   }
  }
 }
 return Promise.resolve().then(function() {
  return getBinary(wasmBinaryFile);
 });
}

function createWasm() {
 var info = {
  "a": asmLibraryArg
 };
 function receiveInstance(instance, module) {
  var exports = instance.exports;
  Module["asm"] = exports;
  wasmMemory = Module["asm"]["C"];
  updateGlobalBufferAndViews(wasmMemory.buffer);
  wasmTable = Module["asm"]["H"];
  addOnInit(Module["asm"]["D"]);
  removeRunDependency("wasm-instantiate");
 }
 addRunDependency("wasm-instantiate");
 function receiveInstantiationResult(result) {
  receiveInstance(result["instance"]);
 }
 function instantiateArrayBuffer(receiver) {
  return getBinaryPromise().then(function(binary) {
   var result = WebAssembly.instantiate(binary, info);
   return result;
  }).then(receiver, function(reason) {
   err("failed to asynchronously prepare wasm: " + reason);
   abort(reason);
  });
 }
 function instantiateAsync() {
  if (!wasmBinary && typeof WebAssembly.instantiateStreaming === "function" && !isDataURI(wasmBinaryFile) && !isFileURI(wasmBinaryFile) && typeof fetch === "function") {
   return fetch(wasmBinaryFile, {
    credentials: "same-origin"
   }).then(function(response) {
    var result = WebAssembly.instantiateStreaming(response, info);
    return result.then(receiveInstantiationResult, function(reason) {
     err("wasm streaming compile failed: " + reason);
     err("falling back to ArrayBuffer instantiation");
     return instantiateArrayBuffer(receiveInstantiationResult);
    });
   });
  } else {
   return instantiateArrayBuffer(receiveInstantiationResult);
  }
 }
 if (Module["instantiateWasm"]) {
  try {
   var exports = Module["instantiateWasm"](info, receiveInstance);
   return exports;
  } catch (e) {
   err("Module.instantiateWasm callback failed with error: " + e);
   return false;
  }
 }
 instantiateAsync();
 return {};
}

function callRuntimeCallbacks(callbacks) {
 while (callbacks.length > 0) {
  var callback = callbacks.shift();
  if (typeof callback == "function") {
   callback(Module);
   continue;
  }
  var func = callback.func;
  if (typeof func === "number") {
   if (callback.arg === undefined) {
    wasmTable.get(func)();
   } else {
    wasmTable.get(func)(callback.arg);
   }
  } else {
   func(callback.arg === undefined ? null : callback.arg);
  }
 }
}

var runtimeKeepaliveCounter = 0;

function keepRuntimeAlive() {
 return noExitRuntime || runtimeKeepaliveCounter > 0;
}

function abortOnCannotGrowMemory(requestedSize) {
 abort("OOM");
}

function _emscripten_resize_heap(requestedSize) {
 var oldSize = HEAPU8.length;
 requestedSize = requestedSize >>> 0;
 abortOnCannotGrowMemory(requestedSize);
}

function _emscripten_set_main_loop_timing(mode, value) {
 Browser.mainLoop.timingMode = mode;
 Browser.mainLoop.timingValue = value;
 if (!Browser.mainLoop.func) {
  return 1;
 }
 if (!Browser.mainLoop.running) {
  Browser.mainLoop.running = true;
 }
 if (mode == 0) {
  Browser.mainLoop.scheduler = function Browser_mainLoop_scheduler_setTimeout() {
   var timeUntilNextTick = Math.max(0, Browser.mainLoop.tickStartTime + value - _emscripten_get_now()) | 0;
   setTimeout(Browser.mainLoop.runner, timeUntilNextTick);
  };
  Browser.mainLoop.method = "timeout";
 } else if (mode == 1) {
  Browser.mainLoop.scheduler = function Browser_mainLoop_scheduler_rAF() {
   Browser.requestAnimationFrame(Browser.mainLoop.runner);
  };
  Browser.mainLoop.method = "rAF";
 } else if (mode == 2) {
  if (typeof setImmediate === "undefined") {
   var setImmediates = [];
   var emscriptenMainLoopMessageId = "setimmediate";
   var Browser_setImmediate_messageHandler = function(event) {
    if (event.data === emscriptenMainLoopMessageId || event.data.target === emscriptenMainLoopMessageId) {
     event.stopPropagation();
     setImmediates.shift()();
    }
   };
   addEventListener("message", Browser_setImmediate_messageHandler, true);
   setImmediate = function Browser_emulated_setImmediate(func) {
    setImmediates.push(func);
    if (ENVIRONMENT_IS_WORKER) {
     if (Module["setImmediates"] === undefined) Module["setImmediates"] = [];
     Module["setImmediates"].push(func);
     postMessage({
      target: emscriptenMainLoopMessageId
     });
    } else postMessage(emscriptenMainLoopMessageId, "*");
   };
  }
  Browser.mainLoop.scheduler = function Browser_mainLoop_scheduler_setImmediate() {
   setImmediate(Browser.mainLoop.runner);
  };
  Browser.mainLoop.method = "immediate";
 }
 return 0;
}

var _emscripten_get_now;

if (ENVIRONMENT_IS_NODE) {
 _emscripten_get_now = function() {
  var t = process["hrtime"]();
  return t[0] * 1e3 + t[1] / 1e6;
 };
} else if (typeof dateNow !== "undefined") {
 _emscripten_get_now = dateNow;
} else _emscripten_get_now = function() {
 return performance.now();
};

function _exit(status) {
 exit(status);
}

function maybeExit() {
 if (!keepRuntimeAlive()) {
  try {
   _exit(EXITSTATUS);
  } catch (e) {
   if (e instanceof ExitStatus) {
    return;
   }
   throw e;
  }
 }
}

function setMainLoop(browserIterationFunc, fps, simulateInfiniteLoop, arg, noSetTiming) {
 assert(!Browser.mainLoop.func, "emscripten_set_main_loop: there can only be one main loop function at once: call emscripten_cancel_main_loop to cancel the previous one before setting a new one with different parameters.");
 Browser.mainLoop.func = browserIterationFunc;
 Browser.mainLoop.arg = arg;
 var thisMainLoopId = Browser.mainLoop.currentlyRunningMainloop;
 function checkIsRunning() {
  if (thisMainLoopId < Browser.mainLoop.currentlyRunningMainloop) {
   maybeExit();
   return false;
  }
  return true;
 }
 Browser.mainLoop.running = false;
 Browser.mainLoop.runner = function Browser_mainLoop_runner() {
  if (ABORT) return;
  if (Browser.mainLoop.queue.length > 0) {
   var start = Date.now();
   var blocker = Browser.mainLoop.queue.shift();
   blocker.func(blocker.arg);
   if (Browser.mainLoop.remainingBlockers) {
    var remaining = Browser.mainLoop.remainingBlockers;
    var next = remaining % 1 == 0 ? remaining - 1 : Math.floor(remaining);
    if (blocker.counted) {
     Browser.mainLoop.remainingBlockers = next;
    } else {
     next = next + .5;
     Browser.mainLoop.remainingBlockers = (8 * remaining + next) / 9;
    }
   }
   console.log('main loop blocker "' + blocker.name + '" took ' + (Date.now() - start) + " ms");
   Browser.mainLoop.updateStatus();
   if (!checkIsRunning()) return;
   setTimeout(Browser.mainLoop.runner, 0);
   return;
  }
  if (!checkIsRunning()) return;
  Browser.mainLoop.currentFrameNumber = Browser.mainLoop.currentFrameNumber + 1 | 0;
  if (Browser.mainLoop.timingMode == 1 && Browser.mainLoop.timingValue > 1 && Browser.mainLoop.currentFrameNumber % Browser.mainLoop.timingValue != 0) {
   Browser.mainLoop.scheduler();
   return;
  } else if (Browser.mainLoop.timingMode == 0) {
   Browser.mainLoop.tickStartTime = _emscripten_get_now();
  }
  GL.newRenderingFrameStarted();
  Browser.mainLoop.runIter(browserIterationFunc);
  if (!checkIsRunning()) return;
  if (typeof SDL === "object" && SDL.audio && SDL.audio.queueNewAudioData) SDL.audio.queueNewAudioData();
  Browser.mainLoop.scheduler();
 };
 if (!noSetTiming) {
  if (fps && fps > 0) _emscripten_set_main_loop_timing(0, 1e3 / fps); else _emscripten_set_main_loop_timing(1, 1);
  Browser.mainLoop.scheduler();
 }
 if (simulateInfiniteLoop) {
  throw "unwind";
 }
}

function callUserCallback(func, synchronous) {
 if (ABORT) {
  return;
 }
 if (synchronous) {
  func();
  return;
 }
 try {
  func();
 } catch (e) {
  if (e instanceof ExitStatus) {
   return;
  } else if (e !== "unwind") {
   if (e && typeof e === "object" && e.stack) err("exception thrown: " + [ e, e.stack ]);
   throw e;
  }
 }
}

var Browser = {
 mainLoop: {
  running: false,
  scheduler: null,
  method: "",
  currentlyRunningMainloop: 0,
  func: null,
  arg: 0,
  timingMode: 0,
  timingValue: 0,
  currentFrameNumber: 0,
  queue: [],
  pause: function() {
   Browser.mainLoop.scheduler = null;
   Browser.mainLoop.currentlyRunningMainloop++;
  },
  resume: function() {
   Browser.mainLoop.currentlyRunningMainloop++;
   var timingMode = Browser.mainLoop.timingMode;
   var timingValue = Browser.mainLoop.timingValue;
   var func = Browser.mainLoop.func;
   Browser.mainLoop.func = null;
   setMainLoop(func, 0, false, Browser.mainLoop.arg, true);
   _emscripten_set_main_loop_timing(timingMode, timingValue);
   Browser.mainLoop.scheduler();
  },
  updateStatus: function() {
   if (Module["setStatus"]) {
    var message = Module["statusMessage"] || "Please wait...";
    var remaining = Browser.mainLoop.remainingBlockers;
    var expected = Browser.mainLoop.expectedBlockers;
    if (remaining) {
     if (remaining < expected) {
      Module["setStatus"](message + " (" + (expected - remaining) + "/" + expected + ")");
     } else {
      Module["setStatus"](message);
     }
    } else {
     Module["setStatus"]("");
    }
   }
  },
  runIter: function(func) {
   if (ABORT) return;
   if (Module["preMainLoop"]) {
    var preRet = Module["preMainLoop"]();
    if (preRet === false) {
     return;
    }
   }
   callUserCallback(func);
   if (Module["postMainLoop"]) Module["postMainLoop"]();
  }
 },
 isFullscreen: false,
 pointerLock: false,
 moduleContextCreatedCallbacks: [],
 workers: [],
 init: function() {
  if (!Module["preloadPlugins"]) Module["preloadPlugins"] = [];
  if (Browser.initted) return;
  Browser.initted = true;
  try {
   new Blob();
   Browser.hasBlobConstructor = true;
  } catch (e) {
   Browser.hasBlobConstructor = false;
   console.log("warning: no blob constructor, cannot create blobs with mimetypes");
  }
  Browser.BlobBuilder = typeof MozBlobBuilder != "undefined" ? MozBlobBuilder : typeof WebKitBlobBuilder != "undefined" ? WebKitBlobBuilder : !Browser.hasBlobConstructor ? console.log("warning: no BlobBuilder") : null;
  Browser.URLObject = typeof window != "undefined" ? window.URL ? window.URL : window.webkitURL : undefined;
  if (!Module.noImageDecoding && typeof Browser.URLObject === "undefined") {
   console.log("warning: Browser does not support creating object URLs. Built-in browser image decoding will not be available.");
   Module.noImageDecoding = true;
  }
  var imagePlugin = {};
  imagePlugin["canHandle"] = function imagePlugin_canHandle(name) {
   return !Module.noImageDecoding && /\.(jpg|jpeg|png|bmp)$/i.test(name);
  };
  imagePlugin["handle"] = function imagePlugin_handle(byteArray, name, onload, onerror) {
   var b = null;
   if (Browser.hasBlobConstructor) {
    try {
     b = new Blob([ byteArray ], {
      type: Browser.getMimetype(name)
     });
     if (b.size !== byteArray.length) {
      b = new Blob([ new Uint8Array(byteArray).buffer ], {
       type: Browser.getMimetype(name)
      });
     }
    } catch (e) {
     warnOnce("Blob constructor present but fails: " + e + "; falling back to blob builder");
    }
   }
   if (!b) {
    var bb = new Browser.BlobBuilder();
    bb.append(new Uint8Array(byteArray).buffer);
    b = bb.getBlob();
   }
   var url = Browser.URLObject.createObjectURL(b);
   var img = new Image();
   img.onload = function img_onload() {
    assert(img.complete, "Image " + name + " could not be decoded");
    var canvas = document.createElement("canvas");
    canvas.width = img.width;
    canvas.height = img.height;
    var ctx = canvas.getContext("2d");
    ctx.drawImage(img, 0, 0);
    Module["preloadedImages"][name] = canvas;
    Browser.URLObject.revokeObjectURL(url);
    if (onload) onload(byteArray);
   };
   img.onerror = function img_onerror(event) {
    console.log("Image " + url + " could not be decoded");
    if (onerror) onerror();
   };
   img.src = url;
  };
  Module["preloadPlugins"].push(imagePlugin);
  var audioPlugin = {};
  audioPlugin["canHandle"] = function audioPlugin_canHandle(name) {
   return !Module.noAudioDecoding && name.substr(-4) in {
    ".ogg": 1,
    ".wav": 1,
    ".mp3": 1
   };
  };
  audioPlugin["handle"] = function audioPlugin_handle(byteArray, name, onload, onerror) {
   var done = false;
   function finish(audio) {
    if (done) return;
    done = true;
    Module["preloadedAudios"][name] = audio;
    if (onload) onload(byteArray);
   }
   function fail() {
    if (done) return;
    done = true;
    Module["preloadedAudios"][name] = new Audio();
    if (onerror) onerror();
   }
   if (Browser.hasBlobConstructor) {
    try {
     var b = new Blob([ byteArray ], {
      type: Browser.getMimetype(name)
     });
    } catch (e) {
     return fail();
    }
    var url = Browser.URLObject.createObjectURL(b);
    var audio = new Audio();
    audio.addEventListener("canplaythrough", function() {
     finish(audio);
    }, false);
    audio.onerror = function audio_onerror(event) {
     if (done) return;
     console.log("warning: browser could not fully decode audio " + name + ", trying slower base64 approach");
     function encode64(data) {
      var BASE = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
      var PAD = "=";
      var ret = "";
      var leftchar = 0;
      var leftbits = 0;
      for (var i = 0; i < data.length; i++) {
       leftchar = leftchar << 8 | data[i];
       leftbits += 8;
       while (leftbits >= 6) {
        var curr = leftchar >> leftbits - 6 & 63;
        leftbits -= 6;
        ret += BASE[curr];
       }
      }
      if (leftbits == 2) {
       ret += BASE[(leftchar & 3) << 4];
       ret += PAD + PAD;
      } else if (leftbits == 4) {
       ret += BASE[(leftchar & 15) << 2];
       ret += PAD;
      }
      return ret;
     }
     audio.src = "data:audio/x-" + name.substr(-3) + ";base64," + encode64(byteArray);
     finish(audio);
    };
    audio.src = url;
    Browser.safeSetTimeout(function() {
     finish(audio);
    }, 1e4);
   } else {
    return fail();
   }
  };
  Module["preloadPlugins"].push(audioPlugin);
  function pointerLockChange() {
   Browser.pointerLock = document["pointerLockElement"] === Module["canvas"] || document["mozPointerLockElement"] === Module["canvas"] || document["webkitPointerLockElement"] === Module["canvas"] || document["msPointerLockElement"] === Module["canvas"];
  }
  var canvas = Module["canvas"];
  if (canvas) {
   canvas.requestPointerLock = canvas["requestPointerLock"] || canvas["mozRequestPointerLock"] || canvas["webkitRequestPointerLock"] || canvas["msRequestPointerLock"] || function() {};
   canvas.exitPointerLock = document["exitPointerLock"] || document["mozExitPointerLock"] || document["webkitExitPointerLock"] || document["msExitPointerLock"] || function() {};
   canvas.exitPointerLock = canvas.exitPointerLock.bind(document);
   document.addEventListener("pointerlockchange", pointerLockChange, false);
   document.addEventListener("mozpointerlockchange", pointerLockChange, false);
   document.addEventListener("webkitpointerlockchange", pointerLockChange, false);
   document.addEventListener("mspointerlockchange", pointerLockChange, false);
   if (Module["elementPointerLock"]) {
    canvas.addEventListener("click", function(ev) {
     if (!Browser.pointerLock && Module["canvas"].requestPointerLock) {
      Module["canvas"].requestPointerLock();
      ev.preventDefault();
     }
    }, false);
   }
  }
 },
 createContext: function(canvas, useWebGL, setInModule, webGLContextAttributes) {
  if (useWebGL && Module.ctx && canvas == Module.canvas) return Module.ctx;
  var ctx;
  var contextHandle;
  if (useWebGL) {
   var contextAttributes = {
    antialias: false,
    alpha: false,
    majorVersion: typeof WebGL2RenderingContext !== "undefined" ? 2 : 1
   };
   if (webGLContextAttributes) {
    for (var attribute in webGLContextAttributes) {
     contextAttributes[attribute] = webGLContextAttributes[attribute];
    }
   }
   if (typeof GL !== "undefined") {
    contextHandle = GL.createContext(canvas, contextAttributes);
    if (contextHandle) {
     ctx = GL.getContext(contextHandle).GLctx;
    }
   }
  } else {
   ctx = canvas.getContext("2d");
  }
  if (!ctx) return null;
  if (setInModule) {
   if (!useWebGL) assert(typeof GLctx === "undefined", "cannot set in module if GLctx is used, but we are a non-GL context that would replace it");
   Module.ctx = ctx;
   if (useWebGL) GL.makeContextCurrent(contextHandle);
   Module.useWebGL = useWebGL;
   Browser.moduleContextCreatedCallbacks.forEach(function(callback) {
    callback();
   });
   Browser.init();
  }
  return ctx;
 },
 destroyContext: function(canvas, useWebGL, setInModule) {},
 fullscreenHandlersInstalled: false,
 lockPointer: undefined,
 resizeCanvas: undefined,
 requestFullscreen: function(lockPointer, resizeCanvas) {
  Browser.lockPointer = lockPointer;
  Browser.resizeCanvas = resizeCanvas;
  if (typeof Browser.lockPointer === "undefined") Browser.lockPointer = true;
  if (typeof Browser.resizeCanvas === "undefined") Browser.resizeCanvas = false;
  var canvas = Module["canvas"];
  function fullscreenChange() {
   Browser.isFullscreen = false;
   var canvasContainer = canvas.parentNode;
   if ((document["fullscreenElement"] || document["mozFullScreenElement"] || document["msFullscreenElement"] || document["webkitFullscreenElement"] || document["webkitCurrentFullScreenElement"]) === canvasContainer) {
    canvas.exitFullscreen = Browser.exitFullscreen;
    if (Browser.lockPointer) canvas.requestPointerLock();
    Browser.isFullscreen = true;
    if (Browser.resizeCanvas) {
     Browser.setFullscreenCanvasSize();
    } else {
     Browser.updateCanvasDimensions(canvas);
    }
   } else {
    canvasContainer.parentNode.insertBefore(canvas, canvasContainer);
    canvasContainer.parentNode.removeChild(canvasContainer);
    if (Browser.resizeCanvas) {
     Browser.setWindowedCanvasSize();
    } else {
     Browser.updateCanvasDimensions(canvas);
    }
   }
   if (Module["onFullScreen"]) Module["onFullScreen"](Browser.isFullscreen);
   if (Module["onFullscreen"]) Module["onFullscreen"](Browser.isFullscreen);
  }
  if (!Browser.fullscreenHandlersInstalled) {
   Browser.fullscreenHandlersInstalled = true;
   document.addEventListener("fullscreenchange", fullscreenChange, false);
   document.addEventListener("mozfullscreenchange", fullscreenChange, false);
   document.addEventListener("webkitfullscreenchange", fullscreenChange, false);
   document.addEventListener("MSFullscreenChange", fullscreenChange, false);
  }
  var canvasContainer = document.createElement("div");
  canvas.parentNode.insertBefore(canvasContainer, canvas);
  canvasContainer.appendChild(canvas);
  canvasContainer.requestFullscreen = canvasContainer["requestFullscreen"] || canvasContainer["mozRequestFullScreen"] || canvasContainer["msRequestFullscreen"] || (canvasContainer["webkitRequestFullscreen"] ? function() {
   canvasContainer["webkitRequestFullscreen"](Element["ALLOW_KEYBOARD_INPUT"]);
  } : null) || (canvasContainer["webkitRequestFullScreen"] ? function() {
   canvasContainer["webkitRequestFullScreen"](Element["ALLOW_KEYBOARD_INPUT"]);
  } : null);
  canvasContainer.requestFullscreen();
 },
 exitFullscreen: function() {
  if (!Browser.isFullscreen) {
   return false;
  }
  var CFS = document["exitFullscreen"] || document["cancelFullScreen"] || document["mozCancelFullScreen"] || document["msExitFullscreen"] || document["webkitCancelFullScreen"] || function() {};
  CFS.apply(document, []);
  return true;
 },
 nextRAF: 0,
 fakeRequestAnimationFrame: function(func) {
  var now = Date.now();
  if (Browser.nextRAF === 0) {
   Browser.nextRAF = now + 1e3 / 60;
  } else {
   while (now + 2 >= Browser.nextRAF) {
    Browser.nextRAF += 1e3 / 60;
   }
  }
  var delay = Math.max(Browser.nextRAF - now, 0);
  setTimeout(func, delay);
 },
 requestAnimationFrame: function(func) {
  if (typeof requestAnimationFrame === "function") {
   requestAnimationFrame(func);
   return;
  }
  var RAF = Browser.fakeRequestAnimationFrame;
  RAF(func);
 },
 safeRequestAnimationFrame: function(func) {
  return Browser.requestAnimationFrame(function() {
   callUserCallback(func);
  });
 },
 safeSetTimeout: function(func, timeout) {
  return setTimeout(function() {
   callUserCallback(func);
  }, timeout);
 },
 getMimetype: function(name) {
  return {
   "jpg": "image/jpeg",
   "jpeg": "image/jpeg",
   "png": "image/png",
   "bmp": "image/bmp",
   "ogg": "audio/ogg",
   "wav": "audio/wav",
   "mp3": "audio/mpeg"
  }[name.substr(name.lastIndexOf(".") + 1)];
 },
 getUserMedia: function(func) {
  if (!window.getUserMedia) {
   window.getUserMedia = navigator["getUserMedia"] || navigator["mozGetUserMedia"];
  }
  window.getUserMedia(func);
 },
 getMovementX: function(event) {
  return event["movementX"] || event["mozMovementX"] || event["webkitMovementX"] || 0;
 },
 getMovementY: function(event) {
  return event["movementY"] || event["mozMovementY"] || event["webkitMovementY"] || 0;
 },
 getMouseWheelDelta: function(event) {
  var delta = 0;
  switch (event.type) {
  case "DOMMouseScroll":
   delta = event.detail / 3;
   break;

  case "mousewheel":
   delta = event.wheelDelta / 120;
   break;

  case "wheel":
   delta = event.deltaY;
   switch (event.deltaMode) {
   case 0:
    delta /= 100;
    break;

   case 1:
    delta /= 3;
    break;

   case 2:
    delta *= 80;
    break;

   default:
    throw "unrecognized mouse wheel delta mode: " + event.deltaMode;
   }
   break;

  default:
   throw "unrecognized mouse wheel event: " + event.type;
  }
  return delta;
 },
 mouseX: 0,
 mouseY: 0,
 mouseMovementX: 0,
 mouseMovementY: 0,
 touches: {},
 lastTouches: {},
 calculateMouseEvent: function(event) {
  if (Browser.pointerLock) {
   if (event.type != "mousemove" && "mozMovementX" in event) {
    Browser.mouseMovementX = Browser.mouseMovementY = 0;
   } else {
    Browser.mouseMovementX = Browser.getMovementX(event);
    Browser.mouseMovementY = Browser.getMovementY(event);
   }
   if (typeof SDL != "undefined") {
    Browser.mouseX = SDL.mouseX + Browser.mouseMovementX;
    Browser.mouseY = SDL.mouseY + Browser.mouseMovementY;
   } else {
    Browser.mouseX += Browser.mouseMovementX;
    Browser.mouseY += Browser.mouseMovementY;
   }
  } else {
   var rect = Module["canvas"].getBoundingClientRect();
   var cw = Module["canvas"].width;
   var ch = Module["canvas"].height;
   var scrollX = typeof window.scrollX !== "undefined" ? window.scrollX : window.pageXOffset;
   var scrollY = typeof window.scrollY !== "undefined" ? window.scrollY : window.pageYOffset;
   if (event.type === "touchstart" || event.type === "touchend" || event.type === "touchmove") {
    var touch = event.touch;
    if (touch === undefined) {
     return;
    }
    var adjustedX = touch.pageX - (scrollX + rect.left);
    var adjustedY = touch.pageY - (scrollY + rect.top);
    adjustedX = adjustedX * (cw / rect.width);
    adjustedY = adjustedY * (ch / rect.height);
    var coords = {
     x: adjustedX,
     y: adjustedY
    };
    if (event.type === "touchstart") {
     Browser.lastTouches[touch.identifier] = coords;
     Browser.touches[touch.identifier] = coords;
    } else if (event.type === "touchend" || event.type === "touchmove") {
     var last = Browser.touches[touch.identifier];
     if (!last) last = coords;
     Browser.lastTouches[touch.identifier] = last;
     Browser.touches[touch.identifier] = coords;
    }
    return;
   }
   var x = event.pageX - (scrollX + rect.left);
   var y = event.pageY - (scrollY + rect.top);
   x = x * (cw / rect.width);
   y = y * (ch / rect.height);
   Browser.mouseMovementX = x - Browser.mouseX;
   Browser.mouseMovementY = y - Browser.mouseY;
   Browser.mouseX = x;
   Browser.mouseY = y;
  }
 },
 asyncLoad: function(url, onload, onerror, noRunDep) {
  var dep = !noRunDep ? getUniqueRunDependency("al " + url) : "";
  readAsync(url, function(arrayBuffer) {
   assert(arrayBuffer, 'Loading data file "' + url + '" failed (no arrayBuffer).');
   onload(new Uint8Array(arrayBuffer));
   if (dep) removeRunDependency(dep);
  }, function(event) {
   if (onerror) {
    onerror();
   } else {
    throw 'Loading data file "' + url + '" failed.';
   }
  });
  if (dep) addRunDependency(dep);
 },
 resizeListeners: [],
 updateResizeListeners: function() {
  var canvas = Module["canvas"];
  Browser.resizeListeners.forEach(function(listener) {
   listener(canvas.width, canvas.height);
  });
 },
 setCanvasSize: function(width, height, noUpdates) {
  var canvas = Module["canvas"];
  Browser.updateCanvasDimensions(canvas, width, height);
  if (!noUpdates) Browser.updateResizeListeners();
 },
 windowedWidth: 0,
 windowedHeight: 0,
 setFullscreenCanvasSize: function() {
  if (typeof SDL != "undefined") {
   var flags = HEAPU32[SDL.screen >> 2];
   flags = flags | 8388608;
   HEAP32[SDL.screen >> 2] = flags;
  }
  Browser.updateCanvasDimensions(Module["canvas"]);
  Browser.updateResizeListeners();
 },
 setWindowedCanvasSize: function() {
  if (typeof SDL != "undefined") {
   var flags = HEAPU32[SDL.screen >> 2];
   flags = flags & ~8388608;
   HEAP32[SDL.screen >> 2] = flags;
  }
  Browser.updateCanvasDimensions(Module["canvas"]);
  Browser.updateResizeListeners();
 },
 updateCanvasDimensions: function(canvas, wNative, hNative) {
  if (wNative && hNative) {
   canvas.widthNative = wNative;
   canvas.heightNative = hNative;
  } else {
   wNative = canvas.widthNative;
   hNative = canvas.heightNative;
  }
  var w = wNative;
  var h = hNative;
  if (Module["forcedAspectRatio"] && Module["forcedAspectRatio"] > 0) {
   if (w / h < Module["forcedAspectRatio"]) {
    w = Math.round(h * Module["forcedAspectRatio"]);
   } else {
    h = Math.round(w / Module["forcedAspectRatio"]);
   }
  }
  if ((document["fullscreenElement"] || document["mozFullScreenElement"] || document["msFullscreenElement"] || document["webkitFullscreenElement"] || document["webkitCurrentFullScreenElement"]) === canvas.parentNode && typeof screen != "undefined") {
   var factor = Math.min(screen.width / w, screen.height / h);
   w = Math.round(w * factor);
   h = Math.round(h * factor);
  }
  if (Browser.resizeCanvas) {
   if (canvas.width != w) canvas.width = w;
   if (canvas.height != h) canvas.height = h;
   if (typeof canvas.style != "undefined") {
    canvas.style.removeProperty("width");
    canvas.style.removeProperty("height");
   }
  } else {
   if (canvas.width != wNative) canvas.width = wNative;
   if (canvas.height != hNative) canvas.height = hNative;
   if (typeof canvas.style != "undefined") {
    if (w != wNative || h != hNative) {
     canvas.style.setProperty("width", w + "px", "important");
     canvas.style.setProperty("height", h + "px", "important");
    } else {
     canvas.style.removeProperty("width");
     canvas.style.removeProperty("height");
    }
   }
  }
 },
 wgetRequests: {},
 nextWgetRequestHandle: 0,
 getNextWgetRequestHandle: function() {
  var handle = Browser.nextWgetRequestHandle;
  Browser.nextWgetRequestHandle++;
  return handle;
 }
};

function __webgl_enable_ANGLE_instanced_arrays(ctx) {
 var ext = ctx.getExtension("ANGLE_instanced_arrays");
 if (ext) {
  ctx["vertexAttribDivisor"] = function(index, divisor) {
   ext["vertexAttribDivisorANGLE"](index, divisor);
  };
  ctx["drawArraysInstanced"] = function(mode, first, count, primcount) {
   ext["drawArraysInstancedANGLE"](mode, first, count, primcount);
  };
  ctx["drawElementsInstanced"] = function(mode, count, type, indices, primcount) {
   ext["drawElementsInstancedANGLE"](mode, count, type, indices, primcount);
  };
  return 1;
 }
}

function __webgl_enable_OES_vertex_array_object(ctx) {
 var ext = ctx.getExtension("OES_vertex_array_object");
 if (ext) {
  ctx["createVertexArray"] = function() {
   return ext["createVertexArrayOES"]();
  };
  ctx["deleteVertexArray"] = function(vao) {
   ext["deleteVertexArrayOES"](vao);
  };
  ctx["bindVertexArray"] = function(vao) {
   ext["bindVertexArrayOES"](vao);
  };
  ctx["isVertexArray"] = function(vao) {
   return ext["isVertexArrayOES"](vao);
  };
  return 1;
 }
}

function __webgl_enable_WEBGL_draw_buffers(ctx) {
 var ext = ctx.getExtension("WEBGL_draw_buffers");
 if (ext) {
  ctx["drawBuffers"] = function(n, bufs) {
   ext["drawBuffersWEBGL"](n, bufs);
  };
  return 1;
 }
}

function __webgl_enable_WEBGL_draw_instanced_base_vertex_base_instance(ctx) {
 return !!(ctx.dibvbi = ctx.getExtension("WEBGL_draw_instanced_base_vertex_base_instance"));
}

function __webgl_enable_WEBGL_multi_draw_instanced_base_vertex_base_instance(ctx) {
 return !!(ctx.mdibvbi = ctx.getExtension("WEBGL_multi_draw_instanced_base_vertex_base_instance"));
}

function __webgl_enable_WEBGL_multi_draw(ctx) {
 return !!(ctx.multiDrawWebgl = ctx.getExtension("WEBGL_multi_draw"));
}

var GL = {
 counter: 1,
 buffers: [],
 programs: [],
 framebuffers: [],
 renderbuffers: [],
 textures: [],
 shaders: [],
 vaos: [],
 contexts: [],
 offscreenCanvases: {},
 queries: [],
 samplers: [],
 transformFeedbacks: [],
 syncs: [],
 byteSizeByTypeRoot: 5120,
 byteSizeByType: [ 1, 1, 2, 2, 4, 4, 4, 2, 3, 4, 8 ],
 stringCache: {},
 stringiCache: {},
 unpackAlignment: 4,
 recordError: function recordError(errorCode) {
  if (!GL.lastError) {
   GL.lastError = errorCode;
  }
 },
 getNewId: function(table) {
  var ret = GL.counter++;
  for (var i = table.length; i < ret; i++) {
   table[i] = null;
  }
  return ret;
 },
 MAX_TEMP_BUFFER_SIZE: 2097152,
 numTempVertexBuffersPerSize: 64,
 log2ceilLookup: function(i) {
  return 32 - Math.clz32(i === 0 ? 0 : i - 1);
 },
 generateTempBuffers: function(quads, context) {
  var largestIndex = GL.log2ceilLookup(GL.MAX_TEMP_BUFFER_SIZE);
  context.tempVertexBufferCounters1 = [];
  context.tempVertexBufferCounters2 = [];
  context.tempVertexBufferCounters1.length = context.tempVertexBufferCounters2.length = largestIndex + 1;
  context.tempVertexBuffers1 = [];
  context.tempVertexBuffers2 = [];
  context.tempVertexBuffers1.length = context.tempVertexBuffers2.length = largestIndex + 1;
  context.tempIndexBuffers = [];
  context.tempIndexBuffers.length = largestIndex + 1;
  for (var i = 0; i <= largestIndex; ++i) {
   context.tempIndexBuffers[i] = null;
   context.tempVertexBufferCounters1[i] = context.tempVertexBufferCounters2[i] = 0;
   var ringbufferLength = GL.numTempVertexBuffersPerSize;
   context.tempVertexBuffers1[i] = [];
   context.tempVertexBuffers2[i] = [];
   var ringbuffer1 = context.tempVertexBuffers1[i];
   var ringbuffer2 = context.tempVertexBuffers2[i];
   ringbuffer1.length = ringbuffer2.length = ringbufferLength;
   for (var j = 0; j < ringbufferLength; ++j) {
    ringbuffer1[j] = ringbuffer2[j] = null;
   }
  }
  if (quads) {
   context.tempQuadIndexBuffer = GLctx.createBuffer();
   context.GLctx.bindBuffer(34963, context.tempQuadIndexBuffer);
   var numIndexes = GL.MAX_TEMP_BUFFER_SIZE >> 1;
   var quadIndexes = new Uint16Array(numIndexes);
   var i = 0, v = 0;
   while (1) {
    quadIndexes[i++] = v;
    if (i >= numIndexes) break;
    quadIndexes[i++] = v + 1;
    if (i >= numIndexes) break;
    quadIndexes[i++] = v + 2;
    if (i >= numIndexes) break;
    quadIndexes[i++] = v;
    if (i >= numIndexes) break;
    quadIndexes[i++] = v + 2;
    if (i >= numIndexes) break;
    quadIndexes[i++] = v + 3;
    if (i >= numIndexes) break;
    v += 4;
   }
   context.GLctx.bufferData(34963, quadIndexes, 35044);
   context.GLctx.bindBuffer(34963, null);
  }
 },
 getTempVertexBuffer: function getTempVertexBuffer(sizeBytes) {
  var idx = GL.log2ceilLookup(sizeBytes);
  var ringbuffer = GL.currentContext.tempVertexBuffers1[idx];
  var nextFreeBufferIndex = GL.currentContext.tempVertexBufferCounters1[idx];
  GL.currentContext.tempVertexBufferCounters1[idx] = GL.currentContext.tempVertexBufferCounters1[idx] + 1 & GL.numTempVertexBuffersPerSize - 1;
  var vbo = ringbuffer[nextFreeBufferIndex];
  if (vbo) {
   return vbo;
  }
  var prevVBO = GLctx.getParameter(34964);
  ringbuffer[nextFreeBufferIndex] = GLctx.createBuffer();
  GLctx.bindBuffer(34962, ringbuffer[nextFreeBufferIndex]);
  GLctx.bufferData(34962, 1 << idx, 35048);
  GLctx.bindBuffer(34962, prevVBO);
  return ringbuffer[nextFreeBufferIndex];
 },
 getTempIndexBuffer: function getTempIndexBuffer(sizeBytes) {
  var idx = GL.log2ceilLookup(sizeBytes);
  var ibo = GL.currentContext.tempIndexBuffers[idx];
  if (ibo) {
   return ibo;
  }
  var prevIBO = GLctx.getParameter(34965);
  GL.currentContext.tempIndexBuffers[idx] = GLctx.createBuffer();
  GLctx.bindBuffer(34963, GL.currentContext.tempIndexBuffers[idx]);
  GLctx.bufferData(34963, 1 << idx, 35048);
  GLctx.bindBuffer(34963, prevIBO);
  return GL.currentContext.tempIndexBuffers[idx];
 },
 newRenderingFrameStarted: function newRenderingFrameStarted() {
  if (!GL.currentContext) {
   return;
  }
  var vb = GL.currentContext.tempVertexBuffers1;
  GL.currentContext.tempVertexBuffers1 = GL.currentContext.tempVertexBuffers2;
  GL.currentContext.tempVertexBuffers2 = vb;
  vb = GL.currentContext.tempVertexBufferCounters1;
  GL.currentContext.tempVertexBufferCounters1 = GL.currentContext.tempVertexBufferCounters2;
  GL.currentContext.tempVertexBufferCounters2 = vb;
  var largestIndex = GL.log2ceilLookup(GL.MAX_TEMP_BUFFER_SIZE);
  for (var i = 0; i <= largestIndex; ++i) {
   GL.currentContext.tempVertexBufferCounters1[i] = 0;
  }
 },
 getSource: function(shader, count, string, length) {
  var source = "";
  for (var i = 0; i < count; ++i) {
   var len = length ? HEAP32[length + i * 4 >> 2] : -1;
   source += UTF8ToString(HEAP32[string + i * 4 >> 2], len < 0 ? undefined : len);
  }
  var type = GLctx.getShaderParameter(GL.shaders[shader], 35663);
  if (type == 35632) {
   if (GLEmulation.findToken(source, "dFdx") || GLEmulation.findToken(source, "dFdy") || GLEmulation.findToken(source, "fwidth")) {
    source = "#extension GL_OES_standard_derivatives : enable\n" + source;
    var extension = GLctx.getExtension("OES_standard_derivatives");
   }
  }
  return source;
 },
 enabledClientAttribIndices: [],
 enableVertexAttribArray: function enableVertexAttribArray(index) {
  if (!GL.enabledClientAttribIndices[index]) {
   GL.enabledClientAttribIndices[index] = true;
   GLctx.enableVertexAttribArray(index);
  }
 },
 disableVertexAttribArray: function disableVertexAttribArray(index) {
  if (GL.enabledClientAttribIndices[index]) {
   GL.enabledClientAttribIndices[index] = false;
   GLctx.disableVertexAttribArray(index);
  }
 },
 createContext: function(canvas, webGLContextAttributes) {
  if (!canvas.getContextSafariWebGL2Fixed) {
   canvas.getContextSafariWebGL2Fixed = canvas.getContext;
   canvas.getContext = function(ver, attrs) {
    var gl = canvas.getContextSafariWebGL2Fixed(ver, attrs);
    return ver == "webgl" == gl instanceof WebGLRenderingContext ? gl : null;
   };
  }
  var ctx = webGLContextAttributes.majorVersion > 1 ? canvas.getContext("webgl2", webGLContextAttributes) : canvas.getContext("webgl", webGLContextAttributes);
  if (!ctx) return 0;
  var handle = GL.registerContext(ctx, webGLContextAttributes);
  return handle;
 },
 registerContext: function(ctx, webGLContextAttributes) {
  var handle = GL.getNewId(GL.contexts);
  var context = {
   handle: handle,
   attributes: webGLContextAttributes,
   version: webGLContextAttributes.majorVersion,
   GLctx: ctx
  };
  if (ctx.canvas) ctx.canvas.GLctxObject = context;
  GL.contexts[handle] = context;
  if (typeof webGLContextAttributes.enableExtensionsByDefault === "undefined" || webGLContextAttributes.enableExtensionsByDefault) {
   GL.initExtensions(context);
  }
  return handle;
 },
 makeContextCurrent: function(contextHandle) {
  GL.currentContext = GL.contexts[contextHandle];
  Module.ctx = GLctx = GL.currentContext && GL.currentContext.GLctx;
  return !(contextHandle && !GLctx);
 },
 getContext: function(contextHandle) {
  return GL.contexts[contextHandle];
 },
 deleteContext: function(contextHandle) {
  if (GL.currentContext === GL.contexts[contextHandle]) GL.currentContext = null;
  if (typeof JSEvents === "object") JSEvents.removeAllHandlersOnTarget(GL.contexts[contextHandle].GLctx.canvas);
  if (GL.contexts[contextHandle] && GL.contexts[contextHandle].GLctx.canvas) GL.contexts[contextHandle].GLctx.canvas.GLctxObject = undefined;
  GL.contexts[contextHandle] = null;
 },
 initExtensions: function(context) {
  if (!context) context = GL.currentContext;
  if (context.initExtensionsDone) return;
  context.initExtensionsDone = true;
  var GLctx = context.GLctx;
  context.compressionExt = GLctx.getExtension("WEBGL_compressed_texture_s3tc");
  context.anisotropicExt = GLctx.getExtension("EXT_texture_filter_anisotropic");
  __webgl_enable_ANGLE_instanced_arrays(GLctx);
  __webgl_enable_OES_vertex_array_object(GLctx);
  __webgl_enable_WEBGL_draw_buffers(GLctx);
  __webgl_enable_WEBGL_draw_instanced_base_vertex_base_instance(GLctx);
  __webgl_enable_WEBGL_multi_draw_instanced_base_vertex_base_instance(GLctx);
  if (context.version >= 2) {
   GLctx.disjointTimerQueryExt = GLctx.getExtension("EXT_disjoint_timer_query_webgl2");
  }
  if (context.version < 2 || !GLctx.disjointTimerQueryExt) {
   GLctx.disjointTimerQueryExt = GLctx.getExtension("EXT_disjoint_timer_query");
  }
  __webgl_enable_WEBGL_multi_draw(GLctx);
  var exts = GLctx.getSupportedExtensions() || [];
  exts.forEach(function(ext) {
   if (!ext.includes("lose_context") && !ext.includes("debug")) {
    GLctx.getExtension(ext);
   }
  });
 }
};

function _glEnable(x0) {
 GLctx["enable"](x0);
}

function _glDisable(x0) {
 GLctx["disable"](x0);
}

function _glIsEnabled(x0) {
 return GLctx["isEnabled"](x0);
}

function writeI53ToI64(ptr, num) {
 HEAPU32[ptr >> 2] = num;
 HEAPU32[ptr + 4 >> 2] = (num - HEAPU32[ptr >> 2]) / 4294967296;
}

function emscriptenWebGLGet(name_, p, type) {
 if (!p) {
  GL.recordError(1281);
  return;
 }
 var ret = undefined;
 switch (name_) {
 case 36346:
  ret = 1;
  break;

 case 36344:
  if (type != 0 && type != 1) {
   GL.recordError(1280);
  }
  return;

 case 34814:
 case 36345:
  ret = 0;
  break;

 case 34466:
  var formats = GLctx.getParameter(34467);
  ret = formats ? formats.length : 0;
  break;

 case 33309:
  if (GL.currentContext.version < 2) {
   GL.recordError(1282);
   return;
  }
  var exts = GLctx.getSupportedExtensions() || [];
  ret = 2 * exts.length;
  break;

 case 33307:
 case 33308:
  if (GL.currentContext.version < 2) {
   GL.recordError(1280);
   return;
  }
  ret = name_ == 33307 ? 3 : 0;
  break;
 }
 if (ret === undefined) {
  var result = GLctx.getParameter(name_);
  switch (typeof result) {
  case "number":
   ret = result;
   break;

  case "boolean":
   ret = result ? 1 : 0;
   break;

  case "string":
   GL.recordError(1280);
   return;

  case "object":
   if (result === null) {
    switch (name_) {
    case 34964:
    case 35725:
    case 34965:
    case 36006:
    case 36007:
    case 32873:
    case 34229:
    case 36662:
    case 36663:
    case 35053:
    case 35055:
    case 36010:
    case 35097:
    case 35869:
    case 32874:
    case 36389:
    case 35983:
    case 35368:
    case 34068:
     {
      ret = 0;
      break;
     }

    default:
     {
      GL.recordError(1280);
      return;
     }
    }
   } else if (result instanceof Float32Array || result instanceof Uint32Array || result instanceof Int32Array || result instanceof Array) {
    for (var i = 0; i < result.length; ++i) {
     switch (type) {
     case 0:
      HEAP32[p + i * 4 >> 2] = result[i];
      break;

     case 2:
      HEAPF32[p + i * 4 >> 2] = result[i];
      break;

     case 4:
      HEAP8[p + i >> 0] = result[i] ? 1 : 0;
      break;
     }
    }
    return;
   } else {
    try {
     ret = result.name | 0;
    } catch (e) {
     GL.recordError(1280);
     err("GL_INVALID_ENUM in glGet" + type + "v: Unknown object returned from WebGL getParameter(" + name_ + ")! (error: " + e + ")");
     return;
    }
   }
   break;

  default:
   GL.recordError(1280);
   err("GL_INVALID_ENUM in glGet" + type + "v: Native code calling glGet" + type + "v(" + name_ + ") and it returns " + result + " of type " + typeof result + "!");
   return;
  }
 }
 switch (type) {
 case 1:
  writeI53ToI64(p, ret);
  break;

 case 0:
  HEAP32[p >> 2] = ret;
  break;

 case 2:
  HEAPF32[p >> 2] = ret;
  break;

 case 4:
  HEAP8[p >> 0] = ret ? 1 : 0;
  break;
 }
}

function _glGetBooleanv(name_, p) {
 emscriptenWebGLGet(name_, p, 4);
}

function _glGetIntegerv(name_, p) {
 emscriptenWebGLGet(name_, p, 0);
}

function stringToNewUTF8(jsString) {
 var length = lengthBytesUTF8(jsString) + 1;
 var cString = _malloc(length);
 stringToUTF8(jsString, cString, length);
 return cString;
}

function _glGetString(name_) {
 var ret = GL.stringCache[name_];
 if (!ret) {
  switch (name_) {
  case 7939:
   var exts = GLctx.getSupportedExtensions() || [];
   exts = exts.concat(exts.map(function(e) {
    return "GL_" + e;
   }));
   ret = stringToNewUTF8(exts.join(" "));
   break;

  case 7936:
  case 7937:
  case 37445:
  case 37446:
   var s = GLctx.getParameter(name_);
   if (!s) {
    GL.recordError(1280);
   }
   ret = s && stringToNewUTF8(s);
   break;

  case 7938:
   var glVersion = GLctx.getParameter(7938);
   if (GL.currentContext.version >= 2) glVersion = "OpenGL ES 3.0 (" + glVersion + ")"; else {
    glVersion = "OpenGL ES 2.0 (" + glVersion + ")";
   }
   ret = stringToNewUTF8(glVersion);
   break;

  case 35724:
   var glslVersion = GLctx.getParameter(35724);
   var ver_re = /^WebGL GLSL ES ([0-9]\.[0-9][0-9]?)(?:$| .*)/;
   var ver_num = glslVersion.match(ver_re);
   if (ver_num !== null) {
    if (ver_num[1].length == 3) ver_num[1] = ver_num[1] + "0";
    glslVersion = "OpenGL ES GLSL ES " + ver_num[1] + " (" + glslVersion + ")";
   }
   ret = stringToNewUTF8(glslVersion);
   break;

  default:
   GL.recordError(1280);
  }
  GL.stringCache[name_] = ret;
 }
 return ret;
}

function _glCreateShader(shaderType) {
 var id = GL.getNewId(GL.shaders);
 GL.shaders[id] = GLctx.createShader(shaderType);
 return id;
}

function _glShaderSource(shader, count, string, length) {
 var source = GL.getSource(shader, count, string, length);
 GLctx.shaderSource(GL.shaders[shader], source);
}

function _glCompileShader(shader) {
 GLctx.compileShader(GL.shaders[shader]);
}

function _glAttachShader(program, shader) {
 GLctx.attachShader(GL.programs[program], GL.shaders[shader]);
}

function _glDetachShader(program, shader) {
 GLctx.detachShader(GL.programs[program], GL.shaders[shader]);
}

function _glUseProgram(program) {
 program = GL.programs[program];
 GLctx.useProgram(program);
 GLctx.currentProgram = program;
}

function _glDeleteProgram(id) {
 if (!id) return;
 var program = GL.programs[id];
 if (!program) {
  GL.recordError(1281);
  return;
 }
 GLctx.deleteProgram(program);
 program.name = 0;
 GL.programs[id] = null;
}

function _glBindAttribLocation(program, index, name) {
 GLctx.bindAttribLocation(GL.programs[program], index, UTF8ToString(name));
}

function _glLinkProgram(program) {
 program = GL.programs[program];
 GLctx.linkProgram(program);
 program.uniformLocsById = 0;
 program.uniformSizeAndIdsByName = {};
}

function _glBindBuffer(target, buffer) {
 if (target == 34962) {
  GLctx.currentArrayBufferBinding = buffer;
  GLImmediate.lastArrayBuffer = buffer;
 } else if (target == 34963) {
  GLctx.currentElementArrayBufferBinding = buffer;
 }
 if (target == 35051) {
  GLctx.currentPixelPackBufferBinding = buffer;
 } else if (target == 35052) {
  GLctx.currentPixelUnpackBufferBinding = buffer;
 }
 GLctx.bindBuffer(target, GL.buffers[buffer]);
}

function _glGetFloatv(name_, p) {
 emscriptenWebGLGet(name_, p, 2);
}

function _glHint(x0, x1) {
 GLctx["hint"](x0, x1);
}

function _glEnableVertexAttribArray(index) {
 GLctx.enableVertexAttribArray(index);
}

function _glDisableVertexAttribArray(index) {
 GLctx.disableVertexAttribArray(index);
}

function _glVertexAttribPointer(index, size, type, normalized, stride, ptr) {
 GLctx.vertexAttribPointer(index, size, type, !!normalized, stride, ptr);
}

function _glActiveTexture(x0) {
 GLctx["activeTexture"](x0);
}

var GLEmulation = {
 fogStart: 0,
 fogEnd: 1,
 fogDensity: 1,
 fogColor: null,
 fogMode: 2048,
 fogEnabled: false,
 MAX_CLIP_PLANES: 6,
 clipPlaneEnabled: [ false, false, false, false, false, false ],
 clipPlaneEquation: [],
 lightingEnabled: false,
 lightModelAmbient: null,
 lightModelLocalViewer: false,
 lightModelTwoSide: false,
 materialAmbient: null,
 materialDiffuse: null,
 materialSpecular: null,
 materialShininess: null,
 materialEmission: null,
 MAX_LIGHTS: 8,
 lightEnabled: [ false, false, false, false, false, false, false, false ],
 lightAmbient: [],
 lightDiffuse: [],
 lightSpecular: [],
 lightPosition: [],
 pointSize: 1,
 vaos: [],
 currentVao: null,
 enabledVertexAttribArrays: {},
 hasRunInit: false,
 findToken: function(source, token) {
  function isIdentChar(ch) {
   if (ch >= 48 && ch <= 57) return true;
   if (ch >= 65 && ch <= 90) return true;
   if (ch >= 97 && ch <= 122) return true;
   return false;
  }
  var i = -1;
  do {
   i = source.indexOf(token, i + 1);
   if (i < 0) {
    break;
   }
   if (i > 0 && isIdentChar(source[i - 1])) {
    continue;
   }
   i += token.length;
   if (i < source.length - 1 && isIdentChar(source[i + 1])) {
    continue;
   }
   return true;
  } while (true);
  return false;
 },
 init: function() {
  if (GLEmulation.hasRunInit) {
   return;
  }
  GLEmulation.hasRunInit = true;
  GLEmulation.fogColor = new Float32Array(4);
  for (var clipPlaneId = 0; clipPlaneId < GLEmulation.MAX_CLIP_PLANES; clipPlaneId++) {
   GLEmulation.clipPlaneEquation[clipPlaneId] = new Float32Array(4);
  }
  GLEmulation.lightModelAmbient = new Float32Array([ .2, .2, .2, 1 ]);
  GLEmulation.materialAmbient = new Float32Array([ .2, .2, .2, 1 ]);
  GLEmulation.materialDiffuse = new Float32Array([ .8, .8, .8, 1 ]);
  GLEmulation.materialSpecular = new Float32Array([ 0, 0, 0, 1 ]);
  GLEmulation.materialShininess = new Float32Array([ 0 ]);
  GLEmulation.materialEmission = new Float32Array([ 0, 0, 0, 1 ]);
  for (var lightId = 0; lightId < GLEmulation.MAX_LIGHTS; lightId++) {
   GLEmulation.lightAmbient[lightId] = new Float32Array([ 0, 0, 0, 1 ]);
   GLEmulation.lightDiffuse[lightId] = lightId ? new Float32Array([ 0, 0, 0, 1 ]) : new Float32Array([ 1, 1, 1, 1 ]);
   GLEmulation.lightSpecular[lightId] = lightId ? new Float32Array([ 0, 0, 0, 1 ]) : new Float32Array([ 1, 1, 1, 1 ]);
   GLEmulation.lightPosition[lightId] = new Float32Array([ 0, 0, 1, 0 ]);
  }
  err("WARNING: using emscripten GL emulation. This is a collection of limited workarounds, do not expect it to work.");
  err("WARNING: using emscripten GL emulation unsafe opts. If weirdness happens, try -s GL_UNSAFE_OPTS=0");
  var validCapabilities = {
   2884: 1,
   3042: 1,
   3024: 1,
   2960: 1,
   2929: 1,
   3089: 1,
   32823: 1,
   32926: 1,
   32928: 1
  };
  var glEnable = _glEnable;
  _glEnable = _emscripten_glEnable = function _glEnable(cap) {
   if (GLImmediate.lastRenderer) GLImmediate.lastRenderer.cleanup();
   if (cap == 2912) {
    if (GLEmulation.fogEnabled != true) {
     GLImmediate.currentRenderer = null;
     GLEmulation.fogEnabled = true;
    }
    return;
   } else if (cap >= 12288 && cap < 12294) {
    var clipPlaneId = cap - 12288;
    if (GLEmulation.clipPlaneEnabled[clipPlaneId] != true) {
     GLImmediate.currentRenderer = null;
     GLEmulation.clipPlaneEnabled[clipPlaneId] = true;
    }
    return;
   } else if (cap >= 16384 && cap < 16392) {
    var lightId = cap - 16384;
    if (GLEmulation.lightEnabled[lightId] != true) {
     GLImmediate.currentRenderer = null;
     GLEmulation.lightEnabled[lightId] = true;
    }
    return;
   } else if (cap == 2896) {
    if (GLEmulation.lightingEnabled != true) {
     GLImmediate.currentRenderer = null;
     GLEmulation.lightingEnabled = true;
    }
    return;
   } else if (cap == 3553) {
    return;
   } else if (!(cap in validCapabilities)) {
    return;
   }
   glEnable(cap);
  };
  var glDisable = _glDisable;
  _glDisable = _emscripten_glDisable = function _glDisable(cap) {
   if (GLImmediate.lastRenderer) GLImmediate.lastRenderer.cleanup();
   if (cap == 2912) {
    if (GLEmulation.fogEnabled != false) {
     GLImmediate.currentRenderer = null;
     GLEmulation.fogEnabled = false;
    }
    return;
   } else if (cap >= 12288 && cap < 12294) {
    var clipPlaneId = cap - 12288;
    if (GLEmulation.clipPlaneEnabled[clipPlaneId] != false) {
     GLImmediate.currentRenderer = null;
     GLEmulation.clipPlaneEnabled[clipPlaneId] = false;
    }
    return;
   } else if (cap >= 16384 && cap < 16392) {
    var lightId = cap - 16384;
    if (GLEmulation.lightEnabled[lightId] != false) {
     GLImmediate.currentRenderer = null;
     GLEmulation.lightEnabled[lightId] = false;
    }
    return;
   } else if (cap == 2896) {
    if (GLEmulation.lightingEnabled != false) {
     GLImmediate.currentRenderer = null;
     GLEmulation.lightingEnabled = false;
    }
    return;
   } else if (cap == 3553) {
    return;
   } else if (!(cap in validCapabilities)) {
    return;
   }
   glDisable(cap);
  };
  _glIsEnabled = _emscripten_glIsEnabled = function _glIsEnabled(cap) {
   if (cap == 2912) {
    return GLEmulation.fogEnabled ? 1 : 0;
   } else if (cap >= 12288 && cap < 12294) {
    var clipPlaneId = cap - 12288;
    return GLEmulation.clipPlaneEnabled[clipPlaneId] ? 1 : 0;
   } else if (cap >= 16384 && cap < 16392) {
    var lightId = cap - 16384;
    return GLEmulation.lightEnabled[lightId] ? 1 : 0;
   } else if (cap == 2896) {
    return GLEmulation.lightingEnabled ? 1 : 0;
   } else if (!(cap in validCapabilities)) {
    return 0;
   }
   return GLctx.isEnabled(cap);
  };
  var glGetBooleanv = _glGetBooleanv;
  _glGetBooleanv = _emscripten_glGetBooleanv = function _glGetBooleanv(pname, p) {
   var attrib = GLEmulation.getAttributeFromCapability(pname);
   if (attrib !== null) {
    var result = GLImmediate.enabledClientAttributes[attrib];
    HEAP8[p >> 0] = result === true ? 1 : 0;
    return;
   }
   glGetBooleanv(pname, p);
  };
  var glGetIntegerv = _glGetIntegerv;
  _glGetIntegerv = _emscripten_glGetIntegerv = function _glGetIntegerv(pname, params) {
   switch (pname) {
   case 34018:
    pname = GLctx.MAX_TEXTURE_IMAGE_UNITS;
    break;

   case 35658:
    {
     var result = GLctx.getParameter(GLctx.MAX_VERTEX_UNIFORM_VECTORS);
     HEAP32[params >> 2] = result * 4;
     return;
    }

   case 35657:
    {
     var result = GLctx.getParameter(GLctx.MAX_FRAGMENT_UNIFORM_VECTORS);
     HEAP32[params >> 2] = result * 4;
     return;
    }

   case 35659:
    {
     var result = GLctx.getParameter(GLctx.MAX_VARYING_VECTORS);
     HEAP32[params >> 2] = result * 4;
     return;
    }

   case 34929:
    pname = GLctx.MAX_COMBINED_TEXTURE_IMAGE_UNITS;
    break;

   case 32890:
    {
     var attribute = GLImmediate.clientAttributes[GLImmediate.VERTEX];
     HEAP32[params >> 2] = attribute ? attribute.size : 0;
     return;
    }

   case 32891:
    {
     var attribute = GLImmediate.clientAttributes[GLImmediate.VERTEX];
     HEAP32[params >> 2] = attribute ? attribute.type : 0;
     return;
    }

   case 32892:
    {
     var attribute = GLImmediate.clientAttributes[GLImmediate.VERTEX];
     HEAP32[params >> 2] = attribute ? attribute.stride : 0;
     return;
    }

   case 32897:
    {
     var attribute = GLImmediate.clientAttributes[GLImmediate.COLOR];
     HEAP32[params >> 2] = attribute ? attribute.size : 0;
     return;
    }

   case 32898:
    {
     var attribute = GLImmediate.clientAttributes[GLImmediate.COLOR];
     HEAP32[params >> 2] = attribute ? attribute.type : 0;
     return;
    }

   case 32899:
    {
     var attribute = GLImmediate.clientAttributes[GLImmediate.COLOR];
     HEAP32[params >> 2] = attribute ? attribute.stride : 0;
     return;
    }

   case 32904:
    {
     var attribute = GLImmediate.clientAttributes[GLImmediate.TEXTURE0 + GLImmediate.clientActiveTexture];
     HEAP32[params >> 2] = attribute ? attribute.size : 0;
     return;
    }

   case 32905:
    {
     var attribute = GLImmediate.clientAttributes[GLImmediate.TEXTURE0 + GLImmediate.clientActiveTexture];
     HEAP32[params >> 2] = attribute ? attribute.type : 0;
     return;
    }

   case 32906:
    {
     var attribute = GLImmediate.clientAttributes[GLImmediate.TEXTURE0 + GLImmediate.clientActiveTexture];
     HEAP32[params >> 2] = attribute ? attribute.stride : 0;
     return;
    }

   case 3378:
    {
     HEAP32[params >> 2] = GLEmulation.MAX_CLIP_PLANES;
     return;
    }

   case 2976:
    {
     HEAP32[params >> 2] = GLImmediate.currentMatrix + 5888;
     return;
    }
   }
   glGetIntegerv(pname, params);
  };
  var glGetString = _glGetString;
  _glGetString = _emscripten_glGetString = function _glGetString(name_) {
   if (GL.stringCache[name_]) return GL.stringCache[name_];
   switch (name_) {
   case 7939:
    var ret = stringToNewUTF8((GLctx.getSupportedExtensions() || []).join(" ") + " GL_EXT_texture_env_combine GL_ARB_texture_env_crossbar GL_ATI_texture_env_combine3 GL_NV_texture_env_combine4 GL_EXT_texture_env_dot3 GL_ARB_multitexture GL_ARB_vertex_buffer_object GL_EXT_framebuffer_object GL_ARB_vertex_program GL_ARB_fragment_program GL_ARB_shading_language_100 GL_ARB_shader_objects GL_ARB_vertex_shader GL_ARB_fragment_shader GL_ARB_texture_cube_map GL_EXT_draw_range_elements" + (GL.currentContext.compressionExt ? " GL_ARB_texture_compression GL_EXT_texture_compression_s3tc" : "") + (GL.currentContext.anisotropicExt ? " GL_EXT_texture_filter_anisotropic" : ""));
    GL.stringCache[name_] = ret;
    return ret;
   }
   return glGetString(name_);
  };
  GL.shaderInfos = {};
  var glCreateShader = _glCreateShader;
  _glCreateShader = _emscripten_glCreateShader = function _glCreateShader(shaderType) {
   var id = glCreateShader(shaderType);
   GL.shaderInfos[id] = {
    type: shaderType,
    ftransform: false
   };
   return id;
  };
  function ensurePrecision(source) {
   if (!/precision +(low|medium|high)p +float *;/.test(source)) {
    source = "#ifdef GL_FRAGMENT_PRECISION_HIGH\nprecision highp float;\n#else\nprecision mediump float;\n#endif\n" + source;
   }
   return source;
  }
  _glShaderSource = _emscripten_glShaderSource = function _glShaderSource(shader, count, string, length) {
   var source = GL.getSource(shader, count, string, length);
   if (GL.shaderInfos[shader].type == GLctx.VERTEX_SHADER) {
    var has_pm = source.search(/u_projection/) >= 0;
    var has_mm = source.search(/u_modelView/) >= 0;
    var has_pv = source.search(/a_position/) >= 0;
    var need_pm = 0, need_mm = 0, need_pv = 0;
    var old = source;
    source = source.replace(/ftransform\(\)/g, "(u_projection * u_modelView * a_position)");
    if (old != source) need_pm = need_mm = need_pv = 1;
    old = source;
    source = source.replace(/gl_ProjectionMatrix/g, "u_projection");
    if (old != source) need_pm = 1;
    old = source;
    source = source.replace(/gl_ModelViewMatrixTranspose\[2\]/g, "vec4(u_modelView[0][2], u_modelView[1][2], u_modelView[2][2], u_modelView[3][2])");
    if (old != source) need_mm = 1;
    old = source;
    source = source.replace(/gl_ModelViewMatrix/g, "u_modelView");
    if (old != source) need_mm = 1;
    old = source;
    source = source.replace(/gl_Vertex/g, "a_position");
    if (old != source) need_pv = 1;
    old = source;
    source = source.replace(/gl_ModelViewProjectionMatrix/g, "(u_projection * u_modelView)");
    if (old != source) need_pm = need_mm = 1;
    if (need_pv && !has_pv) source = "attribute vec4 a_position; \n" + source;
    if (need_mm && !has_mm) source = "uniform mat4 u_modelView; \n" + source;
    if (need_pm && !has_pm) source = "uniform mat4 u_projection; \n" + source;
    GL.shaderInfos[shader].ftransform = need_pm || need_mm || need_pv;
    for (var i = 0; i < GLImmediate.MAX_TEXTURES; i++) {
     old = source;
     var need_vtc = source.search("v_texCoord" + i) == -1;
     source = source.replace(new RegExp("gl_TexCoord\\[" + i + "\\]", "g"), "v_texCoord" + i).replace(new RegExp("gl_MultiTexCoord" + i, "g"), "a_texCoord" + i);
     if (source != old) {
      source = "attribute vec4 a_texCoord" + i + "; \n" + source;
      if (need_vtc) {
       source = "varying vec4 v_texCoord" + i + ";   \n" + source;
      }
     }
     old = source;
     source = source.replace(new RegExp("gl_TextureMatrix\\[" + i + "\\]", "g"), "u_textureMatrix" + i);
     if (source != old) {
      source = "uniform mat4 u_textureMatrix" + i + "; \n" + source;
     }
    }
    if (source.includes("gl_FrontColor")) {
     source = "varying vec4 v_color; \n" + source.replace(/gl_FrontColor/g, "v_color");
    }
    if (source.includes("gl_Color")) {
     source = "attribute vec4 a_color; \n" + source.replace(/gl_Color/g, "a_color");
    }
    if (source.includes("gl_Normal")) {
     source = "attribute vec3 a_normal; \n" + source.replace(/gl_Normal/g, "a_normal");
    }
    if (source.includes("gl_FogFragCoord")) {
     source = "varying float v_fogFragCoord;   \n" + source.replace(/gl_FogFragCoord/g, "v_fogFragCoord");
    }
   } else {
    for (i = 0; i < GLImmediate.MAX_TEXTURES; i++) {
     old = source;
     source = source.replace(new RegExp("gl_TexCoord\\[" + i + "\\]", "g"), "v_texCoord" + i);
     if (source != old) {
      source = "varying vec4 v_texCoord" + i + ";   \n" + source;
     }
    }
    if (source.includes("gl_Color")) {
     source = "varying vec4 v_color; \n" + source.replace(/gl_Color/g, "v_color");
    }
    if (source.includes("gl_Fog.color")) {
     source = "uniform vec4 u_fogColor;   \n" + source.replace(/gl_Fog.color/g, "u_fogColor");
    }
    if (source.includes("gl_Fog.end")) {
     source = "uniform float u_fogEnd;   \n" + source.replace(/gl_Fog.end/g, "u_fogEnd");
    }
    if (source.includes("gl_Fog.scale")) {
     source = "uniform float u_fogScale;   \n" + source.replace(/gl_Fog.scale/g, "u_fogScale");
    }
    if (source.includes("gl_Fog.density")) {
     source = "uniform float u_fogDensity;   \n" + source.replace(/gl_Fog.density/g, "u_fogDensity");
    }
    if (source.includes("gl_FogFragCoord")) {
     source = "varying float v_fogFragCoord;   \n" + source.replace(/gl_FogFragCoord/g, "v_fogFragCoord");
    }
    source = ensurePrecision(source);
   }
   GLctx.shaderSource(GL.shaders[shader], source);
  };
  _glCompileShader = _emscripten_glCompileShader = function _glCompileShader(shader) {
   GLctx.compileShader(GL.shaders[shader]);
  };
  GL.programShaders = {};
  var glAttachShader = _glAttachShader;
  _glAttachShader = _emscripten_glAttachShader = function _glAttachShader(program, shader) {
   if (!GL.programShaders[program]) GL.programShaders[program] = [];
   GL.programShaders[program].push(shader);
   glAttachShader(program, shader);
  };
  var glDetachShader = _glDetachShader;
  _glDetachShader = _emscripten_glDetachShader = function _glDetachShader(program, shader) {
   var programShader = GL.programShaders[program];
   if (!programShader) {
    err("WARNING: _glDetachShader received invalid program: " + program);
    return;
   }
   var index = programShader.indexOf(shader);
   programShader.splice(index, 1);
   glDetachShader(program, shader);
  };
  var glUseProgram = _glUseProgram;
  _glUseProgram = _emscripten_glUseProgram = function _glUseProgram(program) {
   if (GL.currProgram != program) {
    GLImmediate.currentRenderer = null;
    GL.currProgram = program;
    GLImmediate.fixedFunctionProgram = 0;
    glUseProgram(program);
   }
  };
  var glDeleteProgram = _glDeleteProgram;
  _glDeleteProgram = _emscripten_glDeleteProgram = function _glDeleteProgram(program) {
   glDeleteProgram(program);
   if (program == GL.currProgram) {
    GLImmediate.currentRenderer = null;
    GL.currProgram = 0;
   }
  };
  var zeroUsedPrograms = {};
  var glBindAttribLocation = _glBindAttribLocation;
  _glBindAttribLocation = _emscripten_glBindAttribLocation = function _glBindAttribLocation(program, index, name) {
   if (index == 0) zeroUsedPrograms[program] = true;
   glBindAttribLocation(program, index, name);
  };
  var glLinkProgram = _glLinkProgram;
  _glLinkProgram = _emscripten_glLinkProgram = function _glLinkProgram(program) {
   if (!(program in zeroUsedPrograms)) {
    GLctx.bindAttribLocation(GL.programs[program], 0, "a_position");
   }
   glLinkProgram(program);
  };
  var glBindBuffer = _glBindBuffer;
  _glBindBuffer = _emscripten_glBindBuffer = function _glBindBuffer(target, buffer) {
   glBindBuffer(target, buffer);
   if (target == GLctx.ARRAY_BUFFER) {
    if (GLEmulation.currentVao) {
     GLEmulation.currentVao.arrayBuffer = buffer;
    }
   } else if (target == GLctx.ELEMENT_ARRAY_BUFFER) {
    if (GLEmulation.currentVao) GLEmulation.currentVao.elementArrayBuffer = buffer;
   }
  };
  var glGetFloatv = _glGetFloatv;
  _glGetFloatv = _emscripten_glGetFloatv = function _glGetFloatv(pname, params) {
   if (pname == 2982) {
    HEAPF32.set(GLImmediate.matrix[0], params >> 2);
   } else if (pname == 2983) {
    HEAPF32.set(GLImmediate.matrix[1], params >> 2);
   } else if (pname == 2984) {
    HEAPF32.set(GLImmediate.matrix[2 + GLImmediate.clientActiveTexture], params >> 2);
   } else if (pname == 2918) {
    HEAPF32.set(GLEmulation.fogColor, params >> 2);
   } else if (pname == 2915) {
    HEAPF32[params >> 2] = GLEmulation.fogStart;
   } else if (pname == 2916) {
    HEAPF32[params >> 2] = GLEmulation.fogEnd;
   } else if (pname == 2914) {
    HEAPF32[params >> 2] = GLEmulation.fogDensity;
   } else if (pname == 2917) {
    HEAPF32[params >> 2] = GLEmulation.fogMode;
   } else if (pname == 2899) {
    HEAPF32[params >> 2] = GLEmulation.lightModelAmbient[0];
    HEAPF32[params + 4 >> 2] = GLEmulation.lightModelAmbient[1];
    HEAPF32[params + 8 >> 2] = GLEmulation.lightModelAmbient[2];
    HEAPF32[params + 12 >> 2] = GLEmulation.lightModelAmbient[3];
   } else {
    glGetFloatv(pname, params);
   }
  };
  var glHint = _glHint;
  _glHint = _emscripten_glHint = function _glHint(target, mode) {
   if (target == 34031) {
    return;
   }
   glHint(target, mode);
  };
  var glEnableVertexAttribArray = _glEnableVertexAttribArray;
  _glEnableVertexAttribArray = _emscripten_glEnableVertexAttribArray = function _glEnableVertexAttribArray(index) {
   glEnableVertexAttribArray(index);
   GLEmulation.enabledVertexAttribArrays[index] = 1;
   if (GLEmulation.currentVao) GLEmulation.currentVao.enabledVertexAttribArrays[index] = 1;
  };
  var glDisableVertexAttribArray = _glDisableVertexAttribArray;
  _glDisableVertexAttribArray = _emscripten_glDisableVertexAttribArray = function _glDisableVertexAttribArray(index) {
   glDisableVertexAttribArray(index);
   delete GLEmulation.enabledVertexAttribArrays[index];
   if (GLEmulation.currentVao) delete GLEmulation.currentVao.enabledVertexAttribArrays[index];
  };
  var glVertexAttribPointer = _glVertexAttribPointer;
  _glVertexAttribPointer = _emscripten_glVertexAttribPointer = function _glVertexAttribPointer(index, size, type, normalized, stride, pointer) {
   glVertexAttribPointer(index, size, type, normalized, stride, pointer);
   if (GLEmulation.currentVao) {
    GLEmulation.currentVao.vertexAttribPointers[index] = [ index, size, type, normalized, stride, pointer ];
   }
  };
 },
 getAttributeFromCapability: function(cap) {
  var attrib = null;
  switch (cap) {
  case 3553:
  case 32888:
   attrib = GLImmediate.TEXTURE0 + GLImmediate.clientActiveTexture;
   break;

  case 32884:
   attrib = GLImmediate.VERTEX;
   break;

  case 32885:
   attrib = GLImmediate.NORMAL;
   break;

  case 32886:
   attrib = GLImmediate.COLOR;
   break;
  }
  return attrib;
 }
};

var GLImmediate = {
 MapTreeLib: null,
 spawnMapTreeLib: function() {
  function CNaiveListMap() {
   var list = [];
   this.insert = function CNaiveListMap_insert(key, val) {
    if (this.contains(key | 0)) return false;
    list.push([ key, val ]);
    return true;
   };
   var __contains_i;
   this.contains = function CNaiveListMap_contains(key) {
    for (__contains_i = 0; __contains_i < list.length; ++__contains_i) {
     if (list[__contains_i][0] === key) return true;
    }
    return false;
   };
   var __get_i;
   this.get = function CNaiveListMap_get(key) {
    for (__get_i = 0; __get_i < list.length; ++__get_i) {
     if (list[__get_i][0] === key) return list[__get_i][1];
    }
    return undefined;
   };
  }
  function CMapTree() {
   function CNLNode() {
    var map = new CNaiveListMap();
    this.child = function CNLNode_child(keyFrag) {
     if (!map.contains(keyFrag | 0)) {
      map.insert(keyFrag | 0, new CNLNode());
     }
     return map.get(keyFrag | 0);
    };
    this.value = undefined;
    this.get = function CNLNode_get() {
     return this.value;
    };
    this.set = function CNLNode_set(val) {
     this.value = val;
    };
   }
   function CKeyView(root) {
    var cur;
    this.reset = function CKeyView_reset() {
     cur = root;
     return this;
    };
    this.reset();
    this.next = function CKeyView_next(keyFrag) {
     cur = cur.child(keyFrag);
     return this;
    };
    this.get = function CKeyView_get() {
     return cur.get();
    };
    this.set = function CKeyView_set(val) {
     cur.set(val);
    };
   }
   var root;
   var staticKeyView;
   this.createKeyView = function CNLNode_createKeyView() {
    return new CKeyView(root);
   };
   this.clear = function CNLNode_clear() {
    root = new CNLNode();
    staticKeyView = this.createKeyView();
   };
   this.clear();
   this.getStaticKeyView = function CNLNode_getStaticKeyView() {
    staticKeyView.reset();
    return staticKeyView;
   };
  }
  return {
   create: function() {
    return new CMapTree();
   }
  };
 },
 TexEnvJIT: null,
 spawnTexEnvJIT: function() {
  var GL_TEXTURE0 = 33984;
  var GL_TEXTURE_1D = 3552;
  var GL_TEXTURE_2D = 3553;
  var GL_TEXTURE_3D = 32879;
  var GL_TEXTURE_CUBE_MAP = 34067;
  var GL_TEXTURE_ENV = 8960;
  var GL_TEXTURE_ENV_MODE = 8704;
  var GL_TEXTURE_ENV_COLOR = 8705;
  var GL_SRC0_RGB = 34176;
  var GL_SRC1_RGB = 34177;
  var GL_SRC2_RGB = 34178;
  var GL_SRC0_ALPHA = 34184;
  var GL_SRC1_ALPHA = 34185;
  var GL_SRC2_ALPHA = 34186;
  var GL_OPERAND0_RGB = 34192;
  var GL_OPERAND1_RGB = 34193;
  var GL_OPERAND2_RGB = 34194;
  var GL_OPERAND0_ALPHA = 34200;
  var GL_OPERAND1_ALPHA = 34201;
  var GL_OPERAND2_ALPHA = 34202;
  var GL_COMBINE_RGB = 34161;
  var GL_COMBINE_ALPHA = 34162;
  var GL_RGB_SCALE = 34163;
  var GL_ALPHA_SCALE = 3356;
  var GL_ADD = 260;
  var GL_BLEND = 3042;
  var GL_REPLACE = 7681;
  var GL_MODULATE = 8448;
  var GL_DECAL = 8449;
  var GL_COMBINE = 34160;
  var GL_SUBTRACT = 34023;
  var GL_INTERPOLATE = 34165;
  var GL_TEXTURE = 5890;
  var GL_CONSTANT = 34166;
  var GL_PRIMARY_COLOR = 34167;
  var GL_PREVIOUS = 34168;
  var GL_SRC_COLOR = 768;
  var GL_ONE_MINUS_SRC_COLOR = 769;
  var GL_SRC_ALPHA = 770;
  var GL_ONE_MINUS_SRC_ALPHA = 771;
  var TEXENVJIT_NAMESPACE_PREFIX = "tej_";
  var TEX_UNIT_UNIFORM_PREFIX = "uTexUnit";
  var TEX_COORD_VARYING_PREFIX = "vTexCoord";
  var PRIM_COLOR_VARYING = "vPrimColor";
  var TEX_MATRIX_UNIFORM_PREFIX = "uTexMatrix";
  var s_texUnits = null;
  var s_activeTexture = 0;
  var s_requiredTexUnitsForPass = [];
  function abort(info) {
   assert(false, "[TexEnvJIT] ABORT: " + info);
  }
  function abort_noSupport(info) {
   abort("No support: " + info);
  }
  function abort_sanity(info) {
   abort("Sanity failure: " + info);
  }
  function genTexUnitSampleExpr(texUnitID) {
   var texUnit = s_texUnits[texUnitID];
   var texType = texUnit.getTexType();
   var func = null;
   switch (texType) {
   case GL_TEXTURE_1D:
    func = "texture2D";
    break;

   case GL_TEXTURE_2D:
    func = "texture2D";
    break;

   case GL_TEXTURE_3D:
    return abort_noSupport("No support for 3D textures.");

   case GL_TEXTURE_CUBE_MAP:
    func = "textureCube";
    break;

   default:
    return abort_sanity("Unknown texType: 0x" + texType.toString(16));
   }
   var texCoordExpr = TEX_COORD_VARYING_PREFIX + texUnitID;
   if (TEX_MATRIX_UNIFORM_PREFIX != null) {
    texCoordExpr = "(" + TEX_MATRIX_UNIFORM_PREFIX + texUnitID + " * " + texCoordExpr + ")";
   }
   return func + "(" + TEX_UNIT_UNIFORM_PREFIX + texUnitID + ", " + texCoordExpr + ".xy)";
  }
  function getTypeFromCombineOp(op) {
   switch (op) {
   case GL_SRC_COLOR:
   case GL_ONE_MINUS_SRC_COLOR:
    return "vec3";

   case GL_SRC_ALPHA:
   case GL_ONE_MINUS_SRC_ALPHA:
    return "float";
   }
   return abort_noSupport("Unsupported combiner op: 0x" + op.toString(16));
  }
  function getCurTexUnit() {
   return s_texUnits[s_activeTexture];
  }
  function genCombinerSourceExpr(texUnitID, constantExpr, previousVar, src, op) {
   var srcExpr = null;
   switch (src) {
   case GL_TEXTURE:
    srcExpr = genTexUnitSampleExpr(texUnitID);
    break;

   case GL_CONSTANT:
    srcExpr = constantExpr;
    break;

   case GL_PRIMARY_COLOR:
    srcExpr = PRIM_COLOR_VARYING;
    break;

   case GL_PREVIOUS:
    srcExpr = previousVar;
    break;

   default:
    return abort_noSupport("Unsupported combiner src: 0x" + src.toString(16));
   }
   var expr = null;
   switch (op) {
   case GL_SRC_COLOR:
    expr = srcExpr + ".rgb";
    break;

   case GL_ONE_MINUS_SRC_COLOR:
    expr = "(vec3(1.0) - " + srcExpr + ".rgb)";
    break;

   case GL_SRC_ALPHA:
    expr = srcExpr + ".a";
    break;

   case GL_ONE_MINUS_SRC_ALPHA:
    expr = "(1.0 - " + srcExpr + ".a)";
    break;

   default:
    return abort_noSupport("Unsupported combiner op: 0x" + op.toString(16));
   }
   return expr;
  }
  function valToFloatLiteral(val) {
   if (val == Math.round(val)) return val + ".0";
   return val;
  }
  function CTexEnv() {
   this.mode = GL_MODULATE;
   this.colorCombiner = GL_MODULATE;
   this.alphaCombiner = GL_MODULATE;
   this.colorScale = 1;
   this.alphaScale = 1;
   this.envColor = [ 0, 0, 0, 0 ];
   this.colorSrc = [ GL_TEXTURE, GL_PREVIOUS, GL_CONSTANT ];
   this.alphaSrc = [ GL_TEXTURE, GL_PREVIOUS, GL_CONSTANT ];
   this.colorOp = [ GL_SRC_COLOR, GL_SRC_COLOR, GL_SRC_ALPHA ];
   this.alphaOp = [ GL_SRC_ALPHA, GL_SRC_ALPHA, GL_SRC_ALPHA ];
   this.traverseKey = {
    7681: 0,
    8448: 1,
    260: 2,
    3042: 3,
    8449: 4,
    34160: 5,
    34023: 3,
    34165: 4,
    5890: 0,
    34166: 1,
    34167: 2,
    34168: 3,
    768: 0,
    769: 1,
    770: 2,
    771: 3
   };
   this.key0 = -1;
   this.key1 = 0;
   this.key2 = 0;
   this.computeKey0 = function() {
    var k = this.traverseKey;
    var key = k[this.mode] * 1638400;
    key += k[this.colorCombiner] * 327680;
    key += k[this.alphaCombiner] * 65536;
    key += (this.colorScale - 1) * 16384;
    key += (this.alphaScale - 1) * 4096;
    key += k[this.colorSrc[0]] * 1024;
    key += k[this.colorSrc[1]] * 256;
    key += k[this.colorSrc[2]] * 64;
    key += k[this.alphaSrc[0]] * 16;
    key += k[this.alphaSrc[1]] * 4;
    key += k[this.alphaSrc[2]];
    return key;
   };
   this.computeKey1 = function() {
    var k = this.traverseKey;
    key = k[this.colorOp[0]] * 4096;
    key += k[this.colorOp[1]] * 1024;
    key += k[this.colorOp[2]] * 256;
    key += k[this.alphaOp[0]] * 16;
    key += k[this.alphaOp[1]] * 4;
    key += k[this.alphaOp[2]];
    return key;
   };
   this.computeKey2 = function() {
    return this.envColor[0] * 16777216 + this.envColor[1] * 65536 + this.envColor[2] * 256 + 1 + this.envColor[3];
   };
   this.recomputeKey = function() {
    this.key0 = this.computeKey0();
    this.key1 = this.computeKey1();
    this.key2 = this.computeKey2();
   };
   this.invalidateKey = function() {
    this.key0 = -1;
    GLImmediate.currentRenderer = null;
   };
  }
  function CTexUnit() {
   this.env = new CTexEnv();
   this.enabled_tex1D = false;
   this.enabled_tex2D = false;
   this.enabled_tex3D = false;
   this.enabled_texCube = false;
   this.texTypesEnabled = 0;
   this.traverseState = function CTexUnit_traverseState(keyView) {
    if (this.texTypesEnabled) {
     if (this.env.key0 == -1) {
      this.env.recomputeKey();
     }
     keyView.next(this.texTypesEnabled | this.env.key0 << 4);
     keyView.next(this.env.key1);
     keyView.next(this.env.key2);
    } else {
     keyView.next(0);
    }
   };
  }
  CTexUnit.prototype.enabled = function CTexUnit_enabled() {
   return this.texTypesEnabled;
  };
  CTexUnit.prototype.genPassLines = function CTexUnit_genPassLines(passOutputVar, passInputVar, texUnitID) {
   if (!this.enabled()) {
    return [ "vec4 " + passOutputVar + " = " + passInputVar + ";" ];
   }
   var lines = this.env.genPassLines(passOutputVar, passInputVar, texUnitID).join("\n");
   var texLoadLines = "";
   var texLoadRegex = /(texture.*?\(.*?\))/g;
   var loadCounter = 0;
   var load;
   while (load = texLoadRegex.exec(lines)) {
    var texLoadExpr = load[1];
    var secondOccurrence = lines.slice(load.index + 1).indexOf(texLoadExpr);
    if (secondOccurrence != -1) {
     var prefix = TEXENVJIT_NAMESPACE_PREFIX + "env" + texUnitID + "_";
     var texLoadVar = prefix + "texload" + loadCounter++;
     var texLoadLine = "vec4 " + texLoadVar + " = " + texLoadExpr + ";\n";
     texLoadLines += texLoadLine + "\n";
     lines = lines.split(texLoadExpr).join(texLoadVar);
     texLoadRegex = /(texture.*\(.*\))/g;
    }
   }
   return [ texLoadLines + lines ];
  };
  CTexUnit.prototype.getTexType = function CTexUnit_getTexType() {
   if (this.enabled_texCube) {
    return GL_TEXTURE_CUBE_MAP;
   } else if (this.enabled_tex3D) {
    return GL_TEXTURE_3D;
   } else if (this.enabled_tex2D) {
    return GL_TEXTURE_2D;
   } else if (this.enabled_tex1D) {
    return GL_TEXTURE_1D;
   }
   return 0;
  };
  CTexEnv.prototype.genPassLines = function CTexEnv_genPassLines(passOutputVar, passInputVar, texUnitID) {
   switch (this.mode) {
   case GL_REPLACE:
    {
     return [ "vec4 " + passOutputVar + " = " + genTexUnitSampleExpr(texUnitID) + ";" ];
    }

   case GL_ADD:
    {
     var prefix = TEXENVJIT_NAMESPACE_PREFIX + "env" + texUnitID + "_";
     var texVar = prefix + "tex";
     var colorVar = prefix + "color";
     var alphaVar = prefix + "alpha";
     return [ "vec4 " + texVar + " = " + genTexUnitSampleExpr(texUnitID) + ";", "vec3 " + colorVar + " = " + passInputVar + ".rgb + " + texVar + ".rgb;", "float " + alphaVar + " = " + passInputVar + ".a * " + texVar + ".a;", "vec4 " + passOutputVar + " = vec4(" + colorVar + ", " + alphaVar + ");" ];
    }

   case GL_MODULATE:
    {
     var line = [ "vec4 " + passOutputVar, " = ", passInputVar, " * ", genTexUnitSampleExpr(texUnitID), ";" ];
     return [ line.join("") ];
    }

   case GL_DECAL:
    {
     var prefix = TEXENVJIT_NAMESPACE_PREFIX + "env" + texUnitID + "_";
     var texVar = prefix + "tex";
     var colorVar = prefix + "color";
     var alphaVar = prefix + "alpha";
     return [ "vec4 " + texVar + " = " + genTexUnitSampleExpr(texUnitID) + ";", [ "vec3 " + colorVar + " = ", passInputVar + ".rgb * (1.0 - " + texVar + ".a)", " + ", texVar + ".rgb * " + texVar + ".a", ";" ].join(""), "float " + alphaVar + " = " + passInputVar + ".a;", "vec4 " + passOutputVar + " = vec4(" + colorVar + ", " + alphaVar + ");" ];
    }

   case GL_BLEND:
    {
     var prefix = TEXENVJIT_NAMESPACE_PREFIX + "env" + texUnitID + "_";
     var texVar = prefix + "tex";
     var colorVar = prefix + "color";
     var alphaVar = prefix + "alpha";
     return [ "vec4 " + texVar + " = " + genTexUnitSampleExpr(texUnitID) + ";", [ "vec3 " + colorVar + " = ", passInputVar + ".rgb * (1.0 - " + texVar + ".rgb)", " + ", PRIM_COLOR_VARYING + ".rgb * " + texVar + ".rgb", ";" ].join(""), "float " + alphaVar + " = " + texVar + ".a;", "vec4 " + passOutputVar + " = vec4(" + colorVar + ", " + alphaVar + ");" ];
    }

   case GL_COMBINE:
    {
     var prefix = TEXENVJIT_NAMESPACE_PREFIX + "env" + texUnitID + "_";
     var colorVar = prefix + "color";
     var alphaVar = prefix + "alpha";
     var colorLines = this.genCombinerLines(true, colorVar, passInputVar, texUnitID, this.colorCombiner, this.colorSrc, this.colorOp);
     var alphaLines = this.genCombinerLines(false, alphaVar, passInputVar, texUnitID, this.alphaCombiner, this.alphaSrc, this.alphaOp);
     var scaledColor = this.colorScale == 1 ? colorVar : colorVar + " * " + valToFloatLiteral(this.colorScale);
     var scaledAlpha = this.alphaScale == 1 ? alphaVar : alphaVar + " * " + valToFloatLiteral(this.alphaScale);
     var line = [ "vec4 " + passOutputVar, " = ", "vec4(", scaledColor, ", ", scaledAlpha, ")", ";" ].join("");
     return [].concat(colorLines, alphaLines, [ line ]);
    }
   }
   return abort_noSupport("Unsupported TexEnv mode: 0x" + this.mode.toString(16));
  };
  CTexEnv.prototype.genCombinerLines = function CTexEnv_getCombinerLines(isColor, outputVar, passInputVar, texUnitID, combiner, srcArr, opArr) {
   var argsNeeded = null;
   switch (combiner) {
   case GL_REPLACE:
    argsNeeded = 1;
    break;

   case GL_MODULATE:
   case GL_ADD:
   case GL_SUBTRACT:
    argsNeeded = 2;
    break;

   case GL_INTERPOLATE:
    argsNeeded = 3;
    break;

   default:
    return abort_noSupport("Unsupported combiner: 0x" + combiner.toString(16));
   }
   var constantExpr = [ "vec4(", valToFloatLiteral(this.envColor[0]), ", ", valToFloatLiteral(this.envColor[1]), ", ", valToFloatLiteral(this.envColor[2]), ", ", valToFloatLiteral(this.envColor[3]), ")" ].join("");
   var src0Expr = argsNeeded >= 1 ? genCombinerSourceExpr(texUnitID, constantExpr, passInputVar, srcArr[0], opArr[0]) : null;
   var src1Expr = argsNeeded >= 2 ? genCombinerSourceExpr(texUnitID, constantExpr, passInputVar, srcArr[1], opArr[1]) : null;
   var src2Expr = argsNeeded >= 3 ? genCombinerSourceExpr(texUnitID, constantExpr, passInputVar, srcArr[2], opArr[2]) : null;
   var outputType = isColor ? "vec3" : "float";
   var lines = null;
   switch (combiner) {
   case GL_REPLACE:
    {
     var line = [ outputType + " " + outputVar, " = ", src0Expr, ";" ];
     lines = [ line.join("") ];
     break;
    }

   case GL_MODULATE:
    {
     var line = [ outputType + " " + outputVar + " = ", src0Expr + " * " + src1Expr, ";" ];
     lines = [ line.join("") ];
     break;
    }

   case GL_ADD:
    {
     var line = [ outputType + " " + outputVar + " = ", src0Expr + " + " + src1Expr, ";" ];
     lines = [ line.join("") ];
     break;
    }

   case GL_SUBTRACT:
    {
     var line = [ outputType + " " + outputVar + " = ", src0Expr + " - " + src1Expr, ";" ];
     lines = [ line.join("") ];
     break;
    }

   case GL_INTERPOLATE:
    {
     var prefix = TEXENVJIT_NAMESPACE_PREFIX + "env" + texUnitID + "_";
     var arg2Var = prefix + "colorSrc2";
     var arg2Line = getTypeFromCombineOp(this.colorOp[2]) + " " + arg2Var + " = " + src2Expr + ";";
     var line = [ outputType + " " + outputVar, " = ", src0Expr + " * " + arg2Var, " + ", src1Expr + " * (1.0 - " + arg2Var + ")", ";" ];
     lines = [ arg2Line, line.join("") ];
     break;
    }

   default:
    return abort_sanity("Unmatched TexEnv.colorCombiner?");
   }
   return lines;
  };
  return {
   init: function(gl, specifiedMaxTextureImageUnits) {
    var maxTexUnits = 0;
    if (specifiedMaxTextureImageUnits) {
     maxTexUnits = specifiedMaxTextureImageUnits;
    } else if (gl) {
     maxTexUnits = gl.getParameter(gl.MAX_TEXTURE_IMAGE_UNITS);
    }
    s_texUnits = [];
    for (var i = 0; i < maxTexUnits; i++) {
     s_texUnits.push(new CTexUnit());
    }
   },
   setGLSLVars: function(uTexUnitPrefix, vTexCoordPrefix, vPrimColor, uTexMatrixPrefix) {
    TEX_UNIT_UNIFORM_PREFIX = uTexUnitPrefix;
    TEX_COORD_VARYING_PREFIX = vTexCoordPrefix;
    PRIM_COLOR_VARYING = vPrimColor;
    TEX_MATRIX_UNIFORM_PREFIX = uTexMatrixPrefix;
   },
   genAllPassLines: function(resultDest, indentSize) {
    indentSize = indentSize || 0;
    s_requiredTexUnitsForPass.length = 0;
    var lines = [];
    var lastPassVar = PRIM_COLOR_VARYING;
    for (var i = 0; i < s_texUnits.length; i++) {
     if (!s_texUnits[i].enabled()) continue;
     s_requiredTexUnitsForPass.push(i);
     var prefix = TEXENVJIT_NAMESPACE_PREFIX + "env" + i + "_";
     var passOutputVar = prefix + "result";
     var newLines = s_texUnits[i].genPassLines(passOutputVar, lastPassVar, i);
     lines = lines.concat(newLines, [ "" ]);
     lastPassVar = passOutputVar;
    }
    lines.push(resultDest + " = " + lastPassVar + ";");
    var indent = "";
    for (var i = 0; i < indentSize; i++) indent += " ";
    var output = indent + lines.join("\n" + indent);
    return output;
   },
   getUsedTexUnitList: function() {
    return s_requiredTexUnitsForPass;
   },
   getActiveTexture: function() {
    return s_activeTexture;
   },
   traverseState: function(keyView) {
    for (var i = 0; i < s_texUnits.length; i++) {
     s_texUnits[i].traverseState(keyView);
    }
   },
   getTexUnitType: function(texUnitID) {
    return s_texUnits[texUnitID].getTexType();
   },
   hook_activeTexture: function(texture) {
    s_activeTexture = texture - GL_TEXTURE0;
    if (GLImmediate.currentMatrix >= 2) {
     GLImmediate.currentMatrix = 2 + s_activeTexture;
    }
   },
   hook_enable: function(cap) {
    var cur = getCurTexUnit();
    switch (cap) {
    case GL_TEXTURE_1D:
     if (!cur.enabled_tex1D) {
      GLImmediate.currentRenderer = null;
      cur.enabled_tex1D = true;
      cur.texTypesEnabled |= 1;
     }
     break;

    case GL_TEXTURE_2D:
     if (!cur.enabled_tex2D) {
      GLImmediate.currentRenderer = null;
      cur.enabled_tex2D = true;
      cur.texTypesEnabled |= 2;
     }
     break;

    case GL_TEXTURE_3D:
     if (!cur.enabled_tex3D) {
      GLImmediate.currentRenderer = null;
      cur.enabled_tex3D = true;
      cur.texTypesEnabled |= 4;
     }
     break;

    case GL_TEXTURE_CUBE_MAP:
     if (!cur.enabled_texCube) {
      GLImmediate.currentRenderer = null;
      cur.enabled_texCube = true;
      cur.texTypesEnabled |= 8;
     }
     break;
    }
   },
   hook_disable: function(cap) {
    var cur = getCurTexUnit();
    switch (cap) {
    case GL_TEXTURE_1D:
     if (cur.enabled_tex1D) {
      GLImmediate.currentRenderer = null;
      cur.enabled_tex1D = false;
      cur.texTypesEnabled &= ~1;
     }
     break;

    case GL_TEXTURE_2D:
     if (cur.enabled_tex2D) {
      GLImmediate.currentRenderer = null;
      cur.enabled_tex2D = false;
      cur.texTypesEnabled &= ~2;
     }
     break;

    case GL_TEXTURE_3D:
     if (cur.enabled_tex3D) {
      GLImmediate.currentRenderer = null;
      cur.enabled_tex3D = false;
      cur.texTypesEnabled &= ~4;
     }
     break;

    case GL_TEXTURE_CUBE_MAP:
     if (cur.enabled_texCube) {
      GLImmediate.currentRenderer = null;
      cur.enabled_texCube = false;
      cur.texTypesEnabled &= ~8;
     }
     break;
    }
   },
   hook_texEnvf: function(target, pname, param) {
    if (target != GL_TEXTURE_ENV) return;
    var env = getCurTexUnit().env;
    switch (pname) {
    case GL_RGB_SCALE:
     if (env.colorScale != param) {
      env.invalidateKey();
      env.colorScale = param;
     }
     break;

    case GL_ALPHA_SCALE:
     if (env.alphaScale != param) {
      env.invalidateKey();
      env.alphaScale = param;
     }
     break;

    default:
     err("WARNING: Unhandled `pname` in call to `glTexEnvf`.");
    }
   },
   hook_texEnvi: function(target, pname, param) {
    if (target != GL_TEXTURE_ENV) return;
    var env = getCurTexUnit().env;
    switch (pname) {
    case GL_TEXTURE_ENV_MODE:
     if (env.mode != param) {
      env.invalidateKey();
      env.mode = param;
     }
     break;

    case GL_COMBINE_RGB:
     if (env.colorCombiner != param) {
      env.invalidateKey();
      env.colorCombiner = param;
     }
     break;

    case GL_COMBINE_ALPHA:
     if (env.alphaCombiner != param) {
      env.invalidateKey();
      env.alphaCombiner = param;
     }
     break;

    case GL_SRC0_RGB:
     if (env.colorSrc[0] != param) {
      env.invalidateKey();
      env.colorSrc[0] = param;
     }
     break;

    case GL_SRC1_RGB:
     if (env.colorSrc[1] != param) {
      env.invalidateKey();
      env.colorSrc[1] = param;
     }
     break;

    case GL_SRC2_RGB:
     if (env.colorSrc[2] != param) {
      env.invalidateKey();
      env.colorSrc[2] = param;
     }
     break;

    case GL_SRC0_ALPHA:
     if (env.alphaSrc[0] != param) {
      env.invalidateKey();
      env.alphaSrc[0] = param;
     }
     break;

    case GL_SRC1_ALPHA:
     if (env.alphaSrc[1] != param) {
      env.invalidateKey();
      env.alphaSrc[1] = param;
     }
     break;

    case GL_SRC2_ALPHA:
     if (env.alphaSrc[2] != param) {
      env.invalidateKey();
      env.alphaSrc[2] = param;
     }
     break;

    case GL_OPERAND0_RGB:
     if (env.colorOp[0] != param) {
      env.invalidateKey();
      env.colorOp[0] = param;
     }
     break;

    case GL_OPERAND1_RGB:
     if (env.colorOp[1] != param) {
      env.invalidateKey();
      env.colorOp[1] = param;
     }
     break;

    case GL_OPERAND2_RGB:
     if (env.colorOp[2] != param) {
      env.invalidateKey();
      env.colorOp[2] = param;
     }
     break;

    case GL_OPERAND0_ALPHA:
     if (env.alphaOp[0] != param) {
      env.invalidateKey();
      env.alphaOp[0] = param;
     }
     break;

    case GL_OPERAND1_ALPHA:
     if (env.alphaOp[1] != param) {
      env.invalidateKey();
      env.alphaOp[1] = param;
     }
     break;

    case GL_OPERAND2_ALPHA:
     if (env.alphaOp[2] != param) {
      env.invalidateKey();
      env.alphaOp[2] = param;
     }
     break;

    case GL_RGB_SCALE:
     if (env.colorScale != param) {
      env.invalidateKey();
      env.colorScale = param;
     }
     break;

    case GL_ALPHA_SCALE:
     if (env.alphaScale != param) {
      env.invalidateKey();
      env.alphaScale = param;
     }
     break;

    default:
     err("WARNING: Unhandled `pname` in call to `glTexEnvi`.");
    }
   },
   hook_texEnvfv: function(target, pname, params) {
    if (target != GL_TEXTURE_ENV) return;
    var env = getCurTexUnit().env;
    switch (pname) {
    case GL_TEXTURE_ENV_COLOR:
     {
      for (var i = 0; i < 4; i++) {
       var param = HEAPF32[params + i * 4 >> 2];
       if (env.envColor[i] != param) {
        env.invalidateKey();
        env.envColor[i] = param;
       }
      }
      break;
     }

    default:
     err("WARNING: Unhandled `pname` in call to `glTexEnvfv`.");
    }
   },
   hook_getTexEnviv: function(target, pname, param) {
    if (target != GL_TEXTURE_ENV) return;
    var env = getCurTexUnit().env;
    switch (pname) {
    case GL_TEXTURE_ENV_MODE:
     HEAP32[param >> 2] = env.mode;
     return;

    case GL_TEXTURE_ENV_COLOR:
     HEAP32[param >> 2] = Math.max(Math.min(env.envColor[0] * 255, 255, -255));
     HEAP32[param + 1 >> 2] = Math.max(Math.min(env.envColor[1] * 255, 255, -255));
     HEAP32[param + 2 >> 2] = Math.max(Math.min(env.envColor[2] * 255, 255, -255));
     HEAP32[param + 3 >> 2] = Math.max(Math.min(env.envColor[3] * 255, 255, -255));
     return;

    case GL_COMBINE_RGB:
     HEAP32[param >> 2] = env.colorCombiner;
     return;

    case GL_COMBINE_ALPHA:
     HEAP32[param >> 2] = env.alphaCombiner;
     return;

    case GL_SRC0_RGB:
     HEAP32[param >> 2] = env.colorSrc[0];
     return;

    case GL_SRC1_RGB:
     HEAP32[param >> 2] = env.colorSrc[1];
     return;

    case GL_SRC2_RGB:
     HEAP32[param >> 2] = env.colorSrc[2];
     return;

    case GL_SRC0_ALPHA:
     HEAP32[param >> 2] = env.alphaSrc[0];
     return;

    case GL_SRC1_ALPHA:
     HEAP32[param >> 2] = env.alphaSrc[1];
     return;

    case GL_SRC2_ALPHA:
     HEAP32[param >> 2] = env.alphaSrc[2];
     return;

    case GL_OPERAND0_RGB:
     HEAP32[param >> 2] = env.colorOp[0];
     return;

    case GL_OPERAND1_RGB:
     HEAP32[param >> 2] = env.colorOp[1];
     return;

    case GL_OPERAND2_RGB:
     HEAP32[param >> 2] = env.colorOp[2];
     return;

    case GL_OPERAND0_ALPHA:
     HEAP32[param >> 2] = env.alphaOp[0];
     return;

    case GL_OPERAND1_ALPHA:
     HEAP32[param >> 2] = env.alphaOp[1];
     return;

    case GL_OPERAND2_ALPHA:
     HEAP32[param >> 2] = env.alphaOp[2];
     return;

    case GL_RGB_SCALE:
     HEAP32[param >> 2] = env.colorScale;
     return;

    case GL_ALPHA_SCALE:
     HEAP32[param >> 2] = env.alphaScale;
     return;

    default:
     err("WARNING: Unhandled `pname` in call to `glGetTexEnvi`.");
    }
   },
   hook_getTexEnvfv: function(target, pname, param) {
    if (target != GL_TEXTURE_ENV) return;
    var env = getCurTexUnit().env;
    switch (pname) {
    case GL_TEXTURE_ENV_COLOR:
     HEAPF32[param >> 2] = env.envColor[0];
     HEAPF32[param + 4 >> 2] = env.envColor[1];
     HEAPF32[param + 8 >> 2] = env.envColor[2];
     HEAPF32[param + 12 >> 2] = env.envColor[3];
     return;
    }
   }
  };
 },
 vertexData: null,
 vertexDataU8: null,
 tempData: null,
 indexData: null,
 vertexCounter: 0,
 mode: -1,
 rendererCache: null,
 rendererComponents: [],
 rendererComponentPointer: 0,
 lastRenderer: null,
 lastArrayBuffer: null,
 lastProgram: null,
 lastStride: -1,
 matrix: [],
 matrixStack: [],
 currentMatrix: 0,
 tempMatrix: null,
 matricesModified: false,
 useTextureMatrix: false,
 VERTEX: 0,
 NORMAL: 1,
 COLOR: 2,
 TEXTURE0: 3,
 NUM_ATTRIBUTES: -1,
 MAX_TEXTURES: -1,
 totalEnabledClientAttributes: 0,
 enabledClientAttributes: [ 0, 0 ],
 clientAttributes: [],
 liveClientAttributes: [],
 currentRenderer: null,
 modifiedClientAttributes: false,
 clientActiveTexture: 0,
 clientColor: null,
 usedTexUnitList: [],
 fixedFunctionProgram: null,
 setClientAttribute: function setClientAttribute(name, size, type, stride, pointer) {
  var attrib = GLImmediate.clientAttributes[name];
  if (!attrib) {
   for (var i = 0; i <= name; i++) {
    if (!GLImmediate.clientAttributes[i]) {
     GLImmediate.clientAttributes[i] = {
      name: name,
      size: size,
      type: type,
      stride: stride,
      pointer: pointer,
      offset: 0
     };
    }
   }
  } else {
   attrib.name = name;
   attrib.size = size;
   attrib.type = type;
   attrib.stride = stride;
   attrib.pointer = pointer;
   attrib.offset = 0;
  }
  GLImmediate.modifiedClientAttributes = true;
 },
 addRendererComponent: function addRendererComponent(name, size, type) {
  if (!GLImmediate.rendererComponents[name]) {
   GLImmediate.rendererComponents[name] = 1;
   GLImmediate.enabledClientAttributes[name] = true;
   GLImmediate.setClientAttribute(name, size, type, 0, GLImmediate.rendererComponentPointer);
   GLImmediate.rendererComponentPointer += size * GL.byteSizeByType[type - GL.byteSizeByTypeRoot];
   GL.enableVertexAttribArray(name);
  } else {
   GLImmediate.rendererComponents[name]++;
  }
 },
 disableBeginEndClientAttributes: function disableBeginEndClientAttributes() {
  for (var i = 0; i < GLImmediate.NUM_ATTRIBUTES; i++) {
   if (GLImmediate.rendererComponents[i]) GLImmediate.enabledClientAttributes[i] = false;
  }
 },
 getRenderer: function getRenderer() {
  if (GLImmediate.currentRenderer) {
   return GLImmediate.currentRenderer;
  }
  var attributes = GLImmediate.liveClientAttributes;
  var cacheMap = GLImmediate.rendererCache;
  var keyView = cacheMap.getStaticKeyView().reset();
  var enabledAttributesKey = 0;
  for (var i = 0; i < attributes.length; i++) {
   enabledAttributesKey |= 1 << attributes[i].name;
  }
  var fogParam = 0;
  if (GLEmulation.fogEnabled) {
   switch (GLEmulation.fogMode) {
   case 2049:
    fogParam = 1;
    break;

   case 9729:
    fogParam = 2;
    break;

   default:
    fogParam = 3;
    break;
   }
  }
  enabledAttributesKey = enabledAttributesKey << 2 | fogParam;
  for (var clipPlaneId = 0; clipPlaneId < GLEmulation.MAX_CLIP_PLANES; clipPlaneId++) {
   enabledAttributesKey = enabledAttributesKey << 1 | GLEmulation.clipPlaneEnabled[clipPlaneId];
  }
  enabledAttributesKey = enabledAttributesKey << 1 | GLEmulation.lightingEnabled;
  for (var lightId = 0; lightId < GLEmulation.MAX_LIGHTS; lightId++) {
   enabledAttributesKey = enabledAttributesKey << 1 | (GLEmulation.lightingEnabled ? GLEmulation.lightEnabled[lightId] : 0);
  }
  enabledAttributesKey = enabledAttributesKey << 1 | (GLImmediate.mode == GLctx.POINTS ? 1 : 0);
  keyView.next(enabledAttributesKey);
  GLImmediate.TexEnvJIT.traverseState(keyView);
  var renderer = keyView.get();
  if (!renderer) {
   renderer = GLImmediate.createRenderer();
   GLImmediate.currentRenderer = renderer;
   keyView.set(renderer);
   return renderer;
  }
  GLImmediate.currentRenderer = renderer;
  return renderer;
 },
 createRenderer: function createRenderer(renderer) {
  var useCurrProgram = !!GL.currProgram;
  var hasTextures = false;
  for (var i = 0; i < GLImmediate.MAX_TEXTURES; i++) {
   var texAttribName = GLImmediate.TEXTURE0 + i;
   if (!GLImmediate.enabledClientAttributes[texAttribName]) continue;
   hasTextures = true;
  }
  var ret = {
   init: function init() {
    var uTexUnitPrefix = "u_texUnit";
    var aTexCoordPrefix = "a_texCoord";
    var vTexCoordPrefix = "v_texCoord";
    var vPrimColor = "v_color";
    var uTexMatrixPrefix = GLImmediate.useTextureMatrix ? "u_textureMatrix" : null;
    if (useCurrProgram) {
     if (GL.shaderInfos[GL.programShaders[GL.currProgram][0]].type == GLctx.VERTEX_SHADER) {
      this.vertexShader = GL.shaders[GL.programShaders[GL.currProgram][0]];
      this.fragmentShader = GL.shaders[GL.programShaders[GL.currProgram][1]];
     } else {
      this.vertexShader = GL.shaders[GL.programShaders[GL.currProgram][1]];
      this.fragmentShader = GL.shaders[GL.programShaders[GL.currProgram][0]];
     }
     this.program = GL.programs[GL.currProgram];
     this.usedTexUnitList = [];
    } else {
     if (GLEmulation.fogEnabled) {
      switch (GLEmulation.fogMode) {
      case 2049:
       var fogFormula = "  float fog = exp(-u_fogDensity * u_fogDensity * ecDistance * ecDistance); \n";
       break;

      case 9729:
       var fogFormula = "  float fog = (u_fogEnd - ecDistance) * u_fogScale; \n";
       break;

      default:
       var fogFormula = "  float fog = exp(-u_fogDensity * ecDistance); \n";
       break;
      }
     }
     GLImmediate.TexEnvJIT.setGLSLVars(uTexUnitPrefix, vTexCoordPrefix, vPrimColor, uTexMatrixPrefix);
     var fsTexEnvPass = GLImmediate.TexEnvJIT.genAllPassLines("gl_FragColor", 2);
     var texUnitAttribList = "";
     var texUnitVaryingList = "";
     var texUnitUniformList = "";
     var vsTexCoordInits = "";
     this.usedTexUnitList = GLImmediate.TexEnvJIT.getUsedTexUnitList();
     for (var i = 0; i < this.usedTexUnitList.length; i++) {
      var texUnit = this.usedTexUnitList[i];
      texUnitAttribList += "attribute vec4 " + aTexCoordPrefix + texUnit + ";\n";
      texUnitVaryingList += "varying vec4 " + vTexCoordPrefix + texUnit + ";\n";
      texUnitUniformList += "uniform sampler2D " + uTexUnitPrefix + texUnit + ";\n";
      vsTexCoordInits += "  " + vTexCoordPrefix + texUnit + " = " + aTexCoordPrefix + texUnit + ";\n";
      if (GLImmediate.useTextureMatrix) {
       texUnitUniformList += "uniform mat4 " + uTexMatrixPrefix + texUnit + ";\n";
      }
     }
     var vsFogVaryingInit = null;
     if (GLEmulation.fogEnabled) {
      vsFogVaryingInit = "  v_fogFragCoord = abs(ecPosition.z);\n";
     }
     var vsPointSizeDefs = null;
     var vsPointSizeInit = null;
     if (GLImmediate.mode == GLctx.POINTS) {
      vsPointSizeDefs = "uniform float u_pointSize;\n";
      vsPointSizeInit = "  gl_PointSize = u_pointSize;\n";
     }
     var vsClipPlaneDefs = "";
     var vsClipPlaneInit = "";
     var fsClipPlaneDefs = "";
     var fsClipPlanePass = "";
     for (var clipPlaneId = 0; clipPlaneId < GLEmulation.MAX_CLIP_PLANES; clipPlaneId++) {
      if (GLEmulation.clipPlaneEnabled[clipPlaneId]) {
       vsClipPlaneDefs += "uniform vec4 u_clipPlaneEquation" + clipPlaneId + ";";
       vsClipPlaneDefs += "varying float v_clipDistance" + clipPlaneId + ";";
       vsClipPlaneInit += "  v_clipDistance" + clipPlaneId + " = dot(ecPosition, u_clipPlaneEquation" + clipPlaneId + ");";
       fsClipPlaneDefs += "varying float v_clipDistance" + clipPlaneId + ";";
       fsClipPlanePass += "  if(v_clipDistance" + clipPlaneId + " < 0.0) discard;";
      }
     }
     var vsLightingDefs = "";
     var vsLightingPass = "";
     if (GLEmulation.lightingEnabled) {
      vsLightingDefs += "attribute vec3 a_normal;";
      vsLightingDefs += "uniform mat3 u_normalMatrix;";
      vsLightingDefs += "uniform vec4 u_lightModelAmbient;";
      vsLightingDefs += "uniform vec4 u_materialAmbient;";
      vsLightingDefs += "uniform vec4 u_materialDiffuse;";
      vsLightingDefs += "uniform vec4 u_materialSpecular;";
      vsLightingDefs += "uniform float u_materialShininess;";
      vsLightingDefs += "uniform vec4 u_materialEmission;";
      vsLightingPass += "  vec3 ecNormal = normalize(u_normalMatrix * a_normal);";
      vsLightingPass += "  v_color.w = u_materialDiffuse.w;";
      vsLightingPass += "  v_color.xyz = u_materialEmission.xyz;";
      vsLightingPass += "  v_color.xyz += u_lightModelAmbient.xyz * u_materialAmbient.xyz;";
      for (var lightId = 0; lightId < GLEmulation.MAX_LIGHTS; lightId++) {
       if (GLEmulation.lightEnabled[lightId]) {
        vsLightingDefs += "uniform vec4 u_lightAmbient" + lightId + ";";
        vsLightingDefs += "uniform vec4 u_lightDiffuse" + lightId + ";";
        vsLightingDefs += "uniform vec4 u_lightSpecular" + lightId + ";";
        vsLightingDefs += "uniform vec4 u_lightPosition" + lightId + ";";
        vsLightingPass += "  {";
        vsLightingPass += "    vec3 lightDirection = normalize(u_lightPosition" + lightId + ").xyz;";
        vsLightingPass += "    vec3 halfVector = normalize(lightDirection + vec3(0,0,1));";
        vsLightingPass += "    vec3 ambient = u_lightAmbient" + lightId + ".xyz * u_materialAmbient.xyz;";
        vsLightingPass += "    float diffuseI = max(dot(ecNormal, lightDirection), 0.0);";
        vsLightingPass += "    float specularI = max(dot(ecNormal, halfVector), 0.0);";
        vsLightingPass += "    vec3 diffuse = diffuseI * u_lightDiffuse" + lightId + ".xyz * u_materialDiffuse.xyz;";
        vsLightingPass += "    specularI = (diffuseI > 0.0 && specularI > 0.0) ? exp(u_materialShininess * log(specularI)) : 0.0;";
        vsLightingPass += "    vec3 specular = specularI * u_lightSpecular" + lightId + ".xyz * u_materialSpecular.xyz;";
        vsLightingPass += "    v_color.xyz += ambient + diffuse + specular;";
        vsLightingPass += "  }";
       }
      }
      vsLightingPass += "  v_color = clamp(v_color, 0.0, 1.0);";
     }
     var vsSource = [ "attribute vec4 a_position;", "attribute vec4 a_color;", "varying vec4 v_color;", texUnitAttribList, texUnitVaryingList, GLEmulation.fogEnabled ? "varying float v_fogFragCoord;" : null, "uniform mat4 u_modelView;", "uniform mat4 u_projection;", vsPointSizeDefs, vsClipPlaneDefs, vsLightingDefs, "void main()", "{", "  vec4 ecPosition = u_modelView * a_position;", "  gl_Position = u_projection * ecPosition;", "  v_color = a_color;", vsTexCoordInits, vsFogVaryingInit, vsPointSizeInit, vsClipPlaneInit, vsLightingPass, "}", "" ].join("\n").replace(/\n\n+/g, "\n");
     this.vertexShader = GLctx.createShader(GLctx.VERTEX_SHADER);
     GLctx.shaderSource(this.vertexShader, vsSource);
     GLctx.compileShader(this.vertexShader);
     var fogHeaderIfNeeded = null;
     if (GLEmulation.fogEnabled) {
      fogHeaderIfNeeded = [ "", "varying float v_fogFragCoord; ", "uniform vec4 u_fogColor;      ", "uniform float u_fogEnd;       ", "uniform float u_fogScale;     ", "uniform float u_fogDensity;   ", "float ffog(in float ecDistance) { ", fogFormula, "  fog = clamp(fog, 0.0, 1.0); ", "  return fog;                 ", "}", "" ].join("\n");
     }
     var fogPass = null;
     if (GLEmulation.fogEnabled) {
      fogPass = "gl_FragColor = vec4(mix(u_fogColor.rgb, gl_FragColor.rgb, ffog(v_fogFragCoord)), gl_FragColor.a);\n";
     }
     var fsSource = [ "precision mediump float;", texUnitVaryingList, texUnitUniformList, "varying vec4 v_color;", fogHeaderIfNeeded, fsClipPlaneDefs, "void main()", "{", fsClipPlanePass, fsTexEnvPass, fogPass, "}", "" ].join("\n").replace(/\n\n+/g, "\n");
     this.fragmentShader = GLctx.createShader(GLctx.FRAGMENT_SHADER);
     GLctx.shaderSource(this.fragmentShader, fsSource);
     GLctx.compileShader(this.fragmentShader);
     this.program = GLctx.createProgram();
     GLctx.attachShader(this.program, this.vertexShader);
     GLctx.attachShader(this.program, this.fragmentShader);
     GLctx.bindAttribLocation(this.program, GLImmediate.VERTEX, "a_position");
     GLctx.bindAttribLocation(this.program, GLImmediate.COLOR, "a_color");
     GLctx.bindAttribLocation(this.program, GLImmediate.NORMAL, "a_normal");
     var maxVertexAttribs = GLctx.getParameter(GLctx.MAX_VERTEX_ATTRIBS);
     for (var i = 0; i < GLImmediate.MAX_TEXTURES && GLImmediate.TEXTURE0 + i < maxVertexAttribs; i++) {
      GLctx.bindAttribLocation(this.program, GLImmediate.TEXTURE0 + i, "a_texCoord" + i);
      GLctx.bindAttribLocation(this.program, GLImmediate.TEXTURE0 + i, aTexCoordPrefix + i);
     }
     GLctx.linkProgram(this.program);
    }
    this.textureMatrixVersion = [ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ];
    this.positionLocation = GLctx.getAttribLocation(this.program, "a_position");
    this.texCoordLocations = [];
    for (var i = 0; i < GLImmediate.MAX_TEXTURES; i++) {
     if (!GLImmediate.enabledClientAttributes[GLImmediate.TEXTURE0 + i]) {
      this.texCoordLocations[i] = -1;
      continue;
     }
     if (useCurrProgram) {
      this.texCoordLocations[i] = GLctx.getAttribLocation(this.program, "a_texCoord" + i);
     } else {
      this.texCoordLocations[i] = GLctx.getAttribLocation(this.program, aTexCoordPrefix + i);
     }
    }
    this.colorLocation = GLctx.getAttribLocation(this.program, "a_color");
    if (!useCurrProgram) {
     var prevBoundProg = GLctx.getParameter(GLctx.CURRENT_PROGRAM);
     GLctx.useProgram(this.program);
     {
      for (var i = 0; i < this.usedTexUnitList.length; i++) {
       var texUnitID = this.usedTexUnitList[i];
       var texSamplerLoc = GLctx.getUniformLocation(this.program, uTexUnitPrefix + texUnitID);
       GLctx.uniform1i(texSamplerLoc, texUnitID);
      }
     }
     GLctx.vertexAttrib4fv(this.colorLocation, [ 1, 1, 1, 1 ]);
     GLctx.useProgram(prevBoundProg);
    }
    this.textureMatrixLocations = [];
    for (var i = 0; i < GLImmediate.MAX_TEXTURES; i++) {
     this.textureMatrixLocations[i] = GLctx.getUniformLocation(this.program, "u_textureMatrix" + i);
    }
    this.normalLocation = GLctx.getAttribLocation(this.program, "a_normal");
    this.modelViewLocation = GLctx.getUniformLocation(this.program, "u_modelView");
    this.projectionLocation = GLctx.getUniformLocation(this.program, "u_projection");
    this.normalMatrixLocation = GLctx.getUniformLocation(this.program, "u_normalMatrix");
    this.hasTextures = hasTextures;
    this.hasNormal = GLImmediate.enabledClientAttributes[GLImmediate.NORMAL] && GLImmediate.clientAttributes[GLImmediate.NORMAL].size > 0 && this.normalLocation >= 0;
    this.hasColor = this.colorLocation === 0 || this.colorLocation > 0;
    this.floatType = GLctx.FLOAT;
    this.fogColorLocation = GLctx.getUniformLocation(this.program, "u_fogColor");
    this.fogEndLocation = GLctx.getUniformLocation(this.program, "u_fogEnd");
    this.fogScaleLocation = GLctx.getUniformLocation(this.program, "u_fogScale");
    this.fogDensityLocation = GLctx.getUniformLocation(this.program, "u_fogDensity");
    this.hasFog = !!(this.fogColorLocation || this.fogEndLocation || this.fogScaleLocation || this.fogDensityLocation);
    this.pointSizeLocation = GLctx.getUniformLocation(this.program, "u_pointSize");
    this.hasClipPlane = false;
    this.clipPlaneEquationLocation = [];
    for (var clipPlaneId = 0; clipPlaneId < GLEmulation.MAX_CLIP_PLANES; clipPlaneId++) {
     this.clipPlaneEquationLocation[clipPlaneId] = GLctx.getUniformLocation(this.program, "u_clipPlaneEquation" + clipPlaneId);
     this.hasClipPlane = this.hasClipPlane || this.clipPlaneEquationLocation[clipPlaneId];
    }
    this.hasLighting = GLEmulation.lightingEnabled;
    this.lightModelAmbientLocation = GLctx.getUniformLocation(this.program, "u_lightModelAmbient");
    this.materialAmbientLocation = GLctx.getUniformLocation(this.program, "u_materialAmbient");
    this.materialDiffuseLocation = GLctx.getUniformLocation(this.program, "u_materialDiffuse");
    this.materialSpecularLocation = GLctx.getUniformLocation(this.program, "u_materialSpecular");
    this.materialShininessLocation = GLctx.getUniformLocation(this.program, "u_materialShininess");
    this.materialEmissionLocation = GLctx.getUniformLocation(this.program, "u_materialEmission");
    this.lightAmbientLocation = [];
    this.lightDiffuseLocation = [];
    this.lightSpecularLocation = [];
    this.lightPositionLocation = [];
    for (var lightId = 0; lightId < GLEmulation.MAX_LIGHTS; lightId++) {
     this.lightAmbientLocation[lightId] = GLctx.getUniformLocation(this.program, "u_lightAmbient" + lightId);
     this.lightDiffuseLocation[lightId] = GLctx.getUniformLocation(this.program, "u_lightDiffuse" + lightId);
     this.lightSpecularLocation[lightId] = GLctx.getUniformLocation(this.program, "u_lightSpecular" + lightId);
     this.lightPositionLocation[lightId] = GLctx.getUniformLocation(this.program, "u_lightPosition" + lightId);
    }
   },
   prepare: function prepare() {
    var arrayBuffer;
    if (!GLctx.currentArrayBufferBinding) {
     var start = GLImmediate.firstVertex * GLImmediate.stride;
     var end = GLImmediate.lastVertex * GLImmediate.stride;
     arrayBuffer = GL.getTempVertexBuffer(end);
    } else {
     arrayBuffer = GLctx.currentArrayBufferBinding;
    }
    var lastRenderer = GLImmediate.lastRenderer;
    var canSkip = this == lastRenderer && arrayBuffer == GLImmediate.lastArrayBuffer && (GL.currProgram || this.program) == GLImmediate.lastProgram && GLImmediate.stride == GLImmediate.lastStride && !GLImmediate.matricesModified;
    if (!canSkip && lastRenderer) lastRenderer.cleanup();
    if (!GLctx.currentArrayBufferBinding) {
     if (arrayBuffer != GLImmediate.lastArrayBuffer) {
      GLctx.bindBuffer(GLctx.ARRAY_BUFFER, arrayBuffer);
      GLImmediate.lastArrayBuffer = arrayBuffer;
     }
     GLctx.bufferSubData(GLctx.ARRAY_BUFFER, start, GLImmediate.vertexData.subarray(start >> 2, end >> 2));
    }
    if (canSkip) return;
    GLImmediate.lastRenderer = this;
    GLImmediate.lastProgram = GL.currProgram || this.program;
    GLImmediate.lastStride == GLImmediate.stride;
    GLImmediate.matricesModified = false;
    if (!GL.currProgram) {
     if (GLImmediate.fixedFunctionProgram != this.program) {
      GLctx.useProgram(this.program);
      GLImmediate.fixedFunctionProgram = this.program;
     }
    }
    if (this.modelViewLocation && this.modelViewMatrixVersion != GLImmediate.matrixVersion[0]) {
     this.modelViewMatrixVersion = GLImmediate.matrixVersion[0];
     GLctx.uniformMatrix4fv(this.modelViewLocation, false, GLImmediate.matrix[0]);
     if (GLEmulation.lightEnabled) {
      var tmpMVinv = GLImmediate.matrixLib.mat4.create(GLImmediate.matrix[0]);
      GLImmediate.matrixLib.mat4.inverse(tmpMVinv);
      GLImmediate.matrixLib.mat4.transpose(tmpMVinv);
      GLctx.uniformMatrix3fv(this.normalMatrixLocation, false, GLImmediate.matrixLib.mat4.toMat3(tmpMVinv));
     }
    }
    if (this.projectionLocation && this.projectionMatrixVersion != GLImmediate.matrixVersion[1]) {
     this.projectionMatrixVersion = GLImmediate.matrixVersion[1];
     GLctx.uniformMatrix4fv(this.projectionLocation, false, GLImmediate.matrix[1]);
    }
    var clientAttributes = GLImmediate.clientAttributes;
    var posAttr = clientAttributes[GLImmediate.VERTEX];
    if (!GLctx.currentArrayBufferBinding) {
     GLctx.vertexAttribPointer(GLImmediate.VERTEX, posAttr.size, posAttr.type, false, GLImmediate.stride, posAttr.offset);
     if (this.hasNormal) {
      var normalAttr = clientAttributes[GLImmediate.NORMAL];
      GLctx.vertexAttribPointer(GLImmediate.NORMAL, normalAttr.size, normalAttr.type, true, GLImmediate.stride, normalAttr.offset);
     }
    }
    if (this.hasTextures) {
     for (var i = 0; i < GLImmediate.MAX_TEXTURES; i++) {
      if (!GLctx.currentArrayBufferBinding) {
       var attribLoc = GLImmediate.TEXTURE0 + i;
       var texAttr = clientAttributes[attribLoc];
       if (texAttr.size) {
        GLctx.vertexAttribPointer(attribLoc, texAttr.size, texAttr.type, false, GLImmediate.stride, texAttr.offset);
       } else {
        GLctx.vertexAttrib4f(attribLoc, 0, 0, 0, 1);
       }
      }
      var t = 2 + i;
      if (this.textureMatrixLocations[i] && this.textureMatrixVersion[t] != GLImmediate.matrixVersion[t]) {
       this.textureMatrixVersion[t] = GLImmediate.matrixVersion[t];
       GLctx.uniformMatrix4fv(this.textureMatrixLocations[i], false, GLImmediate.matrix[t]);
      }
     }
    }
    if (GLImmediate.enabledClientAttributes[GLImmediate.COLOR]) {
     var colorAttr = clientAttributes[GLImmediate.COLOR];
     if (!GLctx.currentArrayBufferBinding) {
      GLctx.vertexAttribPointer(GLImmediate.COLOR, colorAttr.size, colorAttr.type, true, GLImmediate.stride, colorAttr.offset);
     }
    }
    if (this.hasFog) {
     if (this.fogColorLocation) GLctx.uniform4fv(this.fogColorLocation, GLEmulation.fogColor);
     if (this.fogEndLocation) GLctx.uniform1f(this.fogEndLocation, GLEmulation.fogEnd);
     if (this.fogScaleLocation) GLctx.uniform1f(this.fogScaleLocation, 1 / (GLEmulation.fogEnd - GLEmulation.fogStart));
     if (this.fogDensityLocation) GLctx.uniform1f(this.fogDensityLocation, GLEmulation.fogDensity);
    }
    if (this.hasClipPlane) {
     for (var clipPlaneId = 0; clipPlaneId < GLEmulation.MAX_CLIP_PLANES; clipPlaneId++) {
      if (this.clipPlaneEquationLocation[clipPlaneId]) GLctx.uniform4fv(this.clipPlaneEquationLocation[clipPlaneId], GLEmulation.clipPlaneEquation[clipPlaneId]);
     }
    }
    if (this.hasLighting) {
     if (this.lightModelAmbientLocation) GLctx.uniform4fv(this.lightModelAmbientLocation, GLEmulation.lightModelAmbient);
     if (this.materialAmbientLocation) GLctx.uniform4fv(this.materialAmbientLocation, GLEmulation.materialAmbient);
     if (this.materialDiffuseLocation) GLctx.uniform4fv(this.materialDiffuseLocation, GLEmulation.materialDiffuse);
     if (this.materialSpecularLocation) GLctx.uniform4fv(this.materialSpecularLocation, GLEmulation.materialSpecular);
     if (this.materialShininessLocation) GLctx.uniform1f(this.materialShininessLocation, GLEmulation.materialShininess[0]);
     if (this.materialEmissionLocation) GLctx.uniform4fv(this.materialEmissionLocation, GLEmulation.materialEmission);
     for (var lightId = 0; lightId < GLEmulation.MAX_LIGHTS; lightId++) {
      if (this.lightAmbientLocation[lightId]) GLctx.uniform4fv(this.lightAmbientLocation[lightId], GLEmulation.lightAmbient[lightId]);
      if (this.lightDiffuseLocation[lightId]) GLctx.uniform4fv(this.lightDiffuseLocation[lightId], GLEmulation.lightDiffuse[lightId]);
      if (this.lightSpecularLocation[lightId]) GLctx.uniform4fv(this.lightSpecularLocation[lightId], GLEmulation.lightSpecular[lightId]);
      if (this.lightPositionLocation[lightId]) GLctx.uniform4fv(this.lightPositionLocation[lightId], GLEmulation.lightPosition[lightId]);
     }
    }
    if (GLImmediate.mode == GLctx.POINTS) {
     if (this.pointSizeLocation) {
      GLctx.uniform1f(this.pointSizeLocation, GLEmulation.pointSize);
     }
    }
   },
   cleanup: function cleanup() {}
  };
  ret.init();
  return ret;
 },
 setupFuncs: function() {
  GLImmediate.MapTreeLib = GLImmediate.spawnMapTreeLib();
  GLImmediate.spawnMapTreeLib = null;
  GLImmediate.TexEnvJIT = GLImmediate.spawnTexEnvJIT();
  GLImmediate.spawnTexEnvJIT = null;
  GLImmediate.setupHooks();
 },
 setupHooks: function() {
  if (!GLEmulation.hasRunInit) {
   GLEmulation.init();
  }
  var glActiveTexture = _glActiveTexture;
  _glActiveTexture = _emscripten_glActiveTexture = function _glActiveTexture(texture) {
   GLImmediate.TexEnvJIT.hook_activeTexture(texture);
   glActiveTexture(texture);
  };
  var glEnable = _glEnable;
  _glEnable = _emscripten_glEnable = function _glEnable(cap) {
   GLImmediate.TexEnvJIT.hook_enable(cap);
   glEnable(cap);
  };
  var glDisable = _glDisable;
  _glDisable = _emscripten_glDisable = function _glDisable(cap) {
   GLImmediate.TexEnvJIT.hook_disable(cap);
   glDisable(cap);
  };
  _glTexEnvf = _emscripten_glTexEnvf = function _glTexEnvf(target, pname, param) {
   GLImmediate.TexEnvJIT.hook_texEnvf(target, pname, param);
  };
  _glTexEnvi = _emscripten_glTexEnvi = function _glTexEnvi(target, pname, param) {
   GLImmediate.TexEnvJIT.hook_texEnvi(target, pname, param);
  };
  _glTexEnvfv = _emscripten_glTexEnvfv = function _glTexEnvfv(target, pname, param) {
   GLImmediate.TexEnvJIT.hook_texEnvfv(target, pname, param);
  };
  _glGetTexEnviv = function _glGetTexEnviv(target, pname, param) {
   GLImmediate.TexEnvJIT.hook_getTexEnviv(target, pname, param);
  };
  _glGetTexEnvfv = function _glGetTexEnvfv(target, pname, param) {
   GLImmediate.TexEnvJIT.hook_getTexEnvfv(target, pname, param);
  };
  var glGetIntegerv = _glGetIntegerv;
  _glGetIntegerv = _emscripten_glGetIntegerv = function _glGetIntegerv(pname, params) {
   switch (pname) {
   case 35725:
    {
     var cur = GLctx.getParameter(GLctx.CURRENT_PROGRAM);
     if (cur == GLImmediate.fixedFunctionProgram) {
      HEAP32[params >> 2] = 0;
      return;
     }
     break;
    }
   }
   glGetIntegerv(pname, params);
  };
 },
 initted: false,
 init: function() {
  err("WARNING: using emscripten GL immediate mode emulation. This is very limited in what it supports");
  GLImmediate.initted = true;
  if (!Module.useWebGL) return;
  GLImmediate.MAX_TEXTURES = Module["GL_MAX_TEXTURE_IMAGE_UNITS"] || GLctx.getParameter(GLctx.MAX_TEXTURE_IMAGE_UNITS);
  GLImmediate.TexEnvJIT.init(GLctx, GLImmediate.MAX_TEXTURES);
  GLImmediate.NUM_ATTRIBUTES = 3 + GLImmediate.MAX_TEXTURES;
  GLImmediate.clientAttributes = [];
  GLEmulation.enabledClientAttribIndices = [];
  for (var i = 0; i < GLImmediate.NUM_ATTRIBUTES; i++) {
   GLImmediate.clientAttributes.push({});
   GLEmulation.enabledClientAttribIndices.push(false);
  }
  GLImmediate.matrix = [];
  GLImmediate.matrixStack = [];
  GLImmediate.matrixVersion = [];
  for (var i = 0; i < 2 + GLImmediate.MAX_TEXTURES; i++) {
   GLImmediate.matrixStack.push([]);
   GLImmediate.matrixVersion.push(0);
   GLImmediate.matrix.push(GLImmediate.matrixLib.mat4.create());
   GLImmediate.matrixLib.mat4.identity(GLImmediate.matrix[i]);
  }
  GLImmediate.rendererCache = GLImmediate.MapTreeLib.create();
  GLImmediate.tempData = new Float32Array(GL.MAX_TEMP_BUFFER_SIZE >> 2);
  GLImmediate.indexData = new Uint16Array(GL.MAX_TEMP_BUFFER_SIZE >> 1);
  GLImmediate.vertexDataU8 = new Uint8Array(GLImmediate.tempData.buffer);
  GL.generateTempBuffers(true, GL.currentContext);
  GLImmediate.clientColor = new Float32Array([ 1, 1, 1, 1 ]);
 },
 prepareClientAttributes: function prepareClientAttributes(count, beginEnd) {
  if (!GLImmediate.modifiedClientAttributes) {
   GLImmediate.vertexCounter = GLImmediate.stride * count / 4;
   return;
  }
  GLImmediate.modifiedClientAttributes = false;
  var clientStartPointer = 2147483647;
  var bytes = 0;
  var minStride = 2147483647;
  var maxStride = 0;
  var attributes = GLImmediate.liveClientAttributes;
  attributes.length = 0;
  for (var i = 0; i < 3 + GLImmediate.MAX_TEXTURES; i++) {
   if (GLImmediate.enabledClientAttributes[i]) {
    var attr = GLImmediate.clientAttributes[i];
    attributes.push(attr);
    clientStartPointer = Math.min(clientStartPointer, attr.pointer);
    attr.sizeBytes = attr.size * GL.byteSizeByType[attr.type - GL.byteSizeByTypeRoot];
    bytes += attr.sizeBytes;
    minStride = Math.min(minStride, attr.stride);
    maxStride = Math.max(maxStride, attr.stride);
   }
  }
  if ((minStride != maxStride || maxStride < bytes) && !beginEnd) {
   if (!GLImmediate.restrideBuffer) GLImmediate.restrideBuffer = _malloc(GL.MAX_TEMP_BUFFER_SIZE);
   var start = GLImmediate.restrideBuffer;
   bytes = 0;
   for (var i = 0; i < attributes.length; i++) {
    var attr = attributes[i];
    var size = attr.sizeBytes;
    if (size % 4 != 0) size += 4 - size % 4;
    attr.offset = bytes;
    bytes += size;
   }
   for (var i = 0; i < attributes.length; i++) {
    var attr = attributes[i];
    var srcStride = Math.max(attr.sizeBytes, attr.stride);
    if ((srcStride & 3) == 0 && (attr.sizeBytes & 3) == 0) {
     var size4 = attr.sizeBytes >> 2;
     var srcStride4 = Math.max(attr.sizeBytes, attr.stride) >> 2;
     for (var j = 0; j < count; j++) {
      for (var k = 0; k < size4; k++) {
       HEAP32[(start + attr.offset + bytes * j >> 2) + k] = HEAP32[(attr.pointer >> 2) + j * srcStride4 + k];
      }
     }
    } else {
     for (var j = 0; j < count; j++) {
      for (var k = 0; k < attr.sizeBytes; k++) {
       HEAP8[start + attr.offset + bytes * j + k] = HEAP8[attr.pointer + j * srcStride + k];
      }
     }
    }
    attr.pointer = start + attr.offset;
   }
   GLImmediate.stride = bytes;
   GLImmediate.vertexPointer = start;
  } else {
   if (GLctx.currentArrayBufferBinding) {
    GLImmediate.vertexPointer = 0;
   } else {
    GLImmediate.vertexPointer = clientStartPointer;
   }
   for (var i = 0; i < attributes.length; i++) {
    var attr = attributes[i];
    attr.offset = attr.pointer - GLImmediate.vertexPointer;
   }
   GLImmediate.stride = Math.max(maxStride, bytes);
  }
  if (!beginEnd) {
   GLImmediate.vertexCounter = GLImmediate.stride * count / 4;
  }
 },
 flush: function flush(numProvidedIndexes, startIndex, ptr) {
  startIndex = startIndex || 0;
  ptr = ptr || 0;
  var renderer = GLImmediate.getRenderer();
  var numVertexes = 4 * GLImmediate.vertexCounter / GLImmediate.stride;
  if (!numVertexes) return;
  var emulatedElementArrayBuffer = false;
  var numIndexes = 0;
  if (numProvidedIndexes) {
   numIndexes = numProvidedIndexes;
   if (!GLctx.currentArrayBufferBinding && GLImmediate.firstVertex > GLImmediate.lastVertex) {
    for (var i = 0; i < numProvidedIndexes; i++) {
     var currIndex = HEAPU16[ptr + i * 2 >> 1];
     GLImmediate.firstVertex = Math.min(GLImmediate.firstVertex, currIndex);
     GLImmediate.lastVertex = Math.max(GLImmediate.lastVertex, currIndex + 1);
    }
   }
   if (!GLctx.currentElementArrayBufferBinding) {
    var indexBuffer = GL.getTempIndexBuffer(numProvidedIndexes << 1);
    GLctx.bindBuffer(GLctx.ELEMENT_ARRAY_BUFFER, indexBuffer);
    GLctx.bufferSubData(GLctx.ELEMENT_ARRAY_BUFFER, 0, HEAPU16.subarray(ptr >> 1, ptr + (numProvidedIndexes << 1) >> 1));
    ptr = 0;
    emulatedElementArrayBuffer = true;
   }
  } else if (GLImmediate.mode > 6) {
   if (GLImmediate.mode != 7) throw "unsupported immediate mode " + GLImmediate.mode;
   ptr = GLImmediate.firstVertex * 3;
   var numQuads = numVertexes / 4;
   numIndexes = numQuads * 6;
   GLctx.bindBuffer(GLctx.ELEMENT_ARRAY_BUFFER, GL.currentContext.tempQuadIndexBuffer);
   emulatedElementArrayBuffer = true;
   GLImmediate.mode = GLctx.TRIANGLES;
  }
  renderer.prepare();
  if (numIndexes) {
   GLctx.drawElements(GLImmediate.mode, numIndexes, GLctx.UNSIGNED_SHORT, ptr);
  } else {
   GLctx.drawArrays(GLImmediate.mode, startIndex, numVertexes);
  }
  if (emulatedElementArrayBuffer) {
   GLctx.bindBuffer(GLctx.ELEMENT_ARRAY_BUFFER, GL.buffers[GLctx.currentElementArrayBufferBinding] || null);
  }
 }
};

GLImmediate.matrixLib = function() {
 var vec3 = {};
 var mat3 = {};
 var mat4 = {};
 var quat4 = {};
 var MatrixArray = Float32Array;
 vec3.create = function(vec) {
  var dest = new MatrixArray(3);
  if (vec) {
   dest[0] = vec[0];
   dest[1] = vec[1];
   dest[2] = vec[2];
  } else {
   dest[0] = dest[1] = dest[2] = 0;
  }
  return dest;
 };
 vec3.set = function(vec, dest) {
  dest[0] = vec[0];
  dest[1] = vec[1];
  dest[2] = vec[2];
  return dest;
 };
 vec3.add = function(vec, vec2, dest) {
  if (!dest || vec === dest) {
   vec[0] += vec2[0];
   vec[1] += vec2[1];
   vec[2] += vec2[2];
   return vec;
  }
  dest[0] = vec[0] + vec2[0];
  dest[1] = vec[1] + vec2[1];
  dest[2] = vec[2] + vec2[2];
  return dest;
 };
 vec3.subtract = function(vec, vec2, dest) {
  if (!dest || vec === dest) {
   vec[0] -= vec2[0];
   vec[1] -= vec2[1];
   vec[2] -= vec2[2];
   return vec;
  }
  dest[0] = vec[0] - vec2[0];
  dest[1] = vec[1] - vec2[1];
  dest[2] = vec[2] - vec2[2];
  return dest;
 };
 vec3.multiply = function(vec, vec2, dest) {
  if (!dest || vec === dest) {
   vec[0] *= vec2[0];
   vec[1] *= vec2[1];
   vec[2] *= vec2[2];
   return vec;
  }
  dest[0] = vec[0] * vec2[0];
  dest[1] = vec[1] * vec2[1];
  dest[2] = vec[2] * vec2[2];
  return dest;
 };
 vec3.negate = function(vec, dest) {
  if (!dest) {
   dest = vec;
  }
  dest[0] = -vec[0];
  dest[1] = -vec[1];
  dest[2] = -vec[2];
  return dest;
 };
 vec3.scale = function(vec, val, dest) {
  if (!dest || vec === dest) {
   vec[0] *= val;
   vec[1] *= val;
   vec[2] *= val;
   return vec;
  }
  dest[0] = vec[0] * val;
  dest[1] = vec[1] * val;
  dest[2] = vec[2] * val;
  return dest;
 };
 vec3.normalize = function(vec, dest) {
  if (!dest) {
   dest = vec;
  }
  var x = vec[0], y = vec[1], z = vec[2], len = Math.sqrt(x * x + y * y + z * z);
  if (!len) {
   dest[0] = 0;
   dest[1] = 0;
   dest[2] = 0;
   return dest;
  } else if (len === 1) {
   dest[0] = x;
   dest[1] = y;
   dest[2] = z;
   return dest;
  }
  len = 1 / len;
  dest[0] = x * len;
  dest[1] = y * len;
  dest[2] = z * len;
  return dest;
 };
 vec3.cross = function(vec, vec2, dest) {
  if (!dest) {
   dest = vec;
  }
  var x = vec[0], y = vec[1], z = vec[2], x2 = vec2[0], y2 = vec2[1], z2 = vec2[2];
  dest[0] = y * z2 - z * y2;
  dest[1] = z * x2 - x * z2;
  dest[2] = x * y2 - y * x2;
  return dest;
 };
 vec3.length = function(vec) {
  var x = vec[0], y = vec[1], z = vec[2];
  return Math.sqrt(x * x + y * y + z * z);
 };
 vec3.dot = function(vec, vec2) {
  return vec[0] * vec2[0] + vec[1] * vec2[1] + vec[2] * vec2[2];
 };
 vec3.direction = function(vec, vec2, dest) {
  if (!dest) {
   dest = vec;
  }
  var x = vec[0] - vec2[0], y = vec[1] - vec2[1], z = vec[2] - vec2[2], len = Math.sqrt(x * x + y * y + z * z);
  if (!len) {
   dest[0] = 0;
   dest[1] = 0;
   dest[2] = 0;
   return dest;
  }
  len = 1 / len;
  dest[0] = x * len;
  dest[1] = y * len;
  dest[2] = z * len;
  return dest;
 };
 vec3.lerp = function(vec, vec2, lerp, dest) {
  if (!dest) {
   dest = vec;
  }
  dest[0] = vec[0] + lerp * (vec2[0] - vec[0]);
  dest[1] = vec[1] + lerp * (vec2[1] - vec[1]);
  dest[2] = vec[2] + lerp * (vec2[2] - vec[2]);
  return dest;
 };
 vec3.dist = function(vec, vec2) {
  var x = vec2[0] - vec[0], y = vec2[1] - vec[1], z = vec2[2] - vec[2];
  return Math.sqrt(x * x + y * y + z * z);
 };
 vec3.unproject = function(vec, view, proj, viewport, dest) {
  if (!dest) {
   dest = vec;
  }
  var m = mat4.create();
  var v = new MatrixArray(4);
  v[0] = (vec[0] - viewport[0]) * 2 / viewport[2] - 1;
  v[1] = (vec[1] - viewport[1]) * 2 / viewport[3] - 1;
  v[2] = 2 * vec[2] - 1;
  v[3] = 1;
  mat4.multiply(proj, view, m);
  if (!mat4.inverse(m)) {
   return null;
  }
  mat4.multiplyVec4(m, v);
  if (v[3] === 0) {
   return null;
  }
  dest[0] = v[0] / v[3];
  dest[1] = v[1] / v[3];
  dest[2] = v[2] / v[3];
  return dest;
 };
 vec3.str = function(vec) {
  return "[" + vec[0] + ", " + vec[1] + ", " + vec[2] + "]";
 };
 mat3.create = function(mat) {
  var dest = new MatrixArray(9);
  if (mat) {
   dest[0] = mat[0];
   dest[1] = mat[1];
   dest[2] = mat[2];
   dest[3] = mat[3];
   dest[4] = mat[4];
   dest[5] = mat[5];
   dest[6] = mat[6];
   dest[7] = mat[7];
   dest[8] = mat[8];
  }
  return dest;
 };
 mat3.set = function(mat, dest) {
  dest[0] = mat[0];
  dest[1] = mat[1];
  dest[2] = mat[2];
  dest[3] = mat[3];
  dest[4] = mat[4];
  dest[5] = mat[5];
  dest[6] = mat[6];
  dest[7] = mat[7];
  dest[8] = mat[8];
  return dest;
 };
 mat3.identity = function(dest) {
  if (!dest) {
   dest = mat3.create();
  }
  dest[0] = 1;
  dest[1] = 0;
  dest[2] = 0;
  dest[3] = 0;
  dest[4] = 1;
  dest[5] = 0;
  dest[6] = 0;
  dest[7] = 0;
  dest[8] = 1;
  return dest;
 };
 mat3.transpose = function(mat, dest) {
  if (!dest || mat === dest) {
   var a01 = mat[1], a02 = mat[2], a12 = mat[5];
   mat[1] = mat[3];
   mat[2] = mat[6];
   mat[3] = a01;
   mat[5] = mat[7];
   mat[6] = a02;
   mat[7] = a12;
   return mat;
  }
  dest[0] = mat[0];
  dest[1] = mat[3];
  dest[2] = mat[6];
  dest[3] = mat[1];
  dest[4] = mat[4];
  dest[5] = mat[7];
  dest[6] = mat[2];
  dest[7] = mat[5];
  dest[8] = mat[8];
  return dest;
 };
 mat3.toMat4 = function(mat, dest) {
  if (!dest) {
   dest = mat4.create();
  }
  dest[15] = 1;
  dest[14] = 0;
  dest[13] = 0;
  dest[12] = 0;
  dest[11] = 0;
  dest[10] = mat[8];
  dest[9] = mat[7];
  dest[8] = mat[6];
  dest[7] = 0;
  dest[6] = mat[5];
  dest[5] = mat[4];
  dest[4] = mat[3];
  dest[3] = 0;
  dest[2] = mat[2];
  dest[1] = mat[1];
  dest[0] = mat[0];
  return dest;
 };
 mat3.str = function(mat) {
  return "[" + mat[0] + ", " + mat[1] + ", " + mat[2] + ", " + mat[3] + ", " + mat[4] + ", " + mat[5] + ", " + mat[6] + ", " + mat[7] + ", " + mat[8] + "]";
 };
 mat4.create = function(mat) {
  var dest = new MatrixArray(16);
  if (mat) {
   dest[0] = mat[0];
   dest[1] = mat[1];
   dest[2] = mat[2];
   dest[3] = mat[3];
   dest[4] = mat[4];
   dest[5] = mat[5];
   dest[6] = mat[6];
   dest[7] = mat[7];
   dest[8] = mat[8];
   dest[9] = mat[9];
   dest[10] = mat[10];
   dest[11] = mat[11];
   dest[12] = mat[12];
   dest[13] = mat[13];
   dest[14] = mat[14];
   dest[15] = mat[15];
  }
  return dest;
 };
 mat4.set = function(mat, dest) {
  dest[0] = mat[0];
  dest[1] = mat[1];
  dest[2] = mat[2];
  dest[3] = mat[3];
  dest[4] = mat[4];
  dest[5] = mat[5];
  dest[6] = mat[6];
  dest[7] = mat[7];
  dest[8] = mat[8];
  dest[9] = mat[9];
  dest[10] = mat[10];
  dest[11] = mat[11];
  dest[12] = mat[12];
  dest[13] = mat[13];
  dest[14] = mat[14];
  dest[15] = mat[15];
  return dest;
 };
 mat4.identity = function(dest) {
  if (!dest) {
   dest = mat4.create();
  }
  dest[0] = 1;
  dest[1] = 0;
  dest[2] = 0;
  dest[3] = 0;
  dest[4] = 0;
  dest[5] = 1;
  dest[6] = 0;
  dest[7] = 0;
  dest[8] = 0;
  dest[9] = 0;
  dest[10] = 1;
  dest[11] = 0;
  dest[12] = 0;
  dest[13] = 0;
  dest[14] = 0;
  dest[15] = 1;
  return dest;
 };
 mat4.transpose = function(mat, dest) {
  if (!dest || mat === dest) {
   var a01 = mat[1], a02 = mat[2], a03 = mat[3], a12 = mat[6], a13 = mat[7], a23 = mat[11];
   mat[1] = mat[4];
   mat[2] = mat[8];
   mat[3] = mat[12];
   mat[4] = a01;
   mat[6] = mat[9];
   mat[7] = mat[13];
   mat[8] = a02;
   mat[9] = a12;
   mat[11] = mat[14];
   mat[12] = a03;
   mat[13] = a13;
   mat[14] = a23;
   return mat;
  }
  dest[0] = mat[0];
  dest[1] = mat[4];
  dest[2] = mat[8];
  dest[3] = mat[12];
  dest[4] = mat[1];
  dest[5] = mat[5];
  dest[6] = mat[9];
  dest[7] = mat[13];
  dest[8] = mat[2];
  dest[9] = mat[6];
  dest[10] = mat[10];
  dest[11] = mat[14];
  dest[12] = mat[3];
  dest[13] = mat[7];
  dest[14] = mat[11];
  dest[15] = mat[15];
  return dest;
 };
 mat4.determinant = function(mat) {
  var a00 = mat[0], a01 = mat[1], a02 = mat[2], a03 = mat[3], a10 = mat[4], a11 = mat[5], a12 = mat[6], a13 = mat[7], a20 = mat[8], a21 = mat[9], a22 = mat[10], a23 = mat[11], a30 = mat[12], a31 = mat[13], a32 = mat[14], a33 = mat[15];
  return a30 * a21 * a12 * a03 - a20 * a31 * a12 * a03 - a30 * a11 * a22 * a03 + a10 * a31 * a22 * a03 + a20 * a11 * a32 * a03 - a10 * a21 * a32 * a03 - a30 * a21 * a02 * a13 + a20 * a31 * a02 * a13 + a30 * a01 * a22 * a13 - a00 * a31 * a22 * a13 - a20 * a01 * a32 * a13 + a00 * a21 * a32 * a13 + a30 * a11 * a02 * a23 - a10 * a31 * a02 * a23 - a30 * a01 * a12 * a23 + a00 * a31 * a12 * a23 + a10 * a01 * a32 * a23 - a00 * a11 * a32 * a23 - a20 * a11 * a02 * a33 + a10 * a21 * a02 * a33 + a20 * a01 * a12 * a33 - a00 * a21 * a12 * a33 - a10 * a01 * a22 * a33 + a00 * a11 * a22 * a33;
 };
 mat4.inverse = function(mat, dest) {
  if (!dest) {
   dest = mat;
  }
  var a00 = mat[0], a01 = mat[1], a02 = mat[2], a03 = mat[3], a10 = mat[4], a11 = mat[5], a12 = mat[6], a13 = mat[7], a20 = mat[8], a21 = mat[9], a22 = mat[10], a23 = mat[11], a30 = mat[12], a31 = mat[13], a32 = mat[14], a33 = mat[15], b00 = a00 * a11 - a01 * a10, b01 = a00 * a12 - a02 * a10, b02 = a00 * a13 - a03 * a10, b03 = a01 * a12 - a02 * a11, b04 = a01 * a13 - a03 * a11, b05 = a02 * a13 - a03 * a12, b06 = a20 * a31 - a21 * a30, b07 = a20 * a32 - a22 * a30, b08 = a20 * a33 - a23 * a30, b09 = a21 * a32 - a22 * a31, b10 = a21 * a33 - a23 * a31, b11 = a22 * a33 - a23 * a32, d = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06, invDet;
  if (!d) {
   return null;
  }
  invDet = 1 / d;
  dest[0] = (a11 * b11 - a12 * b10 + a13 * b09) * invDet;
  dest[1] = (-a01 * b11 + a02 * b10 - a03 * b09) * invDet;
  dest[2] = (a31 * b05 - a32 * b04 + a33 * b03) * invDet;
  dest[3] = (-a21 * b05 + a22 * b04 - a23 * b03) * invDet;
  dest[4] = (-a10 * b11 + a12 * b08 - a13 * b07) * invDet;
  dest[5] = (a00 * b11 - a02 * b08 + a03 * b07) * invDet;
  dest[6] = (-a30 * b05 + a32 * b02 - a33 * b01) * invDet;
  dest[7] = (a20 * b05 - a22 * b02 + a23 * b01) * invDet;
  dest[8] = (a10 * b10 - a11 * b08 + a13 * b06) * invDet;
  dest[9] = (-a00 * b10 + a01 * b08 - a03 * b06) * invDet;
  dest[10] = (a30 * b04 - a31 * b02 + a33 * b00) * invDet;
  dest[11] = (-a20 * b04 + a21 * b02 - a23 * b00) * invDet;
  dest[12] = (-a10 * b09 + a11 * b07 - a12 * b06) * invDet;
  dest[13] = (a00 * b09 - a01 * b07 + a02 * b06) * invDet;
  dest[14] = (-a30 * b03 + a31 * b01 - a32 * b00) * invDet;
  dest[15] = (a20 * b03 - a21 * b01 + a22 * b00) * invDet;
  return dest;
 };
 mat4.toRotationMat = function(mat, dest) {
  if (!dest) {
   dest = mat4.create();
  }
  dest[0] = mat[0];
  dest[1] = mat[1];
  dest[2] = mat[2];
  dest[3] = mat[3];
  dest[4] = mat[4];
  dest[5] = mat[5];
  dest[6] = mat[6];
  dest[7] = mat[7];
  dest[8] = mat[8];
  dest[9] = mat[9];
  dest[10] = mat[10];
  dest[11] = mat[11];
  dest[12] = 0;
  dest[13] = 0;
  dest[14] = 0;
  dest[15] = 1;
  return dest;
 };
 mat4.toMat3 = function(mat, dest) {
  if (!dest) {
   dest = mat3.create();
  }
  dest[0] = mat[0];
  dest[1] = mat[1];
  dest[2] = mat[2];
  dest[3] = mat[4];
  dest[4] = mat[5];
  dest[5] = mat[6];
  dest[6] = mat[8];
  dest[7] = mat[9];
  dest[8] = mat[10];
  return dest;
 };
 mat4.toInverseMat3 = function(mat, dest) {
  var a00 = mat[0], a01 = mat[1], a02 = mat[2], a10 = mat[4], a11 = mat[5], a12 = mat[6], a20 = mat[8], a21 = mat[9], a22 = mat[10], b01 = a22 * a11 - a12 * a21, b11 = -a22 * a10 + a12 * a20, b21 = a21 * a10 - a11 * a20, d = a00 * b01 + a01 * b11 + a02 * b21, id;
  if (!d) {
   return null;
  }
  id = 1 / d;
  if (!dest) {
   dest = mat3.create();
  }
  dest[0] = b01 * id;
  dest[1] = (-a22 * a01 + a02 * a21) * id;
  dest[2] = (a12 * a01 - a02 * a11) * id;
  dest[3] = b11 * id;
  dest[4] = (a22 * a00 - a02 * a20) * id;
  dest[5] = (-a12 * a00 + a02 * a10) * id;
  dest[6] = b21 * id;
  dest[7] = (-a21 * a00 + a01 * a20) * id;
  dest[8] = (a11 * a00 - a01 * a10) * id;
  return dest;
 };
 mat4.multiply = function(mat, mat2, dest) {
  if (!dest) {
   dest = mat;
  }
  var a00 = mat[0], a01 = mat[1], a02 = mat[2], a03 = mat[3], a10 = mat[4], a11 = mat[5], a12 = mat[6], a13 = mat[7], a20 = mat[8], a21 = mat[9], a22 = mat[10], a23 = mat[11], a30 = mat[12], a31 = mat[13], a32 = mat[14], a33 = mat[15], b00 = mat2[0], b01 = mat2[1], b02 = mat2[2], b03 = mat2[3], b10 = mat2[4], b11 = mat2[5], b12 = mat2[6], b13 = mat2[7], b20 = mat2[8], b21 = mat2[9], b22 = mat2[10], b23 = mat2[11], b30 = mat2[12], b31 = mat2[13], b32 = mat2[14], b33 = mat2[15];
  dest[0] = b00 * a00 + b01 * a10 + b02 * a20 + b03 * a30;
  dest[1] = b00 * a01 + b01 * a11 + b02 * a21 + b03 * a31;
  dest[2] = b00 * a02 + b01 * a12 + b02 * a22 + b03 * a32;
  dest[3] = b00 * a03 + b01 * a13 + b02 * a23 + b03 * a33;
  dest[4] = b10 * a00 + b11 * a10 + b12 * a20 + b13 * a30;
  dest[5] = b10 * a01 + b11 * a11 + b12 * a21 + b13 * a31;
  dest[6] = b10 * a02 + b11 * a12 + b12 * a22 + b13 * a32;
  dest[7] = b10 * a03 + b11 * a13 + b12 * a23 + b13 * a33;
  dest[8] = b20 * a00 + b21 * a10 + b22 * a20 + b23 * a30;
  dest[9] = b20 * a01 + b21 * a11 + b22 * a21 + b23 * a31;
  dest[10] = b20 * a02 + b21 * a12 + b22 * a22 + b23 * a32;
  dest[11] = b20 * a03 + b21 * a13 + b22 * a23 + b23 * a33;
  dest[12] = b30 * a00 + b31 * a10 + b32 * a20 + b33 * a30;
  dest[13] = b30 * a01 + b31 * a11 + b32 * a21 + b33 * a31;
  dest[14] = b30 * a02 + b31 * a12 + b32 * a22 + b33 * a32;
  dest[15] = b30 * a03 + b31 * a13 + b32 * a23 + b33 * a33;
  return dest;
 };
 mat4.multiplyVec3 = function(mat, vec, dest) {
  if (!dest) {
   dest = vec;
  }
  var x = vec[0], y = vec[1], z = vec[2];
  dest[0] = mat[0] * x + mat[4] * y + mat[8] * z + mat[12];
  dest[1] = mat[1] * x + mat[5] * y + mat[9] * z + mat[13];
  dest[2] = mat[2] * x + mat[6] * y + mat[10] * z + mat[14];
  return dest;
 };
 mat4.multiplyVec4 = function(mat, vec, dest) {
  if (!dest) {
   dest = vec;
  }
  var x = vec[0], y = vec[1], z = vec[2], w = vec[3];
  dest[0] = mat[0] * x + mat[4] * y + mat[8] * z + mat[12] * w;
  dest[1] = mat[1] * x + mat[5] * y + mat[9] * z + mat[13] * w;
  dest[2] = mat[2] * x + mat[6] * y + mat[10] * z + mat[14] * w;
  dest[3] = mat[3] * x + mat[7] * y + mat[11] * z + mat[15] * w;
  return dest;
 };
 mat4.translate = function(mat, vec, dest) {
  var x = vec[0], y = vec[1], z = vec[2], a00, a01, a02, a03, a10, a11, a12, a13, a20, a21, a22, a23;
  if (!dest || mat === dest) {
   mat[12] = mat[0] * x + mat[4] * y + mat[8] * z + mat[12];
   mat[13] = mat[1] * x + mat[5] * y + mat[9] * z + mat[13];
   mat[14] = mat[2] * x + mat[6] * y + mat[10] * z + mat[14];
   mat[15] = mat[3] * x + mat[7] * y + mat[11] * z + mat[15];
   return mat;
  }
  a00 = mat[0];
  a01 = mat[1];
  a02 = mat[2];
  a03 = mat[3];
  a10 = mat[4];
  a11 = mat[5];
  a12 = mat[6];
  a13 = mat[7];
  a20 = mat[8];
  a21 = mat[9];
  a22 = mat[10];
  a23 = mat[11];
  dest[0] = a00;
  dest[1] = a01;
  dest[2] = a02;
  dest[3] = a03;
  dest[4] = a10;
  dest[5] = a11;
  dest[6] = a12;
  dest[7] = a13;
  dest[8] = a20;
  dest[9] = a21;
  dest[10] = a22;
  dest[11] = a23;
  dest[12] = a00 * x + a10 * y + a20 * z + mat[12];
  dest[13] = a01 * x + a11 * y + a21 * z + mat[13];
  dest[14] = a02 * x + a12 * y + a22 * z + mat[14];
  dest[15] = a03 * x + a13 * y + a23 * z + mat[15];
  return dest;
 };
 mat4.scale = function(mat, vec, dest) {
  var x = vec[0], y = vec[1], z = vec[2];
  if (!dest || mat === dest) {
   mat[0] *= x;
   mat[1] *= x;
   mat[2] *= x;
   mat[3] *= x;
   mat[4] *= y;
   mat[5] *= y;
   mat[6] *= y;
   mat[7] *= y;
   mat[8] *= z;
   mat[9] *= z;
   mat[10] *= z;
   mat[11] *= z;
   return mat;
  }
  dest[0] = mat[0] * x;
  dest[1] = mat[1] * x;
  dest[2] = mat[2] * x;
  dest[3] = mat[3] * x;
  dest[4] = mat[4] * y;
  dest[5] = mat[5] * y;
  dest[6] = mat[6] * y;
  dest[7] = mat[7] * y;
  dest[8] = mat[8] * z;
  dest[9] = mat[9] * z;
  dest[10] = mat[10] * z;
  dest[11] = mat[11] * z;
  dest[12] = mat[12];
  dest[13] = mat[13];
  dest[14] = mat[14];
  dest[15] = mat[15];
  return dest;
 };
 mat4.rotate = function(mat, angle, axis, dest) {
  var x = axis[0], y = axis[1], z = axis[2], len = Math.sqrt(x * x + y * y + z * z), s, c, t, a00, a01, a02, a03, a10, a11, a12, a13, a20, a21, a22, a23, b00, b01, b02, b10, b11, b12, b20, b21, b22;
  if (!len) {
   return null;
  }
  if (len !== 1) {
   len = 1 / len;
   x *= len;
   y *= len;
   z *= len;
  }
  s = Math.sin(angle);
  c = Math.cos(angle);
  t = 1 - c;
  a00 = mat[0];
  a01 = mat[1];
  a02 = mat[2];
  a03 = mat[3];
  a10 = mat[4];
  a11 = mat[5];
  a12 = mat[6];
  a13 = mat[7];
  a20 = mat[8];
  a21 = mat[9];
  a22 = mat[10];
  a23 = mat[11];
  b00 = x * x * t + c;
  b01 = y * x * t + z * s;
  b02 = z * x * t - y * s;
  b10 = x * y * t - z * s;
  b11 = y * y * t + c;
  b12 = z * y * t + x * s;
  b20 = x * z * t + y * s;
  b21 = y * z * t - x * s;
  b22 = z * z * t + c;
  if (!dest) {
   dest = mat;
  } else if (mat !== dest) {
   dest[12] = mat[12];
   dest[13] = mat[13];
   dest[14] = mat[14];
   dest[15] = mat[15];
  }
  dest[0] = a00 * b00 + a10 * b01 + a20 * b02;
  dest[1] = a01 * b00 + a11 * b01 + a21 * b02;
  dest[2] = a02 * b00 + a12 * b01 + a22 * b02;
  dest[3] = a03 * b00 + a13 * b01 + a23 * b02;
  dest[4] = a00 * b10 + a10 * b11 + a20 * b12;
  dest[5] = a01 * b10 + a11 * b11 + a21 * b12;
  dest[6] = a02 * b10 + a12 * b11 + a22 * b12;
  dest[7] = a03 * b10 + a13 * b11 + a23 * b12;
  dest[8] = a00 * b20 + a10 * b21 + a20 * b22;
  dest[9] = a01 * b20 + a11 * b21 + a21 * b22;
  dest[10] = a02 * b20 + a12 * b21 + a22 * b22;
  dest[11] = a03 * b20 + a13 * b21 + a23 * b22;
  return dest;
 };
 mat4.rotateX = function(mat, angle, dest) {
  var s = Math.sin(angle), c = Math.cos(angle), a10 = mat[4], a11 = mat[5], a12 = mat[6], a13 = mat[7], a20 = mat[8], a21 = mat[9], a22 = mat[10], a23 = mat[11];
  if (!dest) {
   dest = mat;
  } else if (mat !== dest) {
   dest[0] = mat[0];
   dest[1] = mat[1];
   dest[2] = mat[2];
   dest[3] = mat[3];
   dest[12] = mat[12];
   dest[13] = mat[13];
   dest[14] = mat[14];
   dest[15] = mat[15];
  }
  dest[4] = a10 * c + a20 * s;
  dest[5] = a11 * c + a21 * s;
  dest[6] = a12 * c + a22 * s;
  dest[7] = a13 * c + a23 * s;
  dest[8] = a10 * -s + a20 * c;
  dest[9] = a11 * -s + a21 * c;
  dest[10] = a12 * -s + a22 * c;
  dest[11] = a13 * -s + a23 * c;
  return dest;
 };
 mat4.rotateY = function(mat, angle, dest) {
  var s = Math.sin(angle), c = Math.cos(angle), a00 = mat[0], a01 = mat[1], a02 = mat[2], a03 = mat[3], a20 = mat[8], a21 = mat[9], a22 = mat[10], a23 = mat[11];
  if (!dest) {
   dest = mat;
  } else if (mat !== dest) {
   dest[4] = mat[4];
   dest[5] = mat[5];
   dest[6] = mat[6];
   dest[7] = mat[7];
   dest[12] = mat[12];
   dest[13] = mat[13];
   dest[14] = mat[14];
   dest[15] = mat[15];
  }
  dest[0] = a00 * c + a20 * -s;
  dest[1] = a01 * c + a21 * -s;
  dest[2] = a02 * c + a22 * -s;
  dest[3] = a03 * c + a23 * -s;
  dest[8] = a00 * s + a20 * c;
  dest[9] = a01 * s + a21 * c;
  dest[10] = a02 * s + a22 * c;
  dest[11] = a03 * s + a23 * c;
  return dest;
 };
 mat4.rotateZ = function(mat, angle, dest) {
  var s = Math.sin(angle), c = Math.cos(angle), a00 = mat[0], a01 = mat[1], a02 = mat[2], a03 = mat[3], a10 = mat[4], a11 = mat[5], a12 = mat[6], a13 = mat[7];
  if (!dest) {
   dest = mat;
  } else if (mat !== dest) {
   dest[8] = mat[8];
   dest[9] = mat[9];
   dest[10] = mat[10];
   dest[11] = mat[11];
   dest[12] = mat[12];
   dest[13] = mat[13];
   dest[14] = mat[14];
   dest[15] = mat[15];
  }
  dest[0] = a00 * c + a10 * s;
  dest[1] = a01 * c + a11 * s;
  dest[2] = a02 * c + a12 * s;
  dest[3] = a03 * c + a13 * s;
  dest[4] = a00 * -s + a10 * c;
  dest[5] = a01 * -s + a11 * c;
  dest[6] = a02 * -s + a12 * c;
  dest[7] = a03 * -s + a13 * c;
  return dest;
 };
 mat4.frustum = function(left, right, bottom, top, near, far, dest) {
  if (!dest) {
   dest = mat4.create();
  }
  var rl = right - left, tb = top - bottom, fn = far - near;
  dest[0] = near * 2 / rl;
  dest[1] = 0;
  dest[2] = 0;
  dest[3] = 0;
  dest[4] = 0;
  dest[5] = near * 2 / tb;
  dest[6] = 0;
  dest[7] = 0;
  dest[8] = (right + left) / rl;
  dest[9] = (top + bottom) / tb;
  dest[10] = -(far + near) / fn;
  dest[11] = -1;
  dest[12] = 0;
  dest[13] = 0;
  dest[14] = -(far * near * 2) / fn;
  dest[15] = 0;
  return dest;
 };
 mat4.perspective = function(fovy, aspect, near, far, dest) {
  var top = near * Math.tan(fovy * Math.PI / 360), right = top * aspect;
  return mat4.frustum(-right, right, -top, top, near, far, dest);
 };
 mat4.ortho = function(left, right, bottom, top, near, far, dest) {
  if (!dest) {
   dest = mat4.create();
  }
  var rl = right - left, tb = top - bottom, fn = far - near;
  dest[0] = 2 / rl;
  dest[1] = 0;
  dest[2] = 0;
  dest[3] = 0;
  dest[4] = 0;
  dest[5] = 2 / tb;
  dest[6] = 0;
  dest[7] = 0;
  dest[8] = 0;
  dest[9] = 0;
  dest[10] = -2 / fn;
  dest[11] = 0;
  dest[12] = -(left + right) / rl;
  dest[13] = -(top + bottom) / tb;
  dest[14] = -(far + near) / fn;
  dest[15] = 1;
  return dest;
 };
 mat4.lookAt = function(eye, center, up, dest) {
  if (!dest) {
   dest = mat4.create();
  }
  var x0, x1, x2, y0, y1, y2, z0, z1, z2, len, eyex = eye[0], eyey = eye[1], eyez = eye[2], upx = up[0], upy = up[1], upz = up[2], centerx = center[0], centery = center[1], centerz = center[2];
  if (eyex === centerx && eyey === centery && eyez === centerz) {
   return mat4.identity(dest);
  }
  z0 = eyex - centerx;
  z1 = eyey - centery;
  z2 = eyez - centerz;
  len = 1 / Math.sqrt(z0 * z0 + z1 * z1 + z2 * z2);
  z0 *= len;
  z1 *= len;
  z2 *= len;
  x0 = upy * z2 - upz * z1;
  x1 = upz * z0 - upx * z2;
  x2 = upx * z1 - upy * z0;
  len = Math.sqrt(x0 * x0 + x1 * x1 + x2 * x2);
  if (!len) {
   x0 = 0;
   x1 = 0;
   x2 = 0;
  } else {
   len = 1 / len;
   x0 *= len;
   x1 *= len;
   x2 *= len;
  }
  y0 = z1 * x2 - z2 * x1;
  y1 = z2 * x0 - z0 * x2;
  y2 = z0 * x1 - z1 * x0;
  len = Math.sqrt(y0 * y0 + y1 * y1 + y2 * y2);
  if (!len) {
   y0 = 0;
   y1 = 0;
   y2 = 0;
  } else {
   len = 1 / len;
   y0 *= len;
   y1 *= len;
   y2 *= len;
  }
  dest[0] = x0;
  dest[1] = y0;
  dest[2] = z0;
  dest[3] = 0;
  dest[4] = x1;
  dest[5] = y1;
  dest[6] = z1;
  dest[7] = 0;
  dest[8] = x2;
  dest[9] = y2;
  dest[10] = z2;
  dest[11] = 0;
  dest[12] = -(x0 * eyex + x1 * eyey + x2 * eyez);
  dest[13] = -(y0 * eyex + y1 * eyey + y2 * eyez);
  dest[14] = -(z0 * eyex + z1 * eyey + z2 * eyez);
  dest[15] = 1;
  return dest;
 };
 mat4.fromRotationTranslation = function(quat, vec, dest) {
  if (!dest) {
   dest = mat4.create();
  }
  var x = quat[0], y = quat[1], z = quat[2], w = quat[3], x2 = x + x, y2 = y + y, z2 = z + z, xx = x * x2, xy = x * y2, xz = x * z2, yy = y * y2, yz = y * z2, zz = z * z2, wx = w * x2, wy = w * y2, wz = w * z2;
  dest[0] = 1 - (yy + zz);
  dest[1] = xy + wz;
  dest[2] = xz - wy;
  dest[3] = 0;
  dest[4] = xy - wz;
  dest[5] = 1 - (xx + zz);
  dest[6] = yz + wx;
  dest[7] = 0;
  dest[8] = xz + wy;
  dest[9] = yz - wx;
  dest[10] = 1 - (xx + yy);
  dest[11] = 0;
  dest[12] = vec[0];
  dest[13] = vec[1];
  dest[14] = vec[2];
  dest[15] = 1;
  return dest;
 };
 mat4.str = function(mat) {
  return "[" + mat[0] + ", " + mat[1] + ", " + mat[2] + ", " + mat[3] + ", " + mat[4] + ", " + mat[5] + ", " + mat[6] + ", " + mat[7] + ", " + mat[8] + ", " + mat[9] + ", " + mat[10] + ", " + mat[11] + ", " + mat[12] + ", " + mat[13] + ", " + mat[14] + ", " + mat[15] + "]";
 };
 quat4.create = function(quat) {
  var dest = new MatrixArray(4);
  if (quat) {
   dest[0] = quat[0];
   dest[1] = quat[1];
   dest[2] = quat[2];
   dest[3] = quat[3];
  }
  return dest;
 };
 quat4.set = function(quat, dest) {
  dest[0] = quat[0];
  dest[1] = quat[1];
  dest[2] = quat[2];
  dest[3] = quat[3];
  return dest;
 };
 quat4.calculateW = function(quat, dest) {
  var x = quat[0], y = quat[1], z = quat[2];
  if (!dest || quat === dest) {
   quat[3] = -Math.sqrt(Math.abs(1 - x * x - y * y - z * z));
   return quat;
  }
  dest[0] = x;
  dest[1] = y;
  dest[2] = z;
  dest[3] = -Math.sqrt(Math.abs(1 - x * x - y * y - z * z));
  return dest;
 };
 quat4.dot = function(quat, quat2) {
  return quat[0] * quat2[0] + quat[1] * quat2[1] + quat[2] * quat2[2] + quat[3] * quat2[3];
 };
 quat4.inverse = function(quat, dest) {
  var q0 = quat[0], q1 = quat[1], q2 = quat[2], q3 = quat[3], dot = q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3, invDot = dot ? 1 / dot : 0;
  if (!dest || quat === dest) {
   quat[0] *= -invDot;
   quat[1] *= -invDot;
   quat[2] *= -invDot;
   quat[3] *= invDot;
   return quat;
  }
  dest[0] = -quat[0] * invDot;
  dest[1] = -quat[1] * invDot;
  dest[2] = -quat[2] * invDot;
  dest[3] = quat[3] * invDot;
  return dest;
 };
 quat4.conjugate = function(quat, dest) {
  if (!dest || quat === dest) {
   quat[0] *= -1;
   quat[1] *= -1;
   quat[2] *= -1;
   return quat;
  }
  dest[0] = -quat[0];
  dest[1] = -quat[1];
  dest[2] = -quat[2];
  dest[3] = quat[3];
  return dest;
 };
 quat4.length = function(quat) {
  var x = quat[0], y = quat[1], z = quat[2], w = quat[3];
  return Math.sqrt(x * x + y * y + z * z + w * w);
 };
 quat4.normalize = function(quat, dest) {
  if (!dest) {
   dest = quat;
  }
  var x = quat[0], y = quat[1], z = quat[2], w = quat[3], len = Math.sqrt(x * x + y * y + z * z + w * w);
  if (len === 0) {
   dest[0] = 0;
   dest[1] = 0;
   dest[2] = 0;
   dest[3] = 0;
   return dest;
  }
  len = 1 / len;
  dest[0] = x * len;
  dest[1] = y * len;
  dest[2] = z * len;
  dest[3] = w * len;
  return dest;
 };
 quat4.add = function(quat, quat2, dest) {
  if (!dest || quat === dest) {
   quat[0] += quat2[0];
   quat[1] += quat2[1];
   quat[2] += quat2[2];
   quat[3] += quat2[3];
   return quat;
  }
  dest[0] = quat[0] + quat2[0];
  dest[1] = quat[1] + quat2[1];
  dest[2] = quat[2] + quat2[2];
  dest[3] = quat[3] + quat2[3];
  return dest;
 };
 quat4.multiply = function(quat, quat2, dest) {
  if (!dest) {
   dest = quat;
  }
  var qax = quat[0], qay = quat[1], qaz = quat[2], qaw = quat[3], qbx = quat2[0], qby = quat2[1], qbz = quat2[2], qbw = quat2[3];
  dest[0] = qax * qbw + qaw * qbx + qay * qbz - qaz * qby;
  dest[1] = qay * qbw + qaw * qby + qaz * qbx - qax * qbz;
  dest[2] = qaz * qbw + qaw * qbz + qax * qby - qay * qbx;
  dest[3] = qaw * qbw - qax * qbx - qay * qby - qaz * qbz;
  return dest;
 };
 quat4.multiplyVec3 = function(quat, vec, dest) {
  if (!dest) {
   dest = vec;
  }
  var x = vec[0], y = vec[1], z = vec[2], qx = quat[0], qy = quat[1], qz = quat[2], qw = quat[3], ix = qw * x + qy * z - qz * y, iy = qw * y + qz * x - qx * z, iz = qw * z + qx * y - qy * x, iw = -qx * x - qy * y - qz * z;
  dest[0] = ix * qw + iw * -qx + iy * -qz - iz * -qy;
  dest[1] = iy * qw + iw * -qy + iz * -qx - ix * -qz;
  dest[2] = iz * qw + iw * -qz + ix * -qy - iy * -qx;
  return dest;
 };
 quat4.scale = function(quat, val, dest) {
  if (!dest || quat === dest) {
   quat[0] *= val;
   quat[1] *= val;
   quat[2] *= val;
   quat[3] *= val;
   return quat;
  }
  dest[0] = quat[0] * val;
  dest[1] = quat[1] * val;
  dest[2] = quat[2] * val;
  dest[3] = quat[3] * val;
  return dest;
 };
 quat4.toMat3 = function(quat, dest) {
  if (!dest) {
   dest = mat3.create();
  }
  var x = quat[0], y = quat[1], z = quat[2], w = quat[3], x2 = x + x, y2 = y + y, z2 = z + z, xx = x * x2, xy = x * y2, xz = x * z2, yy = y * y2, yz = y * z2, zz = z * z2, wx = w * x2, wy = w * y2, wz = w * z2;
  dest[0] = 1 - (yy + zz);
  dest[1] = xy + wz;
  dest[2] = xz - wy;
  dest[3] = xy - wz;
  dest[4] = 1 - (xx + zz);
  dest[5] = yz + wx;
  dest[6] = xz + wy;
  dest[7] = yz - wx;
  dest[8] = 1 - (xx + yy);
  return dest;
 };
 quat4.toMat4 = function(quat, dest) {
  if (!dest) {
   dest = mat4.create();
  }
  var x = quat[0], y = quat[1], z = quat[2], w = quat[3], x2 = x + x, y2 = y + y, z2 = z + z, xx = x * x2, xy = x * y2, xz = x * z2, yy = y * y2, yz = y * z2, zz = z * z2, wx = w * x2, wy = w * y2, wz = w * z2;
  dest[0] = 1 - (yy + zz);
  dest[1] = xy + wz;
  dest[2] = xz - wy;
  dest[3] = 0;
  dest[4] = xy - wz;
  dest[5] = 1 - (xx + zz);
  dest[6] = yz + wx;
  dest[7] = 0;
  dest[8] = xz + wy;
  dest[9] = yz - wx;
  dest[10] = 1 - (xx + yy);
  dest[11] = 0;
  dest[12] = 0;
  dest[13] = 0;
  dest[14] = 0;
  dest[15] = 1;
  return dest;
 };
 quat4.slerp = function(quat, quat2, slerp, dest) {
  if (!dest) {
   dest = quat;
  }
  var cosHalfTheta = quat[0] * quat2[0] + quat[1] * quat2[1] + quat[2] * quat2[2] + quat[3] * quat2[3], halfTheta, sinHalfTheta, ratioA, ratioB;
  if (Math.abs(cosHalfTheta) >= 1) {
   if (dest !== quat) {
    dest[0] = quat[0];
    dest[1] = quat[1];
    dest[2] = quat[2];
    dest[3] = quat[3];
   }
   return dest;
  }
  halfTheta = Math.acos(cosHalfTheta);
  sinHalfTheta = Math.sqrt(1 - cosHalfTheta * cosHalfTheta);
  if (Math.abs(sinHalfTheta) < .001) {
   dest[0] = quat[0] * .5 + quat2[0] * .5;
   dest[1] = quat[1] * .5 + quat2[1] * .5;
   dest[2] = quat[2] * .5 + quat2[2] * .5;
   dest[3] = quat[3] * .5 + quat2[3] * .5;
   return dest;
  }
  ratioA = Math.sin((1 - slerp) * halfTheta) / sinHalfTheta;
  ratioB = Math.sin(slerp * halfTheta) / sinHalfTheta;
  dest[0] = quat[0] * ratioA + quat2[0] * ratioB;
  dest[1] = quat[1] * ratioA + quat2[1] * ratioB;
  dest[2] = quat[2] * ratioA + quat2[2] * ratioB;
  dest[3] = quat[3] * ratioA + quat2[3] * ratioB;
  return dest;
 };
 quat4.str = function(quat) {
  return "[" + quat[0] + ", " + quat[1] + ", " + quat[2] + ", " + quat[3] + "]";
 };
 return {
  vec3: vec3,
  mat3: mat3,
  mat4: mat4,
  quat4: quat4
 };
}();

function _glBegin(mode) {
 GLImmediate.enabledClientAttributes_preBegin = GLImmediate.enabledClientAttributes;
 GLImmediate.enabledClientAttributes = [];
 GLImmediate.clientAttributes_preBegin = GLImmediate.clientAttributes;
 GLImmediate.clientAttributes = [];
 for (var i = 0; i < GLImmediate.clientAttributes_preBegin.length; i++) {
  GLImmediate.clientAttributes.push({});
 }
 GLImmediate.mode = mode;
 GLImmediate.vertexCounter = 0;
 var components = GLImmediate.rendererComponents = [];
 for (var i = 0; i < GLImmediate.NUM_ATTRIBUTES; i++) {
  components[i] = 0;
 }
 GLImmediate.rendererComponentPointer = 0;
 GLImmediate.vertexData = GLImmediate.tempData;
}

function _glClear(x0) {
 GLctx["clear"](x0);
}

function _glClearColor(x0, x1, x2, x3) {
 GLctx["clearColor"](x0, x1, x2, x3);
}

function _emscripten_glColor4f(r, g, b, a) {
 r = Math.max(Math.min(r, 1), 0);
 g = Math.max(Math.min(g, 1), 0);
 b = Math.max(Math.min(b, 1), 0);
 a = Math.max(Math.min(a, 1), 0);
 if (GLImmediate.mode >= 0) {
  var start = GLImmediate.vertexCounter << 2;
  GLImmediate.vertexDataU8[start + 0] = r * 255;
  GLImmediate.vertexDataU8[start + 1] = g * 255;
  GLImmediate.vertexDataU8[start + 2] = b * 255;
  GLImmediate.vertexDataU8[start + 3] = a * 255;
  GLImmediate.vertexCounter++;
  GLImmediate.addRendererComponent(GLImmediate.COLOR, 4, GLctx.UNSIGNED_BYTE);
 } else {
  GLImmediate.clientColor[0] = r;
  GLImmediate.clientColor[1] = g;
  GLImmediate.clientColor[2] = b;
  GLImmediate.clientColor[3] = a;
  GLctx.vertexAttrib4fv(GLImmediate.COLOR, GLImmediate.clientColor);
 }
}

function _glColor3f(r, g, b) {
 _emscripten_glColor4f(r, g, b, 1);
}

function _glEnd() {
 GLImmediate.prepareClientAttributes(GLImmediate.rendererComponents[GLImmediate.VERTEX], true);
 GLImmediate.firstVertex = 0;
 GLImmediate.lastVertex = GLImmediate.vertexCounter / (GLImmediate.stride >> 2);
 GLImmediate.flush();
 GLImmediate.disableBeginEndClientAttributes();
 GLImmediate.mode = -1;
 GLImmediate.enabledClientAttributes = GLImmediate.enabledClientAttributes_preBegin;
 GLImmediate.clientAttributes = GLImmediate.clientAttributes_preBegin;
 GLImmediate.currentRenderer = null;
 GLImmediate.modifiedClientAttributes = true;
}

function _glLoadIdentity() {
 GLImmediate.matricesModified = true;
 GLImmediate.matrixVersion[GLImmediate.currentMatrix] = GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1 | 0;
 GLImmediate.matrixLib.mat4.identity(GLImmediate.matrix[GLImmediate.currentMatrix]);
}

function _glMatrixMode(mode) {
 if (mode == 5888) {
  GLImmediate.currentMatrix = 0;
 } else if (mode == 5889) {
  GLImmediate.currentMatrix = 1;
 } else if (mode == 5890) {
  GLImmediate.useTextureMatrix = true;
  GLImmediate.currentMatrix = 2 + GLImmediate.TexEnvJIT.getActiveTexture();
 } else {
  throw "Wrong mode " + mode + " passed to glMatrixMode";
 }
}

function _glRotatef(angle, x, y, z) {
 GLImmediate.matricesModified = true;
 GLImmediate.matrixVersion[GLImmediate.currentMatrix] = GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1 | 0;
 GLImmediate.matrixLib.mat4.rotate(GLImmediate.matrix[GLImmediate.currentMatrix], angle * Math.PI / 180, [ x, y, z ]);
}

function _glTranslatef(x, y, z) {
 GLImmediate.matricesModified = true;
 GLImmediate.matrixVersion[GLImmediate.currentMatrix] = GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1 | 0;
 GLImmediate.matrixLib.mat4.translate(GLImmediate.matrix[GLImmediate.currentMatrix], [ x, y, z ]);
}

function _glVertex2f(x, y) {
 GLImmediate.vertexData[GLImmediate.vertexCounter++] = x;
 GLImmediate.vertexData[GLImmediate.vertexCounter++] = y;
 GLImmediate.vertexData[GLImmediate.vertexCounter++] = 0;
 GLImmediate.vertexData[GLImmediate.vertexCounter++] = 1;
 GLImmediate.addRendererComponent(GLImmediate.VERTEX, 4, GLctx.FLOAT);
}

function _glViewport(x0, x1, x2, x3) {
 GLctx["viewport"](x0, x1, x2, x3);
}

function _emscripten_glOrtho(left, right, bottom, top_, nearVal, farVal) {
 GLImmediate.matricesModified = true;
 GLImmediate.matrixVersion[GLImmediate.currentMatrix] = GLImmediate.matrixVersion[GLImmediate.currentMatrix] + 1 | 0;
 GLImmediate.matrixLib.mat4.multiply(GLImmediate.matrix[GLImmediate.currentMatrix], GLImmediate.matrixLib.mat4.ortho(left, right, bottom, top_, nearVal, farVal));
}

function _gluOrtho2D(left, right, bottom, top) {
 _emscripten_glOrtho(left, right, bottom, top, -1, 1);
}

function _glutPostRedisplay() {
 if (GLUT.displayFunc && !GLUT.requestedAnimationFrame) {
  GLUT.requestedAnimationFrame = true;
  Browser.requestAnimationFrame(function() {
   GLUT.requestedAnimationFrame = false;
   Browser.mainLoop.runIter(function() {
    wasmTable.get(GLUT.displayFunc)();
   });
  });
 }
}

var GLUT = {
 initTime: null,
 idleFunc: null,
 displayFunc: null,
 keyboardFunc: null,
 keyboardUpFunc: null,
 specialFunc: null,
 specialUpFunc: null,
 reshapeFunc: null,
 motionFunc: null,
 passiveMotionFunc: null,
 mouseFunc: null,
 buttons: 0,
 modifiers: 0,
 initWindowWidth: 256,
 initWindowHeight: 256,
 initDisplayMode: 18,
 windowX: 0,
 windowY: 0,
 windowWidth: 0,
 windowHeight: 0,
 requestedAnimationFrame: false,
 saveModifiers: function(event) {
  GLUT.modifiers = 0;
  if (event["shiftKey"]) GLUT.modifiers += 1;
  if (event["ctrlKey"]) GLUT.modifiers += 2;
  if (event["altKey"]) GLUT.modifiers += 4;
 },
 onMousemove: function(event) {
  var lastX = Browser.mouseX;
  var lastY = Browser.mouseY;
  Browser.calculateMouseEvent(event);
  var newX = Browser.mouseX;
  var newY = Browser.mouseY;
  if (newX == lastX && newY == lastY) return;
  if (GLUT.buttons == 0 && event.target == Module["canvas"] && GLUT.passiveMotionFunc) {
   event.preventDefault();
   GLUT.saveModifiers(event);
   wasmTable.get(GLUT.passiveMotionFunc)(lastX, lastY);
  } else if (GLUT.buttons != 0 && GLUT.motionFunc) {
   event.preventDefault();
   GLUT.saveModifiers(event);
   wasmTable.get(GLUT.motionFunc)(lastX, lastY);
  }
 },
 getSpecialKey: function(keycode) {
  var key = null;
  switch (keycode) {
  case 8:
   key = 120;
   break;

  case 46:
   key = 111;
   break;

  case 112:
   key = 1;
   break;

  case 113:
   key = 2;
   break;

  case 114:
   key = 3;
   break;

  case 115:
   key = 4;
   break;

  case 116:
   key = 5;
   break;

  case 117:
   key = 6;
   break;

  case 118:
   key = 7;
   break;

  case 119:
   key = 8;
   break;

  case 120:
   key = 9;
   break;

  case 121:
   key = 10;
   break;

  case 122:
   key = 11;
   break;

  case 123:
   key = 12;
   break;

  case 37:
   key = 100;
   break;

  case 38:
   key = 101;
   break;

  case 39:
   key = 102;
   break;

  case 40:
   key = 103;
   break;

  case 33:
   key = 104;
   break;

  case 34:
   key = 105;
   break;

  case 36:
   key = 106;
   break;

  case 35:
   key = 107;
   break;

  case 45:
   key = 108;
   break;

  case 16:
  case 5:
   key = 112;
   break;

  case 6:
   key = 113;
   break;

  case 17:
  case 3:
   key = 114;
   break;

  case 4:
   key = 115;
   break;

  case 18:
  case 2:
   key = 116;
   break;

  case 1:
   key = 117;
   break;
  }
  return key;
 },
 getASCIIKey: function(event) {
  if (event["ctrlKey"] || event["altKey"] || event["metaKey"]) return null;
  var keycode = event["keyCode"];
  if (48 <= keycode && keycode <= 57) return keycode;
  if (65 <= keycode && keycode <= 90) return event["shiftKey"] ? keycode : keycode + 32;
  if (96 <= keycode && keycode <= 105) return keycode - 48;
  if (106 <= keycode && keycode <= 111) return keycode - 106 + 42;
  switch (keycode) {
  case 9:
  case 13:
  case 27:
  case 32:
  case 61:
   return keycode;
  }
  var s = event["shiftKey"];
  switch (keycode) {
  case 186:
   return s ? 58 : 59;

  case 187:
   return s ? 43 : 61;

  case 188:
   return s ? 60 : 44;

  case 189:
   return s ? 95 : 45;

  case 190:
   return s ? 62 : 46;

  case 191:
   return s ? 63 : 47;

  case 219:
   return s ? 123 : 91;

  case 220:
   return s ? 124 : 47;

  case 221:
   return s ? 125 : 93;

  case 222:
   return s ? 34 : 39;
  }
  return null;
 },
 onKeydown: function(event) {
  if (GLUT.specialFunc || GLUT.keyboardFunc) {
   var key = GLUT.getSpecialKey(event["keyCode"]);
   if (key !== null) {
    if (GLUT.specialFunc) {
     event.preventDefault();
     GLUT.saveModifiers(event);
     wasmTable.get(GLUT.specialFunc)(key, Browser.mouseX, Browser.mouseY);
    }
   } else {
    key = GLUT.getASCIIKey(event);
    if (key !== null && GLUT.keyboardFunc) {
     event.preventDefault();
     GLUT.saveModifiers(event);
     wasmTable.get(GLUT.keyboardFunc)(key, Browser.mouseX, Browser.mouseY);
    }
   }
  }
 },
 onKeyup: function(event) {
  if (GLUT.specialUpFunc || GLUT.keyboardUpFunc) {
   var key = GLUT.getSpecialKey(event["keyCode"]);
   if (key !== null) {
    if (GLUT.specialUpFunc) {
     event.preventDefault();
     GLUT.saveModifiers(event);
     wasmTable.get(GLUT.specialUpFunc)(key, Browser.mouseX, Browser.mouseY);
    }
   } else {
    key = GLUT.getASCIIKey(event);
    if (key !== null && GLUT.keyboardUpFunc) {
     event.preventDefault();
     GLUT.saveModifiers(event);
     wasmTable.get(GLUT.keyboardUpFunc)(key, Browser.mouseX, Browser.mouseY);
    }
   }
  }
 },
 touchHandler: function(event) {
  if (event.target != Module["canvas"]) {
   return;
  }
  var touches = event.changedTouches, main = touches[0], type = "";
  switch (event.type) {
  case "touchstart":
   type = "mousedown";
   break;

  case "touchmove":
   type = "mousemove";
   break;

  case "touchend":
   type = "mouseup";
   break;

  default:
   return;
  }
  var simulatedEvent = document.createEvent("MouseEvent");
  simulatedEvent.initMouseEvent(type, true, true, window, 1, main.screenX, main.screenY, main.clientX, main.clientY, false, false, false, false, 0, null);
  main.target.dispatchEvent(simulatedEvent);
  event.preventDefault();
 },
 onMouseButtonDown: function(event) {
  Browser.calculateMouseEvent(event);
  GLUT.buttons |= 1 << event["button"];
  if (event.target == Module["canvas"] && GLUT.mouseFunc) {
   try {
    event.target.setCapture();
   } catch (e) {}
   event.preventDefault();
   GLUT.saveModifiers(event);
   wasmTable.get(GLUT.mouseFunc)(event["button"], 0, Browser.mouseX, Browser.mouseY);
  }
 },
 onMouseButtonUp: function(event) {
  Browser.calculateMouseEvent(event);
  GLUT.buttons &= ~(1 << event["button"]);
  if (GLUT.mouseFunc) {
   event.preventDefault();
   GLUT.saveModifiers(event);
   wasmTable.get(GLUT.mouseFunc)(event["button"], 1, Browser.mouseX, Browser.mouseY);
  }
 },
 onMouseWheel: function(event) {
  Browser.calculateMouseEvent(event);
  var e = window.event || event;
  var delta = -Browser.getMouseWheelDelta(event);
  delta = delta == 0 ? 0 : delta > 0 ? Math.max(delta, 1) : Math.min(delta, -1);
  var button = 3;
  if (delta < 0) {
   button = 4;
  }
  if (GLUT.mouseFunc) {
   event.preventDefault();
   GLUT.saveModifiers(event);
   wasmTable.get(GLUT.mouseFunc)(button, 0, Browser.mouseX, Browser.mouseY);
  }
 },
 onFullscreenEventChange: function(event) {
  var width;
  var height;
  if (document["fullscreen"] || document["fullScreen"] || document["mozFullScreen"] || document["webkitIsFullScreen"]) {
   width = screen["width"];
   height = screen["height"];
  } else {
   width = GLUT.windowWidth;
   height = GLUT.windowHeight;
   document.removeEventListener("fullscreenchange", GLUT.onFullscreenEventChange, true);
   document.removeEventListener("mozfullscreenchange", GLUT.onFullscreenEventChange, true);
   document.removeEventListener("webkitfullscreenchange", GLUT.onFullscreenEventChange, true);
  }
  Browser.setCanvasSize(width, height, true);
  if (GLUT.reshapeFunc) {
   wasmTable.get(GLUT.reshapeFunc)(width, height);
  }
  _glutPostRedisplay();
 }
};

function _glutCreateWindow(name) {
 var contextAttributes = {
  antialias: (GLUT.initDisplayMode & 128) != 0,
  depth: (GLUT.initDisplayMode & 16) != 0,
  stencil: (GLUT.initDisplayMode & 32) != 0,
  alpha: (GLUT.initDisplayMode & 8) != 0
 };
 Module.ctx = Browser.createContext(Module["canvas"], true, true, contextAttributes);
 return Module.ctx ? 1 : 0;
}

function _glutDisplayFunc(func) {
 GLUT.displayFunc = func;
}

function _glutGet(type) {
 switch (type) {
 case 100:
  return 0;

 case 101:
  return 0;

 case 102:
  return Module["canvas"].width;

 case 103:
  return Module["canvas"].height;

 case 200:
  return Module["canvas"].width;

 case 201:
  return Module["canvas"].height;

 case 500:
  return 0;

 case 501:
  return 0;

 case 502:
  return GLUT.initWindowWidth;

 case 503:
  return GLUT.initWindowHeight;

 case 700:
  var now = Date.now();
  return now - GLUT.initTime;

 case 105:
  return Module.ctx.getContextAttributes().stencil ? 8 : 0;

 case 106:
  return Module.ctx.getContextAttributes().depth ? 8 : 0;

 case 110:
  return Module.ctx.getContextAttributes().alpha ? 8 : 0;

 case 120:
  return Module.ctx.getContextAttributes().antialias ? 1 : 0;

 default:
  throw "glutGet(" + type + ") not implemented yet";
 }
}

function _glutIdleFunc(func) {
 function callback() {
  if (GLUT.idleFunc) {
   wasmTable.get(GLUT.idleFunc)();
   Browser.safeSetTimeout(callback, 4);
  }
 }
 if (!GLUT.idleFunc) {
  Browser.safeSetTimeout(callback, 0);
 }
 GLUT.idleFunc = func;
}

function _glutInit(argcp, argv) {
 GLUT.initTime = Date.now();
 var isTouchDevice = "ontouchstart" in document.documentElement;
 if (isTouchDevice) {
  window.addEventListener("touchmove", GLUT.touchHandler, true);
  window.addEventListener("touchstart", GLUT.touchHandler, true);
  window.addEventListener("touchend", GLUT.touchHandler, true);
 }
 window.addEventListener("keydown", GLUT.onKeydown, true);
 window.addEventListener("keyup", GLUT.onKeyup, true);
 window.addEventListener("mousemove", GLUT.onMousemove, true);
 window.addEventListener("mousedown", GLUT.onMouseButtonDown, true);
 window.addEventListener("mouseup", GLUT.onMouseButtonUp, true);
 window.addEventListener("mousewheel", GLUT.onMouseWheel, true);
 window.addEventListener("DOMMouseScroll", GLUT.onMouseWheel, true);
 Browser.resizeListeners.push(function(width, height) {
  if (GLUT.reshapeFunc) {
   wasmTable.get(GLUT.reshapeFunc)(width, height);
  }
 });
 __ATEXIT__.push(function() {
  if (isTouchDevice) {
   window.removeEventListener("touchmove", GLUT.touchHandler, true);
   window.removeEventListener("touchstart", GLUT.touchHandler, true);
   window.removeEventListener("touchend", GLUT.touchHandler, true);
  }
  window.removeEventListener("keydown", GLUT.onKeydown, true);
  window.removeEventListener("keyup", GLUT.onKeyup, true);
  window.removeEventListener("mousemove", GLUT.onMousemove, true);
  window.removeEventListener("mousedown", GLUT.onMouseButtonDown, true);
  window.removeEventListener("mouseup", GLUT.onMouseButtonUp, true);
  window.removeEventListener("mousewheel", GLUT.onMouseWheel, true);
  window.removeEventListener("DOMMouseScroll", GLUT.onMouseWheel, true);
  Module["canvas"].width = Module["canvas"].height = 1;
 });
}

function _glutInitDisplayMode(mode) {
 GLUT.initDisplayMode = mode;
}

function _glutInitWindowPosition(x, y) {}

function _glutInitWindowSize(width, height) {
 Browser.setCanvasSize(GLUT.initWindowWidth = width, GLUT.initWindowHeight = height);
}

function _glutKeyboardFunc(func) {
 GLUT.keyboardFunc = func;
}

function _glutKeyboardUpFunc(func) {
 GLUT.keyboardUpFunc = func;
}

function _glutReshapeWindow(width, height) {
 Browser.exitFullscreen();
 Browser.setCanvasSize(width, height, true);
 if (GLUT.reshapeFunc) {
  wasmTable.get(GLUT.reshapeFunc)(width, height);
 }
 _glutPostRedisplay();
}

function _glutMainLoop() {
 _glutReshapeWindow(Module["canvas"].width, Module["canvas"].height);
 _glutPostRedisplay();
 throw "unwind";
}

function _glutMotionFunc(func) {
 GLUT.motionFunc = func;
}

function _glutMouseFunc(func) {
 GLUT.mouseFunc = func;
}

function _glutSwapBuffers() {}

GLImmediate.setupFuncs();

Browser.moduleContextCreatedCallbacks.push(function() {
 GLImmediate.init();
});

Module["requestFullscreen"] = function Module_requestFullscreen(lockPointer, resizeCanvas) {
 Browser.requestFullscreen(lockPointer, resizeCanvas);
};

Module["requestAnimationFrame"] = function Module_requestAnimationFrame(func) {
 Browser.requestAnimationFrame(func);
};

Module["setCanvasSize"] = function Module_setCanvasSize(width, height, noUpdates) {
 Browser.setCanvasSize(width, height, noUpdates);
};

Module["pauseMainLoop"] = function Module_pauseMainLoop() {
 Browser.mainLoop.pause();
};

Module["resumeMainLoop"] = function Module_resumeMainLoop() {
 Browser.mainLoop.resume();
};

Module["getUserMedia"] = function Module_getUserMedia() {
 Browser.getUserMedia();
};

Module["createContext"] = function Module_createContext(canvas, useWebGL, setInModule, webGLContextAttributes) {
 return Browser.createContext(canvas, useWebGL, setInModule, webGLContextAttributes);
};

var GLctx;

GLEmulation.init();

var asmLibraryArg = {
 "i": _emscripten_resize_heap,
 "g": _glBegin,
 "B": _glClear,
 "A": _glClearColor,
 "e": _glColor3f,
 "h": _glEnd,
 "c": _glLoadIdentity,
 "b": _glMatrixMode,
 "z": _glRotatef,
 "j": _glTranslatef,
 "a": _glVertex2f,
 "y": _glViewport,
 "d": _gluOrtho2D,
 "x": _glutCreateWindow,
 "w": _glutDisplayFunc,
 "v": _glutGet,
 "u": _glutIdleFunc,
 "t": _glutInit,
 "s": _glutInitDisplayMode,
 "r": _glutInitWindowPosition,
 "q": _glutInitWindowSize,
 "p": _glutKeyboardFunc,
 "o": _glutKeyboardUpFunc,
 "n": _glutMainLoop,
 "m": _glutMotionFunc,
 "l": _glutMouseFunc,
 "f": _glutPostRedisplay,
 "k": _glutSwapBuffers
};

var asm = createWasm();

var ___wasm_call_ctors = Module["___wasm_call_ctors"] = function() {
 return (___wasm_call_ctors = Module["___wasm_call_ctors"] = Module["asm"]["D"]).apply(null, arguments);
};

var _main = Module["_main"] = function() {
 return (_main = Module["_main"] = Module["asm"]["E"]).apply(null, arguments);
};

var stackAlloc = Module["stackAlloc"] = function() {
 return (stackAlloc = Module["stackAlloc"] = Module["asm"]["F"]).apply(null, arguments);
};

var _malloc = Module["_malloc"] = function() {
 return (_malloc = Module["_malloc"] = Module["asm"]["G"]).apply(null, arguments);
};

var calledRun;

function ExitStatus(status) {
 this.name = "ExitStatus";
 this.message = "Program terminated with exit(" + status + ")";
 this.status = status;
}

var calledMain = false;

dependenciesFulfilled = function runCaller() {
 if (!calledRun) run();
 if (!calledRun) dependenciesFulfilled = runCaller;
};

function callMain(args) {
 var entryFunction = Module["_main"];
 args = args || [];
 var argc = args.length + 1;
 var argv = stackAlloc((argc + 1) * 4);
 HEAP32[argv >> 2] = allocateUTF8OnStack(thisProgram);
 for (var i = 1; i < argc; i++) {
  HEAP32[(argv >> 2) + i] = allocateUTF8OnStack(args[i - 1]);
 }
 HEAP32[(argv >> 2) + argc] = 0;
 try {
  var ret = entryFunction(argc, argv);
  exit(ret, true);
 } catch (e) {
  if (e instanceof ExitStatus) {
   return;
  } else if (e == "unwind") {
   return;
  } else {
   var toLog = e;
   if (e && typeof e === "object" && e.stack) {
    toLog = [ e, e.stack ];
   }
   err("exception thrown: " + toLog);
   quit_(1, e);
  }
 } finally {
  calledMain = true;
 }
}

function run(args) {
 args = args || arguments_;
 if (runDependencies > 0) {
  return;
 }
 preRun();
 if (runDependencies > 0) {
  return;
 }
 function doRun() {
  if (calledRun) return;
  calledRun = true;
  Module["calledRun"] = true;
  if (ABORT) return;
  initRuntime();
  preMain();
  if (Module["onRuntimeInitialized"]) Module["onRuntimeInitialized"]();
  if (shouldRunNow) callMain(args);
  postRun();
 }
 if (Module["setStatus"]) {
  Module["setStatus"]("Running...");
  setTimeout(function() {
   setTimeout(function() {
    Module["setStatus"]("");
   }, 1);
   doRun();
  }, 1);
 } else {
  doRun();
 }
}

Module["run"] = run;

function exit(status, implicit) {
 EXITSTATUS = status;
 if (implicit && keepRuntimeAlive() && status === 0) {
  return;
 }
 if (keepRuntimeAlive()) {} else {
  exitRuntime();
  if (Module["onExit"]) Module["onExit"](status);
  ABORT = true;
 }
 quit_(status, new ExitStatus(status));
}

if (Module["preInit"]) {
 if (typeof Module["preInit"] == "function") Module["preInit"] = [ Module["preInit"] ];
 while (Module["preInit"].length > 0) {
  Module["preInit"].pop()();
 }
}

var shouldRunNow = true;

if (Module["noInitialRun"]) shouldRunNow = false;

run();
