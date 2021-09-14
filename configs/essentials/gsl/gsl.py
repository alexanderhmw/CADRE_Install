from MakeIt import ConfigFunc

name = "gsl"
version_list = [2, 7]

config = {
    "name": name,
    "version": ConfigFunc.version_str(version_list, "_"),
    "pkg": {
        'url': 'https://ftp.gnu.org/gnu/gsl/gsl-{}.tar.gz'.format(ConfigFunc.version_str(version_list, ".")),
        'type': 'tar.gz'
    },
    "params": {
        'src': 'gsl-{}'.format(ConfigFunc.version_str(version_list, ".")),
        'type': 'script',
        'args': ['autoreconf -i -f -v', './configure --prefix=/usr --enable-maintainer-mode --disable-doxygen'],
        'make': ['CXXFLAGS="-std=c++11"', 'MAKEINFO=true', '-j8'],
        'install': ['CXXFLAGS="-std=c++11"', 'MAKEINFO=true']
    }
}

ConfigFunc.export_config_json(__file__, config)
