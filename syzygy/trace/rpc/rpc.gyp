# Copyright 2012 Google Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

{
  'variables': {
    'chromium_code': 1,
    'midl_out_dir': '<(SHARED_INTERMEDIATE_DIR)/syzygy/trace/rpc',
  },
  'target_defaults': {
    'all_dependent_settings': {
      'include_dirs': ['<(SHARED_INTERMEDIATE_DIR)'],
    },
  },
  'targets': [
    {
      'target_name': 'call_trace_rpc_lib',
      'type': 'static_library',
      # Build our IDL file to the shared intermediate directory using the
      # midl_rpc.gypi include (because the default rules for .idl files are
      # specific to COM interfaces). This include expects the prefix and
      # midl_out_dir variables to be defined.
      'variables': {
        'prefix': 'CallTrace',
      },
      'includes': ['../../build/midl_rpc.gypi'],
      'sources': ['call_trace_rpc.idl'],
      'dependencies': [
        '<(src)/syzygy/common/rpc/rpc.gyp:common_rpc_lib',
        '<(src)/syzygy/trace/protocol/protocol.gyp:protocol_lib',
      ],
      'outputs': [
        '<(SHARED_INTERMEDIATE_DIR)/syzygy/trace/rpc/call_trace_rpc.h',
        '<(SHARED_INTERMEDIATE_DIR)/syzygy/trace/rpc/call_trace_rpc_c.cc',
        '<(SHARED_INTERMEDIATE_DIR)/syzygy/trace/rpc/call_trace_rpc_s.cc',
      ],
      'process_outputs_as_sources': 1,
      # This target exports a hard dependency because it generates header files.
      'hard_dependency': 1,
    },
    {
      'target_name': 'logger_rpc_lib',
      'type': 'static_library',
      # Build our IDL file to the shared intermediate directory using the
      # midl_rpc.gypi include (because the default rules for .idl files are
      # specific to COM interfaces). This include expects the prefix and
      # midl_out_dir variables to be defined.
      'variables': {
        'prefix': 'Logger',
      },
      'includes': ['../../build/midl_rpc.gypi'],
      'sources': ['logger_rpc.idl'],
      'dependencies': [
        '<(src)/syzygy/common/rpc/rpc.gyp:common_rpc_lib',
        '<(src)/syzygy/trace/protocol/protocol.gyp:protocol_lib',
      ],
      # This target exports a hard dependency because it generates header files.
      'hard_dependency': 1,
    },
  ],
}
