test_calculate_bounds_exe = executable('test_calculate_bounds', 'test_calculate_bounds.c', dependencies: voxflat_dep, build_by_default: false)
test('calculate bounds minimal', test_calculate_bounds_exe, args: [files('data/minimal.vox'), '-1', '-1', '-1', '0', '0', '0'])
test('calculate bounds transforms', test_calculate_bounds_exe, args: [files('data/transforms.vox'), '-6', '-8', '-1', '15', '12', '7'])

test_count_voxels_exe = executable('test_count_voxels', 'test_count_voxels.c', dependencies: voxflat_dep, build_by_default: false)
test('count voxels minimal', test_count_voxels_exe, args: [files('data/minimal.vox'), '3'])
test('count voxels transforms', test_count_voxels_exe, args: [files('data/transforms.vox'), '73'])

test_open_memory_exe = executable('test_open_memory', 'test_open_memory.c', dependencies: voxflat_dep, build_by_default: false)
test('open memory', test_open_memory_exe, args: [files('data/minimal.vox')])

test_open_error_exe = executable('test_open_error', 'test_open_error.c', dependencies: voxflat_dep, build_by_default: false)
test('open error file open', test_open_error_exe, args: ['file-does-not-exist', '1'])

test_read_unexpected_eof_exe = executable('test_read_unexpected_eof', 'test_read_unexpected_eof.c', dependencies: voxflat_dep, build_by_default: false)
test('read unexpected eof', test_read_unexpected_eof_exe, args: files('data/minimal.vox'))

diff_prog = find_program('diff', 'fc', required: false)
if get_option('tools').enabled() and diff_prog.found()
    test('vox2txt minimal', diff_prog, args: [
        custom_target('vox2txt_minimal', command: [vox2txt_exe, '@INPUT@', '@OUTPUT@'], input: files('data/minimal.vox'), output: 'result_minimal.txt'),
        files('data/minimal.txt')
    ])
    test('vox2txt transforms', diff_prog, args: [
        custom_target('vox2txt_transforms', command: [vox2txt_exe, '@INPUT@', '@OUTPUT@'], input: files('data/transforms.vox'), output: 'result_transforms.txt'),
        files('data/transforms.txt')
    ])
    test('vox2qef minimal', diff_prog, args: [
        custom_target('vox2qef_minimal', command: [vox2qef_exe, '@INPUT@', '@OUTPUT@'], input: files('data/minimal.vox'), output: 'result_minimal.qef'),
        files('data/minimal.qef')
    ])
    test('vox2qef transforms', diff_prog, args: [
        custom_target('vox2qef_transforms', command: [vox2qef_exe, '@INPUT@', '@OUTPUT@'], input: files('data/transforms.vox'), output: 'result_transforms.qef'),
        files('data/transforms.qef')
    ])
endif
