doxygen_prog = find_program('doxygen', required: get_option('doc'))
if doxygen_prog.found()
    doxyfile = configure_file(
        input: 'Doxyfile.in',
        output: 'Doxyfile',
        configuration: {'OUTPUT_DIR': meson.current_build_dir(), 'SOURCE_ROOT': meson.project_source_root()},
    )
    custom_target(
        input : doxyfile,
        depend_files: [header_files, files('mainpage.md')],
        command: [doxygen_prog, '@INPUT@'],
        output: 'html',
        install: true,
        install_dir: join_paths(get_option('datadir'), 'doc', meson.project_name())
    )
endif
