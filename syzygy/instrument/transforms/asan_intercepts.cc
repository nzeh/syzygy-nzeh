// Copyright 2014 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Defines the list of functions to be intercepted by Asan instrumentation.

#include "syzygy/instrument/transforms/asan_intercepts.h"

#include <stddef.h>

namespace instrument {
namespace transforms {

const MD5Hash kHashes_memchr[] = {
    {"3549cc2f365403c679287c34325b8925"},  // VS2010, VS2013, VS2015.
    {""} };
const MD5Hash kHashes_strcspn[] = {
    {"c2e8480d30ceeeb2e9e39b545c82c98c"},  // VS2010 and VS2013.
    {""} };
const MD5Hash kHashes_strlen[] = {
    {"20e07f6e772c47e6cbfc13db5eafa757"},  // VS2010 and VS2013.
    {""} };
const MD5Hash kHashes_strnlen[] = {
    {"09d4062ec47f5e7fd25a19bc60c4bd8e"},  // VS2013.
    {"7f4492174275ca903993fef8d7f8ef77"},  // VS2015.
    {""} };
const MD5Hash kHashes_strpbrk[] = {
    {"9af2e6d499d25ad4628c58a25dbcde1e"},  // VS2010 and VS2013.
    {""} };
const MD5Hash kHashes_strspn[] = {
    {"79b6a33a1b03b482be14afff061d7c68"},  // VS2010 and VS2013.
    {""} };
const MD5Hash kHashes_strncpy[] = {
    {"aed1dd2372364f66f4d126eefb073070"},  // VS2010, VS2013, VS2015.
    {""} };
const MD5Hash kHashes_strncat[] = {
    {"9cc9e9a57cdd695606caf6cbf532d88e"},  // VS2010, VS2013, VS2015.
    {""} };
const MD5Hash kHashes_memcpy[] = {
    {"da1805f40d6e92f6ac497c66ac969e61"},  // VS2010.
    {"270406ea8a9e931f2c0db8a7f0b5d698"},  // VS2013.
    {"efbdeed39029c2d07800b504e28b5df6"},  // VS2015.
    {""} };
const MD5Hash kHashes_memmove[] = {
    {"da1805f40d6e92f6ac497c66ac969e61"},  // VS2010.
    {"270406ea8a9e931f2c0db8a7f0b5d698"},  // VS2013.
    {"efbdeed39029c2d07800b504e28b5df6"},  // VS2015.
    {""} };
const MD5Hash kHashes_memset[] = {
    {"5fcb11b79692c753845cf26dfa42e74c"},  // VS2010.
    {"4900d834c35bb195ab8af6f91d648d6d"},  // VS2013.
    {"2e1f679969390b71b0b28ae4153b53df"},  // VS2015.
    {""} };
const MD5Hash kHashes_strrchr[] = {
    {"f849347be44ddb17a4fc3c64b90f8cca"},  // VS2010.
    {"17575b2dc3a7fd3b277d0cd798f507df"},  // VS2013, VS2015.
    {""} };
const MD5Hash kHashes_strcmp[] = {
    {"865502e059de8a9dc6cee8ef05b1a586"},  // VS2010.
    {"3de87a84bf545bd485f846c1b9456bcb"},  // VS2013.
    {""} };
const MD5Hash kHashes_strstr[] = {
    {"cdfbaae199dcc8272681c021fab9d664"},  // VS2010.
    {"1926bd8c94118f97819d604ec5afee30"},  // VS2013.
    {""} };
const MD5Hash kHashes_wcsnlen[] = {
    {"3764327beb7392f3b841b72b89f94af5"},  // VS2013.
    {"2059f3897a59cf8a0fe6d1803c90af77"},  // VS2015.
    {""} };
const MD5Hash kHashes_wcsrchr[] = {
    {"dc474260def9e341659230dc2edd13e6"},  // VS2013.
    {"bfb15ac56c29c1dd8c68e9ba25d264a8"},  // VS2015.
    {""} };
const MD5Hash kHashes_wcschr[] = {
    {"3fae79785ec4de9951eac512bc62a27e"},  // VS2013.
    {"941bb6826538a1a40f055cb28c7b3695"},  // VS2015.
    {""} };
const MD5Hash kHashes_wcsstr[] = {
    {"2301f403b55567eae76f3dc58dd777f4"},  // VS2013.
    {"01fb77e5eeab6ae224a705aa6ad5117d"},  // VS2015.
    {""} };

// List of module names.
const char kKernel32[] = "kernel32.dll";

// Functions with the same value for |module| must be consecutive in this
// array.
const AsanIntercept kAsanIntercepts[] = {
  // Heap related kernel32 functions.
  { "HeapCreate", "_HeapCreate@12", kKernel32, NULL, false },
  { "HeapDestroy", "_HeapDestroy@4", kKernel32, NULL, false },
  { "HeapAlloc",  "_HeapAlloc@12", kKernel32, NULL, false },
  { "HeapReAlloc", "_HeapReAlloc@16", kKernel32, NULL, false },
  { "HeapFree", "_HeapFree@12", kKernel32, NULL, false },
  { "HeapSize", "_HeapSize@12", kKernel32, NULL, false },
  { "HeapValidate", "_HeapValidate@12", kKernel32, NULL, false },
  { "HeapCompact", "_HeapCompact@8", kKernel32, NULL, false },
  { "HeapLock", "_HeapLock@4", kKernel32, NULL, false },
  { "HeapUnlock", "_HeapUnlock@4", kKernel32, NULL, false },
  { "HeapWalk", "_HeapWalk@8", kKernel32, NULL, false },
  { "HeapSetInformation", "_HeapSetInformation@16", kKernel32, NULL, false },
  { "HeapQueryInformation", "_HeapQueryInformation@20", kKernel32, NULL,
    false },

// Bring in the list of system interceptors that have been automatically
// generated.
#include "syzygy/agent/asan/gen/system_interceptors_instrumentation_filter.gen"

  // C-runtime functions. For Chrome these are always statically linked, but
  // they *could* be imported from one of several different versions of the
  // runtime library.
  // TODO(chrisha): Add support for intercepting these via import redirection?
  //     This might involve handling multiple possible module names per
  //     function.
  { "memchr", "_memchr", NULL, kHashes_memchr, true },
  { "memcpy", "_memcpy", NULL, kHashes_memcpy, true },
  { "memmove", "_memmove", NULL, kHashes_memmove, true },
  { "memset", "_memset", NULL, kHashes_memset, true },
  { "strlen", "_strlen", NULL, kHashes_strlen, true },
  { "strnlen", "_strnlen", NULL, kHashes_strnlen, true },
  { "strncpy", "_strncpy", NULL, kHashes_strncpy, true },
  { "strncat", "_strncat", NULL, kHashes_strncat, true },
  { "strrchr", "_strrchr", NULL, kHashes_strrchr, true },
  { "wcsnlen", "_wcsnlen", NULL, kHashes_wcsnlen, true },
  { "wcsrchr", "_wcsrchr", NULL, kHashes_wcsrchr, true },
  { "wcschr", "_wcschr", NULL, kHashes_wcschr, true },
  { "wcsstr", "_wcsstr", NULL, kHashes_wcsstr, true },

  // Terminating entry.
  { NULL, NULL, NULL, NULL, false },
};

const char kUndecoratedAsanInterceptPrefix[] = "asan_";
const char kDecoratedAsanInterceptPrefix[] = "_asan";
const char kDecoratedImportPrefix[] = "__imp_";

const char kUndecoratedHotPatchingAsanInterceptPrefix[] = "hp_asan_";

}  // namespace transforms
}  // namespace instrument
