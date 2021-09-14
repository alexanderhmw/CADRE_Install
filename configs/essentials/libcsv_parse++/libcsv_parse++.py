from MakeIt import ConfigFunc

name = "libcsv_parse++"
version_list = [1, 0, 0]

config = {
    "name": name,
    "version": ConfigFunc.version_str(version_list, "_"),
    "pkg": {
        'url': 'https://storage.googleapis.com/google-code-archive-downloads/v2/code.google.com/csv-parser-cplusplus/libcsv_parser++-{}.tar.gz'.format(ConfigFunc.version_str(version_list, ".")),
        'type': 'tar.gz'
    },
    "params": {
        'src': 'libcsv_parser++-1.0.0',
        'type': 'script',
        'args': ['./configure'],
        'make': ['-j8']
    }
}

ConfigFunc.export_config_json(__file__, config)
