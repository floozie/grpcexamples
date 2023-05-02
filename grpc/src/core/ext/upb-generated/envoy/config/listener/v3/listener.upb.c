/* This file was generated by upbc (the upb compiler) from the input
 * file:
 *
 *     envoy/config/listener/v3/listener.proto
 *
 * Do not edit -- your changes will be discarded when the file is
 * regenerated. */

#include <stddef.h>
#include "upb/msg_internal.h"
#include "envoy/config/listener/v3/listener.upb.h"
#include "envoy/config/accesslog/v3/accesslog.upb.h"
#include "envoy/config/core/v3/address.upb.h"
#include "envoy/config/core/v3/base.upb.h"
#include "envoy/config/core/v3/extension.upb.h"
#include "envoy/config/core/v3/socket_option.upb.h"
#include "envoy/config/listener/v3/api_listener.upb.h"
#include "envoy/config/listener/v3/listener_components.upb.h"
#include "envoy/config/listener/v3/udp_listener_config.upb.h"
#include "google/protobuf/duration.upb.h"
#include "google/protobuf/wrappers.upb.h"
#include "xds/annotations/v3/status.upb.h"
#include "xds/core/v3/collection_entry.upb.h"
#include "xds/type/matcher/v3/matcher.upb.h"
#include "envoy/annotations/deprecation.upb.h"
#include "udpa/annotations/security.upb.h"
#include "udpa/annotations/status.upb.h"
#include "udpa/annotations/versioning.upb.h"
#include "validate/validate.upb.h"

#include "upb/port_def.inc"

static const upb_MiniTable_Sub envoy_config_listener_v3_AdditionalAddress_submsgs[2] = {
  {.submsg = &envoy_config_core_v3_Address_msginit},
  {.submsg = &envoy_config_core_v3_SocketOptionsOverride_msginit},
};

static const upb_MiniTable_Field envoy_config_listener_v3_AdditionalAddress__fields[2] = {
  {1, UPB_SIZE(4, 8), UPB_SIZE(1, 1), 0, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {2, UPB_SIZE(8, 16), UPB_SIZE(2, 2), 1, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
};

const upb_MiniTable envoy_config_listener_v3_AdditionalAddress_msginit = {
  &envoy_config_listener_v3_AdditionalAddress_submsgs[0],
  &envoy_config_listener_v3_AdditionalAddress__fields[0],
  UPB_SIZE(16, 24), 2, kUpb_ExtMode_NonExtendable, 2, 255, 0,
};

static const upb_MiniTable_Sub envoy_config_listener_v3_ListenerCollection_submsgs[1] = {
  {.submsg = &xds_core_v3_CollectionEntry_msginit},
};

static const upb_MiniTable_Field envoy_config_listener_v3_ListenerCollection__fields[1] = {
  {1, UPB_SIZE(0, 0), UPB_SIZE(0, 0), 0, 11, kUpb_FieldMode_Array | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
};

const upb_MiniTable envoy_config_listener_v3_ListenerCollection_msginit = {
  &envoy_config_listener_v3_ListenerCollection_submsgs[0],
  &envoy_config_listener_v3_ListenerCollection__fields[0],
  UPB_SIZE(8, 8), 1, kUpb_ExtMode_NonExtendable, 1, 255, 0,
};

static const upb_MiniTable_Sub envoy_config_listener_v3_Listener_submsgs[23] = {
  {.submsg = &envoy_config_core_v3_Address_msginit},
  {.submsg = &envoy_config_listener_v3_FilterChain_msginit},
  {.submsg = &google_protobuf_BoolValue_msginit},
  {.submsg = &google_protobuf_UInt32Value_msginit},
  {.submsg = &envoy_config_core_v3_Metadata_msginit},
  {.submsg = &envoy_config_listener_v3_Listener_DeprecatedV1_msginit},
  {.submsg = &envoy_config_listener_v3_ListenerFilter_msginit},
  {.submsg = &google_protobuf_BoolValue_msginit},
  {.submsg = &google_protobuf_BoolValue_msginit},
  {.submsg = &google_protobuf_UInt32Value_msginit},
  {.submsg = &envoy_config_core_v3_SocketOption_msginit},
  {.submsg = &google_protobuf_Duration_msginit},
  {.submsg = &envoy_config_listener_v3_UdpListenerConfig_msginit},
  {.submsg = &envoy_config_listener_v3_ApiListener_msginit},
  {.submsg = &envoy_config_listener_v3_Listener_ConnectionBalanceConfig_msginit},
  {.submsg = &envoy_config_accesslog_v3_AccessLog_msginit},
  {.submsg = &google_protobuf_UInt32Value_msginit},
  {.submsg = &envoy_config_listener_v3_FilterChain_msginit},
  {.submsg = &google_protobuf_BoolValue_msginit},
  {.submsg = &envoy_config_listener_v3_Listener_InternalListenerConfig_msginit},
  {.submsg = &google_protobuf_BoolValue_msginit},
  {.submsg = &xds_type_matcher_v3_Matcher_msginit},
  {.submsg = &envoy_config_listener_v3_AdditionalAddress_msginit},
};

static const upb_MiniTable_Field envoy_config_listener_v3_Listener__fields[31] = {
  {1, UPB_SIZE(24, 24), UPB_SIZE(0, 0), kUpb_NoSub, 9, kUpb_FieldMode_Scalar | (kUpb_FieldRep_StringView << kUpb_FieldRep_Shift)},
  {2, UPB_SIZE(32, 40), UPB_SIZE(1, 1), 0, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {3, UPB_SIZE(36, 48), UPB_SIZE(0, 0), 1, 11, kUpb_FieldMode_Array | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {4, UPB_SIZE(40, 56), UPB_SIZE(2, 2), 2, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {5, UPB_SIZE(44, 64), UPB_SIZE(3, 3), 3, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {6, UPB_SIZE(48, 72), UPB_SIZE(4, 4), 4, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {7, UPB_SIZE(52, 80), UPB_SIZE(5, 5), 5, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {8, UPB_SIZE(4, 4), UPB_SIZE(0, 0), kUpb_NoSub, 5, kUpb_FieldMode_Scalar | (kUpb_FieldRep_4Byte << kUpb_FieldRep_Shift)},
  {9, UPB_SIZE(56, 88), UPB_SIZE(0, 0), 6, 11, kUpb_FieldMode_Array | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {10, UPB_SIZE(60, 96), UPB_SIZE(6, 6), 7, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {11, UPB_SIZE(64, 104), UPB_SIZE(7, 7), 8, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {12, UPB_SIZE(68, 112), UPB_SIZE(8, 8), 9, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {13, UPB_SIZE(72, 120), UPB_SIZE(0, 0), 10, 11, kUpb_FieldMode_Array | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {15, UPB_SIZE(76, 128), UPB_SIZE(9, 9), 11, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {16, UPB_SIZE(8, 8), UPB_SIZE(0, 0), kUpb_NoSub, 5, kUpb_FieldMode_Scalar | (kUpb_FieldRep_4Byte << kUpb_FieldRep_Shift)},
  {17, UPB_SIZE(12, 12), UPB_SIZE(0, 0), kUpb_NoSub, 8, kUpb_FieldMode_Scalar | (kUpb_FieldRep_1Byte << kUpb_FieldRep_Shift)},
  {18, UPB_SIZE(80, 136), UPB_SIZE(10, 10), 12, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {19, UPB_SIZE(84, 144), UPB_SIZE(11, 11), 13, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {20, UPB_SIZE(88, 152), UPB_SIZE(12, 12), 14, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {21, UPB_SIZE(13, 13), UPB_SIZE(0, 0), kUpb_NoSub, 8, kUpb_FieldMode_Scalar | (kUpb_FieldRep_1Byte << kUpb_FieldRep_Shift)},
  {22, UPB_SIZE(92, 160), UPB_SIZE(0, 0), 15, 11, kUpb_FieldMode_Array | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {24, UPB_SIZE(96, 168), UPB_SIZE(13, 13), 16, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {25, UPB_SIZE(100, 176), UPB_SIZE(14, 14), 17, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {26, UPB_SIZE(104, 184), UPB_SIZE(15, 15), 18, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {27, UPB_SIZE(128, 232), UPB_SIZE(-17, -17), 19, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {28, UPB_SIZE(108, 192), UPB_SIZE(0, 0), kUpb_NoSub, 9, kUpb_FieldMode_Scalar | (kUpb_FieldRep_StringView << kUpb_FieldRep_Shift)},
  {29, UPB_SIZE(116, 208), UPB_SIZE(16, 16), 20, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {30, UPB_SIZE(20, 20), UPB_SIZE(0, 0), kUpb_NoSub, 8, kUpb_FieldMode_Scalar | (kUpb_FieldRep_1Byte << kUpb_FieldRep_Shift)},
  {31, UPB_SIZE(21, 21), UPB_SIZE(0, 0), kUpb_NoSub, 8, kUpb_FieldMode_Scalar | (kUpb_FieldRep_1Byte << kUpb_FieldRep_Shift)},
  {32, UPB_SIZE(120, 216), UPB_SIZE(17, 17), 21, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {33, UPB_SIZE(124, 224), UPB_SIZE(0, 0), 22, 11, kUpb_FieldMode_Array | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
};

const upb_MiniTable envoy_config_listener_v3_Listener_msginit = {
  &envoy_config_listener_v3_Listener_submsgs[0],
  &envoy_config_listener_v3_Listener__fields[0],
  UPB_SIZE(136, 240), 31, kUpb_ExtMode_NonExtendable, 13, 255, 0,
};

static const upb_MiniTable_Sub envoy_config_listener_v3_Listener_DeprecatedV1_submsgs[1] = {
  {.submsg = &google_protobuf_BoolValue_msginit},
};

static const upb_MiniTable_Field envoy_config_listener_v3_Listener_DeprecatedV1__fields[1] = {
  {1, UPB_SIZE(4, 8), UPB_SIZE(1, 1), 0, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
};

const upb_MiniTable envoy_config_listener_v3_Listener_DeprecatedV1_msginit = {
  &envoy_config_listener_v3_Listener_DeprecatedV1_submsgs[0],
  &envoy_config_listener_v3_Listener_DeprecatedV1__fields[0],
  UPB_SIZE(8, 16), 1, kUpb_ExtMode_NonExtendable, 1, 255, 0,
};

static const upb_MiniTable_Sub envoy_config_listener_v3_Listener_ConnectionBalanceConfig_submsgs[2] = {
  {.submsg = &envoy_config_listener_v3_Listener_ConnectionBalanceConfig_ExactBalance_msginit},
  {.submsg = &envoy_config_core_v3_TypedExtensionConfig_msginit},
};

static const upb_MiniTable_Field envoy_config_listener_v3_Listener_ConnectionBalanceConfig__fields[2] = {
  {1, UPB_SIZE(4, 8), UPB_SIZE(-1, -1), 0, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
  {2, UPB_SIZE(4, 8), UPB_SIZE(-1, -1), 1, 11, kUpb_FieldMode_Scalar | (kUpb_FieldRep_Pointer << kUpb_FieldRep_Shift)},
};

const upb_MiniTable envoy_config_listener_v3_Listener_ConnectionBalanceConfig_msginit = {
  &envoy_config_listener_v3_Listener_ConnectionBalanceConfig_submsgs[0],
  &envoy_config_listener_v3_Listener_ConnectionBalanceConfig__fields[0],
  UPB_SIZE(8, 16), 2, kUpb_ExtMode_NonExtendable, 2, 255, 0,
};

const upb_MiniTable envoy_config_listener_v3_Listener_ConnectionBalanceConfig_ExactBalance_msginit = {
  NULL,
  NULL,
  UPB_SIZE(0, 0), 0, kUpb_ExtMode_NonExtendable, 0, 255, 0,
};

const upb_MiniTable envoy_config_listener_v3_Listener_InternalListenerConfig_msginit = {
  NULL,
  NULL,
  UPB_SIZE(0, 0), 0, kUpb_ExtMode_NonExtendable, 0, 255, 0,
};

static const upb_MiniTable *messages_layout[7] = {
  &envoy_config_listener_v3_AdditionalAddress_msginit,
  &envoy_config_listener_v3_ListenerCollection_msginit,
  &envoy_config_listener_v3_Listener_msginit,
  &envoy_config_listener_v3_Listener_DeprecatedV1_msginit,
  &envoy_config_listener_v3_Listener_ConnectionBalanceConfig_msginit,
  &envoy_config_listener_v3_Listener_ConnectionBalanceConfig_ExactBalance_msginit,
  &envoy_config_listener_v3_Listener_InternalListenerConfig_msginit,
};

const upb_MiniTable_File envoy_config_listener_v3_listener_proto_upb_file_layout = {
  messages_layout,
  NULL,
  NULL,
  7,
  0,
  0,
};

#include "upb/port_undef.inc"

