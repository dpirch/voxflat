if get_option('tools').enabled()
    vox2txt_exe = executable(
        'vox2txt',
        'vox2txt.c',
        dependencies: voxflat_dep,
        install: true
    )

    vox2qef_exe = executable(
        'vox2qef',
        'vox2qef.c',
        dependencies: voxflat_dep,
        install: true
    )
endif
