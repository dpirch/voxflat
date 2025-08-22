project('voxflat', 'c',
    version: '0.1.0',
    meson_version: '>=1.1',
    license: 'MIT',
    default_options: [
        'c_std=c11',
        'warning_level=3',
        'buildtype=release',
    ]
)

inc_dir = include_directories('include')

if meson.get_compiler('c').get_id() == 'msvc'
    add_project_arguments(['/D_CRT_SECURE_NO_WARNINGS', '/wd4244', '/wd4267'], language : 'c')
endif

subdir('include')
subdir('src')

voxflat_dep = declare_dependency(link_with: voxflat_lib, include_directories: inc_dir)

subdir('tools')
subdir('tests')
subdir('doc')

pkg_mod = import('pkgconfig')
pkg_mod.generate(voxflat_lib, description: 'MagicaVoxel vox file import library')
