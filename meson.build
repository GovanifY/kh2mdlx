project('kh2mdlx', 'cpp')
assimp = dependency('assimp')

src = ['kh2mdlx.cpp']
executable('kh2mdlx', src, dependencies : assimp)

cleaner = find_program('clang-format')
r = run_command(cleaner, '-i', src)
