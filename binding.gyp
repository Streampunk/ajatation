{
  "targets": [{
    "target_name" : "ajatation",
    "include_dirs" : [
      "<!(node -e \"require('nan')\")"
    ],
    "conditions": [
      ['OS=="mac"', {
        'sources' : [ "src/main.cpp", "src/ntv2democommon.cpp", "src/ntv2player.cpp" ],
        'xcode_settings': {
          'GCC_ENABLE_CPP_RTTI': 'YES',
          'MACOSX_DEPLOYMENT_TARGET': '10.7',
          'OTHER_CPLUSPLUSFLAGS': [
            '-std=c++11',
            '-stdlib=libc++'
          ]
        },
        "link_settings": {
          "libraries": [
          ]
        },
        "include_dirs" : [
        ]
      }],
      ['OS=="win"', {
        "sources" : [ 
			"aja/ntv2sdkwin_13.0.0.18/ajaapps/crossplatform/demoapps/ntv2democommon.cpp", 
			"src/utils.cpp", 
			"src/ajatation.cpp", 
			"src/ntv2player.cpp", 
			"src/ntv2capture.cpp", 
			"src/Playback.cpp",
			"src/Capture.cpp",
            "src/gen2ajaTypeMaps.cpp",
            "src/AjaDevice.cpp",
            "src/ntv2sharedcard.cpp",
            "src/BufferStatus.cpp"
		],
        "configurations": {
          "Release": {
		    "msvs_version" : "2013",
            "msvs_settings": {
              "VCCLCompilerTool": {
                "RuntimeTypeInfo": "true"
              }
            }
          }
        },
        "libraries": [
			"-l../aja/ntv2sdkwin_13.0.0.18/lib/libajaanc_vs12_64.lib",
			"-l../aja/ntv2sdkwin_13.0.0.18/lib/libajantv2.lib",
			"-l../aja/ntv2sdkwin_13.0.0.18/lib/libajabase.lib"
        ],
        "include_dirs" : [
          "src/common",
          "aja/ntv2sdkwin_13.0.0.18/ajalibraries",
          "aja/ntv2sdkwin_13.0.0.18/ajalibraries/ajaanc/includes",
          "aja/ntv2sdkwin_13.0.0.18/ajalibraries/ajantv2/includes",
          "aja/ntv2sdkwin_13.0.0.18/ajalibraries/ajantv2/src/win",
		  "aja/ntv2sdkwin_13.0.0.18/ajaapps/crossplatform/demoapps"
        ],
		"defines": [ "MSWindows", "WIN32", "_WINDOWS", "AJA_WINDOWS", "AJA_NO_AUTOIMPORT" ]
      }]
    ]
  }]
}
