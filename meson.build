project('dae2mdlx', 'cpp')
assimp = dependency('assimp')

src = ['dae2mdlx.cpp']
executable('dae2mdlx', src, dependencies : assimp)

cleaner = find_program('clang-format')
r = run_command(cleaner, '-i', src)
