import os
import ycm_core

flags = ['-std=c++11', '-x', 'c++', '-I', 'include', '-I', '/usr/include/']

def DirectoryOfThisScript():
  return os.path.dirname( os.path.abspath( __file__ ) )

def MakeRelativePathsInFlagsAbsolute(flags, working_directory):
  if not working_directory:
    return list(flags)
  new_flags = []
  make_next_absolute = False
  for flag in flags:
    new_flag = flag

    if make_next_absolute:
      make_next_absolute = False
      if not flag.startswith( '/' ):
        new_flag = os.path.join( working_directory, flag )

    path_flag = '-I'
    if flag == path_flag:
      make_next_absolute = True
    elif flag.startswith(path_flag):
      path = flag[len(path_flag): ]
      new_flag = path_flag + os.path.join(working_directory, path)

    if new_flag:
      new_flags.append(new_flag)
  return new_flags

def FlagsForFile(filename, **kwargs):
  relative_to = DirectoryOfThisScript()
  final_flags = MakeRelativePathsInFlagsAbsolute(flags, relative_to)

  return {
    'flags': final_flags,
    'do_cache': True
  }
