# This file is part of TON Key Generator,
# a desktop application for the TON Blockchain project.
#
# For license and copyright information please follow this link:
# https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL

{
  'conditions': [[ 'build_mac', {
    'mac_hardened_runtime': 1,
    'mac_bundle': '1',
    'mac_bundle_resources': [
      '<(res_loc)/art/Images.xcassets',
    ],
    'xcode_settings': {
      'ENABLE_HARDENED_RUNTIME': 'YES',
    },
    'sources': [
      '<(res_loc)/mac/Keygen.entitlements',
    ],
    'xcode_settings': {
      'INFOPLIST_FILE': '<(res_loc)/mac/Keygen.plist',
      'CURRENT_PROJECT_VERSION': '<!(helpers/common/print_version.sh <(DEPTH)/../build/version)',
      'ASSETCATALOG_COMPILER_APPICON_NAME': 'AppIcon',
      'OTHER_LDFLAGS': [
        '-lbsm',
        '-lm',
      ],
    },
    'include_dirs': [
      '/usr/local/include',
    ],
    'library_dirs': [
      '/usr/local/lib',
    ],
    'configurations': {
      'Debug': {
        'xcode_settings': {
          'GCC_OPTIMIZATION_LEVEL': '0',
          'PRODUCT_BUNDLE_IDENTIFIER': 'org.ton.TonKeyGeneratorDebug',
        },
      },
      'Release': {
        'xcode_settings': {
          'DEBUG_INFORMATION_FORMAT': 'dwarf-with-dsym',
          'LLVM_LTO': 'YES',
          'GCC_OPTIMIZATION_LEVEL': 'fast',
          'PRODUCT_BUNDLE_IDENTIFIER': 'org.ton.TonKeyGenerator',
        },
      },
    },
  }]],
}
